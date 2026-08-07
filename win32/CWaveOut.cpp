/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/


#include "CWaveOut.h"
#include <mmdeviceapi.h> // needs to be before snes9x.h, otherwise conflicts with SetFlags macro
#include <Functiondiscoverykeys_devpkey.h>
#include "../snes9x.h"
#include "../apu/apu.h"
#include "wsnes9x.h"
#include <map>
#include <math.h>

#define DRV_QUERYFUNCTIONINSTANCEID       (DRV_RESERVED + 17)
#define DRV_QUERYFUNCTIONINSTANCEIDSIZE   (DRV_RESERVED + 18)

// Length of the click-suppression ramps applied at audio output transitions.
// 1024 stereo frames ~= 23ms at 44.1kHz: pushes the fade's spectral fundamental
// below ~22Hz so it can't be heard as a bass kick. Cosine-shaped (Hann half-
// window) so the endpoints have zero derivative and don't radiate broadband
// splatter the way a linear ramp does.
static const UINT32 kFadeFrames = 1024;
static const UINT32 kFadeBytes  = kFadeFrames * 2 * sizeof(int16);
static const double kPi         = 3.14159265358979323846;
static const DWORD_PTR kFadeoutMarker = 0x57415645; // 'WAVE' in dwUser to flag the tail header

static inline int CosineGainQ15(UINT32 pos, UINT32 len, bool rising)
{
    if (len < 2)        return rising ? 32768 : 0;
    if (pos >= len - 1) return rising ? 32768 : 0;
    const double t = (double)pos / (double)(len - 1);
    const double c = cos(kPi * t);
    const double g = rising ? 0.5 * (1.0 - c) : 0.5 * (1.0 + c);
    return (int)(g * 32768.0 + 0.5);
}

CWaveOut::CWaveOut(void)
{
    hWaveOut = NULL;
    initDone = false;
    bufferCount = 0;
    submittedBytes = 0;
    playedBytesAccum = 0;
    lastPosBytes = 0;
    last_sample_l = 0;
    last_sample_r = 0;
    fade_in_pending = 0;
    fade_in_pos = kFadeFrames;  // "done" — no fade in progress
    fadeout_in_flight = 0;
    preampGainQ12 = 4096;   // unity
    fadeoutBuffer = NULL;
    fadeoutPrepared = false;
    ZeroMemory(&fadeoutHeader, sizeof(fadeoutHeader));
}

CWaveOut::~CWaveOut(void)
{
    DeInitSoundOutput();
}

void CALLBACK CWaveOut::WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	CWaveOut *wo = (CWaveOut*)dwUser;
    if (uMsg == WOM_DONE)
    {
        WAVEHDR *hdr = (WAVEHDR*)dw1;
        if (hdr && hdr->dwUser == kFadeoutMarker)
            InterlockedExchange(&wo->fadeout_in_flight, 0);
        LONG remaining = InterlockedDecrement(&wo->bufferCount);
        if (remaining == 0)
            InterlockedExchange(&wo->fade_in_pending, 1);
        SetEvent(GUI.SoundSyncEvent);
    }
	else if (uMsg == WOM_CLOSE) // also sent on device removals
	{
		// this stops any output from being sent to the non existing device
		wo->initDone = false;
	}
}

