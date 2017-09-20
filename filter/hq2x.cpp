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
#include "gfx.h"
#include "hq2x.h"

#define	Ymask	0xFF0000
#define	Umask	0x00FF00
#define	Vmask	0x0000FF
#define	trY		0x300000
#define	trU		0x000700
#define	trV		0x000006

#ifdef GFX_MULTI_FORMAT
static uint16	Mask_2 = 0, Mask13 = 0;
#else
#define	Mask_2	SECOND_COLOR_MASK
#define	Mask13	FIRST_THIRD_COLOR_MASK
#endif

#define Interp01(c1, c2) \
(c1 == c2) ? c1 : \
(((((c1 & Mask_2) *  3 + (c2 & Mask_2)    ) >> 2) & Mask_2) + \
 ((((c1 & Mask13) *  3 + (c2 & Mask13)    ) >> 2) & Mask13))

#define Interp02(c1, c2, c3) \
(((((c1 & Mask_2) *  2 + (c2 & Mask_2)     + (c3 & Mask_2)    ) >> 2) & Mask_2) + \
 ((((c1 & Mask13) *  2 + (c2 & Mask13)     + (c3 & Mask13)    ) >> 2) & Mask13))

#define Interp03(c1, c2) \
(c1 == c2) ? c1 : \
(((((c1 & Mask_2) *  7 + (c2 & Mask_2)    ) >> 3) & Mask_2) + \
 ((((c1 & Mask13) *  7 + (c2 & Mask13)    ) >> 3) & Mask13))

#define Interp04(c1, c2, c3) \
(((((c1 & Mask_2) *  2 + (c2 & Mask_2) * 7 + (c3 & Mask_2) * 7) >> 4) & Mask_2) + \
 ((((c1 & Mask13) *  2 + (c2 & Mask13) * 7 + (c3 & Mask13) * 7) >> 4) & Mask13))

#define Interp05(c1, c2) \
(c1 == c2) ? c1 : \
(((((c1 & Mask_2)      + (c2 & Mask_2))     >> 1) & Mask_2) + \
 ((((c1 & Mask13)      + (c2 & Mask13))     >> 1) & Mask13))

#define Interp06(c1, c2, c3) \
(((((c1 & Mask_2) *  5 + (c2 & Mask_2) * 2 + (c3 & Mask_2)    ) >> 3) & Mask_2) + \
 ((((c1 & Mask13) *  5 + (c2 & Mask13) * 2 + (c3 & Mask13)    ) >> 3) & Mask13))

#define Interp07(c1, c2, c3) \
(((((c1 & Mask_2) *  6 + (c2 & Mask_2)     + (c3 & Mask_2)    ) >> 3) & Mask_2) + \
 ((((c1 & Mask13) *  6 + (c2 & Mask13)     + (c3 & Mask13)    ) >> 3) & Mask13))

#define Interp08(c1, c2) \
(c1 == c2) ? c1 : \
(((((c1 & Mask_2) *  5 + (c2 & Mask_2) * 3) >> 3) & Mask_2) + \
 ((((c1 & Mask13) *  5 + (c2 & Mask13) * 3) >> 3) & Mask13))

#define Interp09(c1, c2, c3) \
(((((c1 & Mask_2) *  2 + (c2 & Mask_2) * 3 + (c3 & Mask_2) * 3) >> 3) & Mask_2) + \
 ((((c1 & Mask13) *  2 + (c2 & Mask13) * 3 + (c3 & Mask13) * 3) >> 3) & Mask13))

#define Interp10(c1, c2, c3) \
(((((c1 & Mask_2) * 14 + (c2 & Mask_2)     + (c3 & Mask_2)    ) >> 4) & Mask_2) + \
 ((((c1 & Mask13) * 14 + (c2 & Mask13)     + (c3 & Mask13)    ) >> 4) & Mask13))

#define X2PIXEL00_0		*(dp) = w5;
#define X2PIXEL00_10	*(dp) = Interp01(w5, w1);
#define X2PIXEL00_11	*(dp) = Interp01(w5, w4);
#define X2PIXEL00_12	*(dp) = Interp01(w5, w2);
#define X2PIXEL00_20	*(dp) = Interp02(w5, w4, w2);
#define X2PIXEL00_21	*(dp) = Interp02(w5, w1, w2);
#define X2PIXEL00_22	*(dp) = Interp02(w5, w1, w4);
#define X2PIXEL00_60	*(dp) = Interp06(w5, w2, w4);
#define X2PIXEL00_61	*(dp) = Interp06(w5, w4, w2);
#define X2PIXEL00_70	*(dp) = Interp07(w5, w4, w2);
#define X2PIXEL00_90	*(dp) = Interp09(w5, w4, w2);
#define X2PIXEL00_100	*(dp) = Interp10(w5, w4, w2);

#define X2PIXEL01_0		*(dp + 1) = w5;
#define X2PIXEL01_10	*(dp + 1) = Interp01(w5, w3);
#define X2PIXEL01_11	*(dp + 1) = Interp01(w5, w2);
#define X2PIXEL01_12	*(dp + 1) = Interp01(w5, w6);
#define X2PIXEL01_20	*(dp + 1) = Interp02(w5, w2, w6);
#define X2PIXEL01_21	*(dp + 1) = Interp02(w5, w3, w6);
#define X2PIXEL01_22	*(dp + 1) = Interp02(w5, w3, w2);
#define X2PIXEL01_60	*(dp + 1) = Interp06(w5, w6, w2);
#define X2PIXEL01_61	*(dp + 1) = Interp06(w5, w2, w6);
#define X2PIXEL01_70	*(dp + 1) = Interp07(w5, w2, w6);
#define X2PIXEL01_90	*(dp + 1) = Interp09(w5, w2, w6);
#define X2PIXEL01_100	*(dp + 1) = Interp10(w5, w2, w6);

#define X2PIXEL10_0		*(dp + dst1line) = w5;
#define X2PIXEL10_10	*(dp + dst1line) = Interp01(w5, w7);
#define X2PIXEL10_11	*(dp + dst1line) = Interp01(w5, w8);
#define X2PIXEL10_12	*(dp + dst1line) = Interp01(w5, w4);
#define X2PIXEL10_20	*(dp + dst1line) = Interp02(w5, w8, w4);
#define X2PIXEL10_21	*(dp + dst1line) = Interp02(w5, w7, w4);
#define X2PIXEL10_22	*(dp + dst1line) = Interp02(w5, w7, w8);
#define X2PIXEL10_60	*(dp + dst1line) = Interp06(w5, w4, w8);
#define X2PIXEL10_61	*(dp + dst1line) = Interp06(w5, w8, w4);
#define X2PIXEL10_70	*(dp + dst1line) = Interp07(w5, w8, w4);
#define X2PIXEL10_90	*(dp + dst1line) = Interp09(w5, w8, w4);
#define X2PIXEL10_100	*(dp + dst1line) = Interp10(w5, w8, w4);

#define X2PIXEL11_0		*(dp + dst1line + 1) = w5;
#define X2PIXEL11_10	*(dp + dst1line + 1) = Interp01(w5, w9);
#define X2PIXEL11_11	*(dp + dst1line + 1) = Interp01(w5, w6);
#define X2PIXEL11_12	*(dp + dst1line + 1) = Interp01(w5, w8);
#define X2PIXEL11_20	*(dp + dst1line + 1) = Interp02(w5, w6, w8);
#define X2PIXEL11_21	*(dp + dst1line + 1) = Interp02(w5, w9, w8);
#define X2PIXEL11_22	*(dp + dst1line + 1) = Interp02(w5, w9, w6);
#define X2PIXEL11_60	*(dp + dst1line + 1) = Interp06(w5, w8, w6);
#define X2PIXEL11_61	*(dp + dst1line + 1) = Interp06(w5, w6, w8);
#define X2PIXEL11_70	*(dp + dst1line + 1) = Interp07(w5, w6, w8);
#define X2PIXEL11_90	*(dp + dst1line + 1) = Interp09(w5, w6, w8);
#define X2PIXEL11_100	*(dp + dst1line + 1) = Interp10(w5, w6, w8);

#define X3PIXEL00_1M	*(dp) = Interp01(w5, w1);
#define X3PIXEL00_1U	*(dp) = Interp01(w5, w2);
#define X3PIXEL00_1L	*(dp) = Interp01(w5, w4);
#define X3PIXEL00_2		*(dp) = Interp02(w5, w4, w2);
#define X3PIXEL00_4		*(dp) = Interp04(w5, w4, w2);
#define X3PIXEL00_5		*(dp) = Interp05(w4, w2);
#define X3PIXEL00_C		*(dp) = w5;

#define X3PIXEL01_1		*(dp + 1) = Interp01(w5, w2);
#define X3PIXEL01_3		*(dp + 1) = Interp03(w5, w2);
#define X3PIXEL01_6		*(dp + 1) = Interp01(w2, w5);
#define X3PIXEL01_C		*(dp + 1) = w5;

#define X3PIXEL02_1M	*(dp + 2) = Interp01(w5, w3);
#define X3PIXEL02_1U	*(dp + 2) = Interp01(w5, w2);
#define X3PIXEL02_1R	*(dp + 2) = Interp01(w5, w6);
#define X3PIXEL02_2		*(dp + 2) = Interp02(w5, w2, w6);
#define X3PIXEL02_4		*(dp + 2) = Interp04(w5, w2, w6);
#define X3PIXEL02_5		*(dp + 2) = Interp05(w2, w6);
#define X3PIXEL02_C		*(dp + 2) = w5;

#define X3PIXEL10_1		*(dp + dst1line) = Interp01(w5, w4);
#define X3PIXEL10_3		*(dp + dst1line) = Interp03(w5, w4);
#define X3PIXEL10_6		*(dp + dst1line) = Interp01(w4, w5);
#define X3PIXEL10_C		*(dp + dst1line) = w5;

#define X3PIXEL11		*(dp + dst1line + 1) = w5;

#define X3PIXEL12_1		*(dp + dst1line + 2) = Interp01(w5, w6);
#define X3PIXEL12_3		*(dp + dst1line + 2) = Interp03(w5, w6);
#define X3PIXEL12_6		*(dp + dst1line + 2) = Interp01(w6, w5);
#define X3PIXEL12_C		*(dp + dst1line + 2) = w5;

#define X3PIXEL20_1M	*(dp + dst1line + dst1line) = Interp01(w5, w7);
#define X3PIXEL20_1D	*(dp + dst1line + dst1line) = Interp01(w5, w8);
#define X3PIXEL20_1L	*(dp + dst1line + dst1line) = Interp01(w5, w4);
#define X3PIXEL20_2		*(dp + dst1line + dst1line) = Interp02(w5, w8, w4);
#define X3PIXEL20_4		*(dp + dst1line + dst1line) = Interp04(w5, w8, w4);
#define X3PIXEL20_5		*(dp + dst1line + dst1line) = Interp05(w8, w4);
#define X3PIXEL20_C		*(dp + dst1line + dst1line) = w5;

