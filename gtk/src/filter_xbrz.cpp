
/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2016  BearOso,
                             OV2

  (c) Copyright 2011 - 2016  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  S-SMP emulator code used in 1.54+
  (c) Copyright 2016         byuu

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2016  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2016  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones

  Libretro port
  (c) Copyright 2011 - 2016  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/

#include "snes9x.h"
#include "../filter/xbrz.h"
#include <vector>

#define CONVERT_16_TO_32(pixel) \
    (((((pixel) >> 10)        ) << /*RedShift+3*/  19) | \
     ((((pixel) >> 5)   & 0x1f) << /*GreenShift+3*/11) | \
      (((pixel)         & 0x1f) << /*BlueShift+3*/ 3))

#define CONVERT_32_TO_16(pixel) \
    (((((pixel) & 0xf80000) >> 9) | \
      (((pixel) & 0xf800)   >> 6) | \
      (((pixel) & 0xf8)     >> 3)) & 0xffff)

inline
void copyImage16To32(const uint16_t* src, int width, int height, int srcPitch,
                     uint32_t* trg, int yFirst, int yLast)
{
    yFirst = std::max(yFirst, 0);
    yLast  = std::min(yLast, height);
    if (yFirst >= yLast || height <= 0 || width <= 0) return;

    for (int y = yFirst; y < yLast; ++y)
    {
        uint32_t* trgLine = trg + y * width;
        const uint16_t* srcLine = reinterpret_cast<const uint16_t*>(reinterpret_cast<const char*>(src) + y * srcPitch);

        for (int x = 0; x < width; ++x)
            trgLine[x] = CONVERT_16_TO_32(srcLine[x]);
    }
}


//stretch image and convert from ARGB to RGB565/555
inline
void stretchImage32To16(const uint32_t* src, int srcWidth, int srcHeight,
                        uint16_t* trg, int trgWidth, int trgHeight, int trgPitch,
                        int yFirst, int yLast)
{
    yFirst = std::max(yFirst, 0);
    yLast  = std::min(yLast, trgHeight);
    if (yFirst >= yLast || srcHeight <= 0 || srcWidth <= 0) return;

    for (int y = yFirst; y < yLast; ++y)
    {
        uint16_t* trgLine = reinterpret_cast<uint16_t*>(reinterpret_cast<char*>(trg) + y * trgPitch);
        const int ySrc = srcHeight * y / trgHeight;
        const uint32_t* srcLine = src + ySrc * srcWidth;
        for (int x = 0; x < trgWidth; ++x)
        {
            const int xSrc = srcWidth * x / trgWidth;
            trgLine[x] = CONVERT_32_TO_16(srcLine[xSrc]);
        }
    }
}


void xBRZ(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, int scalingFactor)
{

    std::vector<uint32_t> renderBuffer; //raw image
    std::vector<uint32_t> xbrzBuffer;   //scaled image

    if (width  <= 0 || height <= 0)
        return;

    renderBuffer.resize(width * height);

    xbrzBuffer.resize(renderBuffer.size() * scalingFactor * scalingFactor );

    int trgWidth  = width  * scalingFactor;
    int trgHeight = height * scalingFactor;

    copyImage16To32(reinterpret_cast<const uint16_t*>(srcPtr), width, height, srcPitch,
        &renderBuffer[0], 0, height);

    xbrz::scale(scalingFactor, &renderBuffer[0], &xbrzBuffer[0], width, height, xbrz::ColorFormat::RGB, xbrz::ScalerCfg(), 0, height);

    stretchImage32To16(&xbrzBuffer[0], width * scalingFactor, height * scalingFactor,
                       reinterpret_cast<uint16_t*>(dstPtr), trgWidth, trgHeight, dstPitch, 0, height * scalingFactor);

}

void filter_2xBRZ(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
    xBRZ(srcPtr, srcPitch, dstPtr, dstPitch, width, height, 2);
}

void filter_3xBRZ(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
    xBRZ(srcPtr, srcPitch, dstPtr, dstPitch, width, height, 3);
}

void filter_4xBRZ(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
    xBRZ(srcPtr, srcPitch, dstPtr, dstPitch, width, height, 4);
}


