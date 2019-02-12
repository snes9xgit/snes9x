/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

#include "../snes9x.h"
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
};