#define X3PIXEL21_1		*(dp + dst1line + dst1line + 1) = Interp01(w5, w8);
#define X3PIXEL21_3		*(dp + dst1line + dst1line + 1) = Interp03(w5, w8);
#define X3PIXEL21_6		*(dp + dst1line + dst1line + 1) = Interp01(w8, w5);
#define X3PIXEL21_C		*(dp + dst1line + dst1line + 1) = w5;

#define X3PIXEL22_1M	*(dp + dst1line + dst1line + 2) = Interp01(w5, w9);
#define X3PIXEL22_1D	*(dp + dst1line + dst1line + 2) = Interp01(w5, w8);
#define X3PIXEL22_1R	*(dp + dst1line + dst1line + 2) = Interp01(w5, w6);
#define X3PIXEL22_2		*(dp + dst1line + dst1line + 2) = Interp02(w5, w6, w8);
#define X3PIXEL22_4		*(dp + dst1line + dst1line + 2) = Interp04(w5, w6, w8);
#define X3PIXEL22_5		*(dp + dst1line + dst1line + 2) = Interp05(w6, w8);
#define X3PIXEL22_C		*(dp + dst1line + dst1line + 2) = w5;

#define X4PIXEL00_0		*(dp) = w5;
#define X4PIXEL00_11	*(dp) = Interp01(w5, w4);
#define X4PIXEL00_12	*(dp) = Interp01(w5, w2);
#define X4PIXEL00_20	*(dp) = Interp02(w5, w2, w4);
#define X4PIXEL00_50	*(dp) = Interp05(w2, w4);
#define X4PIXEL00_80	*(dp) = Interp08(w5, w1);
#define X4PIXEL00_81	*(dp) = Interp08(w5, w4);
#define X4PIXEL00_82	*(dp) = Interp08(w5, w2);

#define X4PIXEL01_0		*(dp + 1) = w5;
#define X4PIXEL01_10	*(dp + 1) = Interp01(w5, w1);
#define X4PIXEL01_12	*(dp + 1) = Interp01(w5, w2);
#define X4PIXEL01_14	*(dp + 1) = Interp01(w2, w5);
#define X4PIXEL01_21	*(dp + 1) = Interp02(w2, w5, w4);
#define X4PIXEL01_31	*(dp + 1) = Interp03(w5, w4);
#define X4PIXEL01_50	*(dp + 1) = Interp05(w2, w5);
#define X4PIXEL01_60	*(dp + 1) = Interp06(w5, w2, w4);
#define X4PIXEL01_61	*(dp + 1) = Interp06(w5, w2, w1);
#define X4PIXEL01_82	*(dp + 1) = Interp08(w5, w2);
#define X4PIXEL01_83	*(dp + 1) = Interp08(w2, w4);

#define X4PIXEL02_0		*(dp + 2) = w5;
#define X4PIXEL02_10	*(dp + 2) = Interp01(w5, w3);
#define X4PIXEL02_11	*(dp + 2) = Interp01(w5, w2);
#define X4PIXEL02_13	*(dp + 2) = Interp01(w2, w5);
#define X4PIXEL02_21	*(dp + 2) = Interp02(w2, w5, w6);
#define X4PIXEL02_32	*(dp + 2) = Interp03(w5, w6);
#define X4PIXEL02_50	*(dp + 2) = Interp05(w2, w5);
#define X4PIXEL02_60	*(dp + 2) = Interp06(w5, w2, w6);
#define X4PIXEL02_61	*(dp + 2) = Interp06(w5, w2, w3);
#define X4PIXEL02_81	*(dp + 2) = Interp08(w5, w2);
#define X4PIXEL02_83	*(dp + 2) = Interp08(w2, w6);

#define X4PIXEL03_0		*(dp + 3) = w5;
#define X4PIXEL03_11	*(dp + 3) = Interp01(w5, w2);
#define X4PIXEL03_12	*(dp + 3) = Interp01(w5, w6);
#define X4PIXEL03_20	*(dp + 3) = Interp02(w5, w2, w6);
#define X4PIXEL03_50	*(dp + 3) = Interp05(w2, w6);
#define X4PIXEL03_80	*(dp + 3) = Interp08(w5, w3);
#define X4PIXEL03_81	*(dp + 3) = Interp08(w5, w2);
#define X4PIXEL03_82	*(dp + 3) = Interp08(w5, w6);

#define X4PIXEL10_0		*(dp + dst1line) = w5;
#define X4PIXEL10_10	*(dp + dst1line) = Interp01(w5, w1);
#define X4PIXEL10_11	*(dp + dst1line) = Interp01(w5, w4);
#define X4PIXEL10_13	*(dp + dst1line) = Interp01(w4, w5);
#define X4PIXEL10_21	*(dp + dst1line) = Interp02(w4, w5, w2);
#define X4PIXEL10_32	*(dp + dst1line) = Interp03(w5, w2);
#define X4PIXEL10_50	*(dp + dst1line) = Interp05(w4, w5);
#define X4PIXEL10_60	*(dp + dst1line) = Interp06(w5, w4, w2);
#define X4PIXEL10_61	*(dp + dst1line) = Interp06(w5, w4, w1);
#define X4PIXEL10_81	*(dp + dst1line) = Interp08(w5, w4);
#define X4PIXEL10_83	*(dp + dst1line) = Interp08(w4, w2);

#define X4PIXEL11_0		*(dp + dst1line + 1) = w5;
#define X4PIXEL11_30	*(dp + dst1line + 1) = Interp03(w5, w1);
#define X4PIXEL11_31	*(dp + dst1line + 1) = Interp03(w5, w4);
#define X4PIXEL11_32	*(dp + dst1line + 1) = Interp03(w5, w2);
#define X4PIXEL11_70	*(dp + dst1line + 1) = Interp07(w5, w4, w2);

#define X4PIXEL12_0		*(dp + dst1line + 2) = w5;
#define X4PIXEL12_30	*(dp + dst1line + 2) = Interp03(w5, w3);
#define X4PIXEL12_31	*(dp + dst1line + 2) = Interp03(w5, w2);
#define X4PIXEL12_32	*(dp + dst1line + 2) = Interp03(w5, w6);
#define X4PIXEL12_70	*(dp + dst1line + 2) = Interp07(w5, w6, w2);

#define X4PIXEL13_0		*(dp + dst1line + 3) = w5;
#define X4PIXEL13_10	*(dp + dst1line + 3) = Interp01(w5, w3);
#define X4PIXEL13_12	*(dp + dst1line + 3) = Interp01(w5, w6);
#define X4PIXEL13_14	*(dp + dst1line + 3) = Interp01(w6, w5);
#define X4PIXEL13_21	*(dp + dst1line + 3) = Interp02(w6, w5, w2);
#define X4PIXEL13_31	*(dp + dst1line + 3) = Interp03(w5, w2);
#define X4PIXEL13_50	*(dp + dst1line + 3) = Interp05(w6, w5);
#define X4PIXEL13_60	*(dp + dst1line + 3) = Interp06(w5, w6, w2);
#define X4PIXEL13_61	*(dp + dst1line + 3) = Interp06(w5, w6, w3);
#define X4PIXEL13_82	*(dp + dst1line + 3) = Interp08(w5, w6);
#define X4PIXEL13_83	*(dp + dst1line + 3) = Interp08(w6, w2);

#define X4PIXEL20_0		*(dp + dst1line + dst1line) = w5;
#define X4PIXEL20_10	*(dp + dst1line + dst1line) = Interp01(w5, w7);
#define X4PIXEL20_12	*(dp + dst1line + dst1line) = Interp01(w5, w4);
#define X4PIXEL20_14	*(dp + dst1line + dst1line) = Interp01(w4, w5);
#define X4PIXEL20_21	*(dp + dst1line + dst1line) = Interp02(w4, w5, w8);
#define X4PIXEL20_31	*(dp + dst1line + dst1line) = Interp03(w5, w8);
#define X4PIXEL20_50	*(dp + dst1line + dst1line) = Interp05(w4, w5);
#define X4PIXEL20_60	*(dp + dst1line + dst1line) = Interp06(w5, w4, w8);
#define X4PIXEL20_61	*(dp + dst1line + dst1line) = Interp06(w5, w4, w7);
#define X4PIXEL20_82	*(dp + dst1line + dst1line) = Interp08(w5, w4);
#define X4PIXEL20_83	*(dp + dst1line + dst1line) = Interp08(w4, w8);

#define X4PIXEL21_0		*(dp + dst1line + dst1line + 1) = w5;
#define X4PIXEL21_30	*(dp + dst1line + dst1line + 1) = Interp03(w5, w7);
#define X4PIXEL21_31	*(dp + dst1line + dst1line + 1) = Interp03(w5, w8);
#define X4PIXEL21_32	*(dp + dst1line + dst1line + 1) = Interp03(w5, w4);
#define X4PIXEL21_70	*(dp + dst1line + dst1line + 1) = Interp07(w5, w4, w8);

#define X4PIXEL22_0		*(dp + dst1line + dst1line + 2) = w5;
#define X4PIXEL22_30	*(dp + dst1line + dst1line + 2) = Interp03(w5, w9);
#define X4PIXEL22_31	*(dp + dst1line + dst1line + 2) = Interp03(w5, w6);
#define X4PIXEL22_32	*(dp + dst1line + dst1line + 2) = Interp03(w5, w8);
#define X4PIXEL22_70	*(dp + dst1line + dst1line + 2) = Interp07(w5, w6, w8);

#define X4PIXEL23_0		*(dp + dst1line + dst1line + 3) = w5;
#define X4PIXEL23_10	*(dp + dst1line + dst1line + 3) = Interp01(w5, w9);
#define X4PIXEL23_11	*(dp + dst1line + dst1line + 3) = Interp01(w5, w6);
#define X4PIXEL23_13	*(dp + dst1line + dst1line + 3) = Interp01(w6, w5);
#define X4PIXEL23_21	*(dp + dst1line + dst1line + 3) = Interp02(w6, w5, w8);
#define X4PIXEL23_32	*(dp + dst1line + dst1line + 3) = Interp03(w5, w8);
#define X4PIXEL23_50	*(dp + dst1line + dst1line + 3) = Interp05(w6, w5);
#define X4PIXEL23_60	*(dp + dst1line + dst1line + 3) = Interp06(w5, w6, w8);
#define X4PIXEL23_61	*(dp + dst1line + dst1line + 3) = Interp06(w5, w6, w9);
#define X4PIXEL23_81	*(dp + dst1line + dst1line + 3) = Interp08(w5, w6);
#define X4PIXEL23_83	*(dp + dst1line + dst1line + 3) = Interp08(w6, w8);

