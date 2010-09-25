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




/* This is where all the GUI text strings will eventually end up */

#define WINDOW_TITLE "Snes9X v%s for Windows"

// the windows registry is no longer used
//#define MY_REG_KEY   "Software\\Emulators\\Snes9X"
//#define REG_KEY_VER  "1.31"

#define DISCLAIMER_TEXT        "Snes9X v%s for Windows.\r\n" \
        "(c) Copyright 1996 - 2002  Gary Henderson and Jerremy Koot (jkoot@snes9x.com)\r\n" \
        "(c) Copyright 2002 - 2004  Matthew Kendora\r\n" \
        "(c) Copyright 2002 - 2005  Peter Bortas\r\n" \
        "(c) Copyright 2004 - 2005  Joel Yliluoma\r\n" \
        "(c) Copyright 2001 - 2006  John Weidman\r\n" \
        "(c) Copyright 2002 - 2010  Brad Jorsch, funkyass, Kris Bleakley, Nach, zones\r\n" \
		"(c) Copyright 2006 - 2007  nitsuja\r\n" \
        "(c) Copyright 2009 - 2010  BearOso, OV2\r\n\r\n" \
        "Windows Port Authors: Matthew Kendora, funkyass, nitsuja, Nach, blip, OV2.\r\n\r\n" \
	"Snes9X is a Super Nintendo Entertainment System\r\n" \
		"emulator that allows you to play most games designed\r\n" \
		"for the SNES on your PC.\r\n\r\n" \
		"Please visit http://www.snes9x.com for\r\n" \
		"up-to-the-minute information and help on Snes9X.\r\n\r\n" \
		"Nintendo is a trade mark."


#define APP_NAME "Snes9x"
// possible global strings
#define SNES9X_INFO "Snes9x: Information"
#define SNES9X_WARN "Snes9x: WARNING!"
#define SNES9X_DXS "Snes9X: DirectSound"
#define SNES9X_SNDQ "Snes9X: Sound CPU Question"
#define SNES9X_NP_ERROR "Snes9X: NetPlay Error"
#define BUTTON_OK "&OK"
#define BUTTON_CANCEL "&Cancel"

// Gamepad Dialog Strings
#define INPUTCONFIG_TITLE "Input Configuration"
#define INPUTCONFIG_JPTOGGLE "Enabled"
//#define INPUTCONFIG_DIAGTOGGLE "Toggle Diagonals"
//#define INPUTCONFIG_OK "&OK"
//#define INPUTCONFIG_CANCEL "&Cancel"
#define INPUTCONFIG_JPCOMBO "Joypad #%d"
#define INPUTCONFIG_LABEL_UP "Up"
#define INPUTCONFIG_LABEL_DOWN "Down"
#define INPUTCONFIG_LABEL_LEFT "Left"
#define INPUTCONFIG_LABEL_RIGHT "Right"
#define INPUTCONFIG_LABEL_A "A"
#define INPUTCONFIG_LABEL_B "B"
#define INPUTCONFIG_LABEL_X "X"
#define INPUTCONFIG_LABEL_Y "Y"
#define INPUTCONFIG_LABEL_L "L"
#define INPUTCONFIG_LABEL_R "R"
#define INPUTCONFIG_LABEL_START "Start"
#define INPUTCONFIG_LABEL_SELECT "Select"
#define INPUTCONFIG_LABEL_UPLEFT "Up Left"
#define INPUTCONFIG_LABEL_UPRIGHT "Up Right"
#define INPUTCONFIG_LABEL_DOWNRIGHT "Dn Right"
#define INPUTCONFIG_LABEL_DOWNLEFT "Dn Left"
#define INPUTCONFIG_LABEL_BLUE "Blue means the button is already mapped.\nPink means it conflicts with a custom hotkey.\nRed means it's reserved by Windows.\nButtons can be disabled using Escape."
#define INPUTCONFIG_LABEL_UNUSED ""
#define INPUTCONFIG_LABEL_CLEAR_TOGGLES_AND_TURBO "Clear All"
#define INPUTCONFIG_LABEL_MAKE_TURBO "TempTurbo"
#define INPUTCONFIG_LABEL_MAKE_HELD "Autohold"
#define INPUTCONFIG_LABEL_MAKE_TURBO_HELD "Autofire"
#define INPUTCONFIG_LABEL_CONTROLLER_TURBO_PANEL_MOD " Turbo"

