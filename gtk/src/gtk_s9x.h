/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "gtk_config.h"
#include "gtk_s9xwindow.h"

#include <glib/gi18n.h>

#define SNES9X_GTK_AUTHORS "© 2007 - 2023 Brandon Wright (bearoso@gmail.com)"
#define SNES9X_GTK_VERSION "87"

extern Snes9xWindow *top_level;
extern Snes9xConfig *gui_config;

int S9xOpenROM(const char *filename);
void S9xNoROMLoaded();
void S9xROMLoaded();
