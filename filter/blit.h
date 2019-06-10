/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _blit_h_
#define _blit_h_

#include "2xsai.h"
#include "epx.h"
#include "hq2x.h"
#include "snes_ntsc.h"

bool8 S9xBlitFilterInit (void);
void S9xBlitFilterDeinit (void);
void S9xBlitClearDelta (void);
bool8 S9xBlitNTSCFilterInit (void);
void S9xBlitNTSCFilterDeinit (void);
void S9xBlitNTSCFilterSet (const snes_ntsc_setup_t *);
void S9xBlitPixSimple1x1 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixSimple1x2 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixSimple2x1 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixSimple2x2 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixBlend1x1 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixBlend2x1 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixTV1x2 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixTV2x2 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixMixedTV1x2 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixSmooth2x2 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixSuperEagle16 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPix2xSaI16 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixSuper2xSaI16 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixEPX16 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixHQ2x16 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixHQ3x16 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixHQ4x16 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixNTSC16 (uint8 *, int, uint8 *, int, int, int);
void S9xBlitPixHiResNTSC16 (uint8 *, int, uint8 *, int, int, int);

#endif
