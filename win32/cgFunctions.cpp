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

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


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

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


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

HMODULE hCgDll = NULL;
HMODULE hCgD3D9Dll = NULL;
HMODULE hCgGLDll = NULL;

//cg.dll
CGCC cgCreateContext = NULL;
CGDC cgDestroyContext = NULL;
CGGNP cgGetNamedParameter = NULL;
CGGE cgGetError = NULL;
CGGES cgGetErrorString = NULL;
CGGLL cgGetLastListing = NULL;
CGCP cgCreateProgram = NULL;
CGDP cgDestroyProgram = NULL;
//cgD3D9.dll
CGD3DSD cgD3D9SetDevice = NULL;
CGD3DBP cgD3D9BindProgram = NULL;
CGD3DGLVP cgD3D9GetLatestVertexProfile = NULL;
CGD3DGLPP cgD3D9GetLatestPixelProfile = NULL;
CGD3DGOO cgD3D9GetOptimalOptions = NULL;
CGD3DLP cgD3D9LoadProgram = NULL;
CGD3DSUM cgD3D9SetUniformMatrix = NULL;
CGD3DSU cgD3D9SetUniform = NULL;
//cggl.dll
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

bool loadCgFunctions()
{
	if(hCgDll && hCgD3D9Dll && hCgGLDll)
		return true;

	hCgDll = LoadLibrary(TEXT("cg.dll"));
	if(hCgDll) {
		hCgD3D9Dll = LoadLibrary(TEXT("cgD3D9.dll"));
		hCgGLDll = LoadLibrary(TEXT("cgGL.dll"));
	}

	if(!hCgDll || !hCgD3D9Dll || !hCgGLDll) {
		unloadCgLibrary();
		return false;
	}

	//cg.dll
	cgCreateContext = (CGCC)GetProcAddress(hCgDll,"cgCreateContext");
	cgDestroyContext = (CGDC)GetProcAddress(hCgDll,"cgDestroyContext");
	cgGetNamedParameter = (CGGNP)GetProcAddress(hCgDll,"cgGetNamedParameter");
	cgGetError = (CGGE)GetProcAddress(hCgDll,"cgGetError");
	cgGetErrorString = (CGGES)GetProcAddress(hCgDll,"cgGetErrorString");
	cgGetLastListing = (CGGLL)GetProcAddress(hCgDll,"cgGetLastListing");
	cgCreateProgram = (CGCP)GetProcAddress(hCgDll,"cgCreateProgram");
	cgDestroyProgram = (CGDP)GetProcAddress(hCgDll,"cgDestroyProgram");
	//cgD3D9.dll
	cgD3D9SetDevice = (CGD3DSD)GetProcAddress(hCgD3D9Dll,"cgD3D9SetDevice");
	cgD3D9BindProgram = (CGD3DBP)GetProcAddress(hCgD3D9Dll,"cgD3D9BindProgram");
	cgD3D9GetLatestVertexProfile = (CGD3DGLVP)GetProcAddress(hCgD3D9Dll,"cgD3D9GetLatestVertexProfile");
	cgD3D9GetLatestPixelProfile = (CGD3DGLPP)GetProcAddress(hCgD3D9Dll,"cgD3D9GetLatestPixelProfile");
	cgD3D9GetOptimalOptions = (CGD3DGOO)GetProcAddress(hCgD3D9Dll,"cgD3D9GetOptimalOptions");
	cgD3D9LoadProgram = (CGD3DLP)GetProcAddress(hCgD3D9Dll,"cgD3D9LoadProgram");
	cgD3D9SetUniformMatrix = (CGD3DSUM)GetProcAddress(hCgD3D9Dll,"cgD3D9SetUniformMatrix");
	cgD3D9SetUniform = (CGD3DSU)GetProcAddress(hCgD3D9Dll,"cgD3D9SetUniform");
	//cggl.dll
	cgGLSetStateMatrixParameter = (CGGLSSMP)GetProcAddress(hCgGLDll,"cgGLSetStateMatrixParameter");
	cgGLSetParameter2fv = (CGGLSP2FV)GetProcAddress(hCgGLDll,"cgGLSetParameter2fv");
	cgGLSetParameter1f = (CGGLSP1F)GetProcAddress(hCgGLDll,"cgGLSetParameter1f");
	cgGLGetLatestProfile = (CGGLGLP)GetProcAddress(hCgGLDll,"cgGLGetLatestProfile");
	cgGLEnableProfile = (CGGLEP)GetProcAddress(hCgGLDll,"cgGLEnableProfile");
	cgGLDisableProfile = (CGGLDP)GetProcAddress(hCgGLDll,"cgGLDisableProfile");
	cgGLSetOptimalOptions = (CGGLSOO)GetProcAddress(hCgGLDll,"cgGLSetOptimalOptions");
	cgGLLoadProgram = (CGGLLP)GetProcAddress(hCgGLDll,"cgGLLoadProgram");
	cgGLBindProgram = (CGGLBP)GetProcAddress(hCgGLDll,"cgGLBindProgram");
	cgGLSetTextureParameter = (CGGLSTP)GetProcAddress(hCgGLDll,"cgGLSetTextureParameter");
	cgGLEnableTextureParameter = (CGGLETP)GetProcAddress(hCgGLDll,"cgGLEnableTextureParameter");
	cgGLSetParameterPointer = (CGGLSPP)GetProcAddress(hCgGLDll,"cgGLSetParameterPointer");
	cgGLEnableClientState = (CGGLECS)GetProcAddress(hCgGLDll,"cgGLEnableClientState");

	if(
		//cg.dll
		!cgCreateContext ||
		!cgDestroyContext ||
		!cgGetNamedParameter ||
		!cgGetError ||
		!cgGetErrorString ||
		!cgGetLastListing ||
		!cgCreateProgram ||
		!cgDestroyProgram ||
		//cgD3D9.dll
		!cgD3D9SetDevice ||
		!cgD3D9BindProgram ||
		!cgD3D9GetLatestVertexProfile ||
		!cgD3D9GetLatestPixelProfile ||
		!cgD3D9GetOptimalOptions ||
		!cgD3D9LoadProgram ||
		!cgD3D9SetUniformMatrix ||
		!cgD3D9SetUniform ||
		//cggl.dll
		!cgGLSetStateMatrixParameter ||
		!cgGLSetParameter2fv ||
		!cgGLSetParameter1f ||
		!cgGLGetLatestProfile ||
		!cgGLEnableProfile ||
		!cgGLDisableProfile ||
		!cgGLSetOptimalOptions ||
		!cgGLLoadProgram ||
		!cgGLBindProgram ||
		!cgGLSetTextureParameter ||
		!cgGLEnableTextureParameter ||
		!cgGLSetParameterPointer ||
		!cgGLEnableClientState) {
			unloadCgLibrary();
			return false;
	}
		
	return true;
}