// Hotkeys Dialog Strings
#define HOTKEYS_TITLE "Hotkey Configuration"
#define HOTKEYS_CONTROL_MOD "Ctrl + "
#define HOTKEYS_SHIFT_MOD "Shift + "
#define HOTKEYS_ALT_MOD "Alt + "
#define HOTKEYS_LABEL_BLUE "Blue means the hotkey is already mapped.\nPink means it conflicts with a game button.\nRed means it's reserved by Windows.\nA hotkey can be disabled using Escape."
#define HOTKEYS_HKCOMBO "Page %d"
#define HOTKEYS_LABEL_1_1 "speed +"
#define HOTKEYS_LABEL_1_2 "speed -"
#define HOTKEYS_LABEL_1_3 "pause"
#define HOTKEYS_LABEL_1_4 "frame advance"
#define HOTKEYS_LABEL_1_5 "fast forward"
#define HOTKEYS_LABEL_1_6 "skip +"
#define HOTKEYS_LABEL_1_7 "skip -"
#define HOTKEYS_LABEL_1_8 "superscope turbo"
#define HOTKEYS_LABEL_1_9 "superscope pause"
#define HOTKEYS_LABEL_1_10 "show pressed keys"
#define HOTKEYS_LABEL_1_11 "movie frame count"
#define HOTKEYS_LABEL_1_12 "movie read-only"
#define HOTKEYS_LABEL_1_13 "save screenshot"
#define HOTKEYS_LABEL_2_1 "Graphics Layer 1"
#define HOTKEYS_LABEL_2_2 "Graphics Layer 2"
#define HOTKEYS_LABEL_2_3 "Graphics Layer 3"
#define HOTKEYS_LABEL_2_4 "Graphics Layer 4"
#define HOTKEYS_LABEL_2_5 "Sprites Layer"
#define HOTKEYS_LABEL_2_6 "Clipping Windows"
#define HOTKEYS_LABEL_2_7 "Transparency"
#define HOTKEYS_LABEL_2_8 "HDMA Emulation"
#define HOTKEYS_LABEL_2_9 "GLCube Mode"
#define HOTKEYS_LABEL_2_10 "Switch Controllers"
#define HOTKEYS_LABEL_2_11 "Joypad Swap"
#define HOTKEYS_LABEL_2_12 "Reset Game"
#define HOTKEYS_LABEL_2_13 "Toggle Cheats"
#define HOTKEYS_LABEL_3_1 "Turbo A mode"
#define HOTKEYS_LABEL_3_2 "Turbo B mode"
#define HOTKEYS_LABEL_3_3 "Turbo Y mode"
#define HOTKEYS_LABEL_3_4 "Turbo X mode"
#define HOTKEYS_LABEL_3_5 "Turbo L mode"
#define HOTKEYS_LABEL_3_6 "Turbo R mode"
#define HOTKEYS_LABEL_3_7 "Turbo Start mode"
#define HOTKEYS_LABEL_3_8 "Turbo Select mode"
#define HOTKEYS_LABEL_3_9 "Turbo Left mode"
#define HOTKEYS_LABEL_3_10 "Turbo Up mode"
#define HOTKEYS_LABEL_3_11 "Turbo Right mode"
#define HOTKEYS_LABEL_3_12 "Turbo Down mode"
//#define HOTKEYS_LABEL_4_12 "Interpolate Mode 7"
//#define HOTKEYS_LABEL_4_13 "BG Layering hack"

// gaming buttons and axes
#define GAMEDEVICE_JOYNUMPREFIX "(J%x)" // don't change this
#define GAMEDEVICE_JOYBUTPREFIX "#[%d]" // don't change this
#define GAMEDEVICE_XNEG "Left"
#define GAMEDEVICE_XPOS "Right"
#define GAMEDEVICE_YPOS "Up"
#define GAMEDEVICE_YNEG "Down"
#define GAMEDEVICE_POVLEFT "POV Left"
#define GAMEDEVICE_POVRIGHT "POV Right"
#define GAMEDEVICE_POVUP "POV Up"
#define GAMEDEVICE_POVDOWN "POV Down"
#define GAMEDEVICE_POVDNLEFT "POV Dn Left"
#define GAMEDEVICE_POVDNRIGHT "POV Dn Right"
#define GAMEDEVICE_POVUPLEFT  "POV Up Left"
#define GAMEDEVICE_POVUPRIGHT "POV Up Right"
#define GAMEDEVICE_ZPOS "Z Up"
#define GAMEDEVICE_ZNEG "Z Down"
#define GAMEDEVICE_RPOS "R Up"
#define GAMEDEVICE_RNEG "R Down"
#define GAMEDEVICE_UPOS "U Up"
#define GAMEDEVICE_UNEG "U Down"
#define GAMEDEVICE_VPOS "V Up"
#define GAMEDEVICE_VNEG "V Down"
#define GAMEDEVICE_BUTTON "Button %d"

