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
#include "2xsai.h"

#define ALL_COLOR_MASK	(FIRST_COLOR_MASK | SECOND_COLOR_MASK | THIRD_COLOR_MASK)

#ifdef GFX_MULTI_FORMAT
static uint32	colorMask = 0, qcolorMask = 0, lowPixelMask = 0, qlowpixelMask = 0;
#else
#define colorMask		(((~RGB_LOW_BITS_MASK & ALL_COLOR_MASK) << 16) | (~RGB_LOW_BITS_MASK & ALL_COLOR_MASK))
#define qcolorMask		(((~TWO_LOW_BITS_MASK & ALL_COLOR_MASK) << 16) | (~TWO_LOW_BITS_MASK & ALL_COLOR_MASK))
#define lowPixelMask	((RGB_LOW_BITS_MASK << 16) | RGB_LOW_BITS_MASK)
#define qlowpixelMask	((TWO_LOW_BITS_MASK << 16) | TWO_LOW_BITS_MASK)
#endif

static inline int GetResult (uint32, uint32, uint32, uint32);
static inline int GetResult1 (uint32, uint32, uint32, uint32, uint32);
static inline int GetResult2 (uint32, uint32, uint32, uint32, uint32);
static inline uint32 INTERPOLATE (uint32, uint32);
static inline uint32 Q_INTERPOLATE (uint32, uint32, uint32, uint32);


static inline int GetResult (uint32 A, uint32 B, uint32 C, uint32 D)
{
	int	x = 0, y = 0, r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1) r += 1;
	if (y <= 1) r -= 1;

	return (r);
}

static inline int GetResult1 (uint32 A, uint32 B, uint32 C, uint32 D, uint32 E)
{
	int	x = 0, y = 0, r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1) r += 1;
	if (y <= 1) r -= 1;

	return (r);
}

static inline int GetResult2 (uint32 A, uint32 B, uint32 C, uint32 D, uint32 E)
{
	int	x = 0, y = 0, r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1) r -= 1;
	if (y <= 1) r += 1;

	return (r);
}

static inline uint32 INTERPOLATE (uint32 A, uint32 B)
{
	return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask));
}

static inline uint32 Q_INTERPOLATE (uint32 A, uint32 B, uint32 C, uint32 D)
{
	uint32	x = ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
	uint32	y = (A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);

	y = (y >> 2) & qlowpixelMask;

	return (x + y);
}

bool8 S9xBlit2xSaIFilterInit (void)
{
#ifdef GFX_MULTI_FORMAT
	colorMask     = ((~RGB_LOW_BITS_MASK & ALL_COLOR_MASK) << 16) | (~RGB_LOW_BITS_MASK & ALL_COLOR_MASK);
	qcolorMask    = ((~TWO_LOW_BITS_MASK & ALL_COLOR_MASK) << 16) | (~TWO_LOW_BITS_MASK & ALL_COLOR_MASK);
	lowPixelMask  = (RGB_LOW_BITS_MASK << 16) | RGB_LOW_BITS_MASK;
	qlowpixelMask = (TWO_LOW_BITS_MASK << 16) | TWO_LOW_BITS_MASK;
#endif

	return (TRUE);
}

void S9xBlit2xSaIFilterDeinit (void)
{
	return;
}