#define X4PIXEL30_0		*(dp + dst1line + dst1line + dst1line) = w5;
#define X4PIXEL30_11	*(dp + dst1line + dst1line + dst1line) = Interp01(w5, w8);
#define X4PIXEL30_12	*(dp + dst1line + dst1line + dst1line) = Interp01(w5, w4);
#define X4PIXEL30_20	*(dp + dst1line + dst1line + dst1line) = Interp02(w5, w8, w4);
#define X4PIXEL30_50	*(dp + dst1line + dst1line + dst1line) = Interp05(w8, w4);
#define X4PIXEL30_80	*(dp + dst1line + dst1line + dst1line) = Interp08(w5, w7);
#define X4PIXEL30_81	*(dp + dst1line + dst1line + dst1line) = Interp08(w5, w8);
#define X4PIXEL30_82	*(dp + dst1line + dst1line + dst1line) = Interp08(w5, w4);

#define X4PIXEL31_0		*(dp + dst1line + dst1line + dst1line + 1) = w5;
#define X4PIXEL31_10	*(dp + dst1line + dst1line + dst1line + 1) = Interp01(w5, w7);
#define X4PIXEL31_11	*(dp + dst1line + dst1line + dst1line + 1) = Interp01(w5, w8);
#define X4PIXEL31_13	*(dp + dst1line + dst1line + dst1line + 1) = Interp01(w8, w5);
#define X4PIXEL31_21	*(dp + dst1line + dst1line + dst1line + 1) = Interp02(w8, w5, w4);
#define X4PIXEL31_32	*(dp + dst1line + dst1line + dst1line + 1) = Interp03(w5, w4);
#define X4PIXEL31_50	*(dp + dst1line + dst1line + dst1line + 1) = Interp05(w8, w5);
#define X4PIXEL31_60	*(dp + dst1line + dst1line + dst1line + 1) = Interp06(w5, w8, w4);
#define X4PIXEL31_61	*(dp + dst1line + dst1line + dst1line + 1) = Interp06(w5, w8, w7);
#define X4PIXEL31_81	*(dp + dst1line + dst1line + dst1line + 1) = Interp08(w5, w8);
#define X4PIXEL31_83	*(dp + dst1line + dst1line + dst1line + 1) = Interp08(w8, w4);

#define X4PIXEL32_0		*(dp + dst1line + dst1line + dst1line + 2) = w5;
#define X4PIXEL32_10	*(dp + dst1line + dst1line + dst1line + 2) = Interp01(w5, w9);
#define X4PIXEL32_12	*(dp + dst1line + dst1line + dst1line + 2) = Interp01(w5, w8);
#define X4PIXEL32_14	*(dp + dst1line + dst1line + dst1line + 2) = Interp01(w8, w5);
#define X4PIXEL32_21	*(dp + dst1line + dst1line + dst1line + 2) = Interp02(w8, w5, w6);
#define X4PIXEL32_31	*(dp + dst1line + dst1line + dst1line + 2) = Interp03(w5, w6);
#define X4PIXEL32_50	*(dp + dst1line + dst1line + dst1line + 2) = Interp05(w8, w5);
#define X4PIXEL32_60	*(dp + dst1line + dst1line + dst1line + 2) = Interp06(w5, w8, w6);
#define X4PIXEL32_61	*(dp + dst1line + dst1line + dst1line + 2) = Interp06(w5, w8, w9);
#define X4PIXEL32_82	*(dp + dst1line + dst1line + dst1line + 2) = Interp08(w5, w8);
#define X4PIXEL32_83	*(dp + dst1line + dst1line + dst1line + 2) = Interp08(w8, w6);

#define X4PIXEL33_0		*(dp + dst1line + dst1line + dst1line + 3) = w5;
#define X4PIXEL33_11	*(dp + dst1line + dst1line + dst1line + 3) = Interp01(w5, w6);
#define X4PIXEL33_12	*(dp + dst1line + dst1line + dst1line + 3) = Interp01(w5, w8);
#define X4PIXEL33_20	*(dp + dst1line + dst1line + dst1line + 3) = Interp02(w5, w8, w6);
#define X4PIXEL33_50	*(dp + dst1line + dst1line + dst1line + 3) = Interp05(w8, w6);
#define X4PIXEL33_80	*(dp + dst1line + dst1line + dst1line + 3) = Interp08(w5, w9);
#define X4PIXEL33_81	*(dp + dst1line + dst1line + dst1line + 3) = Interp08(w5, w6);
#define X4PIXEL33_82	*(dp + dst1line + dst1line + dst1line + 3) = Interp08(w5, w8);

#define Absolute(c) \
(!(c & (1 << 31)) ? c : (~c + 1))

static int	*RGBtoYUV = NULL;

static void InitLUTs (void);
static inline bool Diff (int, int);


bool8 S9xBlitHQ2xFilterInit (void)
{
	uint32	n = 1 << ((FIRST_COLOR_MASK & 0x8000) ? 16 : 15);
	RGBtoYUV = new int[n];
	if (!RGBtoYUV)
		return (FALSE);

#ifdef GFX_MULTI_FORMAT
	Mask_2 = SECOND_COLOR_MASK;
	Mask13 = FIRST_THIRD_COLOR_MASK;
#endif

	InitLUTs();

	return (TRUE);
}

void S9xBlitHQ2xFilterDeinit (void)
{
	if (RGBtoYUV)
	{
		delete[] RGBtoYUV;
		RGBtoYUV = NULL;
	}
}

static void InitLUTs (void)
{
	uint32	r, g, b;
	int		y, u, v;

	uint32	n = 1 << ((FIRST_COLOR_MASK & 0x8000) ? 16 : 15);
	for (uint32 c = 0 ; c < n ; c++)
	{
		DECOMPOSE_PIXEL(c, r, g, b);
		r <<= 3;
		g <<= 3;
		b <<= 3;

		y = (int) ( 0.256788f * r + 0.504129f * g + 0.097906f * b + 0.5f) + 16;
		u = (int) (-0.148223f * r - 0.290993f * g + 0.439216f * b + 0.5f) + 128;
		v = (int) ( 0.439216f * r - 0.367788f * g - 0.071427f * b + 0.5f) + 128;

		RGBtoYUV[c] = (y << 16) + (u << 8) + v;
	}
}

static inline bool Diff (int c1, int c2)
{
	int	c1y = (c1 & Ymask) - (c2 & Ymask);
	if (Absolute(c1y) > trY)
		return (true);

	int	c1u = (c1 & Umask) - (c2 & Umask);
	if (Absolute(c1u) > trU)
		return (true);

	int	c1v = (c1 & Vmask) - (c2 & Vmask);
	if (Absolute(c1v) > trV)
		return (true);

	return (false);
}