// gaming general
#define GAMEDEVICE_DISABLED "Disabled"

// gaming keys
#define GAMEDEVICE_KEY "#%d"
#define GAMEDEVICE_NUMPADPREFIX "Numpad-%c"
#define GAMEDEVICE_VK_TAB "Tab"
#define GAMEDEVICE_VK_BACK "Backspace"
#define GAMEDEVICE_VK_CLEAR "Delete"
#define GAMEDEVICE_VK_RETURN "Enter"
#define GAMEDEVICE_VK_LSHIFT "LShift"
#define GAMEDEVICE_VK_RSHIFT "RShift"
#define GAMEDEVICE_VK_LCONTROL "LCtrl"
#define GAMEDEVICE_VK_RCONTROL "RCtrl"
#define GAMEDEVICE_VK_LMENU "LAlt"
#define GAMEDEVICE_VK_RMENU "RAlt"
#define GAMEDEVICE_VK_PAUSE "Pause"
#define GAMEDEVICE_VK_CAPITAL "Capslock"
#define GAMEDEVICE_VK_ESCAPE "Disabled"
#define GAMEDEVICE_VK_SPACE "Space"
#define GAMEDEVICE_VK_PRIOR "PgUp"
#define GAMEDEVICE_VK_NEXT "PgDn"
#define GAMEDEVICE_VK_HOME "Home"
#define GAMEDEVICE_VK_END "End"
#define GAMEDEVICE_VK_LEFT "Left"
#define GAMEDEVICE_VK_RIGHT "Right"
#define GAMEDEVICE_VK_UP "Up"
#define GAMEDEVICE_VK_DOWN "Down"
#define GAMEDEVICE_VK_SELECT "Select"
#define GAMEDEVICE_VK_PRINT "Print"
#define GAMEDEVICE_VK_EXECUTE "Execute"
#define GAMEDEVICE_VK_SNAPSHOT "SnapShot"
#define GAMEDEVICE_VK_INSERT "Insert"
#define GAMEDEVICE_VK_DELETE "Delete"
#define GAMEDEVICE_VK_HELP "Help"
#define GAMEDEVICE_VK_LWIN "LWinKey"
#define GAMEDEVICE_VK_RWIN "RWinKey"
#define GAMEDEVICE_VK_APPS "AppKey"
#define GAMEDEVICE_VK_MULTIPLY "Numpad *"
#define GAMEDEVICE_VK_ADD "Numpad +"
#define GAMEDEVICE_VK_SEPARATOR "Separator"
#define GAMEDEVICE_VK_OEM_1 "Semi-Colon"
#define GAMEDEVICE_VK_OEM_7 "Apostrophe"
#define GAMEDEVICE_VK_OEM_COMMA "Comma"
#define GAMEDEVICE_VK_OEM_PERIOD "Period"
#define GAMEDEVICE_VK_SUBTRACT "Numpad -"
#define GAMEDEVICE_VK_DECIMAL "Numpad ."
#define GAMEDEVICE_VK_DIVIDE "Numpad /"
#define GAMEDEVICE_VK_NUMLOCK "Num-lock"
#define GAMEDEVICE_VK_SCROLL "Scroll-lock"
#define GAMEDEVICE_VK_OEM_MINUS "-"
#define GAMEDEVICE_VK_OEM_PLUS "="
#define GAMEDEVICE_VK_SHIFT "Shift"
#define GAMEDEVICE_VK_CONTROL "Control"
#define GAMEDEVICE_VK_MENU "Alt"
#define GAMEDEVICE_VK_OEM_4 "["
#define GAMEDEVICE_VK_OEM_6 "]"
#define GAMEDEVICE_VK_OEM_5 "\\"
#define GAMEDEVICE_VK_OEM_2 "/"
#define GAMEDEVICE_VK_OEM_3 "`"
#define GAMEDEVICE_VK_F1 "F1"
#define GAMEDEVICE_VK_F2 "F2"
#define GAMEDEVICE_VK_F3 "F3"
#define GAMEDEVICE_VK_F4 "F4"
#define GAMEDEVICE_VK_F5 "F5"
#define GAMEDEVICE_VK_F6 "F6"
#define GAMEDEVICE_VK_F7 "F7"
#define GAMEDEVICE_VK_F8 "F8"
#define GAMEDEVICE_VK_F9 "F9"
#define GAMEDEVICE_VK_F10 "F10"
#define GAMEDEVICE_VK_F11 "F11"
#define GAMEDEVICE_VK_F12 "F12"


