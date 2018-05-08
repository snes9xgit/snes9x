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

  (c) Copyright 2009 - 2017  BearOso,
                             OV2

  (c) Copyright 2017         qwertymodo

  (c) Copyright 2011 - 2017  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be
 given)


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
                             Lord Nightmare
 (lord_nightmare@users.sourceforge.net), Matthew Kendora, neviksti

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
  (c) Copyright 2004 - 2017  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2017  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones

  Libretro port
  (c) Copyright 2011 - 2017  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be
 given)


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

#include "cgFunctions.h"
#include <dlfcn.h>
#include <stdio.h>

void* hCgDll = NULL;
void* hCgGLDll = NULL;

// cg.dll
CGCC cgCreateContext = NULL;
CGDC cgDestroyContext = NULL;
CGGNP cgGetNamedParameter = NULL;
CGGE cgGetError = NULL;
CGGES cgGetErrorString = NULL;
CGGLL cgGetLastListing = NULL;
CGCP cgCreateProgram = NULL;
CGDP cgDestroyProgram = NULL;
CGGPRI cgGetParameterResourceIndex = NULL;
CGGFP cgGetFirstParameter = NULL;
CGGNEP cgGetNextParameter = NULL;
CGGPD cgGetParameterDirection = NULL;
CGGPS cgGetParameterSemantic = NULL;
CGGRS cgGetResourceString = NULL;
CGGPV cgGetParameterVariability = NULL;
CGGPT cgGetParameterType = NULL;
CGGFSP cgGetFirstStructParameter = NULL;
CGGPN cgGetParameterName = NULL;
// cggl.dll
CGGLSSMP cgGLSetStateMatrixParameter = NULL;
CGGLSP2FV cgGLSetParameter2fv = NULL;
CGGLSP1F cgGLSetParameter1f = NULL;
CGGLGLP cgGLGetLatestProfile = NULL;
CGGLEP cgGLEnableProfile = NULL;
CGGLDP cgGLDisableProfile = NULL;
CGGLSOO cgGLSetOptimalOptions = NULL;
CGGLLP cgGLLoadProgram = NULL;
CGGLBP cgGLBindProgram = NULL;
CGGLSTP cgGLSetTextureParameter = NULL;
CGGLETP cgGLEnableTextureParameter = NULL;
CGGLSPP cgGLSetParameterPointer = NULL;
CGGLECS cgGLEnableClientState = NULL;
CGGLDCS cgGLDisableClientState = NULL;

