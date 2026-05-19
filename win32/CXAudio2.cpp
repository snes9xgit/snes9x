/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "CXAudio2.h"
#include "../snes9x.h"
#include "../apu/apu.h"
#include "../sgb/sgb.h"
#include "wsnes9x.h"
#include <process.h>
#include "dxerr.h"
#include "commctrl.h"
#include <assert.h>
#include <math.h>

/* CXAudio2
	Implements audio output through XAudio2.
	Basic idea: one master voice and one source voice are created;
	the source voice consumes buffers queued by PushBuffer, plays them through
	the master voice and calls OnBufferEnd after each buffer.
	ProcessSound copies new samples into the buffer and queues them for playback.
*/

/*  Construction/Destruction
*/
// Length of the click-suppression ramps applied at audio output transitions.
// 1024 stereo frames ~= 23ms at 44.1kHz: long enough that the fade's own
// spectral fundamental sits below ~22Hz (sub-audible — speakers can't push
// enough cone displacement to be heard), and short enough that pause/resume
// still feels instantaneous. A shorter window (e.g. 256 frames / 5.8ms)
// places the fundamental around 170Hz and is itself audible as a bass kick.
static const UINT32 kFadeFrames = 1024;
static const UINT32 kFadeBytes  = kFadeFrames * 2 * sizeof(int16);
static const double kPi         = 3.14159265358979323846;

// Raised-cosine ("Hann half-window") gain in Q15 at position pos in [0..len-1].
// rising = true gives a 0->1 ramp, false gives 1->0. Cosine-shaped because a
// linear ramp has a derivative discontinuity at each endpoint that radiates
// broadband splatter on top of the fade's fundamental.
static inline int CosineGainQ15(UINT32 pos, UINT32 len, bool rising)
{
    if (len < 2)        return rising ? 32768 : 0;
    if (pos >= len - 1) return rising ? 32768 : 0;
    const double t  = (double)pos / (double)(len - 1);
    const double c  = cos(kPi * t);
    const double g  = rising ? 0.5 * (1.0 - c) : 0.5 * (1.0 + c);
    return (int)(g * 32768.0 + 0.5);
}

// Sentinel passed as buffer context so OnBufferEnd can recognize the pause-time
// fade-out tail and clear fadeout_in_flight when XAudio2 is done with it.
static void * const kFadeoutContext = (void*)0x1;

CXAudio2::CXAudio2(void)
{
	pXAudio2 = NULL;
	pSourceVoice = NULL;
	pMasterVoice = NULL;

	sum_bufferSize = singleBufferBytes \
		= singleBufferSamples = blockCount = 0;
	soundBuffer = NULL;
	initDone = false;

	last_sample_l = 0;
	last_sample_r = 0;
	fade_in_pending = 0;
	fade_in_pos = kFadeFrames;  // "done" — no fade in progress
	fadeout_in_flight = 0;
	fadeoutBuffer = NULL;

	drainThread = NULL;
	drainShutdown = 0;
	drainThreadId = 0;
}

CXAudio2::~CXAudio2(void)
{
	DeInitXAudio2();
}

INT_PTR CALLBACK DlgXAudio2InitError(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			// display warning icon and set text of sys control (too long for resource)
			HICON aIcn = LoadIcon(NULL, IDI_WARNING);
			SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)aIcn, 0);
			SetDlgItemText(hDlg, IDC_SYSLINK_DX, TEXT("Unable to initialize XAudio2.\
You will not be able to hear any sound effects or music while playing.\n\n\
This is usually caused by not having a recent DirectX9 runtime installed.\n\
You can download the most recent DirectX9 runtime here:\n\n\
<a href=\"https://www.microsoft.com/en-us/download/details.aspx?id=35\">https://www.microsoft.com/en-us/download/details.aspx?id=35</a>"));

			// center dialog on parent
			HWND parent = GetParent(hDlg);
			RECT rcParent, rcSelf;
			GetWindowRect(parent, &rcParent);
			GetWindowRect(hDlg, &rcSelf);

			SetWindowPos(hDlg,
				HWND_TOP,
				rcParent.left + ((rcParent.right - rcParent.left) - (rcSelf.right - rcSelf.left)) / 2,
				rcParent.top + ((rcParent.bottom - rcParent.top) - (rcSelf.bottom - rcSelf.top)) / 2,
				0, 0,
				SWP_NOSIZE);
		}
		return true;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (HIWORD(wParam) == BN_CLICKED) {
				EndDialog(hDlg, IDOK);
				return TRUE;
			}
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
			case NM_CLICK:          // Fall through to the next case.
			case NM_RETURN:
			{
				PNMLINK pNMLink = (PNMLINK)lParam;
				LITEM   item = pNMLink->item;

				// open link with registered application
				ShellExecute(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
				break;
			}
		}
	}

	return FALSE;
}

