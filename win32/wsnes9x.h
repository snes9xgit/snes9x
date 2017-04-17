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





/*****************************************************************************/
/*  Snes9X: Win32                                                            */
/*****************************************************************************/
#if !defined(SNES9X_H_INCLUDED)
#define SNES9X_H_INCLUDED
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <ddraw.h>
#include <mmsystem.h>
#ifndef __BORLANDC__

#include <dsound.h>
#endif
#include "rsrc/resource.h"

#define COUNT(a) (sizeof (a) / sizeof (a[0]))
#define GUI_VERSION 1008

#define MAX_RECENT_GAMES_LIST_SIZE 32
#define MAX_RECENT_HOSTS_LIST_SIZE 16

#include "_tfwopen.h"
#ifdef UNICODE
#define _tToChar WideToUtf8
#define _tFromChar Utf8ToWide
#define _tFromMS932(x) CPToWide(x,932)
#else
#define _tToChar
#define _tFromChar
#define _tFromMS932
#endif

/****************************************************************************/
inline static void Log (const char *str)
{
    FILE *fs = fopen ("snes9x.log", "a");

    if (fs)
    {
      fprintf (fs, "%s\n", str);
      fflush (fs);
      fclose (fs);
    }

}

enum RenderFilter{
	FILTER_NONE = 0,
	FILTER_SIMPLE1X,

	FILTER_SIMPLE2X,
	FILTER_SCANLINES,
	FILTER_TVMODE,
	FILTER_BLARGGCOMP,
	FILTER_BLARGGSVID,
	FILTER_BLARGGRGB,
	FILTER_SUPEREAGLE,
	FILTER_SUPER2XSAI,
	FILTER_2XSAI,
	FILTER_HQ2X,
	FILTER_HQ2XS,
	FILTER_HQ2XBOLD,
	FILTER_EPXA,
	FILTER_EPXB,
	FILTER_EPXC,

	FILTER_SIMPLE3X,
	FILTER_TVMODE3X,
	FILTER_DOTMATRIX3X,
	FILTER_HQ3X,
	FILTER_HQ3XS,
	FILTER_HQ3XBOLD,
	FILTER_LQ3XBOLD,
	FILTER_EPX3,

	FILTER_SIMPLE4X,
	FILTER_HQ4X,

    FILTER_2XBRZ,
    FILTER_3XBRZ,
    FILTER_4XBRZ,
	FILTER_5XBRZ,
	FILTER_6XBRZ,

	NUM_FILTERS
};

enum OutputMethod {
	DIRECTDRAW = 0,
	DIRECT3D,
	OPENGL
};

struct dMode
{
	long height;
	long width;
	long depth;
	long rate;
};

struct sGUI {
    HWND hWnd;
    HMENU hMenu;
    HINSTANCE hInstance;

    DWORD hFrameTimer;
    DWORD hHotkeyTimer;
    HANDLE ClientSemaphore;
    HANDLE FrameTimerSemaphore;
    HANDLE ServerTimerSemaphore;

    BYTE Language;

	//Graphic Settings
	dMode FullscreenMode;
    RenderFilter Scale;
    RenderFilter ScaleHiRes;
	bool BlendHiRes;
	bool AVIHiRes;
    bool DoubleBuffered;
    bool FullScreen;
    bool Stretch;
    bool HeightExtend;
    bool AspectRatio;
	OutputMethod outputMethod;
	int AspectWidth;
	bool AlwaysCenterImage;
	bool EmulateFullscreen;
	bool EmulatedFullscreen;
	bool BilinearFilter;
	bool LocalVidMem;
	bool Vsync;	
	bool shaderEnabled;
	TCHAR D3DshaderFileName[MAX_PATH];
	TCHAR OGLshaderFileName[MAX_PATH];

	bool OGLdisablePBOs;

    bool IgnoreNextMouseMove;
    RECT window_size;
	bool window_maximized;
    int  MouseX;
    int  MouseY;
    unsigned int MouseButtons;
	int superscope_turbo;
	int superscope_pause;
	int FrameAdvanceJustPressed;
    HCURSOR Blank;
    HCURSOR GunSight;
    HCURSOR Arrow;
    int CursorTimer;
    HDC  hDC;
    HACCEL Accelerators;
    bool NeedDepthConvert;
    bool DepthConverted;

