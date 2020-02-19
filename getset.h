/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _GETSET_H_
#define _GETSET_H_

#include "port.h"

enum s9xwrap_t
{
	WRAP_NONE,
	WRAP_BANK,
	WRAP_PAGE
};

enum s9xwriteorder_t
{
	WRITE_01,
	WRITE_10
};

int32 memory_speed(uint32 address);
uint8 S9xGetByte(uint32 Address);
uint16 S9xGetWord(uint32 Address, enum s9xwrap_t w = WRAP_NONE);
void S9xSetByte(uint8 Byte, uint32 Address);
void S9xSetWord(uint16 Word, uint32 Address, enum s9xwrap_t w = WRAP_NONE, enum s9xwriteorder_t o = WRITE_01);
void S9xSetPCBase(uint32 Address);
uint8 * S9xGetBasePointer(uint32 Address);
uint8 * S9xGetMemPointer(uint32 Address);

#endif
