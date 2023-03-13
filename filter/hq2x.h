/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <cstdint>

#ifndef _hq2x_h_
#define _hq2x_h_

bool S9xBlitHQ2xFilterInit(void);
void S9xBlitHQ2xFilterDeinit(void);
void HQ2X_16 (uint8_t *, int, uint8_t *, int, int, int);
void HQ3X_16 (uint8_t *, int, uint8_t *, int, int, int);
void HQ4X_16 (uint8_t *, int, uint8_t *, int, int, int);

#endif