/*  CXAudio2::InitXAudio2
initializes the XAudio2 object
-----
returns true if successful, false otherwise
*/
bool CXAudio2::InitXAudio2(void)
{
	if(initDone)
		return true;

	DeInitXAudio2();

	HRESULT hr;
	if ( FAILED(hr = XAudio2Create( &pXAudio2, 0 , XAUDIO2_DEFAULT_PROCESSOR ) ) ) {
		DXTRACE_ERR_MSGBOX(TEXT("Unable to create XAudio2 object."),hr);
		DialogBox(GUI.hInstance, MAKEINTRESOURCE(IDD_DIALOG_XAUDIO2_INIT_ERROR), GUI.hWnd, DlgXAudio2InitError);
		return false;
	}
	initDone = true;
	pXAudio2->RegisterForCallbacks(this);
	return true;
}

static void MixSpcOverGB(uint8 *dest, int sample_words);

DWORD WINAPI CXAudio2::AudioDrainThreadProc(LPVOID param)
{
	CXAudio2 *self = (CXAudio2 *)param;
	InterlockedExchange((LONG *)&self->drainThreadId, (LONG)GetCurrentThreadId());

	const LONG queue_target = (LONG)self->blockCount - 1;

	while (!self->drainShutdown)
	{
		if (!Settings.SGB_BIOSModeActive || !S9xSGBBIOSGBIsReleased() ||
		    Settings.TurboMode || Settings.Mute || !self->initDone ||
		    !self->soundBuffer)
		{
			Sleep(2);
			continue;
		}

		DWORD spin_start = GetTickCount();
		while (self->bufferCount >= queue_target && !self->drainShutdown)
		{
			SwitchToThread();
			if (GetTickCount() - spin_start > 50) break;
		}
		if (self->drainShutdown) break;

		uint8 *curBuffer = self->soundBuffer + self->writeOffset;
		S9xMixSamples(curBuffer, self->singleBufferSamples);
		MixSpcOverGB(curBuffer, self->singleBufferSamples);
		self->PushBuffer(self->singleBufferBytes, curBuffer, NULL);
		self->writeOffset += self->singleBufferBytes;
		self->writeOffset %= self->sum_bufferSize;

		S9xSGBSetAudioRate(Settings.SoundPlaybackRate);

		extern volatile LONG g_drain_pushes_per_sec;
		static DWORD push_window_start = 0;
		static LONG push_count = 0;
		DWORD now_tick = GetTickCount();
		if (push_window_start == 0) push_window_start = now_tick;
		++push_count;
		if (now_tick - push_window_start >= 1000)
		{
			InterlockedExchange(&g_drain_pushes_per_sec, push_count);
			push_count = 0;
			push_window_start = now_tick;
		}
	}

	return 0;
}

volatile LONG g_drain_pushes_per_sec = 0;