//evil things I found in WinProc

#define WINPROC_TURBOMODE_ON "Fast-Forward Activated"
#define WINPROC_TURBOMODE_OFF "Fast-Forward Deactivated"
#define WINPROC_TURBOMODE_TEXT "Fast-Forward"
#define WINPROC_HDMA_TEXT "HDMA emulation"
#define WINPROC_BG1 "BG#1" //Background Layers
#define WINPROC_BG2 "BG#2"
#define WINPROC_BG3 "BG#3"
#define WINPROC_BG4 "BG#4"
#define WINPROC_SPRITES "Sprites"
#define WINPROC_PADSWAP "Joypad swapping"
#define WINPROC_CONTROLERS0 "Multiplayer 5 on #0"
#define WINPROC_CONTROLERS1 "Joypad on #0"
#define WINPROC_CONTROLERS2 "Mouse on #1"
#define WINPROC_CONTROLERS3 "Mouse on #0"
#define WINPROC_CONTROLERS4 "Superscope on #1"
#define WINPROC_CONTROLERS5 "Justifier 1 on #1"
#define WINPROC_CONTROLERS6 "Justifier 2 on #1"
#define WINPROC_BGHACK "Background layering hack"
#define WINPROC_MODE7INTER "Mode 7 Interpolation"
#define WINPROC_TRANSPARENCY "Transparency effects"
#define WINPROC_CLIPWIN "Graphic clip windows"
#define WINPROC_PAUSE "Pause"
#define WINPROC_EMUFRAMETIME "Emulated frame time: %dms"
#define WINPROC_AUTOSKIP "Auto Frame Skip"
#define WINPROC_FRAMESKIP "Frame skip: %d"
#define WINPROC_TURBO_R_ON "Turbo R Activated"
#define WINPROC_TURBO_R_OFF "Turbo R Deactivated"
#define WINPROC_TURBO_L_ON "Turbo L Activated"
#define WINPROC_TURBO_L_OFF "Turbo L Deactivated"
#define WINPROC_TURBO_X_ON "Turbo X Activated"
#define WINPROC_TURBO_X_OFF "Turbo X Deactivated"
#define WINPROC_TURBO_Y_ON "Turbo Y Activated"
#define WINPROC_TURBO_Y_OFF "Turbo Y Deactivated"
#define WINPROC_TURBO_A_ON "Turbo A Activated"
#define WINPROC_TURBO_A_OFF "Turbo A Deactivated"
#define WINPROC_TURBO_B_ON "Turbo B Activated"
#define WINPROC_TURBO_B_OFF "Turbo B Deactivated"
#define WINPROC_TURBO_SEL_ON "Turbo Select Activated"
#define WINPROC_TURBO_SEL_OFF "Turbo Select Deactivated"
#define WINPROC_TURBO_START_ON "Turbo Start Activated"
#define WINPROC_TURBO_START_OFF "Turbo Start Deactivated"
#define WINPROC_TURBO_LEFT_ON "Turbo Left Activated"
#define WINPROC_TURBO_LEFT_OFF "Turbo Left Deactivated"
#define WINPROC_TURBO_RIGHT_ON "Turbo Right Activated"
#define WINPROC_TURBO_RIGHT_OFF "Turbo Right Deactivated"
#define WINPROC_TURBO_UP_ON "Turbo Up Activated"
#define WINPROC_TURBO_UP_OFF "Turbo Up Deactivated"
#define WINPROC_TURBO_DOWN_ON "Turbo Down Activated"
#define WINPROC_TURBO_DOWN_OFF "Turbo Down Deactivated"
#define WINPROC_FILTER_RESTART "You will need to restart Snes9x before the output image\nprocessing option change will take effect."
#define WINPROC_DISCONNECT "Disconnect from the NetPlay server first."
#define WINPROC_NET_RESTART "Your game will be reset after the ROM has been sent due to\nyour 'Sync Using Reset Game' setting.\n\n"
#define WINPROC_SYNC_SND "Sync sound"

