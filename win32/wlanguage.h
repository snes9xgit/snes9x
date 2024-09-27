/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/* This is where all the GUI text strings will eventually end up */

#define WINDOW_TITLE TEXT("Snes9x")

// the windows registry is no longer used
//#define MY_REG_KEY   "Software\\Emulators\\Snes9x"
//#define REG_KEY_VER  "1.31"

#define DISCLAIMER_TEXT        TEXT("Snes9x v%s for Windows.\r\n\
(c) Copyright 1996 - 2002  Gary Henderson and Jerremy Koot (jkoot@snes9x.com)\r\n\
(c) Copyright 2002 - 2004  Matthew Kendora\r\n\
(c) Copyright 2002 - 2005  Peter Bortas\r\n\
(c) Copyright 2004 - 2005  Joel Yliluoma\r\n\
(c) Copyright 2001 - 2006  John Weidman\r\n\
(c) Copyright 2002 - 2010  Brad Jorsch, funkyass, Kris Bleakley, Nach, zones\r\n\
(c) Copyright 2006 - 2007  nitsuja\r\n\
(c) Copyright 2009 - 2023  BearOso, OV2\r\n\r\n\
Windows Port Authors: Matthew Kendora, funkyass, nitsuja, Nach, blip, OV2.\r\n\r\n\
Snes9x is a Super Nintendo Entertainment System\r\n\
emulator that allows you to play most games designed\r\n\
for the SNES on your PC.\r\n\r\n\
Please visit http://www.snes9x.com for\r\n\
up-to-the-minute information and help on Snes9x.\r\n\r\n\
Nintendo is a trademark.")


#define APP_NAME TEXT("Snes9x")
// possible global strings
#define SNES9X_INFO TEXT("Snes9x: Information")
#define SNES9X_WARN TEXT("Snes9x: WARNING!")
#define SNES9X_DXS TEXT("Snes9x: DirectSound")
#define SNES9X_SNDQ TEXT("Snes9x: Sound CPU Question")
#define SNES9X_NP_ERROR TEXT("Snes9x: NetPlay Error")
#define BUTTON_OK TEXT("&OK")
#define BUTTON_CANCEL TEXT("&Cancel")

// Gamepad Dialog Strings
#define INPUTCONFIG_TITLE TEXT("Input Configuration")
#define INPUTCONFIG_JPTOGGLE TEXT("Enabled")
//#define INPUTCONFIG_DIAGTOGGLE "Toggle Diagonals"
//#define INPUTCONFIG_OK "&OK"
//#define INPUTCONFIG_CANCEL "&Cancel"
#define INPUTCONFIG_JPCOMBO TEXT("Joypad #%d")
#define INPUTCONFIG_LABEL_UP TEXT("Up")
#define INPUTCONFIG_LABEL_DOWN TEXT("Down")
#define INPUTCONFIG_LABEL_LEFT TEXT("Left")
#define INPUTCONFIG_LABEL_RIGHT TEXT("Right")
#define INPUTCONFIG_LABEL_A TEXT("A")
#define INPUTCONFIG_LABEL_B TEXT("B")
#define INPUTCONFIG_LABEL_X TEXT("X")
#define INPUTCONFIG_LABEL_Y TEXT("Y")
#define INPUTCONFIG_LABEL_L TEXT("L")
#define INPUTCONFIG_LABEL_R TEXT("R")
#define INPUTCONFIG_LABEL_START TEXT("Start")
#define INPUTCONFIG_LABEL_SELECT TEXT("Select")
#define INPUTCONFIG_LABEL_UPLEFT TEXT("Up Left")
#define INPUTCONFIG_LABEL_UPRIGHT TEXT("Up Right")
#define INPUTCONFIG_LABEL_DOWNRIGHT TEXT("Dn Right")
#define INPUTCONFIG_LABEL_DOWNLEFT TEXT("Dn Left")
#define INPUTCONFIG_LABEL_BLUE TEXT("Blue means the button is already mapped.\nPink means it conflicts with a custom hotkey.\nRed means it's reserved by Windows.\nButtons can be disabled using Escape.")
#define INPUTCONFIG_LABEL_UNUSED TEXT("")
#define INPUTCONFIG_LABEL_CLEAR_TOGGLES_AND_TURBO TEXT("Clear All")
#define INPUTCONFIG_LABEL_MAKE_TURBO TEXT("TempTurbo")
#define INPUTCONFIG_LABEL_MAKE_HELD TEXT("Autohold")
#define INPUTCONFIG_LABEL_MAKE_TURBO_HELD TEXT("Autofire")
#define INPUTCONFIG_LABEL_CONTROLLER_TURBO_PANEL_MOD TEXT(" Turbo")

