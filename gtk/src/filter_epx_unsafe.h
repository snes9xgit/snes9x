/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __FILTER_EPX_UNSAFE_H
#define __FILTER_EPX_UNSAFE_H

#include <cstdint>
void EPX_16_unsafe(uint8_t *, int, uint8_t *, int, int, int);
void EPX_16_smooth_unsafe(uint8_t *, int, uint8_t *, int, int, int);

#endif /* __FILTER_EPX_UNSAFE_H */
