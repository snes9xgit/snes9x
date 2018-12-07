/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "cpuops.h"
#include "dma.h"
#include "apu/apu.h"
#include "fxemu.h"
#include "snapshot.h"
#ifdef DEBUGGER
#include "debug.h"
#include "missing.h"
#endif

#include "port.h"
#include "filter_epx_unsafe.h"

#undef  AVERAGE_565
#define AVERAGE_565(el0, el1) (((el0) & (el1)) + ((((el0) ^ (el1)) & 0xF7DE) >> 1))

/* Allows vertical overlap. We need this to avoid seams when threading */
void EPX_16_unsafe (uint8 *srcPtr, 
                    uint32 srcPitch, 
                    uint8 *dstPtr, 
                    uint32 dstPitch, 
                    int width, 
                    int height)
{
    uint16  colorX, colorA, colorB, colorC, colorD;
    uint16  *sP, *uP, *lP;
    uint32  *dP1, *dP2;
    int     w;

    for (; height; height--)
    {
        sP  = (uint16 *) srcPtr;
        uP  = (uint16 *) (srcPtr - srcPitch);
        lP  = (uint16 *) (srcPtr + srcPitch);
        dP1 = (uint32 *) dstPtr;
        dP2 = (uint32 *) (dstPtr + dstPitch);

        // left edge

        colorX = *sP;
        colorC = *++sP;
        colorB = *lP++;
        colorD = *uP++;

        if ((colorX != colorC) && (colorB != colorD))
        {
        #ifdef __BIG_ENDIAN__
            *dP1 = (colorX << 16) + ((colorC == colorD) ? colorC : colorX);
            *dP2 = (colorX << 16) + ((colorB == colorC) ? colorB : colorX);
        #else
            *dP1 = colorX + (((colorC == colorD) ? colorC : colorX) << 16);
            *dP2 = colorX + (((colorB == colorC) ? colorB : colorX) << 16);
        #endif
        }
        else
            *dP1 = *dP2 = (colorX << 16) + colorX;

        dP1++;
        dP2++;

        //

        for (w = width - 2; w; w--)
        {
            colorA = colorX;
            colorX = colorC;
            colorC = *++sP;
            colorB = *lP++;
            colorD = *uP++;

            if ((colorA != colorC) && (colorB != colorD))
            {
            #ifdef __BIG_ENDIAN__
                *dP1 = (((colorD == colorA) ? colorD : colorX) << 16) + ((colorC == colorD) ? colorC : colorX);
                *dP2 = (((colorA == colorB) ? colorA : colorX) << 16) + ((colorB == colorC) ? colorB : colorX);
            #else
                *dP1 = ((colorD == colorA) ? colorD : colorX) + (((colorC == colorD) ? colorC : colorX) << 16);
                *dP2 = ((colorA == colorB) ? colorA : colorX) + (((colorB == colorC) ? colorB : colorX) << 16);
            #endif
            }
            else
                *dP1 = *dP2 = (colorX << 16) + colorX;

            dP1++;
            dP2++;
        }

        // right edge

        colorA = colorX;
        colorX = colorC;
        colorB = *lP;
        colorD = *uP;

        if ((colorA != colorX) && (colorB != colorD))
        {
        #ifdef __BIG_ENDIAN__
            *dP1 = (((colorD == colorA) ? colorD : colorX) << 16) + colorX;
            *dP2 = (((colorA == colorB) ? colorA : colorX) << 16) + colorX;
        #else
            *dP1 = ((colorD == colorA) ? colorD : colorX) + (colorX << 16);
            *dP2 = ((colorA == colorB) ? colorA : colorX) + (colorX << 16);
        #endif
        }
        else
            *dP1 = *dP2 = (colorX << 16) + colorX;

        srcPtr += srcPitch;
        dstPtr += dstPitch << 1;
    }
}

/* Blends with edge pixel instead of just using it directly. */
void EPX_16_smooth_unsafe (uint8 *srcPtr,
                           uint32 srcPitch,
                           uint8 *dstPtr,
                           uint32 dstPitch,
                           int width,
                           int height)
{
    uint16  colorX, colorA, colorB, colorC, colorD;
    uint16  *sP, *uP, *lP;
    uint32  *dP1, *dP2;
    int     w;

    for (; height; height--)
    {
        sP  = (uint16 *) srcPtr;
        uP  = (uint16 *) (srcPtr - srcPitch);
        lP  = (uint16 *) (srcPtr + srcPitch);
        dP1 = (uint32 *) dstPtr;
        dP2 = (uint32 *) (dstPtr + dstPitch);

        // left edge

        colorX = *sP;
        colorC = *++sP;
        colorB = *lP++;
        colorD = *uP++;

        if ((colorX != colorC) && (colorB != colorD))
        {
        #ifdef __BIG_ENDIAN__
            *dP1 = (colorX << 16) + ((colorC == colorD) ? AVERAGE_565 (colorC, colorX) : colorX);
            *dP2 = (colorX << 16) + ((colorB == colorC) ? AVERAGE_565 (colorB, colorX) : colorX);
        #else
            *dP1 = colorX + (((colorC == colorD) ? AVERAGE_565 (colorC, colorX) : colorX) << 16);
            *dP2 = colorX + (((colorB == colorC) ? AVERAGE_565 (colorB, colorX) : colorX) << 16);
        #endif
        }
        else
            *dP1 = *dP2 = (colorX << 16) + colorX;

        dP1++;
        dP2++;

        //

        for (w = width - 2; w; w--)
        {
            colorA = colorX;
            colorX = colorC;
            colorC = *++sP;
            colorB = *lP++;
            colorD = *uP++;

            if ((colorA != colorC) && (colorB != colorD))
            {
            #ifdef __BIG_ENDIAN__
                *dP1 = (((colorD == colorA) ? AVERAGE_565 (colorD, colorX) : colorX) << 16) + ((colorC == colorD) ? AVERAGE_565 (colorC, colorX) : colorX);
                *dP2 = (((colorA == colorB) ? AVERAGE_565 (colorA, colorX) : colorX) << 16) + ((colorB == colorC) ? AVERAGE_565 (colorB, colorX) : colorX);
            #else
                *dP1 = ((colorD == colorA) ? AVERAGE_565 (colorD, colorX) : colorX) + (((colorC == colorD) ? AVERAGE_565 (colorC, colorX) : colorX) << 16);
                *dP2 = ((colorA == colorB) ? AVERAGE_565 (colorA, colorX) : colorX) + (((colorB == colorC) ? AVERAGE_565 (colorB, colorX) : colorX) << 16);
            #endif
            }
            else
                *dP1 = *dP2 = (colorX << 16) + colorX;

            dP1++;
            dP2++;
        }

        // right edge

        colorA = colorX;
        colorX = colorC;
        colorB = *lP;
        colorD = *uP;

        if ((colorA != colorX) && (colorB != colorD))
        {
        #ifdef __BIG_ENDIAN__
            *dP1 = (((colorD == colorA) ? AVERAGE_565 (colorD, colorX) : colorX) << 16) + colorX;
            *dP2 = (((colorA == colorB) ? AVERAGE_565 (colorA, colorX) : colorX) << 16) + colorX;
        #else
            *dP1 = ((colorD == colorA) ? AVERAGE_565 (colorD, colorX) : colorX) + (colorX << 16);
            *dP2 = ((colorA == colorB) ? AVERAGE_565 (colorA, colorX) : colorX) + (colorX << 16);
        #endif
        }
        else
            *dP1 = *dP2 = (colorX << 16) + colorX;

        srcPtr += srcPitch;
        dstPtr += dstPitch << 1;
    }
}
