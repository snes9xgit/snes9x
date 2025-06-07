/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include <vector>
#include <string>

void S9xPortSoundInit();
void S9xPortSoundDeinit();
void S9xPortSoundReinit();
void S9xSoundStart();
void S9xSoundStop();

std::vector<std::string> S9xGetSoundDriverNames();
