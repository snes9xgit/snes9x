/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

#ifdef RETROACHIEVEMENTS_SUPPORT

// Registers the GTK platform callbacks (HTTP, dialogs, credential persistence)
// with the shared RetroAchievements core. Safe to call more than once.
void RA_Gtk_RegisterCallbacks();

#endif // RETROACHIEVEMENTS_SUPPORT