	bool InactivePause;
	bool CustomRomOpen;
    bool FASkipsNonInput;
    bool FAMute;
    int  ScreenDepth;
    int  RedShift;
    int  GreenShift;
    int  BlueShift;
    int  ControlForced;
	int  CurrentSaveSlot;
    int  MaxRecentGames;
	int  ControllerOption;
	int  ValidControllerOptions;
	int  SoundChannelEnable;
	bool BackgroundInput;
	bool JoystickHotkeys;
	bool MovieClearSRAM;
	bool MovieStartFromReset;
	bool MovieReadOnly;
	bool NetplayUseJoypad1;
    unsigned int FlipCounter;
    unsigned int NumFlipFrames;

	int SoundDriver;
	int SoundBufferSize;
	bool Mute;
	// used for sync sound synchronization
	CRITICAL_SECTION SoundCritSect;
    HANDLE SoundSyncEvent;

    TCHAR RomDir [_MAX_PATH];
    TCHAR ScreensDir [_MAX_PATH];
    TCHAR MovieDir [_MAX_PATH];
    TCHAR SPCDir [_MAX_PATH];
    TCHAR FreezeFileDir [_MAX_PATH];
    TCHAR SRAMFileDir [_MAX_PATH];
    TCHAR PatchDir [_MAX_PATH];
    TCHAR BiosDir [_MAX_PATH];
	TCHAR SatDir [_MAX_PATH];
	bool LockDirectories;

    TCHAR RecentGames [MAX_RECENT_GAMES_LIST_SIZE][MAX_PATH];
    TCHAR RecentHostNames [MAX_RECENT_HOSTS_LIST_SIZE][MAX_PATH];

	//turbo switches -- SNES-wide
	unsigned short TurboMask;
	COLORREF InfoColor;
	bool HideMenu;
	
	// avi writing
	struct AVIFile* AVIOut;
	
	long FrameCount;
    long LastFrameCount;
    unsigned long IdleCount;

    // rewinding
    bool rewinding;
    unsigned int rewindBufferSize;
    unsigned int rewindGranularity;
};

//TURBO masks
#define TURBO_A_MASK 0x0001
#define TURBO_B_MASK 0x0002
#define TURBO_X_MASK 0x0004
#define TURBO_Y_MASK 0x0008
#define TURBO_L_MASK 0x0010
#define TURBO_R_MASK 0x0020
#define TURBO_STA_MASK 0x0040
#define TURBO_SEL_MASK 0x0080
#define TURBO_LEFT_MASK 0x0100
#define TURBO_UP_MASK 0x0200
#define TURBO_RIGHT_MASK 0x0400
#define TURBO_DOWN_MASK 0x0800

struct sLanguages {
    int idMenu;
    TCHAR *errInitDD;
    TCHAR *errModeDD;
    TCHAR *errInitDS;
    TCHAR *ApplyNeedRestart;
    TCHAR *errFrameTimer;
};

#define CUSTKEY_ALT_MASK   0x01
#define CUSTKEY_CTRL_MASK  0x02
#define CUSTKEY_SHIFT_MASK 0x04

struct SCustomKey {
	WORD key;
	WORD modifiers;
};

struct SCustomKeys {
	SCustomKey SpeedUp;
	SCustomKey SpeedDown;
	SCustomKey Pause;
	SCustomKey FrameAdvance;
	SCustomKey SkipUp;
	SCustomKey SkipDown;
	SCustomKey ScopeTurbo;
	SCustomKey ScopePause;
	SCustomKey FrameCount;
	SCustomKey ReadOnly;
	SCustomKey Save [10];
	SCustomKey Load [10];
	SCustomKey FastForward;
	SCustomKey FastForwardToggle;
	SCustomKey ShowPressed;
	SCustomKey SaveScreenShot;
	SCustomKey SlotPlus;
	SCustomKey SlotMinus;
	SCustomKey SlotSave;
	SCustomKey SlotLoad;
	SCustomKey BGL1;
	SCustomKey BGL2;
	SCustomKey BGL3;
	SCustomKey BGL4;
	SCustomKey BGL5;
	SCustomKey ClippingWindows;
//	SCustomKey BGLHack;
	SCustomKey Transparency;
//  SCustomKey GLCube;
//	SCustomKey HDMA;
//	SCustomKey InterpMode7;
	SCustomKey JoypadSwap;
	SCustomKey SwitchControllers;
	SCustomKey TurboA, TurboB, TurboY, TurboX, TurboL, TurboR, TurboStart, TurboSelect, TurboLeft, TurboUp, TurboRight, TurboDown;
	SCustomKey SelectSave [10];
	SCustomKey ResetGame;
	SCustomKey ToggleCheats;
	SCustomKey QuitS9X;
    SCustomKey Rewind;
};