void HQ2X_16 (uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
	register int	w1, w2, w3, w4, w5, w6, w7, w8, w9;
	register uint32	src1line = srcPitch >> 1;
	register uint32	dst1line = dstPitch >> 1;
	register uint16	*sp = (uint16 *) srcPtr;
	register uint16	*dp = (uint16 *) dstPtr;

	uint32  pattern;
	int		l, y;

	while (height--)
	{
		sp--;

		w1 = *(sp - src1line);
		w4 = *(sp);
		w7 = *(sp + src1line);

		sp++;

		w2 = *(sp - src1line);
		w5 = *(sp);
		w8 = *(sp + src1line);

		for (l = width; l; l--)
		{
			sp++;

			w3 = *(sp - src1line);
			w6 = *(sp);
			w9 = *(sp + src1line);

			y = RGBtoYUV[w5];
			pattern = 0;

			if ((w1 != w5) && (Diff(y, RGBtoYUV[w1]))) pattern |= (1 << 0);
			if ((w2 != w5) && (Diff(y, RGBtoYUV[w2]))) pattern |= (1 << 1);
			if ((w3 != w5) && (Diff(y, RGBtoYUV[w3]))) pattern |= (1 << 2);
			if ((w4 != w5) && (Diff(y, RGBtoYUV[w4]))) pattern |= (1 << 3);
			if ((w6 != w5) && (Diff(y, RGBtoYUV[w6]))) pattern |= (1 << 4);
			if ((w7 != w5) && (Diff(y, RGBtoYUV[w7]))) pattern |= (1 << 5);
			if ((w8 != w5) && (Diff(y, RGBtoYUV[w8]))) pattern |= (1 << 6);
			if ((w9 != w5) && (Diff(y, RGBtoYUV[w9]))) pattern |= (1 << 7);

			switch (pattern)
			{
				case 0:
				case 1:
				case 4:
				case 32:
				case 128:
				case 5:
				case 132:
				case 160:
				case 33:
				case 129:
				case 36:
				case 133:
				case 164:
				case 161:
				case 37:
				case 165:
				{
					X2PIXEL00_20
					X2PIXEL01_20
					X2PIXEL10_20
					X2PIXEL11_20
					break;
				}
				case 2:
				case 34:
				case 130:
				case 162:
				{
					X2PIXEL00_22
					X2PIXEL01_21
					X2PIXEL10_20
					X2PIXEL11_20
					break;
				}
				case 16:
				case 17:
				case 48:
				case 49:
				{
					X2PIXEL00_20
					X2PIXEL01_22
					X2PIXEL10_20
					X2PIXEL11_21
					break;
				}
				case 64:
				case 65:
				case 68:
				case 69:
				{
					X2PIXEL00_20
					X2PIXEL01_20
					X2PIXEL10_21
					X2PIXEL11_22
					break;
				}
				case 8:
				case 12:
				case 136:
				case 140:
				{
					X2PIXEL00_21
					X2PIXEL01_20
					X2PIXEL10_22
					X2PIXEL11_20
					break;
				}
				case 3:
				case 35:
				case 131:
				case 163:
				{
					X2PIXEL00_11
					X2PIXEL01_21
					X2PIXEL10_20
					X2PIXEL11_20
					break;
				}
				case 6:
				case 38:
				case 134:
				case 166:
				{
					X2PIXEL00_22
					X2PIXEL01_12
					X2PIXEL10_20
					X2PIXEL11_20
					break;
				}
				case 20:
				case 21:
				case 52:
				case 53:
				{
					X2PIXEL00_20
					X2PIXEL01_11
					X2PIXEL10_20
					X2PIXEL11_21
					break;
				}
				case 144:
				case 145:
				case 176:
				case 177:
				{
					X2PIXEL00_20
					X2PIXEL01_22
					X2PIXEL10_20
					X2PIXEL11_12
					break;
				}
				case 192:
				case 193:
				case 196:
				case 197:
				{
					X2PIXEL00_20
					X2PIXEL01_20
					X2PIXEL10_21
					X2PIXEL11_11
					break;
				}
				case 96:
				case 97:
				case 100:
				case 101:
				{
					X2PIXEL00_20
					X2PIXEL01_20
					X2PIXEL10_12
					X2PIXEL11_22
					break;
				}
				case 40:
				case 44:
				case 168:
				case 172:
				{
					X2PIXEL00_21
					X2PIXEL01_20
					X2PIXEL10_11
					X2PIXEL11_20
					break;
				}
				case 9:
				case 13:
				case 137:
				case 141:
				{
					X2PIXEL00_12
					X2PIXEL01_20
					X2PIXEL10_22
					X2PIXEL11_20
					break;
				}
				case 18:
				case 50:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_20
					X2PIXEL11_21
					break;
				}
				case 80:
				case 81:
				{
					X2PIXEL00_20
					X2PIXEL01_22
					X2PIXEL10_21
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 72:
				case 76:
				{
					X2PIXEL00_21
					X2PIXEL01_20
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_22
					break;
				}
				case 10:
				case 138:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_21
					X2PIXEL10_22
					X2PIXEL11_20
					break;
				}
				case 66:
				{
					X2PIXEL00_22
					X2PIXEL01_21
					X2PIXEL10_21
					X2PIXEL11_22
					break;
				}
				case 24:
				{
					X2PIXEL00_21
					X2PIXEL01_22
					X2PIXEL10_22
					X2PIXEL11_21
					break;
				}
				case 7:
				case 39:
				case 135:
				{
					X2PIXEL00_11
					X2PIXEL01_12
					X2PIXEL10_20
					X2PIXEL11_20
					break;
				}
				case 148:
				case 149:
				case 180:
				{
					X2PIXEL00_20
					X2PIXEL01_11
					X2PIXEL10_20
					X2PIXEL11_12
					break;
				}
				case 224:
				case 228:
				case 225:
				{
					X2PIXEL00_20
					X2PIXEL01_20
					X2PIXEL10_12
					X2PIXEL11_11
					break;
				}
				case 41:
				case 169:
				case 45:
				{
					X2PIXEL00_12
					X2PIXEL01_20
					X2PIXEL10_11
					X2PIXEL11_20
					break;
				}
				case 22:
				case 54:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_20
					X2PIXEL11_21
					break;
				}
				case 208:
				case 209:
				{
					X2PIXEL00_20
					X2PIXEL01_22
					X2PIXEL10_21
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 104:
				case 108:
				{
					X2PIXEL00_21
					X2PIXEL01_20
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_22
					break;
				}
				case 11:
				case 139:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_21
					X2PIXEL10_22
					X2PIXEL11_20
					break;
				}
				case 19:
				case 51:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL00_11
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL00_60
						X2PIXEL01_90
					}
					X2PIXEL10_20
					X2PIXEL11_21
					break;
				}
				case 146:
				case 178:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
						X2PIXEL11_12
					}
					else
					{
						X2PIXEL01_90
						X2PIXEL11_61
					}
					X2PIXEL10_20
					break;
				}
				case 84:
				case 85:
				{
					X2PIXEL00_20
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL01_11
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL01_60
						X2PIXEL11_90
					}
					X2PIXEL10_21
					break;
				}
				case 112:
				case 113:
				{
					X2PIXEL00_20
					X2PIXEL01_22
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL10_12
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL10_61
						X2PIXEL11_90
					}
					break;
				}
				case 200:
				case 204:
				{
					X2PIXEL00_21
					X2PIXEL01_20
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
						X2PIXEL11_11
					}
					else
					{
						X2PIXEL10_90
						X2PIXEL11_60
					}
					break;
				}
				case 73:
				case 77:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL00_12
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL00_61
						X2PIXEL10_90
					}
					X2PIXEL01_20
					X2PIXEL11_22
					break;
				}
				case 42:
				case 170:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
						X2PIXEL10_11
					}
					else
					{
						X2PIXEL00_90
						X2PIXEL10_60
					}
					X2PIXEL01_21
					X2PIXEL11_20
					break;
				}
				case 14:
				case 142:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
						X2PIXEL01_12
					}
					else
					{
						X2PIXEL00_90
						X2PIXEL01_61
					}
					X2PIXEL10_22
					X2PIXEL11_20
					break;
				}
				case 67:
				{
					X2PIXEL00_11
					X2PIXEL01_21
					X2PIXEL10_21
					X2PIXEL11_22
					break;
				}
				case 70:
				{
					X2PIXEL00_22
					X2PIXEL01_12
					X2PIXEL10_21
					X2PIXEL11_22
					break;
				}
				case 28:
				{
					X2PIXEL00_21
					X2PIXEL01_11
					X2PIXEL10_22
					X2PIXEL11_21
					break;
				}
				case 152:
				{
					X2PIXEL00_21
					X2PIXEL01_22
					X2PIXEL10_22
					X2PIXEL11_12
					break;
				}
				case 194:
				{
					X2PIXEL00_22
					X2PIXEL01_21
					X2PIXEL10_21
					X2PIXEL11_11
					break;
				}
				case 98:
				{
					X2PIXEL00_22
					X2PIXEL01_21
					X2PIXEL10_12
					X2PIXEL11_22
					break;
				}
				case 56:
				{
					X2PIXEL00_21
					X2PIXEL01_22
					X2PIXEL10_11
					X2PIXEL11_21
					break;
				}
				case 25:
				{
					X2PIXEL00_12
					X2PIXEL01_22
					X2PIXEL10_22
					X2PIXEL11_21
					break;
				}
				case 26:
				case 31:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_22
					X2PIXEL11_21
					break;
				}
				case 82:
				case 214:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_21
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 88:
				case 248:
				{
					X2PIXEL00_21
					X2PIXEL01_22
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 74:
				case 107:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_21
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_22
					break;
				}
				case 27:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_10
					X2PIXEL10_22
					X2PIXEL11_21
					break;
				}
				case 86:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_21
					X2PIXEL11_10
					break;
				}
				case 216:
				{
					X2PIXEL00_21
					X2PIXEL01_22
					X2PIXEL10_10
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 106:
				{
					X2PIXEL00_10
					X2PIXEL01_21
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_22
					break;
				}
				case 30:
				{
					X2PIXEL00_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_22
					X2PIXEL11_21
					break;
				}
				case 210:
				{
					X2PIXEL00_22
					X2PIXEL01_10
					X2PIXEL10_21
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 120:
				{
					X2PIXEL00_21
					X2PIXEL01_22
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_10
					break;
				}
				case 75:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_21
					X2PIXEL10_10
					X2PIXEL11_22
					break;
				}
				case 29:
				{
					X2PIXEL00_12
					X2PIXEL01_11
					X2PIXEL10_22
					X2PIXEL11_21
					break;
				}
				case 198:
				{
					X2PIXEL00_22
					X2PIXEL01_12
					X2PIXEL10_21
					X2PIXEL11_11
					break;
				}
				case 184:
				{
					X2PIXEL00_21
					X2PIXEL01_22
					X2PIXEL10_11
					X2PIXEL11_12
					break;
				}
				case 99:
				{
					X2PIXEL00_11
					X2PIXEL01_21
					X2PIXEL10_12
					X2PIXEL11_22
					break;
				}
				case 57:
				{
					X2PIXEL00_12
					X2PIXEL01_22
					X2PIXEL10_11
					X2PIXEL11_21
					break;
				}
				case 71:
				{
					X2PIXEL00_11
					X2PIXEL01_12
					X2PIXEL10_21
					X2PIXEL11_22
					break;
				}
				case 156:
				{
					X2PIXEL00_21
					X2PIXEL01_11
					X2PIXEL10_22
					X2PIXEL11_12
					break;
				}
				case 226:
				{
					X2PIXEL00_22
					X2PIXEL01_21
					X2PIXEL10_12
					X2PIXEL11_11
					break;
				}
				case 60:
				{
					X2PIXEL00_21
					X2PIXEL01_11
					X2PIXEL10_11
					X2PIXEL11_21
					break;
				}
				case 195:
				{
					X2PIXEL00_11
					X2PIXEL01_21
					X2PIXEL10_21
					X2PIXEL11_11
					break;
				}
				case 102:
				{
					X2PIXEL00_22
					X2PIXEL01_12
					X2PIXEL10_12
					X2PIXEL11_22
					break;
				}
				case 153:
				{
					X2PIXEL00_12
					X2PIXEL01_22
					X2PIXEL10_22
					X2PIXEL11_12
					break;
				}
				case 58:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_11
					X2PIXEL11_21
					break;
				}
				case 83:
				{
					X2PIXEL00_11
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_21
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 92:
				{
					X2PIXEL00_21
					X2PIXEL01_11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 202:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					X2PIXEL01_21
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					X2PIXEL11_11
					break;
				}
				case 78:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					X2PIXEL01_12
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					X2PIXEL11_22
					break;
				}
				case 154:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_22
					X2PIXEL11_12
					break;
				}
				case 114:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_12
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 89:
				{
					X2PIXEL00_12
					X2PIXEL01_22
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 90:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 55:
				case 23:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL00_11
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL00_60
						X2PIXEL01_90
					}
					X2PIXEL10_20
					X2PIXEL11_21
					break;
				}
				case 182:
				case 150:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
						X2PIXEL11_12
					}
					else
					{
						X2PIXEL01_90
						X2PIXEL11_61
					}
					X2PIXEL10_20
					break;
				}
				case 213:
				case 212:
				{
					X2PIXEL00_20
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL01_11
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL01_60
						X2PIXEL11_90
					}
					X2PIXEL10_21
					break;
				}
				case 241:
				case 240:
				{
					X2PIXEL00_20
					X2PIXEL01_22
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL10_12
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL10_61
						X2PIXEL11_90
					}
					break;
				}
				case 236:
				case 232:
				{
					X2PIXEL00_21
					X2PIXEL01_20
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
						X2PIXEL11_11
					}
					else
					{
						X2PIXEL10_90
						X2PIXEL11_60
					}
					break;
				}
				case 109:
				case 105:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL00_12
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL00_61
						X2PIXEL10_90
					}
					X2PIXEL01_20
					X2PIXEL11_22
					break;
				}
				case 171:
				case 43:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
						X2PIXEL10_11
					}
					else
					{
						X2PIXEL00_90
						X2PIXEL10_60
					}
					X2PIXEL01_21
					X2PIXEL11_20
					break;
				}
				case 143:
				case 15:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
						X2PIXEL01_12
					}
					else
					{
						X2PIXEL00_90
						X2PIXEL01_61
					}
					X2PIXEL10_22
					X2PIXEL11_20
					break;
				}
				case 124:
				{
					X2PIXEL00_21
					X2PIXEL01_11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_10
					break;
				}
				case 203:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_21
					X2PIXEL10_10
					X2PIXEL11_11
					break;
				}
				case 62:
				{
					X2PIXEL00_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_11
					X2PIXEL11_21
					break;
				}
				case 211:
				{
					X2PIXEL00_11
					X2PIXEL01_10
					X2PIXEL10_21
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 118:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_12
					X2PIXEL11_10
					break;
				}
				case 217:
				{
					X2PIXEL00_12
					X2PIXEL01_22
					X2PIXEL10_10
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 110:
				{
					X2PIXEL00_10
					X2PIXEL01_12
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_22
					break;
				}
				case 155:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_10
					X2PIXEL10_22
					X2PIXEL11_12
					break;
				}
				case 188:
				{
					X2PIXEL00_21
					X2PIXEL01_11
					X2PIXEL10_11
					X2PIXEL11_12
					break;
				}
				case 185:
				{
					X2PIXEL00_12
					X2PIXEL01_22
					X2PIXEL10_11
					X2PIXEL11_12
					break;
				}
				case 61:
				{
					X2PIXEL00_12
					X2PIXEL01_11
					X2PIXEL10_11
					X2PIXEL11_21
					break;
				}
				case 157:
				{
					X2PIXEL00_12
					X2PIXEL01_11
					X2PIXEL10_22
					X2PIXEL11_12
					break;
				}
				case 103:
				{
					X2PIXEL00_11
					X2PIXEL01_12
					X2PIXEL10_12
					X2PIXEL11_22
					break;
				}
				case 227:
				{
					X2PIXEL00_11
					X2PIXEL01_21
					X2PIXEL10_12
					X2PIXEL11_11
					break;
				}
				case 230:
				{
					X2PIXEL00_22
					X2PIXEL01_12
					X2PIXEL10_12
					X2PIXEL11_11
					break;
				}
				case 199:
				{
					X2PIXEL00_11
					X2PIXEL01_12
					X2PIXEL10_21
					X2PIXEL11_11
					break;
				}
				case 220:
				{
					X2PIXEL00_21
					X2PIXEL01_11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 158:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_22
					X2PIXEL11_12
					break;
				}
				case 234:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					X2PIXEL01_21
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_11
					break;
				}
				case 242:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_12
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 59:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_11
					X2PIXEL11_21
					break;
				}
				case 121:
				{
					X2PIXEL00_12
					X2PIXEL01_22
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 87:
				{
					X2PIXEL00_11
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_21
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 79:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_12
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					X2PIXEL11_22
					break;
				}
				case 122:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 94:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 218:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 91:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 229:
				{
					X2PIXEL00_20
					X2PIXEL01_20
					X2PIXEL10_12
					X2PIXEL11_11
					break;
				}
				case 167:
				{
					X2PIXEL00_11
					X2PIXEL01_12
					X2PIXEL10_20
					X2PIXEL11_20
					break;
				}
				case 173:
				{
					X2PIXEL00_12
					X2PIXEL01_20
					X2PIXEL10_11
					X2PIXEL11_20
					break;
				}
				case 181:
				{
					X2PIXEL00_20
					X2PIXEL01_11
					X2PIXEL10_20
					X2PIXEL11_12
					break;
				}
				case 186:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_11
					X2PIXEL11_12
					break;
				}
				case 115:
				{
					X2PIXEL00_11
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_12
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 93:
				{
					X2PIXEL00_12
					X2PIXEL01_11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 206:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					X2PIXEL01_12
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					X2PIXEL11_11
					break;
				}
				case 205:
				case 201:
				{
					X2PIXEL00_12
					X2PIXEL01_20
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_10
					}
					else
					{
						X2PIXEL10_70
					}
					X2PIXEL11_11
					break;
				}
				case 174:
				case 46:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_10
					}
					else
					{
						X2PIXEL00_70
					}
					X2PIXEL01_12
					X2PIXEL10_11
					X2PIXEL11_20
					break;
				}
				case 179:
				case 147:
				{
					X2PIXEL00_11
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_10
					}
					else
					{
						X2PIXEL01_70
					}
					X2PIXEL10_20
					X2PIXEL11_12
					break;
				}
				case 117:
				case 116:
				{
					X2PIXEL00_20
					X2PIXEL01_11
					X2PIXEL10_12
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_10
					}
					else
					{
						X2PIXEL11_70
					}
					break;
				}
				case 189:
				{
					X2PIXEL00_12
					X2PIXEL01_11
					X2PIXEL10_11
					X2PIXEL11_12
					break;
				}
				case 231:
				{
					X2PIXEL00_11
					X2PIXEL01_12
					X2PIXEL10_12
					X2PIXEL11_11
					break;
				}
				case 126:
				{
					X2PIXEL00_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_10
					break;
				}
				case 219:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_10
					X2PIXEL10_10
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 125:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL00_12
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL00_61
						X2PIXEL10_90
					}
					X2PIXEL01_11
					X2PIXEL11_10
					break;
				}
				case 221:
				{
					X2PIXEL00_12
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL01_11
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL01_60
						X2PIXEL11_90
					}
					X2PIXEL10_10
					break;
				}
				case 207:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
						X2PIXEL01_12
					}
					else
					{
						X2PIXEL00_90
						X2PIXEL01_61
					}
					X2PIXEL10_10
					X2PIXEL11_11
					break;
				}
				case 238:
				{
					X2PIXEL00_10
					X2PIXEL01_12
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
						X2PIXEL11_11
					}
					else
					{
						X2PIXEL10_90
						X2PIXEL11_60
					}
					break;
				}
				case 190:
				{
					X2PIXEL00_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
						X2PIXEL11_12
					}
					else
					{
						X2PIXEL01_90
						X2PIXEL11_61
					}
					X2PIXEL10_11
					break;
				}
				case 187:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
						X2PIXEL10_11
					}
					else
					{
						X2PIXEL00_90
						X2PIXEL10_60
					}
					X2PIXEL01_10
					X2PIXEL11_12
					break;
				}
				case 243:
				{
					X2PIXEL00_11
					X2PIXEL01_10
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL10_12
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL10_61
						X2PIXEL11_90
					}
					break;
				}
				case 119:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL00_11
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL00_60
						X2PIXEL01_90
					}
					X2PIXEL10_12
					X2PIXEL11_10
					break;
				}
				case 237:
				case 233:
				{
					X2PIXEL00_12
					X2PIXEL01_20
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_100
					}
					X2PIXEL11_11
					break;
				}
				case 175:
				case 47:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_100
					}
					X2PIXEL01_12
					X2PIXEL10_11
					X2PIXEL11_20
					break;
				}
				case 183:
				case 151:
				{
					X2PIXEL00_11
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_100
					}
					X2PIXEL10_20
					X2PIXEL11_12
					break;
				}
				case 245:
				case 244:
				{
					X2PIXEL00_20
					X2PIXEL01_11
					X2PIXEL10_12
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_100
					}
					break;
				}
				case 250:
				{
					X2PIXEL00_10
					X2PIXEL01_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 123:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_10
					break;
				}
				case 95:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_10
					X2PIXEL11_10
					break;
				}
				case 222:
				{
					X2PIXEL00_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_10
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 252:
				{
					X2PIXEL00_21
					X2PIXEL01_11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_100
					}
					break;
				}
				case 249:
				{
					X2PIXEL00_12
					X2PIXEL01_22
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_100
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 235:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_21
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_100
					}
					X2PIXEL11_11
					break;
				}
				case 111:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_100
					}
					X2PIXEL01_12
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_22
					break;
				}
				case 63:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_100
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_11
					X2PIXEL11_21
					break;
				}
				case 159:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_100
					}
					X2PIXEL10_22
					X2PIXEL11_12
					break;
				}
				case 215:
				{
					X2PIXEL00_11
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_100
					}
					X2PIXEL10_21
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 246:
				{
					X2PIXEL00_22
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					X2PIXEL10_12
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_100
					}
					break;
				}
				case 254:
				{
					X2PIXEL00_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_100
					}
					break;
				}
				case 253:
				{
					X2PIXEL00_12
					X2PIXEL01_11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_100
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_100
					}
					break;
				}
				case 251:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					X2PIXEL01_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_100
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 239:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_100
					}
					X2PIXEL01_12
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_100
					}
					X2PIXEL11_11
					break;
				}
				case 127:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_100
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_20
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_20
					}
					X2PIXEL11_10
					break;
				}
				case 191:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_100
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_100
					}
					X2PIXEL10_11
					X2PIXEL11_12
					break;
				}
				case 223:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_20
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_100
					}
					X2PIXEL10_10
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_20
					}
					break;
				}
				case 247:
				{
					X2PIXEL00_11
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_100
					}
					X2PIXEL10_12
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_100
					}
					break;
				}
				case 255:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X2PIXEL00_0
					}
					else
					{
						X2PIXEL00_100
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X2PIXEL01_0
					}
					else
					{
						X2PIXEL01_100
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X2PIXEL10_0
					}
					else
					{
						X2PIXEL10_100
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X2PIXEL11_0
					}
					else
					{
						X2PIXEL11_100
					}
					break;
				}
			}

			w1 = w2; w4 = w5; w7 = w8;
			w2 = w3; w5 = w6; w8 = w9;

			dp += 2;
		}

		dp += (dst1line - width) * 2;
		sp += (src1line - width);
	}
}