bool CWaveOut::SetupSound()
{
	DeInitSoundOutput();

    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = Settings.SoundPlaybackRate;
    wfx.nBlockAlign = 2 * 2;
    wfx.wBitsPerSample = 16;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

	// subtract -1, we added "Default" as first index - Default will yield -1, which is WAVE_MAPPER
	int device_index = FindDeviceIndex(GUI.AudioDevice) - 1;

    waveOutOpen(&hWaveOut, device_index, &wfx, (DWORD_PTR)WaveCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);

    // Fresh device: position restarts at 0 and nothing is in flight, so
    // reset the accounting instead of trusting waveOutReset's callbacks.
    bufferCount = 0;
    submittedBytes = 0;
    playedBytesAccum = 0;
    lastPosBytes = 0;

    UINT32 blockTime = GUI.SoundBufferSize / blockCount;
    singleBufferSamples = (Settings.SoundPlaybackRate * blockTime) / 1000;
    if (singleBufferSamples < 256)
        singleBufferSamples = 256;
    singleBufferSamples *= 2;
    singleBufferBytes = singleBufferSamples * 2;
    sumBufferSize = singleBufferBytes * blockCount;
    writeOffset = 0;
    partialOffset = 0;

    waveHeaders.resize(blockCount);
    for (auto &w : waveHeaders)
    {
        w.lpData = (LPSTR)LocalAlloc(LMEM_FIXED, singleBufferBytes);
        w.dwBufferLength = singleBufferBytes;
        w.dwBytesRecorded = 0;
        w.dwUser = 0;
        w.dwFlags = 0;
        w.dwLoops = 0;
        w.lpNext = 0;
        w.reserved = 0;
        waveOutPrepareHeader(hWaveOut, &w, sizeof(WAVEHDR));
    }

    fadeoutBuffer = (int16*)LocalAlloc(LMEM_FIXED, kFadeBytes);
    ZeroMemory(&fadeoutHeader, sizeof(fadeoutHeader));
    fadeoutHeader.lpData = (LPSTR)fadeoutBuffer;
    fadeoutHeader.dwBufferLength = kFadeBytes;
    fadeoutHeader.dwUser = kFadeoutMarker;
    waveOutPrepareHeader(hWaveOut, &fadeoutHeader, sizeof(WAVEHDR));
    fadeoutPrepared = true;

    // Fade in the first real-audio block so cold start doesn't kick from
    // silence to a non-zero sample. RecoverFromUnderrun's silence padding
    // bypasses SubmitBlock so this flag survives until the first real push.
    InterlockedExchange(&fade_in_pending, 1);

    initDone = true;

    return true;
}

void CWaveOut::SetVolume(double volume)
{
    // Split the request: waveOutSetVolume can only attenuate, so the part
    // above unity becomes a software multiply in SubmitBlock.
    const double hw = (volume > 1.0) ? 1.0 : volume;
    InterlockedExchange(&preampGainQ12, (LONG)((volume > 1.0 ? volume : 1.0) * 4096.0 + 0.5));

    uint32 volumeout = (uint32) (hw * 0xffff);
    waveOutSetVolume(hWaveOut, volumeout + (volumeout << 16));
}

void CWaveOut::BeginPlayback()
{
    waveOutRestart(hWaveOut);
}

bool CWaveOut::InitSoundOutput()
{
    return true;
}

void CWaveOut::DeInitSoundOutput()
{
    if (!hWaveOut)
        return;

    StopPlayback();

    waveOutReset(hWaveOut);

    if (!waveHeaders.empty())
    {
        for (auto &w : waveHeaders)
        {
            waveOutUnprepareHeader(hWaveOut, &w, sizeof(WAVEHDR));
            LocalFree(w.lpData);
        }
    }
    waveHeaders.clear();

    if (fadeoutPrepared)
    {
        waveOutUnprepareHeader(hWaveOut, &fadeoutHeader, sizeof(WAVEHDR));
        fadeoutPrepared = false;
    }
    if (fadeoutBuffer)
    {
        LocalFree(fadeoutBuffer);
        fadeoutBuffer = NULL;
    }
    last_sample_l = 0;
    last_sample_r = 0;
    InterlockedExchange(&fade_in_pending, 0);
    fade_in_pos = kFadeFrames;
    InterlockedExchange(&fadeout_in_flight, 0);

    waveOutClose(hWaveOut);
	hWaveOut = NULL;

    initDone = false;
}

void CWaveOut::StopPlayback()
{
    waveOutPause(hWaveOut);
}

// Bytes actually played since the device was opened, from the driver's play
// cursor — WOM_DONE arrival can lag this by a full winmm pump period.
UINT64 CWaveOut::PlayedBytes()
{
    MMTIME mmt;
    mmt.wType = TIME_BYTES;
    if (!hWaveOut ||
        waveOutGetPosition(hWaveOut, &mmt, sizeof(mmt)) != MMSYSERR_NOERROR)
        return playedBytesAccum;

    DWORD pos;
    if (mmt.wType == TIME_BYTES)
        pos = mmt.u.cb;
    else if (mmt.wType == TIME_SAMPLES)
        pos = mmt.u.sample * (2 * sizeof(int16));
    else
        return playedBytesAccum;

    // Unsigned delta handles the DWORD wrap (~6h at 48kHz stereo).
    playedBytesAccum += (DWORD)(pos - lastPosBytes);
    lastPosBytes = pos;
    return playedBytesAccum;
}

