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
#include "blit.h"

#define ALL_COLOR_MASK	(FIRST_COLOR_MASK | SECOND_COLOR_MASK | THIRD_COLOR_MASK)

#ifdef GFX_MULTI_FORMAT
static uint16	lowPixelMask = 0, qlowPixelMask = 0, highBitsMask = 0;
static uint32	colorMask = 0;
#else
#define lowPixelMask	(RGB_LOW_BITS_MASK)
#define qlowPixelMask	((RGB_HI_BITS_MASK >> 3) | TWO_LOW_BITS_MASK)
#define highBitsMask	(ALL_COLOR_MASK & RGB_REMOVE_LOW_BITS_MASK)
#define colorMask		(((~RGB_HI_BITS_MASK & ALL_COLOR_MASK) << 16) | (~RGB_HI_BITS_MASK & ALL_COLOR_MASK))
#endif

static snes_ntsc_t	*ntsc   = NULL;
static uint8		*XDelta = NULL;


bool8 S9xBlitFilterInit (void)
{
	XDelta = new uint8[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 4];
	if (!XDelta)
		return (FALSE);

	S9xBlitClearDelta();

#ifdef GFX_MULTI_FORMAT
	lowPixelMask  = RGB_LOW_BITS_MASK;
	qlowPixelMask = (RGB_HI_BITS_MASK >> 3) | TWO_LOW_BITS_MASK;
	highBitsMask  = ALL_COLOR_MASK & RGB_REMOVE_LOW_BITS_MASK;
	colorMask     = ((~RGB_HI_BITS_MASK & ALL_COLOR_MASK) << 16) | (~RGB_HI_BITS_MASK & ALL_COLOR_MASK);
#endif

	return (TRUE);
}

void S9xBlitFilterDeinit (void)
{
	if (XDelta)
	{
		delete[] XDelta;
		XDelta = NULL;
	}
}

void S9xBlitClearDelta (void)
{
	uint32	*d = (uint32 *) XDelta;

	for (int y = 0; y < SNES_HEIGHT_EXTENDED; y++)
		for (int x = 0; x < SNES_WIDTH; x++)
			*d++ = 0x80008000;
}

bool8 S9xBlitNTSCFilterInit (void)
{
	ntsc = (snes_ntsc_t *) malloc(sizeof(snes_ntsc_t));
	if (!ntsc)
		return (FALSE);

	snes_ntsc_init(ntsc, &snes_ntsc_composite);
	return (TRUE);
}

void S9xBlitNTSCFilterDeinit (void)
{
	if (ntsc)
	{
		free(ntsc);
		ntsc = NULL;
	}
}

void S9xBlitNTSCFilterSet (const snes_ntsc_setup_t *setup)
{
	snes_ntsc_init(ntsc, setup);
}

void S9xBlitPixSimple1x1 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	width <<= 1;

	for (; height; height--)
	{
		memcpy(dstPtr, srcPtr, width);
		srcPtr += srcRowBytes;
		dstPtr += dstRowBytes;
	}
}

void S9xBlitPixSimple1x2 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	width <<= 1;

	for (; height; height--)
	{
		memcpy(dstPtr, srcPtr, width);
		dstPtr += dstRowBytes;
		memcpy(dstPtr, srcPtr, width);
		srcPtr += srcRowBytes;
		dstPtr += dstRowBytes;
	}
}

void S9xBlitPixSimple2x1 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	for (; height; height--)
	{
		uint16	*dP = (uint16 *) dstPtr, *bP = (uint16 *) srcPtr;

		for (int i = 0; i < (width >> 1); i++)
		{
			*dP++ = *bP;
			*dP++ = *bP++;

			*dP++ = *bP;
			*dP++ = *bP++;
		}

		srcPtr += srcRowBytes;
		dstPtr += dstRowBytes;
	}
}

void S9xBlitPixSimple2x2 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	uint8	*dstPtr2 = dstPtr + dstRowBytes, *deltaPtr = XDelta;
	dstRowBytes <<= 1;

	for (; height; height--)
	{
		uint32	*dP1 = (uint32 *) dstPtr, *dP2 = (uint32 *) dstPtr2, *bP = (uint32 *) srcPtr, *xP = (uint32 *) deltaPtr;
		uint32	currentPixel, lastPixel, currentPixA, currentPixB, colorA, colorB;

		for (int i = 0; i < (width >> 1); i++)
		{
			currentPixel = *bP;
			lastPixel    = *xP;

			if (currentPixel != lastPixel)
			{
			#ifdef MSB_FIRST
				colorA = (currentPixel >> 16) & 0xFFFF;
				colorB = (currentPixel      ) & 0xFFFF;
			#else
				colorA = (currentPixel      ) & 0xFFFF;
				colorB = (currentPixel >> 16) & 0xFFFF;
			#endif

				currentPixA = (colorA << 16) | colorA;
				currentPixB = (colorB << 16) | colorB;

				dP1[0] = currentPixA;
				dP1[1] = currentPixB;
				dP2[0] = currentPixA;
				dP2[1] = currentPixB;

				*xP = *bP;
			}

			bP++;
			xP++;
			dP1 += 2;
			dP2 += 2;
		}

		srcPtr   += srcRowBytes;
		deltaPtr += srcRowBytes;
		dstPtr   += dstRowBytes;
		dstPtr2  += dstRowBytes;
	}
}

