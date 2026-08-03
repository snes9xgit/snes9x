/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

#include <windows.h>
#include "IS9xSoundOutput.h"
#include <mmsystem.h>
#include <vector>

class CWaveOut : public IS9xSoundOutput
{
  private:
    void BeginPlayback(void);
    void StopPlayback(void);
    void ProcessSound(void);
    int GetAvailableBytes();
    void RecoverFromUnderrun();
    void SubmitBlock(WAVEHDR &header);

    HWAVEOUT hWaveOut;
    bool initDone;

    volatile LONG bufferCount;
    UINT32 sumBufferSize;
    UINT32 singleBufferSamples;
    UINT32 singleBufferBytes;
    const UINT32 blockCount = 8;
    UINT32 writeOffset;
    UINT32 partialOffset;
    std::vector<WAVEHDR> waveHeaders;

    // Click suppression at queue underrun / pause boundaries (see CXAudio2 for
    // the rationale). last_sample_l/r ride at the trailing frame of the most
    // recent submission so a fade-out tail can begin where audio stopped;
    // fade_in_pending is the trigger flag, fade_in_pos tracks position across
    // multiple SubmitBlock calls because a single audio block is shorter than
    // the full ramp window.
    int16 last_sample_l;
    int16 last_sample_r;
    volatile LONG fade_in_pending;
    UINT32 fade_in_pos;
    volatile LONG fadeout_in_flight;
    // waveOutSetVolume tops out at unity, so any requested volume above it
    // (the pre-amp gain) is applied to the samples in SubmitBlock. Q12.
    volatile LONG preampGainQ12;
    WAVEHDR fadeoutHeader;
    int16 *fadeoutBuffer;
    bool fadeoutPrepared;

  public:
    CWaveOut(void);
    ~CWaveOut(void);

    // Inherited from IS9xSoundOutput
    bool InitSoundOutput(void);
    void DeInitSoundOutput(void);
    bool SetupSound(void);
    void SetVolume(double volume);
	std::vector<std::wstring> GetDeviceList();
	int FindDeviceIndex(TCHAR *audio_device);
	void OnPauseRequested();
	void OnResumeRequested();

	static void CALLBACK WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
};