// Hotkeys Dialog Strings
#define HOTKEYS_TITLE TEXT("Hotkey Configuration")
#define HOTKEYS_CONTROL_MOD "Ctrl + "
#define HOTKEYS_SHIFT_MOD "Shift + "
#define HOTKEYS_ALT_MOD "Alt + "
#define HOTKEYS_LABEL_BLUE TEXT("Blue means the hotkey is already mapped.\nPink means it conflicts with a game button.\nRed means it's reserved by Windows.\nA hotkey can be disabled using Escape.")
#define HOTKEYS_HKCOMBO TEXT("Page %d")
#define HOTKEYS_LABEL_1_1 TEXT("Speed +:")
#define HOTKEYS_LABEL_1_2 TEXT("Speed -:")
#define HOTKEYS_LABEL_1_3 TEXT("Pause:")
#define HOTKEYS_LABEL_1_4 TEXT("Fast-forward toggle:")
#define HOTKEYS_LABEL_1_5 TEXT("Fast forward:")
#define HOTKEYS_LABEL_1_6 TEXT("Rewind:")
#define HOTKEYS_LABEL_1_7 TEXT("Skip frames +:")
#define HOTKEYS_LABEL_1_8 TEXT("Skip frames -:")
#define HOTKEYS_LABEL_1_9 TEXT("Toggle mute:")
#define HOTKEYS_LABEL_1_10 TEXT("Toggle cheats:")
#define HOTKEYS_LABEL_1_11 TEXT("Quit Snes9x:")
#define HOTKEYS_LABEL_1_12 TEXT("Reset game:")
#define HOTKEYS_LABEL_1_13 TEXT("Save screenshot")
#define HOTKEYS_LABEL_1_14 TEXT("Frame advance:")

#define HOTKEYS_LABEL_2_1 TEXT("Graphics layer 1:")
#define HOTKEYS_LABEL_2_2 TEXT("Graphics layer 2:")
#define HOTKEYS_LABEL_2_3 TEXT("Graphics layer 3:")
#define HOTKEYS_LABEL_2_4 TEXT("Graphics layer 4:")
#define HOTKEYS_LABEL_2_5 TEXT("Sprites layer:")
#define HOTKEYS_LABEL_2_6 TEXT("Clipping Windows:")
#define HOTKEYS_LABEL_2_7 TEXT("Transparency:")
#define HOTKEYS_LABEL_2_8 TEXT("Toggle backdrop:")
#define HOTKEYS_LABEL_2_9 TEXT("SuperScope pause:")
#define HOTKEYS_LABEL_2_10 TEXT("Switch controllers:")
#define HOTKEYS_LABEL_2_11 TEXT("Joypad swap:")
#define HOTKEYS_LABEL_2_12 TEXT("Show pressed keys:")
#define HOTKEYS_LABEL_2_13 TEXT("Movie frame count:")
#define HOTKEYS_LABEL_2_14 TEXT("Movie read-only:")