void SuperEagle (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
    uint16	*bP;
    uint32	*dP;
	uint32	nextline = srcRowBytes >> 1;

	for (; height; height--)
	{
	    bP = (uint16 *) srcPtr;
	    dP = (uint32 *) dstPtr;

		for (int i = 0; i < width; i++)
		{
           	uint32	color1, color2, color3, color4, color5, color6;
           	uint32	colorA0, colorA1, colorA2, colorA3, colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
           	uint32	product1a, product1b, product2a, product2b;

			colorB0 = *(bP - nextline - 1);
			colorB1 = *(bP - nextline    );
			colorB2 = *(bP - nextline + 1);
			colorB3 = *(bP - nextline + 2);

			color4  = *(bP - 1);
			color5  = *(bP    );
			color6  = *(bP + 1);
			colorS2 = *(bP + 2);

			color1  = *(bP + nextline - 1);
			color2  = *(bP + nextline    );
			color3  = *(bP + nextline + 1);
			colorS1 = *(bP + nextline + 2);

			colorA0 = *(bP + nextline + nextline - 1);
			colorA1 = *(bP + nextline + nextline    );
			colorA2 = *(bP + nextline + nextline + 1);
			colorA3 = *(bP + nextline + nextline + 2);

			if (color2 == color6 && color5 != color3)
			{
				product1b = product2a = color2;
				if ((color1 == color2 && color6 == colorS2) || (color2 == colorA1 && color6 == colorB2))
				{
					product1a = INTERPOLATE(color2, color5);
					product1a = INTERPOLATE(color2, product1a);
					product2b = INTERPOLATE(color2, color3);
					product2b = INTERPOLATE(color2, product2b);
				}
				else
				{
					product1a = INTERPOLATE(color5, color6);
					product2b = INTERPOLATE(color2, color3);
				}
			}
			else
			if (color5 == color3 && color2 != color6)
			{
				product2b = product1a = color5;
				if ((colorB1 == color5 && color3 == colorA2) || (color4 == color5 && color3 == colorS1))
				{
					product1b = INTERPOLATE(color5, color6);
					product1b = INTERPOLATE(color5, product1b);
					product2a = INTERPOLATE(color5, color2);
					product2a = INTERPOLATE(color5, product2a);
				}
				else
				{
					product1b = INTERPOLATE(color5, color6);
					product2a = INTERPOLATE(color2, color3);
				}
			}
			else
			if (color5 == color3 && color2 == color6 && color5 != color6)
			{
				int	r = 0;

				r += GetResult(color6, color5, color1,  colorA1);
				r += GetResult(color6, color5, color4,  colorB1);
				r += GetResult(color6, color5, colorA2, colorS1);
				r += GetResult(color6, color5, colorB2, colorS2);

				if (r > 0)
				{
					product1b = product2a = color2;
					product1a = product2b = INTERPOLATE(color5, color6);
				}
				else
				if (r < 0)
				{
					product2b = product1a = color5;
					product1b = product2a = INTERPOLATE(color5, color6);
				}
				else
				{
					product2b = product1a = color5;
					product1b = product2a = color2;
				}
			}
			else
			{
				if ((color2 == color5) || (color3 == color6))
				{
					product1a = color5;
					product2a = color2;
					product1b = color6;
					product2b = color3;
				}
				else
				{
					product1b = product1a = INTERPOLATE(color5, color6);
					product1a = INTERPOLATE(color5, product1a);
					product1b = INTERPOLATE(color6, product1b);

					product2a = product2b = INTERPOLATE(color2, color3);
					product2a = INTERPOLATE(color2, product2a);
					product2b = INTERPOLATE(color3, product2b);
				}
			}

		#ifdef MSB_FIRST
			product1a = (product1a << 16) | product1b;
			product2a = (product2a << 16) | product2b;
		#else
			product1a = product1a | (product1b << 16);
			product2a = product2a | (product2b << 16);
		#endif

			*(dP) = product1a;
			*(dP + (dstRowBytes >> 2)) = product2a;

			bP++;
			dP++;
		}

	    dstPtr += dstRowBytes << 1;
		srcPtr += srcRowBytes;
 	}
}

void _2xSaI (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	uint16 	*bP;
	uint32	*dP;
	uint32	nextline  = srcRowBytes >> 1;

	for (; height; height--)
	{
	    bP = (uint16 *) srcPtr;
	    dP = (uint32 *) dstPtr;

		for (int i = 0; i < width; i++)
		{
			uint32	colorA, colorB, colorC, colorD, colorE, colorF, colorG, colorH, colorI, colorJ, colorK, colorL, colorM, colorN, colorO, colorP;
			uint32	product, product1, product2;

			colorI = *(bP - nextline - 1);
			colorE = *(bP - nextline    );
			colorF = *(bP - nextline + 1);
			colorJ = *(bP - nextline + 2);

			colorG = *(bP - 1);
			colorA = *(bP    );
			colorB = *(bP + 1);
			colorK = *(bP + 2);

			colorH = *(bP + nextline - 1);
			colorC = *(bP + nextline    );
			colorD = *(bP + nextline + 1);
			colorL = *(bP + nextline + 2);

			colorM = *(bP + nextline + nextline - 1);
			colorN = *(bP + nextline + nextline    );
			colorO = *(bP + nextline + nextline + 1);
			colorP = *(bP + nextline + nextline + 2);

			if ((colorA == colorD) && (colorB != colorC))
			{
				if (((colorA == colorE) && (colorB == colorL)) || ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ)))
					product = colorA;
				else
					product = INTERPOLATE(colorA, colorB);

				if (((colorA == colorG) && (colorC == colorO)) || ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM)))
					product1 = colorA;
				else
					product1 = INTERPOLATE(colorA, colorC);

				product2 = colorA;
			}
			else
			if ((colorB == colorC) && (colorA != colorD))
			{
				if (((colorB == colorF) && (colorA == colorH)) || ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)))
					product = colorB;
				else
					product = INTERPOLATE(colorA, colorB);

				if (((colorC == colorH) && (colorA == colorF)) || ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)))
					product1 = colorC;
				else
					product1 = INTERPOLATE(colorA, colorC);

				product2 = colorB;
			}
			else
			if ((colorA == colorD) && (colorB == colorC))
			{
				if (colorA == colorB)
				{
					product  = colorA;
					product1 = colorA;
					product2 = colorA;
				}
				else
				{
					int	r = 0;

					product1 = INTERPOLATE(colorA, colorC);
					product  = INTERPOLATE(colorA, colorB);

					r += GetResult1(colorA, colorB, colorG, colorE, colorI);
					r += GetResult2(colorB, colorA, colorK, colorF, colorJ);
					r += GetResult2(colorB, colorA, colorH, colorN, colorM);
					r += GetResult1(colorA, colorB, colorL, colorO, colorP);

					if (r > 0)
						product2 = colorA;
					else
					if (r < 0)
						product2 = colorB;
					else
						product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
				}
			}
			else
			{
				product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);

				if ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))
					product  = colorA;
				else
				if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))
					product  = colorB;
				else
					product = INTERPOLATE(colorA, colorB);

				if ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM))
					product1 = colorA;
				else
				if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))
					product1 = colorC;
				else
					product1 = INTERPOLATE(colorA, colorC);
			}

		#ifdef MSB_FIRST
			product  = (colorA   << 16) | product;
			product1 = (product1 << 16) | product2;
		#else
			product  = colorA   | (product  << 16);
			product1 = product1 | (product2 << 16);
		#endif

			*(dP) = product;
			*(dP + (dstRowBytes >> 2)) = product1;

			bP++;
			dP++;
		}

	    dstPtr += dstRowBytes << 1;
		srcPtr += srcRowBytes;
    }
}