void unloadCgLibrary()
{
	if(hCgDll)
		FreeLibrary(hCgDll);
	if(hCgD3D9Dll)
		FreeLibrary(hCgD3D9Dll);
	if(hCgGLDll)
		FreeLibrary(hCgGLDll);

	hCgDll = hCgD3D9Dll = hCgGLDll = NULL;

	//cg.dll
	cgCreateContext = NULL;
	cgDestroyContext = NULL;
	cgGetNamedParameter = NULL;
	cgGetError = NULL;
	cgGetErrorString = NULL;
	cgGetLastListing = NULL;
	cgCreateProgram = NULL;
	cgDestroyProgram = NULL;
	//cgD3D9.dll
	cgD3D9SetDevice = NULL;
	cgD3D9BindProgram = NULL;
	cgD3D9GetLatestVertexProfile = NULL;
	cgD3D9GetLatestPixelProfile = NULL;
	cgD3D9GetOptimalOptions = NULL;
	cgD3D9LoadProgram = NULL;
	cgD3D9SetUniformMatrix = NULL;
	cgD3D9SetUniform = NULL;
	//cggl.dll
	cgGLSetStateMatrixParameter = NULL;
	cgGLSetParameter2fv = NULL;
	cgGLGetLatestProfile = NULL;
	cgGLEnableProfile = NULL;
	cgGLDisableProfile = NULL;
	cgGLSetOptimalOptions = NULL;
	cgGLLoadProgram = NULL;
	cgGLBindProgram = NULL;
}