void HQ3X_16 (uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
	register int	w1, w2, w3, w4, w5, w6, w7, w8, w9;
	register uint32	src1line = srcPitch >> 1;
	register uint32	dst1line = dstPitch >> 1;
	register uint16	*sp = (uint16 *) srcPtr;
	register uint16	*dp = (uint16 *) dstPtr;

	uint32  pattern;
	int		l, y;

	while (height--)
	{
		sp--;

		w1 = *(sp - src1line);
		w4 = *(sp);
		w7 = *(sp + src1line);

		sp++;

		w2 = *(sp - src1line);
		w5 = *(sp);
		w8 = *(sp + src1line);

		for (l = width; l; l--)
		{
			sp++;

			w3 = *(sp - src1line);
			w6 = *(sp);
			w9 = *(sp + src1line);

			y = RGBtoYUV[w5];
			pattern = 0;

			if ((w1 != w5) && (Diff(y, RGBtoYUV[w1]))) pattern |= (1 << 0);
			if ((w2 != w5) && (Diff(y, RGBtoYUV[w2]))) pattern |= (1 << 1);
			if ((w3 != w5) && (Diff(y, RGBtoYUV[w3]))) pattern |= (1 << 2);
			if ((w4 != w5) && (Diff(y, RGBtoYUV[w4]))) pattern |= (1 << 3);
			if ((w6 != w5) && (Diff(y, RGBtoYUV[w6]))) pattern |= (1 << 4);
			if ((w7 != w5) && (Diff(y, RGBtoYUV[w7]))) pattern |= (1 << 5);
			if ((w8 != w5) && (Diff(y, RGBtoYUV[w8]))) pattern |= (1 << 6);
			if ((w9 != w5) && (Diff(y, RGBtoYUV[w9]))) pattern |= (1 << 7);

			switch (pattern)
			{
				case 0:
				case 1:
				case 4:
				case 32:
				case 128:
				case 5:
				case 132:
				case 160:
				case 33:
				case 129:
				case 36:
				case 133:
				case 164:
				case 161:
				case 37:
				case 165:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 2:
				case 34:
				case 130:
				case 162:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 16:
				case 17:
				case 48:
				case 49:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 64:
				case 65:
				case 68:
				case 69:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 8:
				case 12:
				case 136:
				case 140:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 3:
				case 35:
				case 131:
				case 163:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 6:
				case 38:
				case 134:
				case 166:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 20:
				case 21:
				case 52:
				case 53:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 144:
				case 145:
				case 176:
				case 177:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 192:
				case 193:
				case 196:
				case 197:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 96:
				case 97:
				case 100:
				case 101:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 40:
				case 44:
				case 168:
				case 172:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 9:
				case 13:
				case 137:
				case 141:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 18:
				case 50:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_1M
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 80:
				case 81:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 72:
				case 76:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_1M
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 10:
				case 138:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 66:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 24:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 7:
				case 39:
				case 135:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 148:
				case 149:
				case 180:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 224:
				case 228:
				case 225:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 41:
				case 169:
				case 45:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 22:
				case 54:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 208:
				case 209:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 104:
				case 108:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 11:
				case 139:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 19:
				case 51:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL00_1L
						X3PIXEL01_C
						X3PIXEL02_1M
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL00_2
						X3PIXEL01_6
						X3PIXEL02_5
						X3PIXEL12_1
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 146:
				case 178:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_1M
						X3PIXEL12_C
						X3PIXEL22_1D
					}
					else
					{
						X3PIXEL01_1
						X3PIXEL02_5
						X3PIXEL12_6
						X3PIXEL22_2
					}
					X3PIXEL00_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_2
					X3PIXEL21_1
					break;
				}
				case 84:
				case 85:
				{
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL02_1U
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL02_2
						X3PIXEL12_6
						X3PIXEL21_1
						X3PIXEL22_5
					}
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1M
					break;
				}
				case 112:
				case 113:
				{
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL20_1L
						X3PIXEL21_C
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL12_1
						X3PIXEL20_2
						X3PIXEL21_6
						X3PIXEL22_5
					}
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					break;
				}
				case 200:
				case 204:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_1M
						X3PIXEL21_C
						X3PIXEL22_1R
					}
					else
					{
						X3PIXEL10_1
						X3PIXEL20_5
						X3PIXEL21_6
						X3PIXEL22_2
					}
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL11
					X3PIXEL12_1
					break;
				}
				case 73:
				case 77:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL00_1U
						X3PIXEL10_C
						X3PIXEL20_1M
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL00_2
						X3PIXEL10_6
						X3PIXEL20_5
						X3PIXEL21_1
					}
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL22_1M
					break;
				}
				case 42:
				case 170:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
						X3PIXEL01_C
						X3PIXEL10_C
						X3PIXEL20_1D
					}
					else
					{
						X3PIXEL00_5
						X3PIXEL01_1
						X3PIXEL10_6
						X3PIXEL20_2
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 14:
				case 142:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
						X3PIXEL01_C
						X3PIXEL02_1R
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_5
						X3PIXEL01_6
						X3PIXEL02_2
						X3PIXEL10_1
					}
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 67:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 70:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 28:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 152:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 194:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 98:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 56:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 25:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 26:
				case 31:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL10_3
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL11
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 82:
				case 214:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 88:
				case 248:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL22_4
					}
					break;
				}
				case 74:
				case 107:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
					}
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 27:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 86:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 216:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 106:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 30:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 210:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 120:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 75:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 29:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 198:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 184:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 99:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 57:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 71:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 156:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 226:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 60:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 195:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 102:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 153:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 58:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 83:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 92:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 202:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 78:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 154:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 114:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1L
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 89:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 90:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 55:
				case 23:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL00_1L
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL00_2
						X3PIXEL01_6
						X3PIXEL02_5
						X3PIXEL12_1
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 182:
				case 150:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
						X3PIXEL22_1D
					}
					else
					{
						X3PIXEL01_1
						X3PIXEL02_5
						X3PIXEL12_6
						X3PIXEL22_2
					}
					X3PIXEL00_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_2
					X3PIXEL21_1
					break;
				}
				case 213:
				case 212:
				{
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL02_1U
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL02_2
						X3PIXEL12_6
						X3PIXEL21_1
						X3PIXEL22_5
					}
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1M
					break;
				}
				case 241:
				case 240:
				{
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL20_1L
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_1
						X3PIXEL20_2
						X3PIXEL21_6
						X3PIXEL22_5
					}
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					break;
				}
				case 236:
				case 232:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
						X3PIXEL22_1R
					}
					else
					{
						X3PIXEL10_1
						X3PIXEL20_5
						X3PIXEL21_6
						X3PIXEL22_2
					}
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL11
					X3PIXEL12_1
					break;
				}
				case 109:
				case 105:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL00_1U
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL00_2
						X3PIXEL10_6
						X3PIXEL20_5
						X3PIXEL21_1
					}
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL22_1M
					break;
				}
				case 171:
				case 43:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
						X3PIXEL20_1D
					}
					else
					{
						X3PIXEL00_5
						X3PIXEL01_1
						X3PIXEL10_6
						X3PIXEL20_2
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 143:
				case 15:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL02_1R
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_5
						X3PIXEL01_6
						X3PIXEL02_2
						X3PIXEL10_1
					}
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 124:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 203:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 62:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 211:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 118:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 217:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 110:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 155:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 188:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 185:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 61:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 157:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 103:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 227:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 230:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 199:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 220:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 158:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 234:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1R
					break;
				}
				case 242:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1L
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 59:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 121:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 87:
				{
					X3PIXEL00_1L
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1M
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 79:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					X3PIXEL02_1R
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 122:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 94:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_C
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 218:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_C
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 91:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 229:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 167:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 173:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 181:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 186:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 115:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1L
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 93:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 206:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 205:
				case 201:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_1M
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 174:
				case 46:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_1M
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 179:
				case 147:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_1M
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 117:
				case 116:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1L
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_1M
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 189:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 231:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 126:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 219:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
						X3PIXEL10_3
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 125:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL00_1U
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL00_2
						X3PIXEL10_6
						X3PIXEL20_5
						X3PIXEL21_1
					}
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL22_1M
					break;
				}
				case 221:
				{
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL02_1U
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL02_2
						X3PIXEL12_6
						X3PIXEL21_1
						X3PIXEL22_5
					}
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL20_1M
					break;
				}
				case 207:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL02_1R
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_5
						X3PIXEL01_6
						X3PIXEL02_2
						X3PIXEL10_1
					}
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 238:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
						X3PIXEL22_1R
					}
					else
					{
						X3PIXEL10_1
						X3PIXEL20_5
						X3PIXEL21_6
						X3PIXEL22_2
					}
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL11
					X3PIXEL12_1
					break;
				}
				case 190:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
						X3PIXEL22_1D
					}
					else
					{
						X3PIXEL01_1
						X3PIXEL02_5
						X3PIXEL12_6
						X3PIXEL22_2
					}
					X3PIXEL00_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL20_1D
					X3PIXEL21_1
					break;
				}
				case 187:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
						X3PIXEL20_1D
					}
					else
					{
						X3PIXEL00_5
						X3PIXEL01_1
						X3PIXEL10_6
						X3PIXEL20_2
					}
					X3PIXEL02_1M
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 243:
				{
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL20_1L
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_1
						X3PIXEL20_2
						X3PIXEL21_6
						X3PIXEL22_5
					}
					X3PIXEL00_1L
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL10_1
					X3PIXEL11
					break;
				}
				case 119:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL00_1L
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL00_2
						X3PIXEL01_6
						X3PIXEL02_5
						X3PIXEL12_1
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL20_1L
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 237:
				case 233:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_2
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 175:
				case 47:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_2
					break;
				}
				case 183:
				case 151:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_2
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 245:
				case 244:
				{
					X3PIXEL00_2
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1L
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 250:
				{
					X3PIXEL00_1M
					X3PIXEL01_C
					X3PIXEL02_1M
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL22_4
					}
					break;
				}
				case 123:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
					}
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 95:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL10_3
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL11
					X3PIXEL20_1M
					X3PIXEL21_C
					X3PIXEL22_1M
					break;
				}
				case 222:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 252:
				{
					X3PIXEL00_1M
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 249:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL22_4
					}
					break;
				}
				case 235:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
					}
					X3PIXEL02_1M
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 111:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 63:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1M
					break;
				}
				case 159:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL10_3
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 215:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 246:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1L
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 254:
				{
					X3PIXEL00_1M
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_4
					}
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_4
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL21_3
						X3PIXEL22_2
					}
					break;
				}
				case 253:
				{
					X3PIXEL00_1U
					X3PIXEL01_1
					X3PIXEL02_1U
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 251:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL01_3
					}
					X3PIXEL02_1M
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL10_C
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL10_3
						X3PIXEL20_2
						X3PIXEL21_3
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL12_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL12_3
						X3PIXEL22_4
					}
					break;
				}
				case 239:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					X3PIXEL02_1R
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_1
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					X3PIXEL22_1R
					break;
				}
				case 127:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL01_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_2
						X3PIXEL01_3
						X3PIXEL10_3
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL02_4
						X3PIXEL12_3
					}
					X3PIXEL11
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
						X3PIXEL21_C
					}
					else
					{
						X3PIXEL20_4
						X3PIXEL21_3
					}
					X3PIXEL22_1M
					break;
				}
				case 191:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1D
					X3PIXEL21_1
					X3PIXEL22_1D
					break;
				}
				case 223:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
						X3PIXEL10_C
					}
					else
					{
						X3PIXEL00_4
						X3PIXEL10_3
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL01_C
						X3PIXEL02_C
						X3PIXEL12_C
					}
					else
					{
						X3PIXEL01_3
						X3PIXEL02_2
						X3PIXEL12_3
					}
					X3PIXEL11
					X3PIXEL20_1M
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL21_C
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL21_3
						X3PIXEL22_4
					}
					break;
				}
				case 247:
				{
					X3PIXEL00_1L
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_1
					X3PIXEL11
					X3PIXEL12_C
					X3PIXEL20_1L
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
				case 255:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X3PIXEL00_C
					}
					else
					{
						X3PIXEL00_2
					}
					X3PIXEL01_C
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X3PIXEL02_C
					}
					else
					{
						X3PIXEL02_2
					}
					X3PIXEL10_C
					X3PIXEL11
					X3PIXEL12_C
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X3PIXEL20_C
					}
					else
					{
						X3PIXEL20_2
					}
					X3PIXEL21_C
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X3PIXEL22_C
					}
					else
					{
						X3PIXEL22_2
					}
					break;
				}
			}

			w1 = w2; w4 = w5; w7 = w8;
			w2 = w3; w5 = w6; w8 = w9;

			dp += 3;
		}

		dp += (dst1line - width) * 3;
		sp += (src1line - width);
	}
}