void S9xBlitPixBlend1x1 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	for (; height; height--)
	{
		uint16	*dP = (uint16 *) dstPtr, *bP = (uint16 *) srcPtr;
		uint16	prev, curr;

		prev = *bP;

		for (int i = 0; i < (width >> 1); i++)
		{
			curr  = *bP++;
			*dP++ = (prev & curr) + (((prev ^ curr) & highBitsMask) >> 1);
			prev  = curr;

			curr  = *bP++;
			*dP++ = (prev & curr) + (((prev ^ curr) & highBitsMask) >> 1);
			prev  = curr;
		}

		srcPtr += srcRowBytes;
		dstPtr += dstRowBytes;
	}
}

void S9xBlitPixBlend2x1 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	for (; height; height--)
	{
		uint16	*dP = (uint16 *) dstPtr, *bP = (uint16 *) srcPtr;
		uint16	prev, curr;

		prev = *bP;

		for (int i = 0; i < (width >> 1); i++)
		{
			curr  = *bP++;
			*dP++ = (prev & curr) + (((prev ^ curr) & highBitsMask) >> 1);
			*dP++ = curr;
			prev  = curr;

			curr  = *bP++;
			*dP++ = (prev & curr) + (((prev ^ curr) & highBitsMask) >> 1);
			*dP++ = curr;
			prev  = curr;
		}

		srcPtr += srcRowBytes;
		dstPtr += dstRowBytes;
	}
}

void S9xBlitPixTV1x2 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	uint8	*dstPtr2 = dstPtr + dstRowBytes;
	dstRowBytes <<= 1;

	for (; height; height--)
	{
		uint32	*dP1 = (uint32 *) dstPtr, *dP2 = (uint32 *) dstPtr2, *bP = (uint32 *) srcPtr;
		uint32	product, darkened;

		for (int i = 0; i < (width >> 1); i++)
		{
			product = *dP1++ = *bP++;
			darkened  = (product = (product >> 1) & colorMask);
			darkened += (product = (product >> 1) & colorMask);
			*dP2++  = darkened;
		}

		srcPtr  += srcRowBytes;
		dstPtr  += dstRowBytes;
		dstPtr2 += dstRowBytes;
	}
}