//Emulator Settings

#define EMUSET_TITLE "Emulation Settings"
#define EMUSET_LABEL_DIRECTORY "Directory"
#define EMUSET_BROWSE "&Browse..."
#define EMUSET_LABEL_ASRAM "Auto-Save S-RAM"
#define EMUSET_LABEL_ASRAM_TEXT "seconds after last change (0 disables auto-save)"
#define EMUSET_LABEL_SMAX "Skip at most"
#define EMUSET_LABEL_SMAX_TEXT "frames in auto-frame rate mode"
#define EMUSET_LABEL_STURBO "Skip Rendering"
#define EMUSET_LABEL_STURBO_TEXT "frames in fast-forward mode"
#define EMUSET_TOGGLE_TURBO "Toggled fast-forward mode"

//Netplay Options

#define NPOPT_TITLE "Netplay Options"
#define NPOPT_LABEL_PORTNUM "Socket Port Number"
#define NPOPT_LABEL_PAUSEINTERVAL "Ask Server to Pause when"
#define NPOPT_LABEL_PAUSEINTERVAL_TEXT "frames behind"
#define NPOPT_LABEL_MAXSKIP "Maximum Frame Rate Skip"
#define NPOPT_SYNCBYRESET "Sync By Reset"
#define NPOPT_SENDROM "Send ROM Image to Client on Connect"
#define NPOPT_ACTASSERVER "Act As Server"
#define NPOPT_PORTNUMBLOCK "Port Settings"
#define NPOPT_CLIENTSETTINGSBLOCK "Client Settings"
#define NPOPT_SERVERSETTINGSBLOCK "Server Settings"

//Netplay Connect


#define NPCON_TITLE "Connect to Server"
#define NPCON_LABEL_SERVERADDY "Server Address"
#define NPCON_LABEL_PORTNUM "Port Number"
#define NPCON_CLEARHISTORY "Clear History"
#define NPCON_ENTERHOST "enter host name..."
#define NPCON_PLEASE_ENTERHOST "Please enter a host name."


//Movie Messages

#define MOVIE_FILETYPE_DESCRIPTION "Snes9x Movie File"
#define MOVIE_LABEL_SYNC_DATA_FROM_MOVIE "LOADED FROM MOVIE:"
#define MOVIE_LABEL_SYNC_DATA_NOT_FROM_MOVIE "SETTINGS NOT IN MOVIE; VERIFY:"
#define MOVIE_ERR_COULD_NOT_OPEN "Could not open movie file."
#define MOVIE_ERR_NOT_FOUND_SHORT "File not found."
#define MOVIE_ERR_NOT_FOUND "The movie file was not found or could not be opened."
#define MOVIE_ERR_WRONG_FORMAT_SHORT "Unrecognized format."
#define MOVIE_ERR_WRONG_FORMAT "The movie file is corrupt or in the wrong format."
#define MOVIE_ERR_WRONG_VERSION_SHORT "Unsupported movie version."
#define MOVIE_ERR_WRONG_VERSION MOVIE_ERR_WRONG_VERSION_SHORT " You need a different version of Snes9x to play this movie."
#define MOVIE_ERR_NOFRAMETOGGLE "No movie; can't toggle frame count"
#define MOVIE_ERR_NOREADONLYTOGGLE "No movie; can't toggle read-only"
#define MOVIE_LABEL_AUTHORINFO "Author Info:"
#define MOVIE_LABEL_ERRORINFO "Error Info:"
#define MOVIE_INFO_MISMATCH " <-- MISMATCH !!!"
#define MOVIE_INFO_CURRENTROM "Current ROM:"
#define MOVIE_INFO_MOVIEROM "Movie's ROM:"
#define MOVIE_INFO_ROMNOTSTORED " (not stored in movie file)"
#define MOVIE_INFO_ROMINFO " crc32=%08X, name=%s"
#define MOVIE_INFO_DIRECTORY " Path: %s"
#define MOVIE_WARNING_MISMATCH "WARNING: You don't have the right ROM loaded!"
#define MOVIE_WARNING_OK "Press OK to start playing the movie."
#define MOVIE_LABEL_STARTSETTINGS "Recording Start"
#define MOVIE_LABEL_CONTSETTINGS "Record Controllers"
#define MOVIE_LABEL_SYNCSETTINGS "Misc. Recording Settings"
#define MOVIE_SHUTDOWNMASTER_WARNING "The \"SpeedHacks\" setting in your snes9x.cfg file is on.\nThis makes emulation less CPU-intensive, but also less accurate,\ncausing some games to lag noticeably more than they should.\nYou might want to reconsider recording a movie under these conditions."