int CWaveOut::GetAvailableBytes()
{
    UINT64 played = PlayedBytes();
    UINT64 inFlight = (submittedBytes > played) ? submittedBytes - played : 0;
    if (inFlight > sumBufferSize)
        inFlight = sumBufferSize;
    return (int)(sumBufferSize - (UINT32)inFlight) - (int)partialOffset;
}

// Submit a fully-filled normal block. Applies a fade-in ramp to the head if
// fade_in_pending was set (queue underran or just resumed from a pause-time
// fade-out tail) and captures the trailing stereo frame for any future
// fade-out tail. RecoverFromUnderrun's silence pushes deliberately bypass
// this to keep fade_in_pending armed for the eventual real-audio block.
void CWaveOut::SubmitBlock(WAVEHDR &header)
{
    int16 *samples = (int16*)header.lpData;
    const UINT32 frames = header.dwBufferLength / (2 * sizeof(int16));

    // Pre-amp before the fade ramps so a fade still lands on 0 at its ends.
    const LONG preamp_q12 = InterlockedCompareExchange(&preampGainQ12, 0, 0);
    if (preamp_q12 > 4096 && frames > 0)
    {
        for (UINT32 i = 0; i < frames * 2; ++i)
        {
            int32 v = ((int32)samples[i] * preamp_q12) >> 12;
            if (v >  32767) v =  32767;
            if (v < -32768) v = -32768;
            samples[i] = (int16)v;
        }
    }

    if (frames > 0)
    {
        // Defer fade-in firing until the buffer actually has audio content,
        // so silent boot-time buffers (BIOS code setup before the chime) don't
        // consume the flag and leave the chime onset unfaded. Once content is
        // found we start the ramp at the onset frame within the buffer.
        UINT32 fade_start_frame = 0;
        if (fade_in_pending)
        {
            UINT32 onset = frames;
            for (UINT32 i = 0; i < frames; ++i)
            {
                const int16 l = samples[i*2];
                const int16 r = samples[i*2 + 1];
                if (l > 64 || l < -64 || r > 64 || r < -64)
                {
                    onset = i;
                    break;
                }
            }
            if (onset < frames)
            {
                InterlockedExchange(&fade_in_pending, 0);
                fade_in_pos = 0;
                fade_start_frame = onset;
                // Zero sub-threshold pre-onset samples so the seam to the
                // gain-0 onset is exactly 0->0 instead of ±63->0 (audible
                // single-sample "tak").
                for (UINT32 i = 0; i < onset; ++i)
                {
                    samples[i*2]     = 0;
                    samples[i*2 + 1] = 0;
                }
            }
        }

        if (fade_in_pos < kFadeFrames && fade_start_frame < frames)
        {
            const UINT32 frames_avail   = frames - fade_start_frame;
            const UINT32 frames_left    = kFadeFrames - fade_in_pos;
            const UINT32 frames_to_fade = (frames_left < frames_avail) ? frames_left : frames_avail;
            for (UINT32 i = 0; i < frames_to_fade; ++i)
            {
                const int32  gain_q15 = CosineGainQ15(fade_in_pos + i, kFadeFrames, true);
                const UINT32 idx      = fade_start_frame + i;
                samples[idx*2]     = (int16)(((int32)samples[idx*2]     * gain_q15) >> 15);
                samples[idx*2 + 1] = (int16)(((int32)samples[idx*2 + 1] * gain_q15) >> 15);
            }
            fade_in_pos += frames_to_fade;
        }
    }

    if (frames > 0)
    {
        const UINT32 last = (frames - 1) * 2;
        last_sample_l = samples[last];
        last_sample_r = samples[last + 1];
    }

    // A failed write never gets a WOM_DONE; counting it would leave a
    // phantom entry that blinds underrun recovery forever.
    if (waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        return;
    submittedBytes += header.dwBufferLength;
    InterlockedIncrement(&bufferCount);
}

void CWaveOut::OnPauseRequested()
{
    if (!initDone || !hWaveOut || !fadeoutBuffer || !fadeoutPrepared)
        return;
    // Skip if a previous fade-out tail is still in the queue — re-submitting
    // the same WAVEHDR would race with the audio thread reading from it.
    if (InterlockedCompareExchange(&fadeout_in_flight, 1, 0) != 0)
        return;

    for (UINT32 i = 0; i < kFadeFrames; ++i)
    {
        const int32 gain_q15 = CosineGainQ15(i, kFadeFrames, false);
        fadeoutBuffer[i*2]     = (int16)(((int32)last_sample_l * gain_q15) >> 15);
        fadeoutBuffer[i*2 + 1] = (int16)(((int32)last_sample_r * gain_q15) >> 15);
    }

    if (waveOutWrite(hWaveOut, &fadeoutHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        InterlockedExchange(&fadeout_in_flight, 0);
        return;
    }
    submittedBytes += kFadeBytes;
    InterlockedIncrement(&bufferCount);

    last_sample_l = 0;
    last_sample_r = 0;
    InterlockedExchange(&fade_in_pending, 1);
}

void CWaveOut::OnResumeRequested()
{
    // fade_in_pending already armed in OnPauseRequested or in WaveCallback on
    // underrun; nothing to do here.
}

// Fill the set of blocks preceding writeOffset with silence and write them
// to the output to get the buffer back to 50%
void CWaveOut::RecoverFromUnderrun()
{
    // The partial block predates the underrun; letting it play later would
    // insert a stale-audio blip after the silence cushion.
    partialOffset = 0;

    writeOffset = (writeOffset - (blockCount / 2) + blockCount) % blockCount;

    for (int i = 0; i < blockCount / 2; i++)
    {
        memset(waveHeaders[writeOffset].lpData, 0, singleBufferBytes);
        if (waveOutWrite(hWaveOut, &waveHeaders[writeOffset], sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
        {
            submittedBytes += singleBufferBytes;
            InterlockedIncrement(&bufferCount);
        }
        writeOffset++;
        writeOffset %= blockCount;
    }
}

void CWaveOut::ProcessSound()
{
    // Cheap lower bound first: bufferCount only lags completions, so this
    // never overestimates free space. waveOutGetPosition round-trips into
    // winmm and is far too slow for the per-landing call rate — query the
    // real cursor only once this bound says the queue may be full.
    int freeBytes = ((blockCount - bufferCount) * singleBufferBytes) - partialOffset;

    if (bufferCount == 0)
    {
        RecoverFromUnderrun();
        freeBytes = ((blockCount - bufferCount) * singleBufferBytes) - partialOffset;
    }

    if (Settings.DynamicRateControl)
    {
        S9xUpdateDynamicRate(freeBytes, sumBufferSize);
    }

    UINT32 availableSamples;

    availableSamples = S9xGetSampleCount();

    if (Settings.DynamicRateControl && !Settings.SoundSync)
    {
        // Using rate control, we should always keep the emulator's sound buffers empty to
        // maintain an accurate measurement.
        if (availableSamples > (freeBytes >> 1))
        {
            freeBytes = GetAvailableBytes();
            if (availableSamples > (freeBytes >> 1))
            {
                S9xClearSamples();
                return;
            }
        }
    }

    if (!initDone)
        return;

    if(Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        // no sound sync when speed is not set to 100%
        if ((freeBytes >> 1) < availableSamples)
            freeBytes = GetAvailableBytes();
        const DWORD sync_start = timeGetTime();
        while((freeBytes >> 1) < availableSamples)
        {
            ResetEvent(GUI.SoundSyncEvent);
            // Space may have freed between the check and the reset — waiting
            // now would discard that wakeup and oversleep a full block.
            freeBytes = GetAvailableBytes();
            if ((freeBytes >> 1) >= availableSamples)
                break;
            if (!GUI.AllowSoundSync || timeGetTime() - sync_start > 1000)
            {
                S9xClearSamples();
                return;
            }
            // The event is only a hint; cap the wait so pacing follows the
            // play cursor, never winmm's lagging callback phase.
            WaitForSingleObject(GUI.SoundSyncEvent, 4);
            freeBytes = GetAvailableBytes();
        }
    }


    if (partialOffset != 0) {
        UINT32 samplesleftinblock = (singleBufferBytes - partialOffset) >> 1;
        BYTE *offsetBuffer = (BYTE *)waveHeaders[writeOffset].lpData + partialOffset;

        if (availableSamples < samplesleftinblock)
        {
            S9xMixSamples(offsetBuffer, availableSamples);
            partialOffset += availableSamples << 1;
            availableSamples = 0;
        }
        else
        {
            S9xMixSamples(offsetBuffer, samplesleftinblock);
            partialOffset = 0;
            availableSamples -= samplesleftinblock;
            SubmitBlock(waveHeaders[writeOffset]);
            writeOffset++;
            writeOffset %= blockCount;
        }
    }

    while (availableSamples >= singleBufferSamples && bufferCount < blockCount) {
        BYTE *curBuffer = (BYTE *)waveHeaders[writeOffset].lpData;
        S9xMixSamples(curBuffer, singleBufferSamples);
        SubmitBlock(waveHeaders[writeOffset]);
        writeOffset++;
        writeOffset %= blockCount;
        availableSamples -= singleBufferSamples;
    }

	// need to check this is less than a single buffer, otherwise we have a race condition with bufferCount
    if (availableSamples > 0 && availableSamples < singleBufferSamples && bufferCount < blockCount) {
        S9xMixSamples((BYTE *)waveHeaders[writeOffset].lpData, availableSamples);
        partialOffset = availableSamples << 1;
    }
}

std::vector<std::wstring> CWaveOut::GetDeviceList()
{
    std::vector<std::wstring> device_list;

    device_list.push_back(_T("Default"));

    std::map<std::wstring, std::wstring> endpointId_deviceName_map;

    // try to get device names via multimedia device enumerator, waveOut has a 31 character limit on device names
    // save them in a map with their endpoint id to later match to waveOut devices
    IMMDeviceEnumerator* deviceEnumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);
    if (SUCCEEDED(hr))
    {
        IMMDeviceCollection* renderDevices;
        hr = deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATEMASK_ALL, &renderDevices);
        if (SUCCEEDED(hr))
        {
            UINT count;
            renderDevices->GetCount(&count);
            for (int i = 0; i < count; i++)
            {
                IMMDevice* renderDevice = NULL;
                if (renderDevices->Item(i, &renderDevice) != S_OK)
                {
                    continue;
                }
                WCHAR* pstrEndpointId = NULL;
                hr = renderDevice->GetId(&pstrEndpointId);
                if (SUCCEEDED(hr))
                {
                    std::wstring strEndpoint = pstrEndpointId;
                    CoTaskMemFree(pstrEndpointId);
                    IPropertyStore* propStore;
                    PROPVARIANT propVar;
                    PropVariantInit(&propVar);
                    hr = renderDevice->OpenPropertyStore(STGM_READ, &propStore);
                    if (SUCCEEDED(hr))
                    {
                        hr = propStore->GetValue(PKEY_Device_FriendlyName, &propVar);
                    }
                    if (SUCCEEDED(hr) && propVar.vt == VT_LPWSTR)
                    {
                        endpointId_deviceName_map[strEndpoint] = propVar.pwszVal;
                    }
                    PropVariantClear(&propVar);
                }

                renderDevice->Release();
            }
            renderDevices->Release();
        }
        deviceEnumerator->Release();
    }

    // enum waveOut devices, using mapped names if available
    UINT num_devices = waveOutGetNumDevs();
    for (unsigned int i = 0; i < num_devices; i++)
    {
        WAVEOUTCAPS caps;
        MMRESULT mmr;
        mmr = waveOutGetDevCaps(i, &caps, sizeof(WAVEOUTCAPS));
        if (mmr != MMSYSERR_NOERROR)
        {
            continue;
        }

        // get endpoint id, called function instance id in waveOutMessage
        size_t endpointIdSize = 0;
        mmr = waveOutMessage((HWAVEOUT)IntToPtr(i), DRV_QUERYFUNCTIONINSTANCEIDSIZE, (DWORD_PTR)&endpointIdSize, NULL);
        if (mmr != MMSYSERR_NOERROR)
        {
            continue;
        }
        std::vector<wchar_t> endpointIdBuffer(endpointIdSize);
        mmr = waveOutMessage((HWAVEOUT)IntToPtr(i), DRV_QUERYFUNCTIONINSTANCEID, (DWORD_PTR)endpointIdBuffer.data(), endpointIdSize);
        if (mmr != MMSYSERR_NOERROR)
        {
            continue;
        }
        std::wstring strEndpoint(endpointIdBuffer.data());

        // use name retrieved from mmdevice above if available
        std::wstring devName = caps.szPname;
        if (endpointId_deviceName_map.count(strEndpoint))
        {
            devName = endpointId_deviceName_map[strEndpoint];
        }

        device_list.push_back(devName);
    }

    return device_list;
}

int CWaveOut::FindDeviceIndex(TCHAR *audio_device)
{
    std::vector<std::wstring> device_list = GetDeviceList();

    int index = 0;

    for (int i = 0; i < device_list.size(); i++)
    {
        if (_tcsstr(device_list[i].c_str(), audio_device) != NULL)
        {
            index = i;
            break;
        }
    }

    return index;
}