void S9xBlitPixTV2x2 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	uint8	*dstPtr2 = dstPtr + dstRowBytes, *deltaPtr = XDelta;
	dstRowBytes <<= 1;

	for (; height; height--)
	{
		uint32	*dP1 = (uint32 *) dstPtr, *dP2 = (uint32 *) dstPtr2, *bP = (uint32 *) srcPtr, *xP = (uint32 *) deltaPtr;
		uint32	currentPixel, nextPixel, currentDelta, nextDelta, colorA, colorB, product, darkened;

		for (int i = 0; i < (width >> 1) - 1; i++)
		{
			currentPixel = *bP;
			currentDelta = *xP;
			nextPixel    = *(bP + 1);
			nextDelta    = *(xP + 1);

			if ((currentPixel != currentDelta) || (nextPixel != nextDelta))
			{
				*xP = *bP;

			#ifdef MSB_FIRST
				colorA = (currentPixel >> 16) & 0xFFFF;
				colorB = (currentPixel      ) & 0xFFFF;
			#else
				colorA = (currentPixel      ) & 0xFFFF;
				colorB = (currentPixel >> 16) & 0xFFFF;
			#endif

			#ifdef MSB_FIRST
				*dP1       = product = (colorA << 16) | ((((colorA >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorA & colorB & lowPixelMask))      );
			#else
				*dP1       = product = (colorA      ) | ((((colorA >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorA & colorB & lowPixelMask)) << 16);
			#endif

				darkened  = (product = ((product >> 1) & colorMask));
				darkened += (product = ((product >> 1) & colorMask));
				darkened +=             (product >> 1) & colorMask;

				*dP2       = darkened;

			#ifdef MSB_FIRST
				colorA = (nextPixel    >> 16) & 0xFFFF;
			#else
				colorA = (nextPixel         ) & 0xFFFF;
			#endif

			#ifdef MSB_FIRST
				*(dP1 + 1) = product = (colorB << 16) | ((((colorA >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorA & colorB & lowPixelMask))      );
			#else
				*(dP1 + 1) = product = (colorB      ) | ((((colorA >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorA & colorB & lowPixelMask)) << 16);
			#endif

				darkened  = (product = ((product >> 1) & colorMask));
				darkened += (product = ((product >> 1) & colorMask));
				darkened +=             (product >> 1) & colorMask;

				*(dP2 + 1) = darkened;
			}

			bP++;
			xP++;
			dP1 += 2;
			dP2 += 2;
		}

		// Last 2 Pixels

		currentPixel = *bP;
		currentDelta = *xP;

		if (currentPixel != currentDelta)
		{
			*xP = *bP;

		#ifdef MSB_FIRST
			colorA = (currentPixel >> 16) & 0xFFFF;
			colorB = (currentPixel      ) & 0xFFFF;
		#else
			colorA = (currentPixel      ) & 0xFFFF;
			colorB = (currentPixel >> 16) & 0xFFFF;
		#endif

		#ifdef MSB_FIRST
			*dP1       = product = (colorA << 16) | ((((colorA >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorA & colorB & lowPixelMask))      );
		#else
			*dP1       = product = (colorA      ) | ((((colorA >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorA & colorB & lowPixelMask)) << 16);
		#endif

			darkened  = (product = ((product >> 1) & colorMask));
			darkened += (product = ((product >> 1) & colorMask));
			darkened +=             (product >> 1) & colorMask;

			*dP2       = darkened;

			*(dP1 + 1) = product = (colorB << 16) | colorB;

			darkened  = (product = ((product >> 1) & colorMask));
			darkened += (product = ((product >> 1) & colorMask));
			darkened +=             (product >> 1) & colorMask;

			*(dP2 + 1) = darkened;
		}

		srcPtr   += srcRowBytes;
		deltaPtr += srcRowBytes;
		dstPtr   += dstRowBytes;
		dstPtr2  += dstRowBytes;
	}
}

void S9xBlitPixMixedTV1x2 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	uint8	*dstPtr2 = dstPtr + dstRowBytes, *srcPtr2 = srcPtr + srcRowBytes;
	dstRowBytes <<= 1;

	for (; height > 1; height--)
	{
		uint16	*dP1 = (uint16 *) dstPtr, *dP2 = (uint16 *) dstPtr2, *bP1 = (uint16 *) srcPtr, *bP2 = (uint16 *) srcPtr2;
		uint16	prev, next, mixed;

		for (int i = 0; i < width; i++)
		{
			prev  = *bP1++;
			next  = *bP2++;
			mixed = prev + next + ((prev ^ next) & lowPixelMask);

			*dP1++ = prev;
			*dP2++ = (mixed >> 1) - (mixed >> 4 & qlowPixelMask);
		}

		srcPtr  += srcRowBytes;
		srcPtr2 += srcRowBytes;
		dstPtr  += dstRowBytes;
		dstPtr2 += dstRowBytes;
	}

	// Last 1 line

	uint16	*dP1 = (uint16 *) dstPtr, *dP2 = (uint16 *) dstPtr2, *bP1 = (uint16 *) srcPtr;
	uint16	prev, mixed;

	for (int i = 0; i < width; i++)
	{
		prev = *bP1++;
		mixed = prev + ((prev ^ 0) & lowPixelMask);

		*dP1++ = prev;
		*dP2++ = (mixed >> 1) - (mixed >> 4 & qlowPixelMask);
	}
}

void S9xBlitPixSmooth2x2 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	uint8	*dstPtr2 = dstPtr + dstRowBytes, *deltaPtr = XDelta;
	uint32	lastLinePix[SNES_WIDTH << 1];
	uint8	lastLineChg[SNES_WIDTH >> 1];
	int		lineBytes = width << 1;

	dstRowBytes <<= 1;

	memset(lastLinePix, 0, sizeof(lastLinePix));
	memset(lastLineChg, 0, sizeof(lastLineChg));

	for (; height; height--)
	{
		uint32	*dP1 = (uint32 *) dstPtr, *dP2 = (uint32 *) dstPtr2, *bP = (uint32 *) srcPtr, *xP = (uint32 *) deltaPtr;
		uint32	*lL = lastLinePix;
		uint8	*lC = lastLineChg;
		uint32	currentPixel, nextPixel, currentDelta, nextDelta, lastPix, lastChg, thisChg, currentPixA, currentPixB, colorA, colorB, colorC;
		uint16	savePixel;

		savePixel = *(uint16 *) (srcPtr + lineBytes);
		*(uint16 *) (srcPtr   + lineBytes) = *(uint16 *) (srcPtr + lineBytes - 2);
		*(uint32 *) (deltaPtr + lineBytes) = *(uint32 *) (srcPtr + lineBytes);

		nextPixel = *bP++;
		nextDelta = *xP++;

		for (int i = 0; i < (width >> 1); i++)
		{
			currentPixel = nextPixel;
			currentDelta = nextDelta;
			nextPixel    = *bP++;
			nextDelta    = *xP++;
			lastChg      = *lC;
			thisChg      = (nextPixel - nextDelta) | (currentPixel - currentDelta);

		#ifdef MSB_FIRST
			colorA = (currentPixel >> 16) & 0xFFFF;
			colorB = (currentPixel      ) & 0xFFFF;
			colorC = (nextPixel    >> 16) & 0xFFFF;

			currentPixA = (colorA << 16) | ((((colorA >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorA & colorB & lowPixelMask))      );
			currentPixB = (colorB << 16) | ((((colorC >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorC & colorB & lowPixelMask))      );
		#else
			colorA = (currentPixel      ) & 0xFFFF;
			colorB = (currentPixel >> 16) & 0xFFFF;
			colorC = (nextPixel         ) & 0xFFFF;

			currentPixA = (colorA      ) | ((((colorA >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorA & colorB & lowPixelMask)) << 16);
			currentPixB = (colorB      ) | ((((colorC >> 1) & colorMask) + ((colorB >> 1) & colorMask) + (colorC & colorB & lowPixelMask)) << 16);
		#endif

			if (thisChg | lastChg)
			{
				xP[-2]  = currentPixel;

				lastPix = lL[0];
				dP1[0]  = ((currentPixA >> 1) & colorMask) + ((lastPix >> 1) & colorMask) + (currentPixA & lastPix & lowPixelMask);
				dP2[0]  = currentPixA;
				lL[0]   = currentPixA;

				lastPix = lL[1];
				dP1[1]  = ((currentPixB >> 1) & colorMask) + ((lastPix >> 1) & colorMask) + (currentPixB & lastPix & lowPixelMask);
				dP2[1]  = currentPixB;
				lL[1]   = currentPixB;

				*lC++   = (thisChg != 0);
			}
			else
			{
				lL[0]   = currentPixA;
				lL[1]   = currentPixB;
				*lC++   = 0;
			}

			lL  += 2;
			dP2 += 2;
			dP1 += 2;
		}

		*(uint16 *) (srcPtr + lineBytes) = savePixel;

		srcPtr   += srcRowBytes;
		deltaPtr += srcRowBytes;
		dstPtr   += dstRowBytes;
		dstPtr2  += dstRowBytes;
	}
}

void S9xBlitPixSuper2xSaI16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	Super2xSaI(srcPtr, srcRowBytes, dstPtr, dstRowBytes, width, height);
}

void S9xBlitPix2xSaI16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	_2xSaI(srcPtr, srcRowBytes, dstPtr, dstRowBytes, width, height);
}

void S9xBlitPixSuperEagle16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	SuperEagle(srcPtr, srcRowBytes, dstPtr, dstRowBytes, width, height);
}

void S9xBlitPixEPX16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	EPX_16(srcPtr, srcRowBytes, dstPtr, dstRowBytes, width, height);
}

void S9xBlitPixHQ2x16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	HQ2X_16(srcPtr, srcRowBytes, dstPtr, dstRowBytes, width, height);
}

void S9xBlitPixHQ3x16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	HQ3X_16(srcPtr, srcRowBytes, dstPtr, dstRowBytes, width, height);
}

void S9xBlitPixHQ4x16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	HQ4X_16(srcPtr, srcRowBytes, dstPtr, dstRowBytes, width, height);
}

void S9xBlitPixNTSC16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	snes_ntsc_blit(ntsc, (SNES_NTSC_IN_T const *) srcPtr, srcRowBytes >> 1, 0, width, height, dstPtr, dstRowBytes);
}

void S9xBlitPixHiResNTSC16 (uint8 *srcPtr, int srcRowBytes, uint8 *dstPtr, int dstRowBytes, int width, int height)
{
	snes_ntsc_blit_hires(ntsc, (SNES_NTSC_IN_T const *) srcPtr, srcRowBytes >> 1, 0, width, height, dstPtr, dstRowBytes);
}