#define HOTKEYS_LABEL_3_1 TEXT("Turbo A mode:")
#define HOTKEYS_LABEL_3_2 TEXT("Turbo B mode:")
#define HOTKEYS_LABEL_3_3 TEXT("Turbo Y mode:")
#define HOTKEYS_LABEL_3_4 TEXT("Turbo X mode:")
#define HOTKEYS_LABEL_3_5 TEXT("Turbo L mode:")
#define HOTKEYS_LABEL_3_6 TEXT("Turbo R mode:")
#define HOTKEYS_LABEL_3_7 TEXT("Turbo Start mode:")
#define HOTKEYS_LABEL_3_8 TEXT("Turbo Select mode:")
#define HOTKEYS_LABEL_3_9 TEXT("Turbo Left mode:")
#define HOTKEYS_LABEL_3_10 TEXT("Turbo Up mode:")
#define HOTKEYS_LABEL_3_11 TEXT("Turbo Right mode:")
#define HOTKEYS_LABEL_3_12 TEXT("Turbo Down mode:")
#define HOTKEYS_LABEL_3_13 TEXT("SuperScope turbo:")


#define HOTKEYS_LABEL_4_1 TEXT("Select Slot 0:")
#define HOTKEYS_LABEL_4_2 TEXT("Select Slot 1:")
#define HOTKEYS_LABEL_4_3 TEXT("Select Slot 2:")
#define HOTKEYS_LABEL_4_4 TEXT("Select Slot 3:")
#define HOTKEYS_LABEL_4_5 TEXT("Select Slot 4:")
#define HOTKEYS_LABEL_4_6 TEXT("Select Slot 5:")
#define HOTKEYS_LABEL_4_7 TEXT("Select Slot 6:")
#define HOTKEYS_LABEL_4_8 TEXT("Select Slot 7:")
#define HOTKEYS_LABEL_4_9 TEXT("Select Slot 8:")
#define HOTKEYS_LABEL_4_10 TEXT("Select Slot 9:")
#define HOTKEYS_LABEL_4_11 TEXT("Save to file:")
#define HOTKEYS_LABEL_4_12 TEXT("Load from file:")

#define HOTKEYS_SWITCH_ASPECT_RATIO TEXT("Switch aspect Ratio:")
#define HOTKEYS_CHEAT_EDITOR_DIALOG TEXT("Cheats Editor Dialog:")
#define HOTKEYS_CHEAT_SEARCH_DIALOG TEXT("Cheats Search Dialog:")

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
#define GAMEDEVICE_VK_CLEAR "Clear"
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
#define WINPROC_CONTROLERS7 "M.A.C.S. Rifle on #1"
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
#define WINPROC_REWINDING_TEXT "Rewinding"
#define WINPROC_REWINDING_DISABLED "Rewind is disabled"

//Emulator Settings

#define EMUSET_TITLE TEXT("Emulation Settings")
#define EMUSET_LABEL_DIRECTORY TEXT("Directory")
#define EMUSET_BROWSE TEXT("&Browse...")
#define EMUSET_LABEL_ASRAM TEXT("Auto-Save S-RAM")
#define EMUSET_LABEL_ASRAM_TEXT TEXT("seconds after last change (0 disables auto-save)")
#define EMUSET_LABEL_SMAX TEXT("Skip at most")
#define EMUSET_LABEL_SMAX_TEXT TEXT("frames in auto-frame rate mode")
#define EMUSET_LABEL_STURBO TEXT("Skip Rendering")
#define EMUSET_LABEL_STURBO_TEXT TEXT("frames in fast-forward mode")

//Netplay Options

#define NPOPT_TITLE TEXT("Netplay Options")
#define NPOPT_LABEL_PORTNUM TEXT("Socket Port Number")
#define NPOPT_LABEL_PAUSEINTERVAL TEXT("Ask Server to Pause when")
#define NPOPT_LABEL_PAUSEINTERVAL_TEXT TEXT("frames behind")
#define NPOPT_LABEL_MAXSKIP TEXT("Maximum Frame Rate Skip")
#define NPOPT_SYNCBYRESET TEXT("Sync By Reset")
#define NPOPT_SENDROM TEXT("Send ROM Image to Client on Connect")
#define NPOPT_ACTASSERVER TEXT("Act As Server")
#define NPOPT_PORTNUMBLOCK TEXT("Port Settings")
#define NPOPT_CLIENTSETTINGSBLOCK TEXT("Client Settings")
#define NPOPT_SERVERSETTINGSBLOCK TEXT("Server Settings")

