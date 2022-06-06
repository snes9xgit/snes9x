/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_DISPLAY_H
#define __GTK_DISPLAY_H

#include "gtk_s9x.h"
#include "gtk_display_driver.h"
#include "filter/2xsai.h"
#ifdef USE_HQ2X
#include "filter/hq2x.h"
#endif
#ifdef USE_XBRZ
#include "filter_xbrz.h"
#endif
#include "filter/epx.h"
#include "filter_epx_unsafe.h"
#include "filter/snes_ntsc.h"

enum {
    FILTER_NONE       = 0,
    FILTER_SUPEREAGLE = 1,
    FILTER_2XSAI      = 2,
    FILTER_SUPER2XSAI = 3,
    FILTER_EPX        = 4,
    FILTER_EPX_SMOOTH = 5,
    FILTER_NTSC       = 6,
    FILTER_SCANLINES  = 7,
    FILTER_SIMPLE2X   = 8,
    FILTER_SIMPLE3X   = 9,
    FILTER_SIMPLE4X   = 10,
    FILTER_HQ2X       = 11,
    FILTER_HQ3X       = 12,
    FILTER_HQ4X       = 13,
    FILTER_2XBRZ      = 14,
    FILTER_3XBRZ      = 15,
    FILTER_4XBRZ      = 16,
    NUM_FILTERS       = 17
};

enum {
    NTSC_COMPOSITE = 0,
    NTSC_SVIDEO    = 1,
    NTSC_RGB       = 2
};

struct S9xRect
{
    int x;
    int y;
    int w;
    int h;
};

void S9xRegisterYUVTables(uint8 *y, uint8 *u, uint8 *v);
double S9xGetAspect();
S9xRect S9xApplyAspect(int, int, int, int);
void S9xConvertYUV(void *src_buffer,
                   void *dst_buffer,
                   int src_pitch,
                   int dst_pitch,
                   int width,
                   int height);
void S9xConvert(void *src,
                void *dst,
                int src_pitch,
                int dst_pitch,
                int width,
                int height,
                int bpp);
void S9xConvertMask(void *src,
                    void *dst,
                    int src_pitch,
                    int dst_pitch,
                    int width,
                    int height,
                    int rshift,
                    int gshift,
                    int bshift,
                    int bpp);
void S9xFilter(uint8 *src_buffer,
               int src_pitch,
               uint8 *dst_buffer,
               int dst_pitch,
               int &width,
               int &height);
void apply_filter_scale(int &width, int &height);
void S9xDisplayRefresh(int width, int height);
void S9xReinitDisplay();
void S9xDisplayReconfigure();
void S9xQueryDrivers();
S9xDisplayDriver *S9xDisplayGetDriver();
bool S9xDisplayDriverIsReady();

#endif /* __GTK_DISPLAY_H */
