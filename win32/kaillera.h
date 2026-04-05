/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef KAILLERA_H
#define KAILLERA_H

#include "../kaillera_client.h"
#include "../kaillera_server.h"

#ifdef KAILLERA_SUPPORT

// Custom Windows messages for Kaillera game lifecycle
#define WM_KAILLERA_GAME_START  (WM_USER + 10)
#define WM_KAILLERA_GAME_END    (WM_USER + 11)

// Win32-specific initialization
void Kaillera_Win32_RegisterCallbacks();

#endif // KAILLERA_SUPPORT
#endif // KAILLERA_H
