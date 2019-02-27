/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "IS9xSoundOutput.h"
#include "../snes9x.h"
#include "../apu/apu.h"
#include "wsnes9x.h"
#include "CXAudio2.h"
#include "CWaveOut.h"
#include "win32_sound.h"
#include "win32_display.h"

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

// available sound output methods
CXAudio2 S9xXAudio2;
CWaveOut S9xWaveOut;

// Interface used to access the sound output
IS9xSoundOutput *S9xSoundOutput = &S9xXAudio2;

/*  ReInitSound
reinitializes the sound core with current settings
IN:
mode		-	0 disables sound output, 1 enables
-----
returns true if successful, false otherwise
*/
bool ReInitSound()
{
	if (GUI.AVIOut)
		return false;
	if (GUI.AutomaticInputRate)
	{
		int rate = WinGetAutomaticInputRate();
		if (rate)
			Settings.SoundInputRate = rate;
		else
		{
			GUI.AutomaticInputRate = false;
			Settings.SoundInputRate = 31920;
		}
	}

	Settings.SoundInputRate = CLAMP(Settings.SoundInputRate,31700, 32300);
	Settings.SoundPlaybackRate = CLAMP(Settings.SoundPlaybackRate,8000, 48000);
	S9xSetSoundMute(GUI.Mute);
	if(S9xSoundOutput)
		S9xSoundOutput->DeInitSoundOutput();

    return S9xInitSound(0);
}

void CloseSoundDevice() {
	S9xSoundOutput->DeInitSoundOutput();
	S9xSetSamplesAvailableCallback(NULL,NULL);
}

/*  S9xOpenSoundDevice
called by S9xInitSound - initializes the currently selected sound output and
applies the current sound settings
-----
returns true if successful, false otherwise
*/
bool8 S9xOpenSoundDevice ()
{
	S9xSetSamplesAvailableCallback (NULL, NULL);
	// point the interface to the correct output object
	switch(GUI.SoundDriver) {
		case WIN_WAVEOUT_DRIVER:
			S9xSoundOutput = &S9xWaveOut;
			break;
		case WIN_XAUDIO2_SOUND_DRIVER:
			S9xSoundOutput = &S9xXAudio2;
			break;
		default:	// we default to WaveOut
			GUI.SoundDriver = WIN_WAVEOUT_DRIVER;
			S9xSoundOutput = &S9xWaveOut;
	}
	if(!S9xSoundOutput->InitSoundOutput())
		return false;
	
	if(!S9xSoundOutput->SetupSound())
		return false;
	
	S9xSetSamplesAvailableCallback (S9xSoundCallback, NULL);
	return true;
}

/*  S9xSoundCallback
called by the sound core to process generated samples
*/
void S9xSoundCallback(void *data)
{
	static double last_volume = 1.0;

	// only try to change volume if we actually need to switch it
	double current_volume = (Settings.TurboMode ? GUI.VolumeTurbo : GUI.VolumeRegular) / 100.;
	if (last_volume != current_volume) {
		S9xSoundOutput->SetVolume(current_volume);
		last_volume = current_volume;
	}

	S9xSoundOutput->ProcessSound();
}

/*  GetAvailableSoundDevices
returns a list of output devices available for the current output driver
*/
std::vector<std::wstring> GetAvailableSoundDevices()
{
    return S9xSoundOutput->GetDeviceList();
}

/*  FindAudioDeviceIndex
find an audio device that matches the currently configured audio device string
*/
int FindAudioDeviceIndex(TCHAR *audio_device)
{
    return S9xSoundOutput->FindDeviceIndex(audio_device);
}
