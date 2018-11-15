/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _2xsai_h_
#define _2xsai_h_

bool8 S9xBlit2xSaIFilterInit (void);
void S9xBlit2xSaIFilterDeinit (void);
void SuperEagle (uint8 *, int, uint8 *, int, int, int);
void _2xSaI (uint8 *, int, uint8 *, int, int, int);
void Super2xSaI (uint8 *, int, uint8 *, int, int, int);

#endif
