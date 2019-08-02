/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_SOUND_H
#define __GTK_SOUND_H

void S9xPortSoundInit();
void S9xPortSoundDeinit();
void S9xPortSoundReinit();
void S9xSoundStart();
void S9xSoundStop();

int S9xSoundBase2log(int num);
int S9xSoundPowerof2(int num);

#endif /* __GTK_SOUND_H */