/*  CXAudio2::InitVoices
initializes the voice objects with the current audio settings
-----
returns true if successful, false otherwise
*/
bool CXAudio2::InitVoices(void)
{
	HRESULT hr;
    // subtract -1, we added "Default" as first index
    int device_index = FindDeviceIndex(GUI.AudioDevice) - 1;
    if (device_index < 0)
        device_index = 0;

	if ( FAILED(hr = pXAudio2->CreateMasteringVoice( &pMasterVoice, 2,
		Settings.SoundPlaybackRate, 0, NULL, NULL ) ) ) {
			DXTRACE_ERR_MSGBOX(TEXT("Unable to create mastering voice."),hr);
			return false;
	}

	WAVEFORMATEX wfx;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = Settings.SoundPlaybackRate;
    wfx.nBlockAlign = 2 * 2;
    wfx.wBitsPerSample = 16;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

	if( FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx,
		XAUDIO2_VOICE_NOSRC, XAUDIO2_DEFAULT_FREQ_RATIO, this, NULL, NULL ) ) ) {
			DXTRACE_ERR_MSGBOX(TEXT("Unable to create source voice."),hr);
			return false;
	}

	return true;
}

/*  CXAudio2::DeInitXAudio2
deinitializes all objects
*/
void CXAudio2::DeInitXAudio2(void)
{
	initDone = false;
	DeInitVoices();	
	if(pXAudio2) {
		pXAudio2->Release();
		pXAudio2 = NULL;
	}
}

/*  CXAudio2::DeInitVoices
deinitializes the voice objects and buffers
*/
void CXAudio2::DeInitVoices(void)
{
	if (drainThread)
	{
		InterlockedExchange(&drainShutdown, 1);
		WaitForSingleObject(drainThread, 2000);
		CloseHandle(drainThread);
		drainThread = NULL;
		drainThreadId = 0;
	}
	if(pSourceVoice) {
		StopPlayback();
		pSourceVoice->DestroyVoice();
		pSourceVoice = NULL;
	}
	if(pMasterVoice) {
		pMasterVoice->DestroyVoice();
		pMasterVoice = NULL;
	}
	if(soundBuffer) {
		delete [] soundBuffer;
		soundBuffer = NULL;
	}
	if(fadeoutBuffer) {
		delete [] fadeoutBuffer;
		fadeoutBuffer = NULL;
	}
	last_sample_l = 0;
	last_sample_r = 0;
	InterlockedExchange(&fade_in_pending, 0);
	fade_in_pos = kFadeFrames;
	InterlockedExchange(&fadeout_in_flight, 0);
}

/*  CXAudio2::OnBufferEnd
callback function called by the source voice
IN:
pBufferContext		-	NULL for normal buffers, kFadeoutContext for the
                        pause-time fade-out tail
*/
void CXAudio2::OnBufferEnd(void *pBufferContext)
{
	LONG remaining = InterlockedDecrement(&bufferCount);
	if (pBufferContext == kFadeoutContext)
		InterlockedExchange(&fadeout_in_flight, 0);
	if (remaining == 0)
		InterlockedExchange(&fade_in_pending, 1);
    SetEvent(GUI.SoundSyncEvent);
}