struct SJoypad {
    BOOL Enabled;
    union {
        WORD Left;
        WORD Autohold;
    };
    union {
        WORD Right;
        WORD ClearAll;
    };
    union {
        WORD Up;
        WORD TempTurbo;
    };
    union {
        WORD Down;
        WORD Autofire;
    };
    WORD Left_Up;
    WORD Left_Down;
    WORD Right_Up;
    WORD Right_Down;
    WORD Start;
    WORD Select;
    WORD A;
    WORD B;
    WORD X;
    WORD Y;
    WORD L;
    WORD R;
};

#define S9X_JOY_NEUTRAL 60

struct SJoyState{
    bool Attached;
    JOYCAPS Caps;
    int Threshold;
    bool Left;
    bool Right;
    bool Up;
    bool Down;
    bool PovLeft;
    bool PovRight;
    bool PovUp;
    bool PovDown;
    bool PovDnLeft;
    bool PovDnRight;
    bool PovUpLeft;
    bool PovUpRight;
    bool RUp;
    bool RDown;
    bool UUp;
    bool UDown;
    bool VUp;
    bool VDown;
    bool ZUp;
    bool ZDown;
    bool Button[32];
};

enum
{
	SNES_JOYPAD,
	SNES_MOUSE,
	SNES_SUPERSCOPE,
	SNES_MULTIPLAYER5,
	SNES_JUSTIFIER,
	SNES_MOUSE_SWAPPED,
	SNES_MULTIPLAYER8,
	SNES_JUSTIFIER_2,
	SNES_MAX_CONTROLLER_OPTIONS
};

/*****************************************************************************/

void SetInfoDlgColor(unsigned char r, unsigned char g, unsigned char b);

extern struct sGUI GUI;
extern struct sLanguages Languages[];
extern struct SJoypad Joypad[16];
extern struct SJoypad ToggleJoypadStorage[8];
extern struct SJoypad TurboToggleJoypadStorage[8];
extern struct SCustomKeys CustomKeys;

enum
{
    WIN_SNES9X_DIRECT_SOUND_DRIVER=0,
    WIN_FMOD_DIRECT_SOUND_DRIVER,
    WIN_FMOD_WAVE_SOUND_DRIVER,
    WIN_FMOD_A3D_SOUND_DRIVER,
	WIN_XAUDIO2_SOUND_DRIVER,
	WIN_FMODEX_DEFAULT_DRIVER,
	WIN_FMODEX_ASIO_DRIVER,
	WIN_FMODEX_OPENAL_DRIVER
};

#define S9X_REG_KEY_BASE MY_REG_KEY
#define S9X_REG_KEY_VERSION REG_KEY_VER

#define EXT_WIDTH (MAX_SNES_WIDTH + 4)
#define EXT_PITCH (EXT_WIDTH * 2)
#define EXT_HEIGHT (MAX_SNES_HEIGHT + 4)
// Offset into buffer to allow a two pixel border around the whole rendered
// SNES image. This is a speed up hack to allow some of the image processing
// routines to access black pixel data outside the normal bounds of the buffer.
#define EXT_OFFSET (EXT_PITCH * 2 + 2 * 2)

#define WIN32_WHITE RGB(255,255,255)

/*****************************************************************************/

void S9xSetWinPixelFormat ();
//int CheckKey( WORD Key, int OldJoypad);
//void TranslateKey(WORD keyz,char *out);

#define S9X_CONF_FILE_NAME "snes9x.conf"

const char* GetFilterName(RenderFilter filterID);
int GetFilterScale(RenderFilter filterID);
bool GetFilterHiResSupport(RenderFilter filterID);
const TCHAR * S9xGetDirectoryT (enum s9x_getdirtype);
RECT GetWindowMargins(HWND hwnd, UINT width);

#endif // !defined(SNES9X_H_INCLUDED)
