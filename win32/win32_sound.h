/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef WIN32_SOUND_H
#define WIN32_SOUND_H

#include <vector>
#include <string>

class IS9xSoundOutput;
extern IS9xSoundOutput *S9xSoundOutput;

bool ReInitSound();
void ApplyLiveSoundSettings();   // no device rebuild - see win32_sound.cpp
void S9xSoundCallback(void *data);
void CloseSoundDevice();
std::vector<std::wstring> GetAvailableSoundDevices();
int FindAudioDeviceIndex(TCHAR *audio_device);

#endif
