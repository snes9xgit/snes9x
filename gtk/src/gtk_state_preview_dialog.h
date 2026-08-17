/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "gtk_compat.h"

/* Shows the win32-style save/load-with-preview picker: a grid of the current
 * bank's slots with their screenshots, plus a bank selector. Returns the flat
 * state index (bank * SAVE_SLOTS_PER_BANK + slot) that was picked, or -1 if
 * the dialog was cancelled. */
int S9xStatePreviewDialog(Gtk::Window &parent, bool is_save);