//Netplay Connect


#define NPCON_TITLE TEXT("Connect to Server")
#define NPCON_LABEL_SERVERADDY TEXT("Server Address")
#define NPCON_LABEL_PORTNUM TEXT("Port Number")
#define NPCON_CLEARHISTORY TEXT("Clear History")
#define NPCON_ENTERHOST TEXT("enter host name...")
#define NPCON_PLEASE_ENTERHOST TEXT("Please enter a host name.")


//Movie Messages

#define MOVIE_FILETYPE_DESCRIPTION TEXT("Snes9x Movie File")
#define MOVIE_LABEL_SYNC_DATA_FROM_MOVIE TEXT("LOADED FROM MOVIE:")
#define MOVIE_LABEL_SYNC_DATA_NOT_FROM_MOVIE TEXT("SETTINGS NOT IN MOVIE; VERIFY:")
#define MOVIE_ERR_COULD_NOT_OPEN TEXT("Could not open movie file.")
#define MOVIE_ERR_NOT_FOUND_SHORT TEXT("File not found.")
#define MOVIE_ERR_NOT_FOUND TEXT("The movie file was not found or could not be opened.")
#define MOVIE_ERR_WRONG_FORMAT_SHORT TEXT("Unrecognized format.")
#define MOVIE_ERR_WRONG_FORMAT TEXT("The movie file is corrupt or in the wrong format.")
#define MOVIE_ERR_WRONG_VERSION_SHORT TEXT("Unsupported movie version.")
#define MOVIE_ERR_WRONG_VERSION MOVIE_ERR_WRONG_VERSION_SHORT TEXT(" You need a different version of Snes9x to play this movie.")
#define MOVIE_ERR_NOFRAMETOGGLE "No movie; can't toggle frame count"
#define MOVIE_ERR_NOREADONLYTOGGLE "No movie; can't toggle read-only"
#define MOVIE_LABEL_AUTHORINFO TEXT("Author Info:")
#define MOVIE_LABEL_ERRORINFO TEXT("Error Info:")
#define MOVIE_INFO_MISMATCH TEXT(" <-- MISMATCH !!!")
#define MOVIE_INFO_CURRENTROM TEXT("Current ROM:")
#define MOVIE_INFO_MOVIEROM TEXT("Movie's ROM:")
#define MOVIE_INFO_ROMNOTSTORED TEXT(" (not stored in movie file)")
#define MOVIE_INFO_ROMINFO TEXT(" crc32=%08X, name=%s")
#define MOVIE_INFO_DIRECTORY TEXT(" Path: %s")
#define MOVIE_WARNING_MISMATCH TEXT("WARNING: You don't have the right ROM loaded!")
#define MOVIE_WARNING_OK TEXT("Press OK to start playing the movie.")
#define MOVIE_LABEL_STARTSETTINGS TEXT("Recording Start")
#define MOVIE_LABEL_CONTSETTINGS TEXT("Record Controllers")
#define MOVIE_LABEL_SYNCSETTINGS TEXT("Misc. Recording Settings")

// Save Messages

#define FREEZE_INFO_SET_SLOT_N "Set save slot %03d, bank %03d [%s]"

// AVI Messages

#define AVI_CONFIGURATION_CHANGED "AVI recording stopped (configuration settings changed)."
#define AVI_CREATION_FAILED "Failed to create AVI file."

// Cheat or Cheat Search Messages

#define SEARCH_TITLE_RANGEERROR TEXT("Range Error")
#define SEARCH_TITLE_CHEATERROR TEXT("Snes9x Cheat Error")
#define SEARCH_ERR_INVALIDNEWVALUE TEXT("You have entered an out of range or invalid value for the new value")
#define SEARCH_ERR_INVALIDCURVALUE TEXT("You have entered an out of range or invalid value for\n\
                                   the current value. This value is used when a cheat is unapplied.\n\
                                   (If left blank, no value is restored when the cheat is unapplied)")