// Save Messages

#define FREEZE_INFO_SET_SLOT_N "set slot 00%d"

// AVI Messages

#define AVI_CONFIGURATION_CHANGED "AVI recording stopped (configuration settings changed)."
#define AVI_CREATION_FAILED "Failed to create AVI file."

// Cheat or Cheat Search Messages

#define SEARCH_TITLE_RANGEERROR "Range Error"
#define SEARCH_TITLE_CHEATERROR "Snes9x Cheat Error"
#define SEARCH_ERR_INVALIDNEWVALUE "You have entered an out of range or invalid value for the new value"
#define SEARCH_ERR_INVALIDCURVALUE "You have entered an out of range or invalid value for\n"\
                                   "the current value. This value is used when a cheat is unapplied.\n"\
                                   "(If left blank, no value is restored when the cheat is unapplied)"
#define SEARCH_ERR_INVALIDSEARCHVALUE "Please enter a valid value for a search!"
#define SEARCH_COLUMN_ADDRESS "Address"
#define SEARCH_COLUMN_VALUE "Value"
#define SEARCH_COLUMN_DESCRIPTION "Description"

// ROM dialog

#define ROM_COLUMN_FILENAME "File"
#define ROM_COLUMN_DESCRIPTION "Description"
#define ROM_COLUMN_SIZE "Size"
#define ROM_OPTION_AUTODETECT "Auto-Detect"
#define ROM_OPTION_FORCEHEADER "Force Header"
#define ROM_OPTION_FORCENOHEADER "Force No Header"
#define ROM_OPTION_FORCEPAL "Force PAL"
#define ROM_OPTION_FORCENTSC "Force NTSC"
#define ROM_OPTION_FORCEHIROM "Force HiROM"
#define ROM_OPTION_FORCELOROM "Force LoROM"
#define ROM_OPTION_NONINTERLEAVED "Force not interleaved"
#define ROM_OPTION_MODE1 "Force mode 1"
#define ROM_OPTION_MODE2 "Force mode 2"
#define ROM_OPTION_GD24 "Force GD24"
#define ROM_ITEM_NOTAROM "Not a ROM"
#define ROM_ITEM_CANTOPEN "Can't Open File"
#define ROM_ITEM_DESCNOTAVAILABLE "(Not Available)"
#define ROM_ITEM_COMPRESSEDROMDESCRIPTION ""

// Settings

#define SETTINGS_TITLE_SELECTFOLDER "Select Folder"
#define SETTINGS_OPTION_DIRECTORY_ROMS "Roms"
#define SETTINGS_OPTION_DIRECTORY_SCREENS "Screenshots"
#define SETTINGS_OPTION_DIRECTORY_MOVIES "Movies"
#define SETTINGS_OPTION_DIRECTORY_SPCS "SPCs"
#define SETTINGS_OPTION_DIRECTORY_SAVES "Saves"
#define SETTINGS_OPTION_DIRECTORY_SRAM "SRAM"
#define SETTINGS_OPTION_DIRECTORY_PATCHESANDCHEATS "Patch&Cheat"
#define SETTINGS_OPTION_DIRECTORY_BIOS "BIOS files"

// Misc.

#define INPUT_INFO_DISPLAY_ENABLED "Input display enabled."
#define INPUT_INFO_DISPLAY_DISABLED "Input display disabled."

#define FILE_INFO_AVI_FILE_TYPE "AVI file"
#define FILE_INFO_TXT_FILE_TYPE "Text file"
#define FILE_INFO_ROM_FILE_TYPE "ROM files or archives"
#define FILE_INFO_UNCROM_FILE_TYPE "Uncompressed ROM files"
#define FILE_INFO_ANY_FILE_TYPE "All files"

#define ERR_ROM_NOT_FOUND "ROM image \"%s\" was not found or could not be opened."
#define SRM_SAVE_FAILED "Failed to save SRM file."

#define CHEATS_INFO_ENABLED "Cheats enabled."
#define CHEATS_INFO_DISABLED "Cheats disabled."
#define CHEATS_INFO_ENABLED_NONE "Cheats enabled. (None are active.)"

#define MULTICART_BIOS_NOT_FOUND "not found!"
#define MULTICART_BIOS_FOUND ""

#define ABOUT_DIALOG_TITLE "About "
