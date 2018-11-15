/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef RENDER_H
#define RENDER_H


/* Render.h
 * ----------
 * This file contains all the prototypes, structers and variables used and defined by render.cpp
 */

struct SSurface {
	unsigned char *Surface;

	unsigned int Pitch;
	unsigned int Width, Height;
};

void RenderMethod(SSurface Src, SSurface Dst, RECT *);

void SelectRenderMethod();
void InitRenderFilters();
void DeInitRenderFilters();

RECT GetFilterOutputSize(SSurface Src);

#endif