#define SEARCH_ERR_INVALIDSEARCHVALUE TEXT("Please enter a valid value for a search!")
#define SEARCH_COLUMN_CODE TEXT("Code")
#define SEARCH_COLUMN_ADDRESS TEXT("Address")
#define SEARCH_COLUMN_VALUE TEXT("Value")
#define SEARCH_COLUMN_DESCRIPTION TEXT("Description")

// ROM dialog

#define ROM_COLUMN_FILENAME TEXT("File")
#define ROM_COLUMN_DESCRIPTION TEXT("Description")
#define ROM_COLUMN_SIZE TEXT("Size")
#define ROM_OPTION_AUTODETECT TEXT("Auto-Detect")
#define ROM_OPTION_FORCEHEADER TEXT("Force Header")
#define ROM_OPTION_FORCENOHEADER TEXT("Force No Header")
#define ROM_OPTION_FORCEPAL TEXT("Force PAL")
#define ROM_OPTION_FORCENTSC TEXT("Force NTSC")
#define ROM_OPTION_FORCEHIROM TEXT("Force HiROM")
#define ROM_OPTION_FORCELOROM TEXT("Force LoROM")
#define ROM_OPTION_NONINTERLEAVED TEXT("Force not interleaved")
#define ROM_OPTION_MODE1 TEXT("Force mode 1")
#define ROM_OPTION_MODE2 TEXT("Force mode 2")
#define ROM_OPTION_GD24 TEXT("Force GD24")
#define ROM_ITEM_NOTAROM TEXT("Not a ROM")
#define ROM_ITEM_CANTOPEN TEXT("Can't Open File")
#define ROM_ITEM_DESCNOTAVAILABLE TEXT("(Not Available)")
#define ROM_ITEM_COMPRESSEDROMDESCRIPTION TEXT("")

// Settings

#define SETTINGS_TITLE_SELECTFOLDER TEXT("Select Folder")
#define SETTINGS_OPTION_DIRECTORY_ROMS TEXT("Roms")
#define SETTINGS_OPTION_DIRECTORY_SCREENS TEXT("Screenshots")
#define SETTINGS_OPTION_DIRECTORY_MOVIES TEXT("Movies")
#define SETTINGS_OPTION_DIRECTORY_SPCS TEXT("SPCs")
#define SETTINGS_OPTION_DIRECTORY_SAVES TEXT("Saves")
#define SETTINGS_OPTION_DIRECTORY_SRAM TEXT("SRAM")
#define SETTINGS_OPTION_DIRECTORY_PATCHES TEXT("Patch")
#define SETTINGS_OPTION_DIRECTORY_CHEATS TEXT("Cheat")
#define SETTINGS_OPTION_DIRECTORY_BIOS TEXT("BIOS files")
#define SETTINGS_OPTION_DIRECTORY_SATDATA TEXT("Satellaview")

// Misc.

#define INPUT_INFO_DISPLAY_ENABLED "Input display enabled."
#define INPUT_INFO_DISPLAY_DISABLED "Input display disabled."

#define FILE_INFO_AVI_FILE_TYPE TEXT("AVI file")
#define FILE_INFO_TXT_FILE_TYPE TEXT("Text file")
#define FILE_INFO_ROM_FILE_TYPE TEXT("ROM files or archives")
#define FILE_INFO_UNCROM_FILE_TYPE TEXT("Uncompressed ROM files")
#define FILE_INFO_ANY_FILE_TYPE TEXT("All files")

#define ERR_ROM_NOT_FOUND "ROM image \"%s\" was not found or could not be opened."
#define SRM_SAVE_FAILED "Failed to save SRM file."

#define INFO_SAVE_SPC "Saving SPC Data."

#define MPAK_SAVE_FAILED "Failed to save Memory Pack."

#define CHEATS_INFO_ENABLED "Cheats enabled."
#define CHEATS_INFO_DISABLED "Cheats disabled."
#define CHEATS_INFO_ENABLED_NONE "Cheats enabled. (None are active.)"

#define MULTICART_BIOS_NOT_FOUND TEXT("not found!")
#define MULTICART_BIOS_FOUND TEXT("")

#define ABOUT_DIALOG_TITLE TEXT("About ")