void HQ4X_16 (uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
	register int	w1, w2, w3, w4, w5, w6, w7, w8, w9;
	register uint32	src1line = srcPitch >> 1;
	register uint32	dst1line = dstPitch >> 1;
	register uint16	*sp = (uint16 *) srcPtr;
	register uint16	*dp = (uint16 *) dstPtr;

	uint32  pattern;
	int		l, y;

	while (height--)
	{
		sp--;

		w1 = *(sp - src1line);
		w4 = *(sp);
		w7 = *(sp + src1line);

		sp++;

		w2 = *(sp - src1line);
		w5 = *(sp);
		w8 = *(sp + src1line);

		for (l = width; l; l--)
		{
			sp++;

			w3 = *(sp - src1line);
			w6 = *(sp);
			w9 = *(sp + src1line);

			y = RGBtoYUV[w5];
			pattern = 0;

			if ((w1 != w5) && (Diff(y, RGBtoYUV[w1]))) pattern |= (1 << 0);
			if ((w2 != w5) && (Diff(y, RGBtoYUV[w2]))) pattern |= (1 << 1);
			if ((w3 != w5) && (Diff(y, RGBtoYUV[w3]))) pattern |= (1 << 2);
			if ((w4 != w5) && (Diff(y, RGBtoYUV[w4]))) pattern |= (1 << 3);
			if ((w6 != w5) && (Diff(y, RGBtoYUV[w6]))) pattern |= (1 << 4);
			if ((w7 != w5) && (Diff(y, RGBtoYUV[w7]))) pattern |= (1 << 5);
			if ((w8 != w5) && (Diff(y, RGBtoYUV[w8]))) pattern |= (1 << 6);
			if ((w9 != w5) && (Diff(y, RGBtoYUV[w9]))) pattern |= (1 << 7);

			switch (pattern)
			{
				case 0:
				case 1:
				case 4:
				case 32:
				case 128:
				case 5:
				case 132:
				case 160:
				case 33:
				case 129:
				case 36:
				case 133:
				case 164:
				case 161:
				case 37:
				case 165:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 2:
				case 34:
				case 130:
				case 162:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 16:
				case 17:
				case 48:
				case 49:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 64:
				case 65:
				case 68:
				case 69:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 8:
				case 12:
				case 136:
				case 140:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 3:
				case 35:
				case 131:
				case 163:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 6:
				case 38:
				case 134:
				case 166:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 20:
				case 21:
				case 52:
				case 53:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 144:
				case 145:
				case 176:
				case 177:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 192:
				case 193:
				case 196:
				case 197:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 96:
				case 97:
				case 100:
				case 101:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 40:
				case 44:
				case 168:
				case 172:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 9:
				case 13:
				case 137:
				case 141:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 18:
				case 50:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL12_0
						X4PIXEL13_50
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 80:
				case 81:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_61
					X4PIXEL21_30
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 72:
				case 76:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_70
					X4PIXEL13_60
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL21_0
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 10:
				case 138:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
						X4PIXEL11_0
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 66:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 24:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 7:
				case 39:
				case 135:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 148:
				case 149:
				case 180:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 224:
				case 228:
				case 225:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 41:
				case 169:
				case 45:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 22:
				case 54:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_0
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 208:
				case 209:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 104:
				case 108:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_70
					X4PIXEL13_60
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 11:
				case 139:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 19:
				case 51:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL00_81
						X4PIXEL01_31
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL00_12
						X4PIXEL01_14
						X4PIXEL02_83
						X4PIXEL03_50
						X4PIXEL12_70
						X4PIXEL13_21
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 146:
				case 178:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
						X4PIXEL23_32
						X4PIXEL33_82
					}
					else
					{
						X4PIXEL02_21
						X4PIXEL03_50
						X4PIXEL12_70
						X4PIXEL13_83
						X4PIXEL23_13
						X4PIXEL33_11
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_32
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_82
					break;
				}
				case 84:
				case 85:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_81
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL03_81
						X4PIXEL13_31
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL03_12
						X4PIXEL13_14
						X4PIXEL22_70
						X4PIXEL23_83
						X4PIXEL32_21
						X4PIXEL33_50
					}
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_31
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 112:
				case 113:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_82
					X4PIXEL21_32
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL30_82
						X4PIXEL31_32
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_70
						X4PIXEL23_21
						X4PIXEL30_11
						X4PIXEL31_13
						X4PIXEL32_83
						X4PIXEL33_50
					}
					break;
				}
				case 200:
				case 204:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_70
					X4PIXEL13_60
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
						X4PIXEL32_31
						X4PIXEL33_81
					}
					else
					{
						X4PIXEL20_21
						X4PIXEL21_70
						X4PIXEL30_50
						X4PIXEL31_83
						X4PIXEL32_14
						X4PIXEL33_12
					}
					X4PIXEL22_31
					X4PIXEL23_81
					break;
				}
				case 73:
				case 77:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL00_82
						X4PIXEL10_32
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL00_11
						X4PIXEL10_13
						X4PIXEL20_83
						X4PIXEL21_70
						X4PIXEL30_50
						X4PIXEL31_21
					}
					X4PIXEL01_82
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL11_32
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 42:
				case 170:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
						X4PIXEL20_31
						X4PIXEL30_81
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_21
						X4PIXEL10_83
						X4PIXEL11_70
						X4PIXEL20_14
						X4PIXEL30_12
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL21_31
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL31_81
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 14:
				case 142:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL02_32
						X4PIXEL03_82
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_83
						X4PIXEL02_13
						X4PIXEL03_11
						X4PIXEL10_21
						X4PIXEL11_70
					}
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 67:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 70:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 28:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 152:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 194:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 98:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 56:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 25:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 26:
				case 31:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL11_0
					X4PIXEL12_0
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 82:
				case 214:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_0
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 88:
				case 248:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					break;
				}
				case 74:
				case 107:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_61
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 27:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 86:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_0
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 216:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 106:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_61
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 30:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_0
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 210:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 120:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 75:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 29:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 198:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 184:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 99:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 57:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 71:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 156:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 226:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 60:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 195:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 102:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 153:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 58:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 83:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL20_61
					X4PIXEL21_30
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 92:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_31
					X4PIXEL13_31
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					break;
				}
				case 202:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL12_30
					X4PIXEL13_61
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 78:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL12_32
					X4PIXEL13_82
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 154:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 114:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL20_82
					X4PIXEL21_32
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					X4PIXEL30_82
					X4PIXEL31_32
					break;
				}
				case 89:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_30
					X4PIXEL13_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					break;
				}
				case 90:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					break;
				}
				case 55:
				case 23:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL00_81
						X4PIXEL01_31
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL12_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL00_12
						X4PIXEL01_14
						X4PIXEL02_83
						X4PIXEL03_50
						X4PIXEL12_70
						X4PIXEL13_21
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 182:
				case 150:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL12_0
						X4PIXEL13_0
						X4PIXEL23_32
						X4PIXEL33_82
					}
					else
					{
						X4PIXEL02_21
						X4PIXEL03_50
						X4PIXEL12_70
						X4PIXEL13_83
						X4PIXEL23_13
						X4PIXEL33_11
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_32
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_82
					break;
				}
				case 213:
				case 212:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_81
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL03_81
						X4PIXEL13_31
						X4PIXEL22_0
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL03_12
						X4PIXEL13_14
						X4PIXEL22_70
						X4PIXEL23_83
						X4PIXEL32_21
						X4PIXEL33_50
					}
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_31
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 241:
				case 240:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_82
					X4PIXEL21_32
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_0
						X4PIXEL23_0
						X4PIXEL30_82
						X4PIXEL31_32
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL22_70
						X4PIXEL23_21
						X4PIXEL30_11
						X4PIXEL31_13
						X4PIXEL32_83
						X4PIXEL33_50
					}
					break;
				}
				case 236:
				case 232:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_70
					X4PIXEL13_60
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL21_0
						X4PIXEL30_0
						X4PIXEL31_0
						X4PIXEL32_31
						X4PIXEL33_81
					}
					else
					{
						X4PIXEL20_21
						X4PIXEL21_70
						X4PIXEL30_50
						X4PIXEL31_83
						X4PIXEL32_14
						X4PIXEL33_12
					}
					X4PIXEL22_31
					X4PIXEL23_81
					break;
				}
				case 109:
				case 105:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL00_82
						X4PIXEL10_32
						X4PIXEL20_0
						X4PIXEL21_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL00_11
						X4PIXEL10_13
						X4PIXEL20_83
						X4PIXEL21_70
						X4PIXEL30_50
						X4PIXEL31_21
					}
					X4PIXEL01_82
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL11_32
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 171:
				case 43:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
						X4PIXEL11_0
						X4PIXEL20_31
						X4PIXEL30_81
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_21
						X4PIXEL10_83
						X4PIXEL11_70
						X4PIXEL20_14
						X4PIXEL30_12
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL21_31
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL31_81
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 143:
				case 15:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL02_32
						X4PIXEL03_82
						X4PIXEL10_0
						X4PIXEL11_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_83
						X4PIXEL02_13
						X4PIXEL03_11
						X4PIXEL10_21
						X4PIXEL11_70
					}
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 124:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_31
					X4PIXEL13_31
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 203:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 62:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_0
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 211:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 118:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_0
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 217:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 110:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_32
					X4PIXEL13_82
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 155:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 188:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 185:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 61:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 157:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 103:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 227:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 230:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 199:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 220:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_31
					X4PIXEL13_31
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					break;
				}
				case 158:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL12_0
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 234:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL12_30
					X4PIXEL13_61
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 242:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_82
					X4PIXEL31_32
					break;
				}
				case 59:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL11_0
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 121:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_30
					X4PIXEL13_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					break;
				}
				case 87:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_0
					X4PIXEL20_61
					X4PIXEL21_30
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 79:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL11_0
					X4PIXEL12_32
					X4PIXEL13_82
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 122:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					break;
				}
				case 94:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL12_0
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					break;
				}
				case 218:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					break;
				}
				case 91:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL11_0
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					break;
				}
				case 229:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 167:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 173:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 181:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 186:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 115:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL20_82
					X4PIXEL21_32
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					X4PIXEL30_82
					X4PIXEL31_32
					break;
				}
				case 93:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_31
					X4PIXEL13_31
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					break;
				}
				case 206:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL12_32
					X4PIXEL13_82
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 205:
				case 201:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_70
					X4PIXEL13_60
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_10
						X4PIXEL21_30
						X4PIXEL30_80
						X4PIXEL31_10
					}
					else
					{
						X4PIXEL20_12
						X4PIXEL21_0
						X4PIXEL30_20
						X4PIXEL31_11
					}
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 174:
				case 46:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_80
						X4PIXEL01_10
						X4PIXEL10_10
						X4PIXEL11_30
					}
					else
					{
						X4PIXEL00_20
						X4PIXEL01_12
						X4PIXEL10_11
						X4PIXEL11_0
					}
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 179:
				case 147:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_10
						X4PIXEL03_80
						X4PIXEL12_30
						X4PIXEL13_10
					}
					else
					{
						X4PIXEL02_11
						X4PIXEL03_20
						X4PIXEL12_0
						X4PIXEL13_12
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 117:
				case 116:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_82
					X4PIXEL21_32
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_30
						X4PIXEL23_10
						X4PIXEL32_10
						X4PIXEL33_80
					}
					else
					{
						X4PIXEL22_0
						X4PIXEL23_11
						X4PIXEL32_12
						X4PIXEL33_20
					}
					X4PIXEL30_82
					X4PIXEL31_32
					break;
				}
				case 189:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 231:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 126:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_0
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 219:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 125:
				{
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL00_82
						X4PIXEL10_32
						X4PIXEL20_0
						X4PIXEL21_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL00_11
						X4PIXEL10_13
						X4PIXEL20_83
						X4PIXEL21_70
						X4PIXEL30_50
						X4PIXEL31_21
					}
					X4PIXEL01_82
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL11_32
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 221:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_81
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL03_81
						X4PIXEL13_31
						X4PIXEL22_0
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL03_12
						X4PIXEL13_14
						X4PIXEL22_70
						X4PIXEL23_83
						X4PIXEL32_21
						X4PIXEL33_50
					}
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_31
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 207:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL02_32
						X4PIXEL03_82
						X4PIXEL10_0
						X4PIXEL11_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_83
						X4PIXEL02_13
						X4PIXEL03_11
						X4PIXEL10_21
						X4PIXEL11_70
					}
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_31
					X4PIXEL23_81
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 238:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_32
					X4PIXEL13_82
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL21_0
						X4PIXEL30_0
						X4PIXEL31_0
						X4PIXEL32_31
						X4PIXEL33_81
					}
					else
					{
						X4PIXEL20_21
						X4PIXEL21_70
						X4PIXEL30_50
						X4PIXEL31_83
						X4PIXEL32_14
						X4PIXEL33_12
					}
					X4PIXEL22_31
					X4PIXEL23_81
					break;
				}
				case 190:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL12_0
						X4PIXEL13_0
						X4PIXEL23_32
						X4PIXEL33_82
					}
					else
					{
						X4PIXEL02_21
						X4PIXEL03_50
						X4PIXEL12_70
						X4PIXEL13_83
						X4PIXEL23_13
						X4PIXEL33_11
					}
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_32
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_82
					break;
				}
				case 187:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
						X4PIXEL11_0
						X4PIXEL20_31
						X4PIXEL30_81
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_21
						X4PIXEL10_83
						X4PIXEL11_70
						X4PIXEL20_14
						X4PIXEL30_12
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL21_31
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL31_81
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 243:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_82
					X4PIXEL21_32
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL22_0
						X4PIXEL23_0
						X4PIXEL30_82
						X4PIXEL31_32
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL22_70
						X4PIXEL23_21
						X4PIXEL30_11
						X4PIXEL31_13
						X4PIXEL32_83
						X4PIXEL33_50
					}
					break;
				}
				case 119:
				{
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL00_81
						X4PIXEL01_31
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL12_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL00_12
						X4PIXEL01_14
						X4PIXEL02_83
						X4PIXEL03_50
						X4PIXEL12_70
						X4PIXEL13_21
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 237:
				case 233:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_60
					X4PIXEL03_20
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_70
					X4PIXEL13_60
					X4PIXEL20_0
					X4PIXEL21_0
					X4PIXEL22_31
					X4PIXEL23_81
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL30_0
					}
					else
					{
						X4PIXEL30_20
					}
					X4PIXEL31_0
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 175:
				case 47:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
					}
					else
					{
						X4PIXEL00_20
					}
					X4PIXEL01_0
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_0
					X4PIXEL11_0
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_70
					X4PIXEL23_60
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_60
					X4PIXEL33_20
					break;
				}
				case 183:
				case 151:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL03_0
					}
					else
					{
						X4PIXEL03_20
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_0
					X4PIXEL13_0
					X4PIXEL20_60
					X4PIXEL21_70
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_20
					X4PIXEL31_60
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 245:
				case 244:
				{
					X4PIXEL00_20
					X4PIXEL01_60
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_60
					X4PIXEL11_70
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_0
					X4PIXEL23_0
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL33_20
					}
					break;
				}
				case 250:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_30
					X4PIXEL13_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					break;
				}
				case 123:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_10
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 95:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL11_0
					X4PIXEL12_0
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_80
					X4PIXEL31_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 222:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_0
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 252:
				{
					X4PIXEL00_80
					X4PIXEL01_61
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_31
					X4PIXEL13_31
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_0
					X4PIXEL23_0
					X4PIXEL32_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL33_20
					}
					break;
				}
				case 249:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_61
					X4PIXEL03_80
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_0
					X4PIXEL21_0
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL30_0
					}
					else
					{
						X4PIXEL30_20
					}
					X4PIXEL31_0
					break;
				}
				case 235:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_61
					X4PIXEL20_0
					X4PIXEL21_0
					X4PIXEL22_31
					X4PIXEL23_81
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL30_0
					}
					else
					{
						X4PIXEL30_20
					}
					X4PIXEL31_0
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 111:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
					}
					else
					{
						X4PIXEL00_20
					}
					X4PIXEL01_0
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_0
					X4PIXEL11_0
					X4PIXEL12_32
					X4PIXEL13_82
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_61
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 63:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
					}
					else
					{
						X4PIXEL00_20
					}
					X4PIXEL01_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_0
					X4PIXEL11_0
					X4PIXEL12_0
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_61
					X4PIXEL33_80
					break;
				}
				case 159:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL03_0
					}
					else
					{
						X4PIXEL03_20
					}
					X4PIXEL11_0
					X4PIXEL12_0
					X4PIXEL13_0
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_80
					X4PIXEL31_61
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 215:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL03_0
					}
					else
					{
						X4PIXEL03_20
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_0
					X4PIXEL13_0
					X4PIXEL20_61
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 246:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_61
					X4PIXEL11_30
					X4PIXEL12_0
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_0
					X4PIXEL23_0
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL33_20
					}
					break;
				}
				case 254:
				{
					X4PIXEL00_80
					X4PIXEL01_10
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_10
					X4PIXEL11_30
					X4PIXEL12_0
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_0
					X4PIXEL23_0
					X4PIXEL32_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL33_20
					}
					break;
				}
				case 253:
				{
					X4PIXEL00_82
					X4PIXEL01_82
					X4PIXEL02_81
					X4PIXEL03_81
					X4PIXEL10_32
					X4PIXEL11_32
					X4PIXEL12_31
					X4PIXEL13_31
					X4PIXEL20_0
					X4PIXEL21_0
					X4PIXEL22_0
					X4PIXEL23_0
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL30_0
					}
					else
					{
						X4PIXEL30_20
					}
					X4PIXEL31_0
					X4PIXEL32_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL33_20
					}
					break;
				}
				case 251:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_10
					X4PIXEL03_80
					X4PIXEL11_0
					X4PIXEL12_30
					X4PIXEL13_10
					X4PIXEL20_0
					X4PIXEL21_0
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL30_0
					}
					else
					{
						X4PIXEL30_20
					}
					X4PIXEL31_0
					break;
				}
				case 239:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
					}
					else
					{
						X4PIXEL00_20
					}
					X4PIXEL01_0
					X4PIXEL02_32
					X4PIXEL03_82
					X4PIXEL10_0
					X4PIXEL11_0
					X4PIXEL12_32
					X4PIXEL13_82
					X4PIXEL20_0
					X4PIXEL21_0
					X4PIXEL22_31
					X4PIXEL23_81
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL30_0
					}
					else
					{
						X4PIXEL30_20
					}
					X4PIXEL31_0
					X4PIXEL32_31
					X4PIXEL33_81
					break;
				}
				case 127:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
					}
					else
					{
						X4PIXEL00_20
					}
					X4PIXEL01_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL02_0
						X4PIXEL03_0
						X4PIXEL13_0
					}
					else
					{
						X4PIXEL02_50
						X4PIXEL03_50
						X4PIXEL13_50
					}
					X4PIXEL10_0
					X4PIXEL11_0
					X4PIXEL12_0
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL20_0
						X4PIXEL30_0
						X4PIXEL31_0
					}
					else
					{
						X4PIXEL20_50
						X4PIXEL30_50
						X4PIXEL31_50
					}
					X4PIXEL21_0
					X4PIXEL22_30
					X4PIXEL23_10
					X4PIXEL32_10
					X4PIXEL33_80
					break;
				}
				case 191:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
					}
					else
					{
						X4PIXEL00_20
					}
					X4PIXEL01_0
					X4PIXEL02_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL03_0
					}
					else
					{
						X4PIXEL03_20
					}
					X4PIXEL10_0
					X4PIXEL11_0
					X4PIXEL12_0
					X4PIXEL13_0
					X4PIXEL20_31
					X4PIXEL21_31
					X4PIXEL22_32
					X4PIXEL23_32
					X4PIXEL30_81
					X4PIXEL31_81
					X4PIXEL32_82
					X4PIXEL33_82
					break;
				}
				case 223:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
						X4PIXEL01_0
						X4PIXEL10_0
					}
					else
					{
						X4PIXEL00_50
						X4PIXEL01_50
						X4PIXEL10_50
					}
					X4PIXEL02_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL03_0
					}
					else
					{
						X4PIXEL03_20
					}
					X4PIXEL11_0
					X4PIXEL12_0
					X4PIXEL13_0
					X4PIXEL20_10
					X4PIXEL21_30
					X4PIXEL22_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL23_0
						X4PIXEL32_0
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL23_50
						X4PIXEL32_50
						X4PIXEL33_50
					}
					X4PIXEL30_80
					X4PIXEL31_10
					break;
				}
				case 247:
				{
					X4PIXEL00_81
					X4PIXEL01_31
					X4PIXEL02_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL03_0
					}
					else
					{
						X4PIXEL03_20
					}
					X4PIXEL10_81
					X4PIXEL11_31
					X4PIXEL12_0
					X4PIXEL13_0
					X4PIXEL20_82
					X4PIXEL21_32
					X4PIXEL22_0
					X4PIXEL23_0
					X4PIXEL30_82
					X4PIXEL31_32
					X4PIXEL32_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL33_20
					}
					break;
				}
				case 255:
				{
					if (Diff(RGBtoYUV[w4], RGBtoYUV[w2]))
					{
						X4PIXEL00_0
					}
					else
					{
						X4PIXEL00_20
					}
					X4PIXEL01_0
					X4PIXEL02_0
					if (Diff(RGBtoYUV[w2], RGBtoYUV[w6]))
					{
						X4PIXEL03_0
					}
					else
					{
						X4PIXEL03_20
					}
					X4PIXEL10_0
					X4PIXEL11_0
					X4PIXEL12_0
					X4PIXEL13_0
					X4PIXEL20_0
					X4PIXEL21_0
					X4PIXEL22_0
					X4PIXEL23_0
					if (Diff(RGBtoYUV[w8], RGBtoYUV[w4]))
					{
						X4PIXEL30_0
					}
					else
					{
						X4PIXEL30_20
					}
					X4PIXEL31_0
					X4PIXEL32_0
					if (Diff(RGBtoYUV[w6], RGBtoYUV[w8]))
					{
						X4PIXEL33_0
					}
					else
					{
						X4PIXEL33_20
					}
					break;
				}
			}

			w1 = w2; w4 = w5; w7 = w8;
			w2 = w3; w5 = w6; w8 = w9;

			dp += 4;
		}

		dp += (dst1line - width) * 4;
		sp += (src1line - width);
	}
}
