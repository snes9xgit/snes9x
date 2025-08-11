/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _filter_bicubic_win_h_
#define _filter_bicubic_win_h_

void filter_bicubic4x_standard(uint8_t* srcPtr, int srcPitch, uint8_t* dstPtr, int dstPitch, int srcWidth, int srcHeight);

#endif
