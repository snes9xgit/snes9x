/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_FILE_H
#define __GTK_FILE_H

char *S9xOpenROMDialog ();
const char *S9xBasenameNoExt (const char *);

void S9xLoadState (const char *filename);
void S9xSaveState (const char *filename);

void S9xQuickSaveSlot (int slot);
void S9xQuickLoadSlot (int slot);

#endif /* __GTK_FILE_H */