void Super2xSaI (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
    uint16	*bP;
	uint32	*dP;
	uint32	nextline = srcRowBytes >> 1;

	for (; height; height--)
	{
		bP = (uint16 *) srcPtr;
	    dP = (uint32 *) dstPtr;

		for (int i = 0; i < width; i++)
		{
           	uint32	color1, color2, color3, color4, color5, color6;
           	uint32	colorA0, colorA1, colorA2, colorA3, colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
           	uint32	product1a, product1b, product2a, product2b;

			colorB0 = *(bP - nextline - 1);
			colorB1 = *(bP - nextline    );
			colorB2 = *(bP - nextline + 1);
			colorB3 = *(bP - nextline + 2);

			color4  = *(bP - 1);
			color5  = *(bP    );
			color6  = *(bP + 1);
			colorS2 = *(bP + 2);

			color1  = *(bP + nextline - 1);
			color2  = *(bP + nextline    );
			color3  = *(bP + nextline + 1);
			colorS1 = *(bP + nextline + 2);

			colorA0 = *(bP + nextline + nextline - 1);
			colorA1 = *(bP + nextline + nextline    );
			colorA2 = *(bP + nextline + nextline + 1);
			colorA3 = *(bP + nextline + nextline + 2);

			if (color2 == color6 && color5 != color3)
            	product2b = product1b = color2;
			else
			if (color5 == color3 && color2 != color6)
                product2b = product1b = color5;
			else
			if (color5 == color3 && color2 == color6 && color5 != color6)
			{
				int	r = 0;

				r += GetResult(color6, color5, color1,  colorA1);
				r += GetResult(color6, color5, color4,  colorB1);
				r += GetResult(color6, color5, colorA2, colorS1);
				r += GetResult(color6, color5, colorB2, colorS2);

				if (r > 0)
					product2b = product1b = color6;
				else
				if (r < 0)
					product2b = product1b = color5;
				else
					product2b = product1b = INTERPOLATE(color5, color6);
			}
			else
			{
				if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
					product2b = Q_INTERPOLATE(color3, color3, color3, color2);
				else
				if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
					product2b = Q_INTERPOLATE(color2, color2, color2, color3);
				else
					product2b = INTERPOLATE(color2, color3);

				if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
					product1b = Q_INTERPOLATE(color6, color6, color6, color5);
				else
				if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
					product1b = Q_INTERPOLATE(color6, color5, color5, color5);
				else
					product1b = INTERPOLATE (color5, color6);
			}

			if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
				product2a = INTERPOLATE(color2, color5);
			else
			if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
				product2a = INTERPOLATE(color2, color5);
			else
				product2a = color2;

			if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
				product1a = INTERPOLATE(color2, color5);
			else
			if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
				product1a = INTERPOLATE(color2, color5);
			else
				product1a = color5;

		#ifdef MSB_FIRST
			product1a = (product1a << 16) | product1b;
			product2a = (product2a << 16) | product2b;
		#else
			product1a = product1a | (product1b << 16);
			product2a = product2a | (product2b << 16);
		#endif

			*(dP) = product1a;
			*(dP +(dstRowBytes >> 2)) = product2a;

			bP++;
			dP++;
		}

	    dstPtr += dstRowBytes << 1;
		srcPtr += srcRowBytes;
	}
}