/*  CXAudio2::PushBuffer
pushes one buffer onto the source voice playback queue
IN:
AudioBytes		-	size of the buffer
pAudioData		-	pointer to the buffer
pContext		-	context passed to the callback, currently unused
*/
void CXAudio2::PushBuffer(UINT32 AudioBytes,BYTE *pAudioData,void *pContext)
{
	// Fade-in the head of the first buffer pushed after silence so audio
	// doesn't kick back in on a non-zero sample. Skip for the fade-out tail
	// itself (it's already a ramp).
	//
	// The trigger flag is consumed only when the buffer actually has audio
	// content — first boot in particular pushes ~tens of ms of silent buffers
	// while the SPC runs BIOS setup before the chime hits, and consuming the
	// fade-in flag on those would leave the chime onset unfaded. Once content
	// is detected we start the fade *at the onset frame within the buffer*,
	// not at frame 0, so the silence-to-audio step itself is the moment the
	// ramp begins.
	if (pContext != kFadeoutContext)
	{
		int16 *samples = (int16*)pAudioData;
		const UINT32 frames_in_buf = AudioBytes / (2 * sizeof(int16));
		UINT32 fade_start_frame = 0;

		if (fade_in_pending)
		{
			UINT32 onset = frames_in_buf;
			for (UINT32 i = 0; i < frames_in_buf; ++i)
			{
				const int16 l = samples[i*2];
				const int16 r = samples[i*2 + 1];
				if (l > 64 || l < -64 || r > 64 || r < -64)
				{
					onset = i;
					break;
				}
			}
			if (onset < frames_in_buf)
			{
				InterlockedExchange(&fade_in_pending, 0);
				fade_in_pos = 0;
				fade_start_frame = onset;
				// Pre-onset samples were below the detection threshold but
				// can still be non-zero (sub-threshold tail of a slow
				// envelope ramp). Zero them so the seam to the gain-0 onset
				// is exactly 0->0; otherwise the (±63 -> 0) step is heard
				// as a single-sample "tak".
				for (UINT32 i = 0; i < onset; ++i)
				{
					samples[i*2]     = 0;
					samples[i*2 + 1] = 0;
				}
			}
		}

		if (fade_in_pos < kFadeFrames && fade_start_frame < frames_in_buf)
		{
			const UINT32 frames_avail   = frames_in_buf - fade_start_frame;
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

	// Capture trailing stereo frame so a later fade-out tail can start where
	// real audio stopped, avoiding a discontinuity at the seam.
	if (AudioBytes >= 2 * sizeof(int16))
	{
		const int16 *samples = (const int16*)pAudioData;
		const UINT32 last = (AudioBytes / sizeof(int16)) - 2;
		last_sample_l = samples[last];
		last_sample_r = samples[last + 1];
	}

	XAUDIO2_BUFFER xa2buffer={0};
	xa2buffer.AudioBytes=AudioBytes;
	xa2buffer.pAudioData=pAudioData;
	xa2buffer.pContext=pContext;
	InterlockedIncrement(&bufferCount);
	pSourceVoice->SubmitSourceBuffer(&xa2buffer);
}

// Build and submit a short ramp from the trailing real-audio sample down to
// silence. Called from S9xSetPause when the main loop is about to stop feeding
// the queue (menu open, window move, focus loss). The tail rides the back of
// any in-flight buffers so the queue drains to zero smoothly. Resuming after
// pause is handled implicitly: OnBufferEnd flips fade_in_pending when the
// queue empties, and the next normal push gets a head ramp.
void CXAudio2::OnPauseRequested()
{
	if (!initDone || !pSourceVoice)
		return;
	if (!fadeoutBuffer)
		return;
	// Skip if we already have a fade-out tail in the queue. Re-submitting
	// would race with XAudio2 still reading from fadeoutBuffer.
	if (InterlockedCompareExchange(&fadeout_in_flight, 1, 0) != 0)
		return;

	for (UINT32 i = 0; i < kFadeFrames; ++i)
	{
		const int32 gain_q15 = CosineGainQ15(i, kFadeFrames, false);
		fadeoutBuffer[i*2]     = (int16)(((int32)last_sample_l * gain_q15) >> 15);
		fadeoutBuffer[i*2 + 1] = (int16)(((int32)last_sample_r * gain_q15) >> 15);
	}

	XAUDIO2_BUFFER xa2buffer = {0};
	xa2buffer.AudioBytes = kFadeBytes;
	xa2buffer.pAudioData = (BYTE*)fadeoutBuffer;
	xa2buffer.pContext   = kFadeoutContext;
	InterlockedIncrement(&bufferCount);
	pSourceVoice->SubmitSourceBuffer(&xa2buffer);

	last_sample_l = 0;
	last_sample_r = 0;
	// Arm fade-in for the next normal buffer regardless of whether the queue
	// fully drains before resume — the fade-out itself ends at zero, so the
	// successor needs to ramp up from zero either way.
	InterlockedExchange(&fade_in_pending, 1);
}

void CXAudio2::OnResumeRequested()
{
	// Resume path is covered by fade_in_pending set in OnPauseRequested or
	// OnBufferEnd-on-underrun; nothing to do here.
}

/*  CXAudio2::SetupSound
applies current sound settings by recreating the voice objects and buffers
-----
returns true if successful, false otherwise
*/
bool CXAudio2::SetupSound()
{
	if(!initDone)
		return false;

	DeInitVoices();

	blockCount = 8;
	UINT32 blockTime = GUI.SoundBufferSize / blockCount;

	singleBufferSamples = (Settings.SoundPlaybackRate * blockTime) / 1000;
    singleBufferSamples *= 2;
	singleBufferBytes = singleBufferSamples * 2;
	sum_bufferSize = singleBufferBytes * blockCount;

    if (InitVoices())
    {
		soundBuffer = new uint8[sum_bufferSize];
		fadeoutBuffer = new int16[kFadeFrames * 2];
		writeOffset = 0;
		partialOffset = 0;
		// Fade in the first buffer so cold start doesn't kick from silence
		// to a non-zero sample.
		InterlockedExchange(&fade_in_pending, 1);
    }
	else {
		DeInitVoices();
		return false;
	}

	bufferCount = 0;

	BeginPlayback();


    return true;
}

void CXAudio2::SetVolume(double volume)
{
	pSourceVoice->SetVolume(volume);
}

void CXAudio2::BeginPlayback()
{
	pSourceVoice->Start(0);
}

void CXAudio2::StopPlayback()
{
	pSourceVoice->Stop(0);
}

int CXAudio2::GetAvailableBytes()
{
    return ((blockCount - bufferCount) * singleBufferBytes) - partialOffset;
}

static void MixSpcOverGB(uint8 *dest, int sample_words)
{
    const bool sgb_bios_mix = Settings.SGB_BIOSModeActive && S9xSGBBIOSGBIsReleased();
    int16_t *out16 = (int16_t *)dest;
    const int frames = sample_words / 2;

    if (!sgb_bios_mix)
    {
        S9xAudioWaveformPushMix(out16, frames);
        return;
    }

    static const int GB_GAIN_Q8  = 242;
    static const int SPC_GAIN_Q8 = 512;

    int16_t spc_buf[2048];
    int cap = (sample_words < 2048) ? sample_words : 2048;
    int n = S9xPullSpcOutput(spc_buf, cap);
    int i = 0;
    for (; i < n; ++i)
    {
        int32_t gb    = ((int32_t)out16[i]   * GB_GAIN_Q8)  >> 8;
        int32_t spc   = ((int32_t)spc_buf[i] * SPC_GAIN_Q8) >> 8;
        int32_t mixed = gb + spc;
        if (mixed >  32767) mixed =  32767;
        if (mixed < -32768) mixed = -32768;
        out16[i] = (int16_t)mixed;
    }
    for (; i < sample_words; ++i)
    {
        int32_t gb = ((int32_t)out16[i] * GB_GAIN_Q8) >> 8;
        if (gb >  32767) gb =  32767;
        if (gb < -32768) gb = -32768;
        out16[i] = (int16_t)gb;
    }

    // Silence the SPC-over-GB mix until the GB completes its boot ROM
    // (writes $FF50 → boot_handoff_captured). Bridges the audible click
    // between BIOS-released and the GB taking over the audio path —
    // the click sample stayed in DSound's queue from the pre-release
    // SPC stream and emerged after the mix path switched, so neither
    // muting GB nor SPC individually killed it. Zeroing the post-mix
    // output through this window is the only thing that does.
    if (!S9xSGBBootHandoffCaptured())
        memset(out16, 0, sample_words * sizeof(int16_t));

    S9xAudioWaveformPushMix(out16, frames);
}

/*  CXAudio2::ProcessSound
The mixing function called by the sound core when new samples are available.
SoundBuffer is divided into blockCount blocks. If there are enough available samples and a free block,
the block is filled and queued to the source voice. bufferCount is increased by pushbuffer and decreased by
the OnBufferComplete callback.
*/
void CXAudio2::ProcessSound()
{
	if (Settings.SGB_BIOSModeActive && S9xSGBBIOSGBIsReleased() &&
	    drainThread != NULL && GetCurrentThreadId() != drainThreadId)
	{
		return;
	}

	int freeBytes = GetAvailableBytes();

	if (Settings.DynamicRateControl)
	{
		S9xUpdateDynamicRate(freeBytes, sum_bufferSize);
	}

	UINT32 availableSamples;

	availableSamples = S9xGetSampleCount();

	if (Settings.SGB_BIOSModeActive && S9xSGBBIOSGBIsReleased())
	{
		S9xSpcAdjustRate(1.0);
	}
	else
	{
		S9xSpcResetDrc();
	}

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

	if(!initDone)
		return;

    if(Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        // no sound sync when speed is not set to 100%
        const bool sgb_bios_mix = Settings.SGB_BIOSModeActive && S9xSGBBIOSGBIsReleased();
        const UINT32 wait_threshold = sgb_bios_mix
            ? singleBufferSamples
            : availableSamples;
        while((freeBytes >> 1) < wait_threshold)
        {
            if (bufferCount == 0)
                break;
            ResetEvent(GUI.SoundSyncEvent);
            if(!GUI.AllowSoundSync || WaitForSingleObject(GUI.SoundSyncEvent, 1000) != WAIT_OBJECT_0)
            {
                S9xClearSamples();
                return;
            }
            freeBytes = GetAvailableBytes();
        }
    }

	if (partialOffset != 0)	{
		assert(partialOffset < singleBufferBytes);
		assert(bufferCount < blockCount);
		BYTE *offsetBuffer = soundBuffer + writeOffset + partialOffset;
		UINT32 samplesleftinblock = (singleBufferBytes - partialOffset) >> 1;

		if (availableSamples < samplesleftinblock)
		{
			S9xMixSamples(offsetBuffer, availableSamples);
			MixSpcOverGB(offsetBuffer, availableSamples);
            partialOffset += availableSamples << 1;
			assert(partialOffset < singleBufferBytes);
			availableSamples = 0;
		}
		else
		{
			S9xMixSamples(offsetBuffer, samplesleftinblock);
			MixSpcOverGB(offsetBuffer, samplesleftinblock);
			partialOffset = 0;
			availableSamples -= samplesleftinblock;
			PushBuffer(singleBufferBytes, soundBuffer + writeOffset, NULL);
			writeOffset += singleBufferBytes;
			writeOffset %= sum_bufferSize;
		}
	}

	while (availableSamples >= singleBufferSamples && bufferCount < blockCount) {
		BYTE *curBuffer = soundBuffer + writeOffset;
		S9xMixSamples(curBuffer, singleBufferSamples);
		MixSpcOverGB(curBuffer, singleBufferSamples);
		PushBuffer(singleBufferBytes, curBuffer, NULL);
		writeOffset += singleBufferBytes;
		writeOffset %= sum_bufferSize;
		availableSamples -= singleBufferSamples;
	}

	// need to check this is less than a single buffer, otherwise we have a race condition with bufferCount
	if (availableSamples > 0 && availableSamples < singleBufferSamples && bufferCount < blockCount) {
		S9xMixSamples(soundBuffer + writeOffset, availableSamples);
		MixSpcOverGB(soundBuffer + writeOffset, availableSamples);
		partialOffset = availableSamples << 1;
		assert(partialOffset < singleBufferBytes);
	}
}

/*  CXAudio2::GetDeviceList
get a list of the available output devices
-----
returns a vector of display names
*/
std::vector<std::wstring> CXAudio2::GetDeviceList()
{
    std::vector<std::wstring> device_list;

    device_list.push_back(_T("Default"));

    return device_list;
}

/*  CXAudio2::FindDeviceIndex
find a device name in the list of possible output devices
-----
returns the index in the device list returned by GetDeviceList
*/
int CXAudio2::FindDeviceIndex(TCHAR *audio_device)
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
