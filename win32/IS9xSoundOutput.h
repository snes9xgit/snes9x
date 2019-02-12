/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef IS9XSOUNDOUTPUT_H
#define IS9XSOUNDOUTPUT_H
#include "../port.h"
#include <vector>
#include <string>

/* IS9xSoundOutput
	Interface for the sound output.
*/

class IS9xSoundOutput {
public:
	// InitSoundOutput should initialize the sound output but not start playback
	virtual bool InitSoundOutput(void)=0;

	// DeInitSoundOutput should stop playback and uninitialize the output
	virtual void DeInitSoundOutput(void)=0;

	// SetupSound should apply the current sound settings for outputbuffers/devices and
	// (re)start playback
	virtual bool SetupSound()=0;

	// SetVolume should set a new volume level (between 0.0 and 1.0)
	virtual void SetVolume(double volume) = 0;

	// ProcessSound should queue new available samples into the Host sound 
    // system. If the sound system is callback based, ProcessSound should move
    // all samples into a buffer that the callback can read, using a critical 
    // section while accessing that buffer for thread-safety.
	virtual void ProcessSound()=0;

    // GetDeviceList should return a list of device strings that can be displayed in a dropdown
    virtual std::vector<std::wstring> GetDeviceList()
    {
        return std::vector<std::wstring>();
    }

    // FindDeviceIndex should try to find a matching index in the device list for a particular device string
    virtual int FindDeviceIndex(TCHAR *audio_device)
    {
        return 0;
    }

};

#endif