bool loadCgFunctions()
{
    if (hCgDll && hCgGLDll)
        return true;

    hCgDll = dlopen("libCg.so", RTLD_LAZY);
    if (hCgDll)
    {
        hCgGLDll = dlopen("libCgGL.so", RTLD_LAZY);
    }

    if (!hCgDll || !hCgGLDll)
    {
        unloadCgLibrary();
        return false;
    }
    // cg.dll
    cgCreateContext = (CGCC)dlsym(hCgDll, "cgCreateContext");
    cgDestroyContext = (CGDC)dlsym(hCgDll, "cgDestroyContext");
    cgGetNamedParameter = (CGGNP)dlsym(hCgDll, "cgGetNamedParameter");
    cgGetError = (CGGE)dlsym(hCgDll, "cgGetError");
    cgGetErrorString = (CGGES)dlsym(hCgDll, "cgGetErrorString");
    cgGetLastListing = (CGGLL)dlsym(hCgDll, "cgGetLastListing");
    cgCreateProgram = (CGCP)dlsym(hCgDll, "cgCreateProgram");
    cgDestroyProgram = (CGDP)dlsym(hCgDll, "cgDestroyProgram");
    cgGetParameterResourceIndex = (CGGPRI)dlsym(hCgDll, "cgGetParameterResourceIndex");
    cgGetFirstParameter = (CGGFP)dlsym(hCgDll, "cgGetFirstParameter");
    cgGetNextParameter = (CGGNEP)dlsym(hCgDll, "cgGetNextParameter");
    cgGetParameterDirection = (CGGPD)dlsym(hCgDll, "cgGetParameterDirection");
    cgGetParameterSemantic = (CGGPS)dlsym(hCgDll, "cgGetParameterSemantic");
    cgGetResourceString = (CGGRS)dlsym(hCgDll, "cgGetResourceString");
    cgGetParameterVariability = (CGGPV)dlsym(hCgDll, "cgGetParameterVariability");
    cgGetParameterType = (CGGPT)dlsym(hCgDll, "cgGetParameterType");
    cgGetFirstStructParameter = (CGGFSP)dlsym(hCgDll, "cgGetFirstStructParameter");
    cgGetParameterName = (CGGPN)dlsym(hCgDll, "cgGetParameterName");
    // cggl.dll
    cgGLSetStateMatrixParameter = (CGGLSSMP)dlsym(hCgGLDll, "cgGLSetStateMatrixParameter");
    cgGLSetParameter2fv = (CGGLSP2FV)dlsym(hCgGLDll, "cgGLSetParameter2fv");
    cgGLSetParameter1f = (CGGLSP1F)dlsym(hCgGLDll, "cgGLSetParameter1f");
    cgGLGetLatestProfile = (CGGLGLP)dlsym(hCgGLDll, "cgGLGetLatestProfile");
    cgGLEnableProfile = (CGGLEP)dlsym(hCgGLDll, "cgGLEnableProfile");
    cgGLDisableProfile = (CGGLDP)dlsym(hCgGLDll, "cgGLDisableProfile");
    cgGLSetOptimalOptions = (CGGLSOO)dlsym(hCgGLDll, "cgGLSetOptimalOptions");
    cgGLLoadProgram = (CGGLLP)dlsym(hCgGLDll, "cgGLLoadProgram");
    cgGLBindProgram = (CGGLBP)dlsym(hCgGLDll, "cgGLBindProgram");
    cgGLSetTextureParameter = (CGGLSTP)dlsym(hCgGLDll, "cgGLSetTextureParameter");
    cgGLEnableTextureParameter = (CGGLETP)dlsym(hCgGLDll, "cgGLEnableTextureParameter");
    cgGLSetParameterPointer = (CGGLSPP)dlsym(hCgGLDll, "cgGLSetParameterPointer");
    cgGLEnableClientState = (CGGLECS)dlsym(hCgGLDll, "cgGLEnableClientState");
    cgGLDisableClientState = (CGGLDCS)dlsym(hCgGLDll, "cgGLDisableClientState");

    if (
        // cg.dll
        !cgCreateContext || !cgDestroyContext || !cgGetNamedParameter || !cgGetError || !cgGetErrorString || !cgGetLastListing || !cgCreateProgram || !cgDestroyProgram || !cgGetParameterResourceIndex || !cgGetFirstParameter || !cgGetNextParameter || !cgGetParameterDirection || !cgGetParameterSemantic || !cgGetResourceString || !cgGetParameterVariability || !cgGetParameterType || !cgGetFirstStructParameter || !cgGetParameterName ||
        // cggl.dll
        !cgGLSetStateMatrixParameter || !cgGLSetParameter2fv || !cgGLSetParameter1f || !cgGLGetLatestProfile || !cgGLEnableProfile || !cgGLDisableProfile || !cgGLSetOptimalOptions || !cgGLLoadProgram || !cgGLBindProgram || !cgGLSetTextureParameter || !cgGLEnableTextureParameter || !cgGLSetParameterPointer || !cgGLEnableClientState || !cgGLDisableClientState)
    {
        unloadCgLibrary();
        return false;
    }

    return true;
}

void unloadCgLibrary()
{
    if (hCgDll)
        dlclose(hCgDll);
    if (hCgGLDll)
        dlclose(hCgGLDll);

    hCgDll = hCgGLDll = NULL;

    // cg.dll
    cgCreateContext = NULL;
    cgDestroyContext = NULL;
    cgGetNamedParameter = NULL;
    cgGetError = NULL;
    cgGetErrorString = NULL;
    cgGetLastListing = NULL;
    cgCreateProgram = NULL;
    cgDestroyProgram = NULL;
    cgGetParameterResourceIndex = NULL;
    cgGetFirstParameter = NULL;
    cgGetNextParameter = NULL;
    cgGetParameterDirection = NULL;
    cgGetParameterSemantic = NULL;
    cgGetResourceString = NULL;
    cgGetParameterVariability = NULL;
    cgGetParameterType = NULL;
    cgGetFirstStructParameter = NULL;
    cgGetParameterName = NULL;
    // cggl.dll
    cgGLSetStateMatrixParameter = NULL;
    cgGLSetParameter2fv = NULL;
    cgGLGetLatestProfile = NULL;
    cgGLEnableProfile = NULL;
    cgGLDisableProfile = NULL;
    cgGLSetOptimalOptions = NULL;
    cgGLLoadProgram = NULL;
    cgGLBindProgram = NULL;
    cgGLEnableClientState = NULL;
    cgGLDisableClientState = NULL;
}
