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
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2010  BearOso,
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
  (c) Copyright 2004 - 2010  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2010  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2010  zones


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



#include <windows.h>
#include "_tfwopen.h"

Utf8ToWide::Utf8ToWide(const char *utf8Chars) {
	int requiredChars = MultiByteToWideChar(CP_UTF8,0,utf8Chars,-1,wideChars,0);
	wideChars = new wchar_t[requiredChars];
	MultiByteToWideChar(CP_UTF8,0,utf8Chars,-1,wideChars,requiredChars);
}

WideToUtf8::WideToUtf8(const wchar_t *wideChars) {
	int requiredChars = WideCharToMultiByte(CP_UTF8,0,wideChars,-1,utf8Chars,0,NULL,NULL);
	utf8Chars = new char[requiredChars];
	WideCharToMultiByte(CP_UTF8,0,wideChars,-1,utf8Chars,requiredChars,NULL,NULL);
}

CPToWide::CPToWide(const char *chars, unsigned int cp) {
   int requiredChars = MultiByteToWideChar(cp,0,chars,-1,wideChars,0);
   wideChars = new wchar_t[requiredChars];
   MultiByteToWideChar(cp,0,chars,-1,wideChars,requiredChars);
}

WideToCP::WideToCP(const wchar_t *wideChars, unsigned int cp) {
	int requiredChars = WideCharToMultiByte(cp,0,wideChars,-1,cpchars,0,NULL,NULL);
	cpchars = new char[requiredChars];
	WideCharToMultiByte(cp,0,wideChars,-1,cpchars,requiredChars,NULL,NULL);
}

#ifdef UNICODE

extern "C" FILE *_tfwopen(const char *filename, const char *mode ) {
	wchar_t mode_w[30];
	lstrcpyn(mode_w,Utf8ToWide(mode),29);
	mode_w[29]=L'\0';
	return _wfopen(Utf8ToWide(filename),mode_w);
}

extern "C" int _twremove(const char *filename ) {
	return _wremove(Utf8ToWide(filename));
}

extern "C" int _twopen(const char *filename, int oflag, int pmode) {
	return _wopen(Utf8ToWide(filename), oflag, pmode);
}

extern "C" int _twaccess(const char *_Filename, int _AccessMode) {
	return _waccess(Utf8ToWide(_Filename), _AccessMode);
}

extern "C" int _twrename(const char *_OldFilename, const char *_NewFilename) {
	return _wrename(Utf8ToWide(_OldFilename), Utf8ToWide(_NewFilename));
}

extern "C" int _twunlink(const char *_Filename) {
	return _wunlink(Utf8ToWide(_Filename));
}

extern "C" int _twchdir(const char *_Path) {
	return _wchdir(Utf8ToWide(_Path));
}

extern "C" int _twmkdir(const char *_Path) {
	return _wmkdir(Utf8ToWide(_Path));
}

extern "C" int _twrmdir(const char *_Path) {
	return _wrmdir(Utf8ToWide(_Path));
}

extern "C" void _twsplitpath(const char *_FullPath, char *_Drive, char *_Dir, char *_Filename, char *_Ext) {
	wchar_t _wDrive[_MAX_PATH];
	wchar_t _wDir[_MAX_PATH];
	wchar_t _wFilename[_MAX_PATH];
	wchar_t _wExt[_MAX_PATH];
	_wsplitpath(Utf8ToWide(_FullPath), _wDrive, _wDir, _wFilename, _wExt);
	strcpy(_Drive, WideToUtf8(_wDrive));
	strcpy(_Dir, WideToUtf8(_wDir));
	strcpy(_Filename, WideToUtf8(_wFilename));
	strcpy(_Ext, WideToUtf8(_wExt));
}

extern "C" void _twmakepath(char *_Path, const char *_Drive, const char *_Dir, const char *_Filename, const char *_Ext) {
	wchar_t wResultPath[_MAX_PATH];
	wcscpy(wResultPath, L"");
	_wmakepath(wResultPath, Utf8ToWide(_Drive), Utf8ToWide(_Dir), Utf8ToWide(_Filename), Utf8ToWide(_Ext));
	strcpy(_Path, WideToUtf8(wResultPath));
}

extern "C" char *_twcsrchr(const char *_Str, int _Ch) {
	wchar_t *wStr = Utf8ToWide(_Str);
	wchar_t *wResult = wcsrchr(wStr, (wchar_t) _Ch);
	if (wResult != NULL)
	{
		wResult[0] = L'\0';
		return (char*) &_Str[strlen(WideToUtf8(wResult))];
	}
	else
	{
		return NULL;
	}
}

#endif // UNICODE
