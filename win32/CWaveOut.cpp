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



CWaveOut::CWaveOut(void)
{
    hWaveOut = NULL;
    initDone = false;
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
        InterlockedDecrement(&wo->bufferCount);
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
    initDone = true;

    return true;
}

void CWaveOut::SetVolume(double volume)
{
    uint32 volumeout = (uint32) (volume * 0xffff);
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

    waveOutClose(hWaveOut);
	hWaveOut = NULL;

    initDone = false;
}

void CWaveOut::StopPlayback()
{
    waveOutPause(hWaveOut);
}

int CWaveOut::GetAvailableBytes()
{
    return ((blockCount - bufferCount) * singleBufferBytes) - partialOffset;
}

// Fill the set of blocks preceding writeOffset with silence and write them
// to the output to get the buffer back to 50%
void CWaveOut::RecoverFromUnderrun()
{
    writeOffset = (writeOffset - (blockCount / 2) + blockCount) % blockCount;

    for (int i = 0; i < blockCount / 2; i++)
    {
        memset(waveHeaders[writeOffset].lpData, 0, singleBufferBytes);
        waveOutWrite(hWaveOut, &waveHeaders[writeOffset], sizeof(WAVEHDR));
        InterlockedIncrement(&bufferCount);
        writeOffset++;
        writeOffset %= blockCount;
    }
}

void CWaveOut::ProcessSound()
{
    int freeBytes = ((blockCount - bufferCount) * singleBufferBytes) - partialOffset;

    if (bufferCount == 0)
        RecoverFromUnderrun();

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
            S9xClearSamples();
            return;
        }
    }

    if (!initDone)
        return;

    if(Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        // no sound sync when speed is not set to 100%
        while((freeBytes >> 1) < availableSamples)
        {
            ResetEvent(GUI.SoundSyncEvent);
            if(!GUI.AllowSoundSync || WaitForSingleObject(GUI.SoundSyncEvent, 1000) != WAIT_OBJECT_0)
            {
                S9xClearSamples();
                return;
            }
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
            waveOutWrite(hWaveOut, &waveHeaders[writeOffset], sizeof(WAVEHDR));
            InterlockedIncrement(&bufferCount);
            writeOffset++;
            writeOffset %= blockCount;
        }
    }

    while (availableSamples >= singleBufferSamples && bufferCount < blockCount) {
        BYTE *curBuffer = (BYTE *)waveHeaders[writeOffset].lpData;
        S9xMixSamples(curBuffer, singleBufferSamples);
        waveOutWrite(hWaveOut, &waveHeaders[writeOffset], sizeof(WAVEHDR));
        InterlockedIncrement(&bufferCount);
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

    // try to enumerate devices via multimedia device enumerator, waveout has a 31 character limit on device names
    IMMDeviceEnumerator* deviceEnumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);
    if (SUCCEEDED(hr))
    {
        IMMDeviceCollection* renderDevices;
        hr = deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &renderDevices);
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
                    device_list.push_back(propVar.pwszVal);
                }
                PropVariantClear(&propVar);
                renderDevice->Release();
            }
            renderDevices->Release();
        }
        deviceEnumerator->Release();
    }

    // if we still only have "default" in the list, use old waveout enumeration
    if (device_list.size() == 1)
    {
        UINT num_devices = waveOutGetNumDevs();

        for (unsigned int i = 0; i < num_devices; i++)
        {
            WAVEOUTCAPS caps;
            if (waveOutGetDevCaps(i, &caps, sizeof(WAVEOUTCAPS)) == MMSYSERR_NOERROR)
            {
                device_list.push_back(caps.szPname);
            }
        }
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
