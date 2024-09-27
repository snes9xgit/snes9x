/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")


//  Win32 GUI code
//  (c) Copyright 2003-2006 blip, Nach, Matthew Kendora, funkyass and nitsuja

#ifdef __MINGW32__
#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0501
#endif

#pragma warning(disable: 4091)
#include <shlobj.h>
#pragma warning(default: 4091)
#include <objidl.h>
#include <shlwapi.h>
#include <Shobjidl.h>
#include <dbt.h>

#include "wsnes9x.h"
#include "win32_sound.h"
#include "win32_display.h"
#include "CCGShader.h"
#include "common/video/opengl/shaders/glsl.h"
#include "CShaderParamDlg.h"
#include "CSaveLoadWithPreviewDlg.h"
#include "../snes9x.h"
#include "../memmap.h"
#include "../cpuexec.h"
#include "../display.h"
#include "../cheats.h"
#include "../netplay.h"
#include "../apu/apu.h"
#include "../movie.h"
#include "../controls.h"
#include "../conffile.h"
#include "../statemanager.h"
#include "AVIOutput.h"
#include "InputCustom.h"
#include <vector>
#include <string>

#ifdef DEBUGGER
#include "../debug.h"
#endif

#if (((defined(_MSC_VER) && _MSC_VER >= 1300)) || defined(__MINGW32__))
	// both MINGW and VS.NET use fstream instead of fstream.h which is deprecated
	#include <fstream>
	using namespace std;
#else
	// for VC++ 6
	#include <fstream.h>
#endif

#include <sys/stat.h>
//#include "string_cache.h"
#include "wlanguage.h"
#include "../language.h"

#include <commctrl.h>
#include <io.h>
#include <time.h>
#include <direct.h>

extern SNPServer NPServer;

#include <ctype.h>

#ifdef _MSC_VER
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif

INT_PTR CALLBACK DlgSoundConf(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgInfoProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgAboutProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgEmulatorProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgEmulatorHacksProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK DlgOpenROMProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgMultiROMProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DlgChildSplitProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgNPProgress(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgNetConnect(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgNPOptions(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgFunky(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgInputConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgHotkeyConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgCheater(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgCheatSearch(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgCheatSearchAdd(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgCreateMovie(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgOpenMovie(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT CALLBACK EnumModesCallback( LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext);
int WinSearchCheatDatabase();
void WinShowCheatEditorDialog();
void WinShowCheatSearchDialog();

VOID CALLBACK HotkeyTimer( UINT idEvent, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

void S9xDetectJoypads();

void S9xWinScanJoypads();

#define NOTKNOWN "Unknown Company "
#define HEADER_SIZE 512
#define INFO_LEN (0xFF - 0xC0)

#define WM_CUSTKEYDOWN	(WM_USER+50)
#define WM_CUSTKEYUP	(WM_USER+51)

#define TIMER_SCANJOYPADS  (99999)
#define NC_SEARCHDB 0x8000

constexpr int MAX_SWITCHABLE_HOTKEY_DIALOG_ITEMS = 14;
constexpr int MAX_SWITCHABLE_HOTKEY_DIALOG_PAGES = 5;

#ifdef UNICODE
#define S9XW_SHARD_PATH SHARD_PATHW
#else
#define S9XW_SHARD_PATH SHARD_PATHA
#endif

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
struct sGUI GUI;
typedef struct sExtList
{
	TCHAR* extension;
	bool compressed;
	struct sExtList* next;
} ExtList;
HANDLE SoundEvent;

ExtList* valid_ext=NULL;
void MakeExtFile(void);
void LoadExts(void);
void ClearExts(void);
static bool ExtensionIsValid(const TCHAR *filename);

extern FILE *trace_fs;
extern SCheatData Cheat;
extern bool8 do_frame_adjust;

TCHAR multiRomA[MAX_PATH] = { 0 }; // lazy, should put in sGUI and add init to {0} somewhere
TCHAR multiRomB[MAX_PATH] = { 0 };

HINSTANCE g_hInst;

#ifdef DEBUGGER
#include "../debug.h"
#endif

struct SJoypad Joypad[16] = {
    {
        true,					/* Joypad 1 enabled */
			VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,	/* Left, Right, Up, Down */
			0, 0, 0, 0,             /* Left_Up, Left_Down, Right_Up, Right_Down */
			VK_SPACE, VK_RETURN,    /* Start, Select */
			'V', 'C',				/* A B */
			'D', 'X',				/* X Y */
			'A', 'S'				/* L R */
    },
    {
			true,                                  /* Joypad 2 enabled */
				'J', 'L', 'I', 'K',	/* Left, Right, Up, Down */
				0, 0, 0, 0,         /* Left_Up, Left_Down, Right_Up, Right_Down */
				'P', 'O',          /* Start, Select */
				'H', 'G',			/* A B */
				'T', 'F',			/* X Y */
				'Y', 'U'			/* L R */
		},
		{
				false,                                  /* Joypad 3 disabled */
					0, 0, 0, 0,
					0, 0, 0, 0,
					0, 0,
					0, 0, 0, 0, 0, 0
			},
			{
					false,                                  /* Joypad 4 disabled */
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0,
						0, 0, 0, 0, 0, 0
				},
				{
						false,                                  /* Joypad 5 disabled */
							0, 0, 0, 0,
							0, 0, 0, 0,
							0, 0,
							0, 0, 0, 0, 0, 0
					},
				{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },/* Joypad 6 disabled */
				{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },/* Joypad 7 disabled */
				{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },/* Joypad 8 disabled */
	{
			false,                                  /* Joypad 1 Turbo disabled */
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,                                  /* Joypad 2 Turbo disabled */
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,                                  /* Joypad 3 Turbo disabled */
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,                                  /* Joypad 4 Turbo disabled */
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,                                  /* Joypad 5 Turbo disabled */
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },/* Joypad 6 Turbo disabled */
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },/* Joypad 7 Turbo disabled */
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },/* Joypad 8 Turbo disabled */
};

// stores on/off toggle info for each key of each controller
SJoypad ToggleJoypadStorage [8] = {
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },
};

SJoypad TurboToggleJoypadStorage [8] = {
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
	{
			false,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0,
				0, 0, 0, 0, 0, 0
		},
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },
			{ false, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, 0 },
};

struct SCustomKeys CustomKeys = {
	{/*VK_OEM_PLUS*/0xBB,0}, // speed+ (=)
	{/*VK_OEM_MINUS*/0xBD,0}, // speed- (-)
	{VK_PAUSE,0}, // pause (PAUSE)
	{/*VK_OEM_5*/0xDC,0}, // frame advance (\)
	{/*VK_OEM_PLUS*/0xBB,CUSTKEY_SHIFT_MASK}, // skip+ (_)
	{/*VK_OEM_MINUS*/0xBD,CUSTKEY_SHIFT_MASK}, // skip- (+)
	{/*VK_OEM_3*/0xC0,0}, // superscope turbo (`)
	{/*VK_OEM_2*/0xBF,0}, // superscope pause (/)
	{/*VK_OEM_PERIOD*/0xBE,0}, // frame counter (.)
	{'8',CUSTKEY_SHIFT_MASK}, // movie read-only (*)
	{{VK_F1,CUSTKEY_SHIFT_MASK}, // save keys
	 {VK_F2,CUSTKEY_SHIFT_MASK},
	 {VK_F3,CUSTKEY_SHIFT_MASK},
	 {VK_F4,CUSTKEY_SHIFT_MASK},
	 {VK_F5,CUSTKEY_SHIFT_MASK},
	 {VK_F6,CUSTKEY_SHIFT_MASK},
	 {VK_F7,CUSTKEY_SHIFT_MASK},
	 {VK_F8,CUSTKEY_SHIFT_MASK},
	 {VK_F9,CUSTKEY_SHIFT_MASK},
	 {VK_F10,CUSTKEY_SHIFT_MASK}},
	{{VK_F1,0}, // load keys
	 {VK_F2,0},
	 {VK_F3,0},
	 {VK_F4,0},
	 {VK_F5,0},
	 {VK_F6,0},
	 {VK_F7,0},
	 {VK_F8,0},
	 {VK_F9,0},
	 {VK_F10,0}},
	{VK_TAB,0}, // fast forward (TAB)
	{0,0}, // fast forward toggle
	{/*VK_OEM_COMMA*/0xBC,0}, // show pressed keys/buttons (,)
	{VK_F12,0}, // save screenshot (F12)
	{0,0}, // slot plus (disabled by default)
	{0,0}, // slot minus (disabled by default)
	{0,0}, // slot save (disabled by default)
	{0,0}, // slot load (disabled by default)
	{0,0}, // bank plus (disabled by default)
	{0,0}, // bank minus (disabled by default)
    {VK_F11,CUSTKEY_SHIFT_MASK}, // dialog save
    {VK_F11,0}, // dialog load
	{0,0}, // background layer 1
	{0,0}, // background layer 2
	{0,0}, // background layer 3
	{0,0}, // background layer 4
	{0,0}, // sprite layer
	{0,0}, // Toggle backdrop
	{0,0}, // Clipping Windows
	{0,0}, // Transparency
	{'6',0}, // Joypad Swap
	{'7',0}, // Switch Controllers
	{VK_NEXT,CUSTKEY_SHIFT_MASK}, // Turbo A
	{VK_END,CUSTKEY_SHIFT_MASK}, // Turbo B
	{VK_HOME,CUSTKEY_SHIFT_MASK}, // Turbo Y
	{VK_PRIOR,CUSTKEY_SHIFT_MASK}, // Turbo X
	{VK_INSERT,CUSTKEY_SHIFT_MASK}, // Turbo L
	{VK_DELETE,CUSTKEY_SHIFT_MASK}, // Turbo R
	{0,0}, // Turbo Start
	{0,0}, // Turbo Select
	{0,0}, // Turbo Left
	{0,0}, // Turbo Up
	{0,0}, // Turbo Right
	{0,0}, // Turbo Down
	{{0,0}, // Select save slot 0
	 {0,0}, // Select save slot 1
	 {0,0}, // Select save slot 2
	 {0,0}, // Select save slot 3
	 {0,0}, // Select save slot 4
	 {0,0}, // Select save slot 5
	 {0,0}, // Select save slot 6
	 {0,0}, // Select save slot 7
	 {0,0}, // Select save slot 8
	 {0,0}}, // Select save slot 9
	{'R',CUSTKEY_CTRL_MASK|CUSTKEY_ALT_MASK}, // Reset Game
	{0,0}, // Toggle Cheats
	{0,0}, // Quit
    {'R',0}, // Rewind
    {0,0}, // Save File Select
    {0,0}, // Load File Select
    {0,0}, // Mute
    {0,0}, // Aspect ratio
    {'G', CUSTKEY_ALT_MASK}, // Cheat Editor Dialog
    {'A', CUSTKEY_ALT_MASK}, // Cheat Search Dialog
};

struct SSoundRates
{
    uint32 rate;
    int ident;
} SoundRates[9] = {
    { 8000, ID_SOUND_8000HZ},
    {11025, ID_SOUND_11025HZ},
    {16000, ID_SOUND_16000HZ},
    {22050, ID_SOUND_22050HZ},
    {30000, ID_SOUND_30000HZ},
	{32000, ID_SOUND_32000HZ},
    {35000, ID_SOUND_35000HZ},
    {44100, ID_SOUND_44100HZ},
    {48000, ID_SOUND_48000HZ}
};

static uint32 FrameTimings[] = {
	4000, 4000, 8333, 11667, 16667, 20000, 33333, 66667, 133333, 300000, 500000, 1000000, 1000000
};

// Languages supported by Snes9x: Windows
// 0 - English [Default]
struct sLanguages Languages[] = {
	{ IDR_MENU_US,
		TEXT("Failed to initialize currently selected display output!\n Try switching to a different output method in the display settings."),
		TEXT("DirectDraw failed to set the selected display mode!"),
		TEXT("DirectSound failed to initialize; no sound will be played."),
		TEXT("These settings won't take effect until you restart the emulator.")}
};

struct OpenMovieParams
{
	TCHAR Path[_MAX_PATH];
	bool8 ReadOnly;
	bool8 DisplayInput;
	uint8 ControllersMask;
	uint8 Opts;
	uint8 SyncFlags;
	wchar_t Metadata[MOVIE_MAX_METADATA];
};

TCHAR g_bankMenuItemStrings[NUM_SAVE_BANKS][20];
TCHAR g_saveMenuItemStrings[NUM_SAVE_BANKS * SAVE_SLOTS_PER_BANK][20];

StateManager stateMan;

std::vector<dMode> dm;
/*****************************************************************************/
/* WinProc                                                                   */
/*****************************************************************************/
void DoAVIOpen(const TCHAR* filename);
void DoAVIClose(int reason);
void RestoreGUIDisplay ();
void RestoreSNESDisplay ();
void CheckDirectoryIsWritable (const char *filename);
static void CheckMenuStates ();
static void ResetFrameTimer ();
static bool LoadROM (const TCHAR *filename, const TCHAR *filename2 = NULL);
static bool LoadROMMulti (const TCHAR *filename, const TCHAR *filename2);
bool8 S9xLoadROMImage (const TCHAR *string);
#ifdef NETPLAY_SUPPORT
static void EnableServer (bool8 enable);
#endif
void WinDeleteRecentGamesList ();
const TCHAR* WinParseCommandLineAndLoadConfigFile (TCHAR *line);
void WinRegisterConfigItems ();
void WinSaveConfigFile ();
void WinSetDefaultValues ();
void WinLockConfigFile ();
void WinUnlockConfigFile ();
void WinCleanupConfigData ();

#include "../ppu.h"
#include "../snapshot.h"
void S9xSetRecentGames ();
void S9xAddToRecentGames (const TCHAR *filename);
void S9xRemoveFromRecentGames (int i);

static void absToRel(TCHAR* relPath, const TCHAR* absPath, const TCHAR* baseDir)
{
	lstrcpy(relPath, absPath);
	if(!_tcsncicmp(absPath, baseDir, lstrlen(baseDir)))
	{
		TCHAR temp [MAX_PATH];
		temp[MAX_PATH-3]=TEXT('\0');
		const TCHAR* relative = absPath+lstrlen(baseDir);
		while(relative[0]==TEXT('\\') || relative[0]==TEXT('/'))
			relative++;
		relPath[0]=TEXT('.'); relPath[1]=TEXT('\\');
		lstrcpy(relPath+2, relative);
	}
}

BOOL SendMenuCommand (UINT uID)
{
	MENUITEMINFO mii;

	CheckMenuStates();

	mii.cbSize = sizeof(mii);
	mii.fMask  = MIIM_STATE;
	if (!GetMenuItemInfo(GUI.hMenu, uID, FALSE, &mii))
		return FALSE;
	if (!(mii.fState & MFS_DISABLED))
		return SendMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(uID),(LPARAM)(NULL));
	else
		return FALSE;
}

BOOL PostMenuCommand (UINT uID)
{
	MENUITEMINFO mii;

	CheckMenuStates();

	mii.cbSize = sizeof(mii);
	mii.fMask  = MIIM_STATE;
	if (!GetMenuItemInfo(GUI.hMenu, uID, FALSE, &mii))
		return FALSE;
	if (!(mii.fState & MFS_DISABLED))
		return PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(uID),(LPARAM)(NULL));
	else
		return FALSE;
}

HMENU GetSubMenuById(UINT uID)
{
	MENUITEMINFO mii;

	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU;
	if (!GetMenuItemInfo(GUI.hMenu, uID, FALSE, &mii))
		return NULL;
	return mii.hSubMenu;
}

void S9xMouseOn ()
{
	if(Settings.StopEmulation)
		return;

    if (GUI.ControllerOption==SNES_MOUSE || GUI.ControllerOption==SNES_MOUSE_SWAPPED)
    {
		if(Settings.Paused)
			SetCursor (GUI.Arrow);
		else
	        SetCursor (NULL);
    }
    else if (GUI.ControllerOption!=SNES_SUPERSCOPE && GUI.ControllerOption!=SNES_JUSTIFIER && GUI.ControllerOption!=SNES_JUSTIFIER_2 && GUI.ControllerOption!=SNES_MACSRIFLE)
    {
        SetCursor (GUI.Arrow);
        GUI.CursorTimer = 60;
    }
    else
	{
		if(Settings.Paused)
			SetCursor (GUI.GunSight);
		else
	        SetCursor (NULL);
	}
}

void ControllerOptionsFromControllers()
{
   enum controllers controller[2];
   int8 ids[4];
   S9xGetController(0, &controller[0], &ids[0], &ids[1], &ids[2], &ids[3]);
   S9xGetController(1, &controller[1], &ids[0], &ids[1], &ids[2], &ids[3]);

   GUI.ControllerOption = SNES_JOYPAD;

   if (controller[0] == CTL_JOYPAD && controller[1] == CTL_MP5)
      GUI.ControllerOption = SNES_MULTIPLAYER5;
   else if (controller[0] == CTL_MP5 && controller[1] == CTL_MP5)
      GUI.ControllerOption = SNES_MULTIPLAYER8;
   else if (controller[0] == CTL_MOUSE && controller[1] == CTL_JOYPAD)
      GUI.ControllerOption = SNES_MOUSE;
   else if (controller[0] == CTL_JOYPAD && controller[1] == CTL_MOUSE)
      GUI.ControllerOption = SNES_MOUSE_SWAPPED;
   else if (controller[0] == CTL_JOYPAD && controller[1] == CTL_SUPERSCOPE)
      GUI.ControllerOption = SNES_SUPERSCOPE;
   else if (controller[0] == CTL_JOYPAD && controller[1] == CTL_JUSTIFIER && !ids[0])
      GUI.ControllerOption = SNES_JUSTIFIER;
   else if (controller[0] == CTL_JOYPAD && controller[1] == CTL_JUSTIFIER && ids[0])
      GUI.ControllerOption = SNES_JUSTIFIER_2;
   else if (controller[0] == CTL_JOYPAD && controller[1] == CTL_MACSRIFLE)
      GUI.ControllerOption = SNES_MACSRIFLE;
   else if (controller[0] == CTL_JOYPAD)
      GUI.ControllerOption = SNES_JOYPAD;

}

void ChangeInputDevice(void)
{
	Settings.MouseMaster = false;
	Settings.JustifierMaster = false;
	Settings.SuperScopeMaster = false;
	Settings.MultiPlayer5Master = false;
	Settings.MacsRifleMaster = false;

	switch(GUI.ControllerOption)
	{
	case SNES_MOUSE:
		Settings.MouseMaster = true;
		S9xSetController(0, CTL_MOUSE,      0, 0, 0, 0);
		S9xSetController(1, CTL_JOYPAD,     1, 0, 0, 0);
		break;
	case SNES_MOUSE_SWAPPED:
		Settings.MouseMaster = true;
		S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
		S9xSetController(1, CTL_MOUSE,      1, 0, 0, 0);
		break;
	case SNES_SUPERSCOPE:
		Settings.SuperScopeMaster = true;
		S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
		S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
		break;
	case SNES_MULTIPLAYER5:
		Settings.MultiPlayer5Master = true;
		S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
		S9xSetController(1, CTL_MP5,        1, 2, 3, 4);
		break;
	case SNES_MULTIPLAYER8:
		Settings.MultiPlayer5Master = true;
		S9xSetController(0, CTL_MP5,        0, 1, 2, 3);
		S9xSetController(1, CTL_MP5,        4, 5, 6, 7);
		break;
	case SNES_JUSTIFIER:
		Settings.JustifierMaster = true;
		S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
		S9xSetController(1, CTL_JUSTIFIER,  0, 0, 0, 0);
		break;
	case SNES_JUSTIFIER_2:
		Settings.JustifierMaster = true;
		S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
		S9xSetController(1, CTL_JUSTIFIER,  1, 0, 0, 0);
		break;
	case SNES_MACSRIFLE:
		Settings.MacsRifleMaster = true;
		S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
		S9xSetController(1, CTL_MACSRIFLE,  0, 0, 0, 0);
		break;
	default:
	case SNES_JOYPAD:
		S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
		if (Joypad[1].Enabled)
			S9xSetController(1, CTL_JOYPAD,     1, 0, 0, 0);
		else
			S9xSetController(1, CTL_NONE,       0, 0, 0, 0);
		break;
	}

    GUI.ControlForced = 0xff;
}

static void CenterCursor()
{
	if(GUI.ControllerOption==SNES_MOUSE || GUI.ControllerOption==SNES_MOUSE_SWAPPED)
	{
		if(GUI.hWnd == GetActiveWindow() && !S9xMoviePlaying())
		{
			POINT cur, middle;
			RECT size;

			GetClientRect (GUI.hWnd, &size);
			middle.x = (size.right - size.left) >> 1;
			middle.y = (size.bottom - size.top) >> 1;
			ClientToScreen (GUI.hWnd, &middle);
			GetCursorPos (&cur);
			int dX = middle.x-cur.x;
			int dY = middle.y-cur.y;
			if(dX || dY)
			{
				GUI.MouseX -= dX;
				GUI.MouseY -= dY;
				SetCursorPos (middle.x, middle.y);
//				GUI.IgnoreNextMouseMove = true;
			}
		}
	}
}


void S9xRestoreWindowTitle ()
{
    TCHAR buf [1024];
    if (Memory.ROMFilename[0])
    {
        char def[_MAX_FNAME];
        _splitpath(Memory.ROMFilename.c_str(), NULL, NULL, def, NULL);
        _stprintf(buf, TEXT("%s - %s %s"), (wchar_t *)Utf8ToWide(def), WINDOW_TITLE, TEXT(VERSION));
    }
    else
        _stprintf(buf, TEXT("%s %s"), WINDOW_TITLE, TEXT(VERSION));

    SetWindowText (GUI.hWnd, buf);
}

void S9xDisplayStateChange (const char *str, bool8 on)
{
    static char string [100];

    sprintf (string, "%s %s", str, on ? "on" : "off");
    S9xSetInfoString (string);
}

static void UpdateScale(RenderFilter & Scale, RenderFilter & NextScale)
{
	Scale = NextScale;
}

static char InfoString [100];
static uint32 prevPadReadFrame = (uint32)-1;
static bool skipNextFrameStop = false;

static void ShowStatusSlotInfo()
{
	static char str[64];

	char filename[_MAX_PATH + 1];
	GetSlotFilename(GUI.CurrentSaveBank * SAVE_SLOTS_PER_BANK + GUI.CurrentSaveSlot, filename);

	bool exists = false;
	struct stat stats;
	if (stat(filename, &stats) == 0)
		exists = true;

	sprintf(str, FREEZE_INFO_SET_SLOT_N, GUI.CurrentSaveSlot, GUI.CurrentSaveBank, exists ? "used" : "empty");
	S9xSetInfoString(str);
}

int HandleKeyMessage(WPARAM wParam, LPARAM lParam)
{
	// update toggles
	for (int J = 0; J < 5; J++)
	{
		extern bool S9xGetState (WORD KeyIdent);
		if(Joypad[J].Enabled && (!S9xGetState(Joypad[J+8].Autohold))) // enabled and Togglify
		{
			SJoypad & p = ToggleJoypadStorage[J];
			if(wParam == Joypad[J].L) p.L = !p.L;
			if(wParam == Joypad[J].R) p.R = !p.R;
			if(wParam == Joypad[J].A) p.A = !p.A;
			if(wParam == Joypad[J].B) p.B = !p.B;
			if(wParam == Joypad[J].Y) p.Y = !p.Y;
			if(wParam == Joypad[J].X) p.X = !p.X;
			if(wParam == Joypad[J].Start) p.Start = !p.Start;
			if(wParam == Joypad[J].Select) p.Select = !p.Select;
			if(wParam == Joypad[J].Left) p.Left = !p.Left;
			if(wParam == Joypad[J].Right) p.Right = !p.Right;
			if(wParam == Joypad[J].Up) p.Up = !p.Up;
			if(wParam == Joypad[J].Down) p.Down = !p.Down;
//			if(wParam == Joypad[J].Left_Down) p.Left_Down = !p.Left_Down;
//			if(wParam == Joypad[J].Left_Up) p.Left_Up = !p.Left_Up;
//			if(wParam == Joypad[J].Right_Down) p.Right_Down = !p.Right_Down;
//			if(wParam == Joypad[J].Right_Up) p.Right_Up = !p.Right_Up;
			if(!Settings.UpAndDown)
			{
				if(p.Left && p.Right)
					p.Left = p.Right = false;
				if(p.Up && p.Down)
					p.Up = p.Down = false;
			}
		}
		if(Joypad[J].Enabled && (!S9xGetState(Joypad[J+8].Autofire))) // enabled and turbo-togglify (TurboTog)
		{
			SJoypad & p = TurboToggleJoypadStorage[J];
			if(wParam == Joypad[J].L) p.L = !p.L;
			if(wParam == Joypad[J].R) p.R = !p.R;
			if(wParam == Joypad[J].A) p.A = !p.A;
			if(wParam == Joypad[J].B) p.B = !p.B;
			if(wParam == Joypad[J].Y) p.Y = !p.Y;
			if(wParam == Joypad[J].X) p.X = !p.X;
			if(wParam == Joypad[J].Start) p.Start = !p.Start;
			if(wParam == Joypad[J].Select) p.Select = !p.Select;
			if(wParam == Joypad[J].Left) p.Left = !p.Left;
			if(wParam == Joypad[J].Right) p.Right = !p.Right;
			if(wParam == Joypad[J].Up) p.Up = !p.Up;
			if(wParam == Joypad[J].Down) p.Down = !p.Down;
//			if(wParam == Joypad[J].Left_Down) p.Left_Down = !p.Left_Down;
//			if(wParam == Joypad[J].Left_Up) p.Left_Up = !p.Left_Up;
//			if(wParam == Joypad[J].Right_Down) p.Right_Down = !p.Right_Down;
//			if(wParam == Joypad[J].Right_Up) p.Right_Up = !p.Right_Up;
			if(!Settings.UpAndDown)
			{
				if(p.Left && p.Right)
					p.Left = p.Right = false;
				if(p.Up && p.Down)
					p.Up = p.Down = false;
			}
		}
		if(wParam == Joypad[J+8].ClearAll) // clear all
		{
			{
				SJoypad & p = ToggleJoypadStorage[J];
				p.L = false;
				p.R = false;
				p.A = false;
				p.B = false;
				p.Y = false;
				p.X = false;
				p.Start = false;
				p.Select = false;
				p.Left = false;
				p.Right = false;
				p.Up = false;
				p.Down = false;
			}
			{
				SJoypad & p = TurboToggleJoypadStorage[J];
				p.L = false;
				p.R = false;
				p.A = false;
				p.B = false;
				p.Y = false;
				p.X = false;
				p.Start = false;
				p.Select = false;
				p.Left = false;
				p.Right = false;
				p.Up = false;
				p.Down = false;
			}
		}
	}


	bool hitHotKey = false;

    // if this is not a gamepad press and background hotkeys are disabled, skip it if we do not have focus
    if (!GUI.BackgroundKeyHotkeys && !(wParam & 0x8000) && GUI.hWnd != GetForegroundWindow())
    {
        return 0;
    }

	if(!(wParam == 0 || wParam == VK_ESCAPE)) // if it's the 'disabled' key, it's never pressed as a hotkey
	{
		int modifiers = 0;
		if(GetAsyncKeyState(VK_MENU))
			modifiers |= CUSTKEY_ALT_MASK;
		if(GetAsyncKeyState(VK_CONTROL))
			modifiers |= CUSTKEY_CTRL_MASK;
		if(GetAsyncKeyState(VK_SHIFT))
			modifiers |= CUSTKEY_SHIFT_MASK;

		{
			for(int i = 0 ; i < SAVE_SLOTS_PER_BANK; i++)
			{
				if(wParam == CustomKeys.Save[i].key
				&& modifiers == CustomKeys.Save[i].modifiers)
				{
					FreezeUnfreezeSlot (GUI.CurrentSaveBank * SAVE_SLOTS_PER_BANK + i, true);
					hitHotKey = true;
				}
				if(wParam == CustomKeys.Load[i].key
				&& modifiers == CustomKeys.Load[i].modifiers)
				{
					FreezeUnfreezeSlot (GUI.CurrentSaveBank * SAVE_SLOTS_PER_BANK + i, false);
					hitHotKey = true;
				}
			}

			if(wParam == CustomKeys.SlotSave.key
			&& modifiers == CustomKeys.SlotSave.modifiers)
			{
				FreezeUnfreezeSlot (GUI.CurrentSaveBank * SAVE_SLOTS_PER_BANK + GUI.CurrentSaveSlot, true);
				hitHotKey = true;
			}
			if(wParam == CustomKeys.SlotLoad.key
			&& modifiers == CustomKeys.SlotLoad.modifiers)
			{
				FreezeUnfreezeSlot (GUI.CurrentSaveBank * SAVE_SLOTS_PER_BANK + GUI.CurrentSaveSlot, false);
				hitHotKey = true;
			}
            if(wParam == CustomKeys.DialogSave.key
                && modifiers == CustomKeys.DialogSave.modifiers)
            {
                FreezeUnfreezeDialogPreview(true);
                hitHotKey = true;
            }
            if(wParam == CustomKeys.DialogLoad.key
                && modifiers == CustomKeys.DialogLoad.modifiers)
            {
                FreezeUnfreezeDialogPreview(false);
                hitHotKey = true;
            }
			if(wParam == CustomKeys.SlotPlus.key
			&& modifiers == CustomKeys.SlotPlus.modifiers)
			{
				GUI.CurrentSaveSlot++;
				if(GUI.CurrentSaveSlot > LAST_SAVE_SLOT_IN_BANK)
					GUI.CurrentSaveSlot = 0;

				ShowStatusSlotInfo();

				hitHotKey = true;
			}
			if(wParam == CustomKeys.SlotMinus.key
			&& modifiers == CustomKeys.SlotMinus.modifiers)
			{
				GUI.CurrentSaveSlot--;
				if(GUI.CurrentSaveSlot < 0)
					GUI.CurrentSaveSlot = 9;

				ShowStatusSlotInfo();

				hitHotKey = true;
			}
			if (wParam == CustomKeys.BankPlus.key
				&& modifiers == CustomKeys.BankPlus.modifiers)
			{
				GUI.CurrentSaveBank++;
				if (GUI.CurrentSaveBank > LAST_SAVE_BANK)
					GUI.CurrentSaveBank = 0;

                ShowStatusSlotInfo();

				hitHotKey = true;
			}
			if (wParam == CustomKeys.BankMinus.key
				&& modifiers == CustomKeys.BankMinus.modifiers)
			{
				GUI.CurrentSaveBank--;
				if (GUI.CurrentSaveBank < 0)
					GUI.CurrentSaveBank = 9;

                ShowStatusSlotInfo();

				hitHotKey = true;
			}
		}


		if(wParam == CustomKeys.FrameAdvance.key
		&& modifiers == CustomKeys.FrameAdvance.modifiers)
		{
			static DWORD lastTime = 0;
			if((timeGetTime() - lastTime) > 20)
			{
				lastTime = timeGetTime();
				if(Settings.Paused || GUI.FASkipsNonInput)
				{
					prevPadReadFrame = (uint32)-1;
					Settings.Paused = false;
					S9xMouseOn();
					GUI.IgnoreNextMouseMove = true;
					Settings.Paused = true;

					Settings.FrameAdvance = true;
					GUI.FrameAdvanceJustPressed = 2;
					// kick the main thread out of GetMessage (just in case)
					SendMessage(GUI.hWnd, WM_NULL, 0, 0);
				}
				else
				{
					Settings.Paused = true;
				}

				CenterCursor();
			}

			hitHotKey = true;
		}
		if(wParam == CustomKeys.FrameCount.key
		&& modifiers == CustomKeys.FrameCount.modifiers)
		{
			if (S9xMovieActive()
#ifdef NETPLAY_SUPPORT
			|| Settings.NetPlay
#endif
			)
				S9xMovieToggleFrameDisplay ();
			else
				S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_ERR_NOFRAMETOGGLE);
			hitHotKey = true;
		}
		if(wParam == CustomKeys.Pause.key
		&& modifiers == CustomKeys.Pause.modifiers)
		{
			Settings.Paused = Settings.Paused ^ true;
			Settings.FrameAdvance = false;
			GUI.FrameAdvanceJustPressed = 0;
			CenterCursor();
			if(!Settings.Paused)
				S9xMouseOn();
			hitHotKey = true;
		}
		if(wParam == CustomKeys.ReadOnly.key
		&& modifiers == CustomKeys.ReadOnly.modifiers)
		{
			if (S9xMovieActive())
				S9xMovieToggleRecState();
			else
				S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_ERR_NOREADONLYTOGGLE);
			hitHotKey = true;
		}
		if(wParam == CustomKeys.FastForward.key
		&& modifiers == CustomKeys.FastForward.modifiers)
		{
			if(!Settings.TurboMode)
				S9xMessage (S9X_INFO, S9X_TURBO_MODE, WINPROC_TURBOMODE_TEXT);
			Settings.TurboMode = TRUE;
			hitHotKey = true;
		}
		if(wParam == CustomKeys.FastForwardToggle.key
		&& modifiers == CustomKeys.FastForwardToggle.modifiers)
		{
			Settings.TurboMode ^= TRUE;
			if (Settings.TurboMode)
				S9xMessage (S9X_INFO, S9X_TURBO_MODE,
				WINPROC_TURBOMODE_ON);
			else
				S9xMessage (S9X_INFO, S9X_TURBO_MODE,
				WINPROC_TURBOMODE_OFF);
			hitHotKey = true;
		}
		if(wParam == CustomKeys.ShowPressed.key
		&& modifiers == CustomKeys.ShowPressed.modifiers)
		{
			Settings.DisplayPressedKeys = Settings.DisplayPressedKeys?0:2;

			if(Settings.DisplayPressedKeys==2)
				S9xMessage(S9X_INFO, S9X_MOVIE_INFO, INPUT_INFO_DISPLAY_ENABLED);
			else
				S9xMessage(S9X_INFO, S9X_MOVIE_INFO, INPUT_INFO_DISPLAY_DISABLED);

			hitHotKey = true;
		}
		if(wParam == CustomKeys.SaveScreenShot.key
		&& modifiers == CustomKeys.SaveScreenShot.modifiers)
		{
			Settings.TakeScreenshot=true;
		}
		if(wParam == CustomKeys.ScopePause.key
		&& modifiers == CustomKeys.ScopePause.modifiers)
		{
			GUI.superscope_pause = 1;
			hitHotKey = true;
		}
		if(wParam == CustomKeys.ScopeTurbo.key
		&& modifiers == CustomKeys.ScopeTurbo.modifiers)
		{
			GUI.superscope_turbo = 1;
			hitHotKey = true;
		}
		if(wParam == CustomKeys.SkipDown.key
		&& modifiers == CustomKeys.SkipDown.modifiers)
		{
			if (Settings.SkipFrames <= 1)
				Settings.SkipFrames = AUTO_FRAMERATE;
			else
				if (Settings.SkipFrames != AUTO_FRAMERATE)
					Settings.SkipFrames--;

				if (Settings.SkipFrames == AUTO_FRAMERATE)
					S9xSetInfoString (WINPROC_AUTOSKIP);
				else
				{
					sprintf (InfoString, WINPROC_FRAMESKIP,
						Settings.SkipFrames - 1);
					S9xSetInfoString (InfoString);
				}
			hitHotKey = true;
		}
		if(wParam == CustomKeys.SkipUp.key
		&& modifiers == CustomKeys.SkipUp.modifiers)
		{
			if (Settings.SkipFrames == AUTO_FRAMERATE)
				Settings.SkipFrames = 1;
			else
				if (Settings.SkipFrames < 10)
					Settings.SkipFrames++;

				if (Settings.SkipFrames == AUTO_FRAMERATE)
					S9xSetInfoString (WINPROC_AUTOSKIP);
				else
				{
					sprintf (InfoString, WINPROC_FRAMESKIP,
						Settings.SkipFrames - 1);
					S9xSetInfoString (InfoString);
				}
			hitHotKey = true;
		}
		if(wParam == CustomKeys.SpeedDown.key
		&& modifiers == CustomKeys.SpeedDown.modifiers)
		{
			// Increase emulated frame time
			int i;
			for(i=1; FrameTimings[i]<Settings.FrameTime; ++i)
				;
			Settings.FrameTime = FrameTimings[i+1];
			ResetFrameTimer ();
//					sprintf (InfoString, WINPROC_EMUFRAMETIME,
//						Settings.FrameTime / 1);
			sprintf (InfoString, "Speed: %.0f%% (%.1f ms/frame)", ((Settings.PAL?Settings.FrameTimePAL:Settings.FrameTimeNTSC) * 100.0f) / (float)Settings.FrameTime, Settings.FrameTime*0.001f);
			S9xSetInfoString (InfoString);
			hitHotKey = true;
		}
		if(wParam == CustomKeys.SpeedUp.key
		&& modifiers == CustomKeys.SpeedUp.modifiers)
		{
			// Decrease emulated frame time
			int i;
			for(i=1; FrameTimings[i]<Settings.FrameTime; ++i)
				;
			Settings.FrameTime = FrameTimings[i-1];

			ResetFrameTimer ();
//					sprintf (InfoString, WINPROC_EMUFRAMETIME,
//						Settings.FrameTime / 1);
			sprintf (InfoString, "Speed: %.0f%% (%.1f ms/frame)", ((Settings.PAL?Settings.FrameTimePAL:Settings.FrameTimeNTSC) * 100.0f) / (float)Settings.FrameTime, Settings.FrameTime*0.001f);
			S9xSetInfoString (InfoString);
			hitHotKey = true;
		}
		if(wParam == CustomKeys.BGL1.key
		&& modifiers == CustomKeys.BGL1.modifiers)
		{
			Settings.BG_Forced ^= 1;
			S9xDisplayStateChange (WINPROC_BG1, !(Settings.BG_Forced & 1));
		}
		if(wParam == CustomKeys.BGL2.key
		&& modifiers == CustomKeys.BGL2.modifiers)
		{
			Settings.BG_Forced ^= 2;
			S9xDisplayStateChange (WINPROC_BG2, !(Settings.BG_Forced & 2));
		}
		if(wParam == CustomKeys.BGL3.key
		&& modifiers == CustomKeys.BGL3.modifiers)
		{
			Settings.BG_Forced ^= 4;
			S9xDisplayStateChange (WINPROC_BG3, !(Settings.BG_Forced & 4));
		}
		if(wParam == CustomKeys.BGL4.key
		&& modifiers == CustomKeys.BGL4.modifiers)
		{
			Settings.BG_Forced ^= 8;
			S9xDisplayStateChange (WINPROC_BG4, !(Settings.BG_Forced & 8));
		}
		if(wParam == CustomKeys.BGL5.key
		&& modifiers == CustomKeys.BGL5.modifiers)
		{
			Settings.BG_Forced ^= 16;
			S9xDisplayStateChange (WINPROC_SPRITES, !(Settings.BG_Forced & 16));
		}
		if(wParam == CustomKeys.ResetGame.key
		&& modifiers == CustomKeys.ResetGame.modifiers)
		{
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_FILE_RESET),(LPARAM)(NULL));
		}
		if(wParam == CustomKeys.ToggleCheats.key
		&& modifiers == CustomKeys.ToggleCheats.modifiers)
		{
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_CHEAT_APPLY),(LPARAM)(NULL));
		}
		if(wParam == CustomKeys.JoypadSwap.key
		&& modifiers == CustomKeys.JoypadSwap.modifiers)
		{
			if(!S9xMoviePlaying())
			{
				S9xApplyCommand(S9xGetCommandT("SwapJoypads"),1,0);
			}
		}
		if(wParam == CustomKeys.SwitchControllers.key
		&& modifiers == CustomKeys.SwitchControllers.modifiers)
		{
			if((!S9xMovieActive() || !S9xMovieGetFrameCounter()))
			{
//				int prevControllerOption = GUI.ControllerOption;
//				do {
					++GUI.ControllerOption %= SNES_MAX_CONTROLLER_OPTIONS;
//				} while(!((1<<GUI.ControllerOption) & GUI.ValidControllerOptions) && prevControllerOption != GUI.ControllerOption);

				ChangeInputDevice();

				if (GUI.ControllerOption == SNES_MOUSE || GUI.ControllerOption == SNES_MOUSE_SWAPPED)
					CenterCursor();

				S9xReportControllers();
			}
		}
		if(wParam == CustomKeys.QuitS9X.key
		&& modifiers == CustomKeys.QuitS9X.modifiers)
		{
			PostMessage(GUI.hWnd,WM_CLOSE,(WPARAM)NULL,(LPARAM)(NULL));
		}
        if(wParam == CustomKeys.Rewind.key
		&& modifiers == CustomKeys.Rewind.modifiers)
		{
            if(!Settings.Rewinding)
                S9xMessage (S9X_INFO, 0, GUI.rewindBufferSize?WINPROC_REWINDING_TEXT:WINPROC_REWINDING_DISABLED);
            Settings.Rewinding = true;
			hitHotKey = true;
        }

		if (wParam == CustomKeys.SaveFileSelect.key
			&& modifiers == CustomKeys.SaveFileSelect.modifiers)
		{
			FreezeUnfreezeDialog(TRUE);
			hitHotKey = true;
		}
		if (wParam == CustomKeys.LoadFileSelect.key
			&& modifiers == CustomKeys.LoadFileSelect.modifiers)
		{
			FreezeUnfreezeDialog(FALSE);
			hitHotKey = true;
		}

		if (wParam == CustomKeys.Mute.key
			&& modifiers == CustomKeys.Mute.modifiers)
		{
			GUI.Mute = !GUI.Mute;
			hitHotKey = true;
		}

		if (wParam == CustomKeys.ToggleBackdrop.key && modifiers == CustomKeys.ToggleBackdrop.modifiers)
		{
			auto cmd = S9xGetCommandT("ToggleBackdrop");
			S9xApplyCommand(cmd, 1, 0);
            hitHotKey = true;
		}

        if (wParam == CustomKeys.AspectRatio.key
            && modifiers == CustomKeys.AspectRatio.modifiers)
        {
            if (GUI.AspectWidth == ASPECT_WIDTH_4_3)
            {
                GUI.AspectWidth = ASPECT_WIDTH_8_7;
            }
            else
            {
                GUI.AspectWidth = ASPECT_WIDTH_4_3;
            }
            hitHotKey = true;
        }
        if (wParam == CustomKeys.CheatEditorDialog.key
            && modifiers == CustomKeys.CheatEditorDialog.modifiers)
        {
            // update menu state
            CheckMenuStates();
            // check menu state if item is enabled
            MENUITEMINFO mii = { 0 };
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_STATE;
            GetMenuItemInfo(GUI.hMenu, ID_CHEAT_ENTER, FALSE, &mii);
            if ((mii.fState & MFS_DISABLED) != MFS_DISABLED)
            {
                WinShowCheatEditorDialog();
            }
            hitHotKey = true;
        }
        if (wParam == CustomKeys.CheatSearchDialog.key
            && modifiers == CustomKeys.CheatSearchDialog.modifiers)
        {
            // update menu state
            CheckMenuStates();
            // check menu state if item is enabled
            MENUITEMINFO mii = { 0 };
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_STATE;
            GetMenuItemInfo(GUI.hMenu, ID_CHEAT_SEARCH, FALSE, &mii);
            if((mii.fState & MFS_DISABLED) != MFS_DISABLED)
            {
                WinShowCheatSearchDialog();
            }
            hitHotKey = true;
        }

		//if(wParam == CustomKeys.BGLHack.key
		//&& modifiers == CustomKeys.BGLHack.modifiers)
		//{
		//	Settings.BGLayering = !Settings.BGLayering;
		//	S9xDisplayStateChange (WINPROC_BGHACK,
		//		Settings.BGLayering);
		//}
		//if(wParam == CustomKeys.InterpMode7.key
		//&& modifiers == CustomKeys.InterpMode7.modifiers)
		//{
		//	Settings.Mode7Interpolate ^= TRUE;
		//	S9xDisplayStateChange (WINPROC_MODE7INTER,
		//		Settings.Mode7Interpolate);
		//}

		if(wParam == CustomKeys.TurboA.key && modifiers == CustomKeys.TurboA.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_A),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboB.key && modifiers == CustomKeys.TurboB.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_B),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboY.key && modifiers == CustomKeys.TurboY.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_Y),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboX.key && modifiers == CustomKeys.TurboX.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_X),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboL.key && modifiers == CustomKeys.TurboL.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_L),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboR.key && modifiers == CustomKeys.TurboR.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_R),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboStart.key && modifiers == CustomKeys.TurboStart.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_START),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboSelect.key && modifiers == CustomKeys.TurboSelect.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_SELECT),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboLeft.key && modifiers == CustomKeys.TurboLeft.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_LEFT),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboUp.key && modifiers == CustomKeys.TurboUp.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_UP),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboRight.key && modifiers == CustomKeys.TurboRight.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_RIGHT),(LPARAM)(NULL));
		if(wParam == CustomKeys.TurboDown.key && modifiers == CustomKeys.TurboDown.modifiers)
			PostMessage(GUI.hWnd, WM_COMMAND, (WPARAM)(ID_TURBO_DOWN),(LPARAM)(NULL));

		for(int i = 0 ; i < SAVE_SLOTS_PER_BANK; i++)
		{
			if(wParam == CustomKeys.SelectSave[i].key && modifiers == CustomKeys.SelectSave[i].modifiers)
			{
				GUI.CurrentSaveSlot = GUI.CurrentSaveBank * SAVE_SLOTS_PER_BANK + i;

				ShowStatusSlotInfo();

				hitHotKey = true;
			}
		}

		if(wParam == CustomKeys.Transparency.key
		&& modifiers == CustomKeys.Transparency.modifiers)
		{
//					if (Settings.SixteenBit)
			{
				Settings.Transparency = !Settings.Transparency;
				S9xDisplayStateChange (WINPROC_TRANSPARENCY,
					Settings.Transparency);
			}
//					else
//					{
//						S9xSetInfoString ("Transparency requires Sixteen Bit mode.");
//					}
		}
		if(wParam == CustomKeys.ClippingWindows.key
		&& modifiers == CustomKeys.ClippingWindows.modifiers)
		{
			Settings.DisableGraphicWindows = !Settings.DisableGraphicWindows;
			S9xDisplayStateChange (WINPROC_CLIPWIN,
				!Settings.DisableGraphicWindows);
		}

		// don't pull down menu if alt is a hotkey or the menu isn't there, unless no game is running
		if(!Settings.StopEmulation && ((wParam == VK_MENU || wParam == VK_F10) && (hitHotKey || GetMenu (GUI.hWnd) == NULL) && !GetAsyncKeyState(VK_F4)))
			return 0;
	}

	if(!hitHotKey)
	switch (wParam)
	{
		case VK_ESCAPE:
			if(
                GUI.outputMethod!=DIRECTDRAW &&
                GUI.FullScreen && !GUI.EmulateFullscreen)
				ToggleFullScreen();
			else
				if (GetMenu (GUI.hWnd) == NULL)
					SetMenu (GUI.hWnd, GUI.hMenu);
				else
					SetMenu (GUI.hWnd, NULL);

			//UpdateBackBuffer();
			if (GetMenu( GUI.hWnd) != NULL)
				DrawMenuBar (GUI.hWnd);
			break;
	}
	return 1;
}

static bool DoOpenRomDialog(TCHAR filename [_MAX_PATH], bool noCustomDlg = false)
{
	if(GUI.CustomRomOpen && !noCustomDlg)
	{
		try
		{
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icex.dwICC   = ICC_LISTVIEW_CLASSES|ICC_TREEVIEW_CLASSES;
			InitCommonControlsEx(&icex); // this could cause failure if the common control DLL isn't found

			return (1 <= DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_OPEN_ROM), GUI.hWnd, DlgOpenROMProc, (LPARAM)filename));
		}
		catch(...) {} // use standard dialog if the special one fails

		GUI.CustomRomOpen = false; // if crashed, turn off custom for next time
	}

	// standard file dialog
	{
		OPENFILENAME ofn;
		static TCHAR szFileName[MAX_PATH] = {0};
		TCHAR szPathName[MAX_PATH];
		_tfullpath(szPathName, S9xGetDirectoryT(ROM_DIR), MAX_PATH);

		// a limited strcat that doesn't mind null characters
#define strcat0(to,from) do{memcpy(to,from,sizeof(from)-1);to+=(sizeof(from)/sizeof(TCHAR))-1;}while(false)

		// make filter string using entries in valid_ext
		TCHAR lpfilter [8192] = {0};
		TCHAR* lpfilterptr = lpfilter;
		for(int i=0; i<2; i++)
		{
			if(!i)
				strcat0(lpfilterptr, FILE_INFO_ROM_FILE_TYPE);
			else
				strcat0(lpfilterptr, FILE_INFO_UNCROM_FILE_TYPE);
			strcat0(lpfilterptr, TEXT("\0"));
			if(valid_ext) // add valid extensions to string
			{
				ExtList* ext = valid_ext;
				int extlen_approx = 0;
				bool first = true;
				while(ext && (extlen_approx < 2048))
				{
					if((!i || !ext->compressed) && ext->extension && lstrlen(ext->extension) < 256)
					{
						if(!first)
							lstrcat(lpfilterptr, TEXT(";*."));
						else
						{
							lstrcat(lpfilterptr, TEXT("*."));
							first = false;
						}
						lstrcat(lpfilterptr, ext->extension);
						extlen_approx += lstrlen(ext->extension) + 3;
					}
					ext = ext->next;
				}
				lpfilterptr += lstrlen(lpfilterptr);
			}
			else
				strcat0(lpfilterptr, TEXT("*.smc"));
			strcat0(lpfilterptr, TEXT("\0"));
		}
		strcat0(lpfilterptr, FILE_INFO_ANY_FILE_TYPE);
		strcat0(lpfilterptr, TEXT("\0*.*\0\0"));

		memset((LPVOID)&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GUI.hWnd;
		ofn.lpstrFilter = lpfilter;
		ofn.lpstrFile = szFileName;
		ofn.lpstrDefExt = TEXT("smc");
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
		ofn.lpstrInitialDir = szPathName;
		if(GetOpenFileName(&ofn))
		{
			_tcsncpy(filename, ofn.lpstrFile, _MAX_PATH);
			return true;
		}
		return false;
	}
}

bool WinMoviePlay(LPCTSTR filename)
{
	struct MovieInfo info;
	int err;

	if (Settings.StopEmulation) {
		SendMenuCommand(ID_FILE_LOAD_GAME);
		if (Settings.StopEmulation)
			return false;
	}

	err = S9xMovieGetInfo(_tToChar(filename), &info);
	if (err != SUCCESS) {
		TCHAR* err_string = MOVIE_ERR_COULD_NOT_OPEN;
		switch(err)
		{
		case FILE_NOT_FOUND:
			err_string = MOVIE_ERR_NOT_FOUND_SHORT;
			break;
		case WRONG_FORMAT:
			err_string = MOVIE_ERR_WRONG_FORMAT_SHORT;
			break;
		case WRONG_VERSION:
			err_string = MOVIE_ERR_WRONG_VERSION_SHORT;
			break;
		}
		S9xSetInfoString(_tToChar(err_string));
		return false;
	}

	while (info.ROMCRC32 != Memory.ROMCRC32) {
		TCHAR temp[512];
		CPToWide wromname(info.ROMName, 932);
		wsprintf(temp, TEXT("Movie's ROM: crc32=%08X, name=%s\nCurrent ROM: crc32=%08X\n\nstill want to play the movie?"),
			info.ROMCRC32, (wchar_t *)wromname, Memory.ROMCRC32);
		int sel = MessageBox(GUI.hWnd, temp, SNES9X_INFO, MB_ABORTRETRYIGNORE|MB_ICONQUESTION);
		switch (sel) {
		case IDABORT:
			return false;
		case IDRETRY:
			SendMenuCommand(ID_FILE_LOAD_GAME);
			if (Settings.StopEmulation)
				return false;
			break;
		default:
			goto romcheck_exit;
		}
	}
	romcheck_exit:

	err = S9xMovieOpen (_tToChar(filename), GUI.MovieReadOnly);
	if(err != SUCCESS)
	{
		TCHAR* err_string = MOVIE_ERR_COULD_NOT_OPEN;
		switch(err)
		{
		case FILE_NOT_FOUND:
			err_string = MOVIE_ERR_NOT_FOUND_SHORT;
			break;
		case WRONG_FORMAT:
			err_string = MOVIE_ERR_WRONG_FORMAT_SHORT;
			break;
		case WRONG_VERSION:
			err_string = MOVIE_ERR_WRONG_VERSION_SHORT;
			break;
		}
		S9xSetInfoString(_tToChar(err_string));
		return false;
	}
	return true;
}

static bool startingMovie = false;

HWND cheatSearchHWND = NULL;

void WinShowCheatSearchDialog()
{
    RestoreGUIDisplay();
    if (!cheatSearchHWND) // create and show non-modal cheat search window
    {
        cheatSearchHWND = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_CHEAT_SEARCH), GUI.hWnd, DlgCheatSearch); // non-modal/modeless
        ShowWindow(cheatSearchHWND, SW_SHOW);
    }
    else // already open so just reactivate the window
    {
        SetActiveWindow(cheatSearchHWND);
    }
    RestoreSNESDisplay();
}

void WinShowCheatEditorDialog()
{
    RestoreGUIDisplay();
    while (DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CHEATER), GUI.hWnd, DlgCheater) == NC_SEARCHDB)
    {
        WinSearchCheatDatabase();
    }
    S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
    RestoreSNESDisplay();
}


#define MOVIE_LOCKED_SETTING	if(S9xMovieActive()) {MessageBox(GUI.hWnd,TEXT("That setting is locked while a movie is active."),TEXT("Notice"),MB_OK|MB_ICONEXCLAMATION); break;}

LRESULT CALLBACK WinProc(
						 HWND hWnd,
						 UINT uMsg,
						 WPARAM wParam,
						 LPARAM lParam)
{
    unsigned int i;
    //bool showFPS;
#ifdef NETPLAY_SUPPORT
    TCHAR hostname [100];
#endif
    switch (uMsg)
    {
	case WM_CREATE:
		g_hInst = ((LPCREATESTRUCT)lParam)->hInstance;
		DragAcceptFiles(hWnd, TRUE);
		return 0;
	case WM_KEYDOWN:
		if(GUI.BackgroundInput && !GUI.InactivePause)
			break;
	case WM_CUSTKEYDOWN:
	case WM_SYSKEYDOWN:
		{
			if(!HandleKeyMessage(wParam,lParam))
				return 0;
	        break;
		}

	case WM_KEYUP:
	case WM_CUSTKEYUP:
		{
			int modifiers = 0;
			if(GetAsyncKeyState(VK_MENU) || wParam == VK_MENU)
				modifiers |= CUSTKEY_ALT_MASK;
			if(GetAsyncKeyState(VK_CONTROL)|| wParam == VK_CONTROL)
				modifiers |= CUSTKEY_CTRL_MASK;
			if(GetAsyncKeyState(VK_SHIFT)|| wParam == VK_SHIFT)
				modifiers |= CUSTKEY_SHIFT_MASK;

			if(wParam == CustomKeys.FastForward.key
			&& modifiers == CustomKeys.FastForward.modifiers)
			{
				Settings.TurboMode = FALSE;
			}
			if(wParam == CustomKeys.ScopePause.key
			&& modifiers == CustomKeys.ScopePause.modifiers)
			{
				GUI.superscope_pause = 0;
			}
            if(wParam == CustomKeys.Rewind.key
		    && modifiers == CustomKeys.Rewind.modifiers)
		    {
                Settings.Rewinding = false;
            }

		}
		break;

	case WM_DROPFILES:
		HDROP hDrop;
		UINT fileCount;
		TCHAR droppedFile[PATH_MAX];

		hDrop = (HDROP)wParam;
		fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		if (fileCount == 1) {
			DragQueryFile(hDrop, 0, droppedFile, PATH_MAX);

			LPCTSTR ext = PathFindExtension(droppedFile);
			if (ExtensionIsValid(droppedFile)) {
				LoadROM(droppedFile);
			}
			else if (lstrcmpi(ext, TEXT(".smv")) == 0) {
				WinMoviePlay(droppedFile);
			}
			else {
				S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Unknown file extension.");
			}
		}
		DragFinish(hDrop);

		return 0;

	case WM_COMMAND:
	{
		int cmd_id = wParam & 0xffff;
		if (cmd_id >= ID_FILE_SAVE0 && cmd_id < ID_FILE_SAVE0 + NUM_SAVE_BANKS * SAVE_SLOTS_PER_BANK)
		{
			FreezeUnfreezeSlot(cmd_id - ID_FILE_SAVE0, TRUE);
			break;
		}
		else if (cmd_id >= ID_FILE_LOAD0 && cmd_id < ID_FILE_LOAD0 + NUM_SAVE_BANKS * SAVE_SLOTS_PER_BANK)
		{
			FreezeUnfreezeSlot(cmd_id - ID_FILE_LOAD0, FALSE);
			break;
		}
		switch (cmd_id)
		{
		case ID_FILE_AVI_RECORDING:
			if (!GUI.AVIOut)
				PostMessage(GUI.hWnd, WM_COMMAND, ID_FILE_WRITE_AVI, NULL);
			else
				PostMessage(GUI.hWnd, WM_COMMAND, ID_FILE_STOP_AVI, NULL);
			break;
		case ID_FILE_WRITE_AVI:
			{
				RestoreGUIDisplay ();  //exit DirectX
				OPENFILENAME  ofn;
				TCHAR  szFileName[MAX_PATH];
				TCHAR  szPathName[MAX_PATH];
				SetCurrentDirectory(S9xGetDirectoryT(DEFAULT_DIR));
				_tfullpath(szPathName, GUI.MovieDir, MAX_PATH);
				_tmkdir(szPathName);

				szFileName[0] = TEXT('\0');

				memset( (LPVOID)&ofn, 0, sizeof(OPENFILENAME) );
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = GUI.hWnd;
				ofn.lpstrFilter = FILE_INFO_AVI_FILE_TYPE TEXT("\0*.avi\0") FILE_INFO_ANY_FILE_TYPE TEXT("\0*.*\0\0");
				ofn.lpstrFile = szFileName;
				ofn.lpstrDefExt = TEXT("avi");
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
				ofn.lpstrInitialDir = szPathName;
				if(GetSaveFileName( &ofn ))
				{
					DoAVIOpen(szFileName);
				}
				RestoreSNESDisplay ();// re-enter after dialog
			}
			break;
		case ID_FILE_STOP_AVI:
			DoAVIClose(0);
			ReInitSound();				// reenable sound output
			break;
		case ID_FILE_MOVIE_STOP:
			S9xMovieStop(FALSE);
			break;
		case ID_FILE_MOVIE_PLAY:
			{
				RestoreGUIDisplay ();  //exit DirectX
				OpenMovieParams op;
				memset(&op, 0, sizeof(op));
				if(DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_OPENMOVIE), hWnd, DlgOpenMovie, (LPARAM)&op) &&
					op.Path[0]!='\0')
				{
					int err=S9xMovieOpen (_tToChar(op.Path), op.ReadOnly);
					if(err!=SUCCESS)
					{
						TCHAR* err_string=MOVIE_ERR_COULD_NOT_OPEN;
						switch(err)
						{
						case FILE_NOT_FOUND:
							err_string=MOVIE_ERR_NOT_FOUND;
							break;
						case WRONG_FORMAT:
							err_string=MOVIE_ERR_WRONG_FORMAT;
							break;
						case WRONG_VERSION:
							err_string=MOVIE_ERR_WRONG_VERSION;
							break;
						}
						MessageBox( hWnd, err_string, SNES9X_INFO, MB_OK);
					}
				}
				RestoreSNESDisplay ();// re-enter after dialog
			}
			break;
		case ID_FILE_MOVIE_RECORD:
			{
				RestoreGUIDisplay ();  //exit DirectX
				OpenMovieParams op;
				memset(&op, 0, sizeof(op));
				if(DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CREATEMOVIE), hWnd, DlgCreateMovie, (LPARAM)&op) &&
					op.Path[0]!='\0')
				{
					startingMovie = true;
					int err=S9xMovieCreate (_tToChar(op.Path), op.ControllersMask, op.Opts, op.Metadata, wcslen(op.Metadata));
					startingMovie = false;
					if(err!=SUCCESS)
					{
						TCHAR* err_string=MOVIE_ERR_COULD_NOT_OPEN;
						switch(err)
						{
						case FILE_NOT_FOUND:
							err_string=MOVIE_ERR_NOT_FOUND;
							break;
						case WRONG_FORMAT:
							err_string=MOVIE_ERR_WRONG_FORMAT;
							break;
						case WRONG_VERSION:
							err_string=MOVIE_ERR_WRONG_VERSION;
							break;
						}
						MessageBox( hWnd, err_string, SNES9X_INFO, MB_OK);
					}
				}
				RestoreSNESDisplay ();// re-enter after dialog
			}
			break;
		case IDM_SNES_JOYPAD:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_JOYPAD;
			ChangeInputDevice();
			break;
		case IDM_ENABLE_MULTITAP:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_MULTIPLAYER5;
			ChangeInputDevice();
			break;
		case IDM_SCOPE_TOGGLE:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_SUPERSCOPE;
			ChangeInputDevice();
			break;
		case IDM_JUSTIFIER:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_JUSTIFIER;
			ChangeInputDevice();
			break;
		case IDM_MOUSE_TOGGLE:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_MOUSE;
			ChangeInputDevice();
			break;
		case IDM_MOUSE_SWAPPED:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_MOUSE_SWAPPED;
			ChangeInputDevice();
			break;
		case IDM_MULTITAP8:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_MULTIPLAYER8;
			ChangeInputDevice();
			break;
		case IDM_JUSTIFIERS:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_JUSTIFIER_2;
			ChangeInputDevice();
			break;
		case IDM_MACSRIFLE_TOGGLE:
			MOVIE_LOCKED_SETTING
			GUI.ControllerOption = SNES_MACSRIFLE;
			ChangeInputDevice();
			break;

			//start turbo
		case ID_TURBO_R:
			GUI.TurboMask^=TURBO_R_MASK;
			if(GUI.TurboMask&TURBO_R_MASK)
				S9xSetInfoString (WINPROC_TURBO_R_ON);
			else S9xSetInfoString (WINPROC_TURBO_R_OFF);
			break;
		case ID_TURBO_L:
			GUI.TurboMask^=TURBO_L_MASK;
			if(GUI.TurboMask&TURBO_L_MASK)
				S9xSetInfoString (WINPROC_TURBO_L_ON);
			else S9xSetInfoString (WINPROC_TURBO_L_OFF);
			break;
		case ID_TURBO_A:
			GUI.TurboMask^=TURBO_A_MASK;
			if(GUI.TurboMask&TURBO_A_MASK)
				S9xSetInfoString (WINPROC_TURBO_A_ON);
			else S9xSetInfoString (WINPROC_TURBO_A_OFF);
			break;
		case ID_TURBO_B:
			GUI.TurboMask^=TURBO_B_MASK;
			if(GUI.TurboMask&TURBO_B_MASK)
				S9xSetInfoString (WINPROC_TURBO_B_ON);
			else S9xSetInfoString (WINPROC_TURBO_B_OFF);
			break;
		case ID_TURBO_Y:
			GUI.TurboMask^=TURBO_Y_MASK;
			if(GUI.TurboMask&TURBO_Y_MASK)
				S9xSetInfoString (WINPROC_TURBO_Y_ON);
			else S9xSetInfoString (WINPROC_TURBO_Y_OFF);
			break;
		case ID_TURBO_X:
			GUI.TurboMask^=TURBO_X_MASK;
			if(GUI.TurboMask&TURBO_X_MASK)
				S9xSetInfoString (WINPROC_TURBO_X_ON);
			else S9xSetInfoString (WINPROC_TURBO_X_OFF);
			break;
		case ID_TURBO_START:
			GUI.TurboMask^=TURBO_STA_MASK;
			if(GUI.TurboMask&TURBO_STA_MASK)
				S9xSetInfoString (WINPROC_TURBO_START_ON);
			else S9xSetInfoString (WINPROC_TURBO_START_OFF);
			break;
		case ID_TURBO_SELECT:
			GUI.TurboMask^=TURBO_SEL_MASK;
			if(GUI.TurboMask&TURBO_SEL_MASK)
				S9xSetInfoString (WINPROC_TURBO_SEL_ON);
			else S9xSetInfoString (WINPROC_TURBO_SEL_OFF);
			break;
		case ID_TURBO_LEFT:
			GUI.TurboMask^=TURBO_LEFT_MASK;
			if(GUI.TurboMask&TURBO_LEFT_MASK)
				S9xSetInfoString (WINPROC_TURBO_LEFT_ON);
			else S9xSetInfoString (WINPROC_TURBO_LEFT_OFF);
			break;
		case ID_TURBO_UP:
			GUI.TurboMask^=TURBO_UP_MASK;
			if(GUI.TurboMask&TURBO_UP_MASK)
				S9xSetInfoString (WINPROC_TURBO_UP_ON);
			else S9xSetInfoString (WINPROC_TURBO_UP_OFF);
			break;
		case ID_TURBO_RIGHT:
			GUI.TurboMask^=TURBO_RIGHT_MASK;
			if(GUI.TurboMask&TURBO_RIGHT_MASK)
				S9xSetInfoString (WINPROC_TURBO_RIGHT_ON);
			else S9xSetInfoString (WINPROC_TURBO_RIGHT_OFF);
			break;
		case ID_TURBO_DOWN:
			GUI.TurboMask^=TURBO_DOWN_MASK;
			if(GUI.TurboMask&TURBO_DOWN_MASK)
				S9xSetInfoString (WINPROC_TURBO_DOWN_ON);
			else S9xSetInfoString (WINPROC_TURBO_DOWN_OFF);
			break;
			//end turbo
		case ID_OPTIONS_DISPLAY:
			{
				RestoreGUIDisplay ();

				if(GUI.FullScreen)
					ToggleFullScreen();
				DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NEWDISPLAY), hWnd, DlgFunky);

				SwitchToGDI();

				RestoreSNESDisplay ();

				S9xSetWinPixelFormat ();
				S9xInitUpdate();

				IPPU.RenderThisFrame = false;

				RECT rect;
				GetClientRect (GUI.hWnd, &rect);
				InvalidateRect (GUI.hWnd, &rect, true);
				break;
			}

		case ID_OPTIONS_JOYPAD:
            RestoreGUIDisplay ();
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_INPUTCONFIG), hWnd, DlgInputConfig);
            RestoreSNESDisplay ();
            ChangeInputDevice ();
            break;

		case ID_OPTIONS_KEYCUSTOM:
            RestoreGUIDisplay ();
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_KEYCUSTOM), hWnd, DlgHotkeyConfig);
            RestoreSNESDisplay ();
            break;

		case ID_EMULATION_BACKGROUNDINPUT:
			GUI.BackgroundInput = !GUI.BackgroundInput;
			if(!GUI.hHotkeyTimer)
				GUI.hHotkeyTimer = timeSetEvent (32, 0, (LPTIMECALLBACK)HotkeyTimer, 0, TIME_PERIODIC);
			break;

        case ID_INPUT_BACKGROUNDKEYBOARDHOTKEYS:
            GUI.BackgroundKeyHotkeys = !GUI.BackgroundKeyHotkeys;
            break;

        case ID_INPUT_DETECTGAMEPADCHANGES:
            S9xDetectJoypads();
            break;

		case ID_FILE_LOADMULTICART:
			{
				RestoreGUIDisplay ();

				const bool ok = (1 <= DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_MULTICART), GUI.hWnd, DlgMultiROMProc, (LPARAM)NULL));

				if(ok)
				{
					LoadROM(multiRomA, multiRomB);
				}

				RestoreSNESDisplay ();
			}
			break;

		case ID_FILE_LOAD_GAME:
			{
				TCHAR filename [_MAX_PATH];

				RestoreGUIDisplay ();

				if(DoOpenRomDialog(filename)) {
					LoadROM(filename);
				}

				RestoreSNESDisplay ();
			}
			break;

		case ID_FILE_EXIT:
            S9xSetPause (PAUSE_EXIT);
            PostMessage (hWnd, WM_CLOSE, 0, 0);
            break;

		case ID_WINDOW_HIDEMENUBAR:
            if( GetMenu( GUI.hWnd) == NULL)
                SetMenu( GUI.hWnd, GUI.hMenu);
            else
                SetMenu( GUI.hWnd, NULL);
            break;

#ifdef NETPLAY_SUPPORT
		case ID_NETPLAY_SERVER:
            S9xRestoreWindowTitle ();
            EnableServer (!Settings.NetPlayServer);
			if(Settings.NetPlayServer)
			{
				TCHAR localhostmsg [512];
				// FIXME: need winsock2.h for this, don't know how to include it
				//struct addrinfo *aiList = NULL;
				//if(getaddrinfo("localhost", Settings.Port, NULL, &aiList) == 0)
				//{
				//	sprintf(localhostmsg, "Your server address is: %s", aiList->ai_canonname);
				//	MessageBox(GUI.hWnd,localhostmsg,"Note",MB_OK);
				//}
				//else
				{
					char localhostname [256];
					gethostname(localhostname,256);
					_stprintf(localhostmsg, TEXT("Your host name is: %s\nYour port number is: %d"), (TCHAR *)_tFromChar(localhostname), Settings.Port);
					MessageBox(GUI.hWnd,localhostmsg,TEXT("Note"),MB_OK);
				}
			}
            break;
        case ID_NETPLAY_CONNECT:
            RestoreGUIDisplay ();
			if(1<=DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_NETCONNECT), hWnd, DlgNetConnect,(LPARAM)&hostname))

            {


				S9xSetPause (PAUSE_NETPLAY_CONNECT);

				if (!S9xNPConnectToServer (_tToChar(hostname), Settings.Port,
					Memory.ROMName))
                {
                    S9xClearPause (PAUSE_NETPLAY_CONNECT);
                }
            }

			RestoreSNESDisplay ();
            break;
        case ID_NETPLAY_DISCONNECT:
            if (Settings.NetPlay)
            {
                Settings.NetPlay = FALSE;
                S9xNPDisconnect ();
            }
            if (Settings.NetPlayServer)
            {
                Settings.NetPlayServer = FALSE;
                S9xNPStopServer ();
            }
            break;
        case ID_NETPLAY_OPTIONS:
			{
				bool8 old_netplay_server = Settings.NetPlayServer;
				RestoreGUIDisplay ();
				if(1<=DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NPOPTIONS), hWnd, DlgNPOptions))
				{
					if (old_netplay_server != Settings.NetPlayServer)
					{
						Settings.NetPlayServer = old_netplay_server;
						S9xRestoreWindowTitle ();
						EnableServer (!Settings.NetPlayServer);
					}
				}
				RestoreSNESDisplay ();
				break;
			}
        case ID_NETPLAY_SYNC:
            S9xNPServerQueueSyncAll ();
            break;
        case ID_NETPLAY_ROM:
            if (NPServer.SyncByReset)
            {
			if (MessageBox (GUI.hWnd, TEXT(WINPROC_NET_RESTART), SNES9X_WARN,
											MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
											break;
            }
            S9xNPServerQueueSendingROMImage ();
            break;
        case ID_NETPLAY_SEND_ROM_ON_CONNECT:
            NPServer.SendROMImageOnConnect ^= TRUE;
            break;
        case ID_NETPLAY_SYNC_BY_RESET:
            NPServer.SyncByReset ^= TRUE;
            break;
#endif
        case ID_SOUND_8000HZ:
		case ID_SOUND_11025HZ:
		case ID_SOUND_16000HZ:
		case ID_SOUND_22050HZ:
		case ID_SOUND_30000HZ:
		case ID_SOUND_35000HZ:
		case ID_SOUND_44100HZ:
		case ID_SOUND_48000HZ:
		case ID_SOUND_32000HZ:
            for( i = 0; i < COUNT(SoundRates); i ++)
				if (SoundRates[i].ident == (int) wParam)
				{
                    Settings.SoundPlaybackRate = SoundRates [i].rate;
					GUI.Mute = false;
					ReInitSound();
                    break;
				}
				break;

		case ID_SOUND_16MS:
			GUI.SoundBufferSize = 16;
			ReInitSound();
			break;
		case ID_SOUND_32MS:
			GUI.SoundBufferSize = 32;
			ReInitSound();
			break;
		case ID_SOUND_48MS:
			GUI.SoundBufferSize = 48;
			ReInitSound();
			break;
		case ID_SOUND_64MS:
			GUI.SoundBufferSize = 64;
			ReInitSound();
			break;
		case ID_SOUND_80MS:
			GUI.SoundBufferSize = 80;
			ReInitSound();
			break;
		case ID_SOUND_96MS:
			GUI.SoundBufferSize = 96;
			ReInitSound();
			break;
		case ID_SOUND_112MS:
			GUI.SoundBufferSize = 112;
			ReInitSound();
			break;
		case ID_SOUND_128MS:
			GUI.SoundBufferSize = 128;
			ReInitSound();
			break;
		case ID_SOUND_144MS:
			GUI.SoundBufferSize = 144;
			ReInitSound();
			break;
		case ID_SOUND_160MS:
			GUI.SoundBufferSize = 160;
			ReInitSound();
			break;
		case ID_SOUND_176MS:
			GUI.SoundBufferSize = 176;
			ReInitSound();
			break;
		case ID_SOUND_192MS:
			GUI.SoundBufferSize = 192;
			ReInitSound();
			break;
		case ID_SOUND_208MS:
			GUI.SoundBufferSize = 208;
			ReInitSound();
			break;


        case ID_CHANNELS_CHANNEL1: S9xToggleSoundChannel(0); break;
        case ID_CHANNELS_CHANNEL2: S9xToggleSoundChannel(1); break;
        case ID_CHANNELS_CHANNEL3: S9xToggleSoundChannel(2); break;
        case ID_CHANNELS_CHANNEL4: S9xToggleSoundChannel(3); break;
        case ID_CHANNELS_CHANNEL5: S9xToggleSoundChannel(4); break;
        case ID_CHANNELS_CHANNEL6: S9xToggleSoundChannel(5); break;
        case ID_CHANNELS_CHANNEL7: S9xToggleSoundChannel(6); break;
        case ID_CHANNELS_CHANNEL8: S9xToggleSoundChannel(7); break;
        case ID_CHANNELS_ENABLEALL: S9xToggleSoundChannel(8); break;

		case ID_SOUND_NOSOUND:
			S9xSetSoundMute(!GUI.Mute);
			GUI.Mute = !GUI.Mute;
            break;

        case ID_SOUND_SYNC:
            Settings.SoundSync = !Settings.SoundSync;
			S9xDisplayStateChange (WINPROC_SYNC_SND, Settings.SoundSync);
            break;
        case ID_SOUND_OPTIONS:
			{
				RestoreGUIDisplay ();
				if(1<=DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_SOUND_OPTS),hWnd,DlgSoundConf, (LPARAM)&Settings))
				{
					ReInitSound();
				}
				RestoreSNESDisplay ();
				break;
			}
		case ID_WINDOW_FULLSCREEN:
			ToggleFullScreen ();
			break;
		case ID_WINDOW_SIZE_1X:
		case ID_WINDOW_SIZE_2X:
		case ID_WINDOW_SIZE_3X:
		case ID_WINDOW_SIZE_4X:
		case ID_WINDOW_SIZE_5X:
		case ID_WINDOW_SIZE_6X:
		case ID_WINDOW_SIZE_7X:
		case ID_WINDOW_SIZE_8X:
		case ID_WINDOW_SIZE_9X:
		case ID_WINDOW_SIZE_10X:
			UINT factor, newWidth, newHeight;
			RECT margins;
			factor = (wParam & 0xffff) - ID_WINDOW_SIZE_1X + 1;
			newWidth = GUI.AspectWidth * factor;
			newHeight = (Settings.ShowOverscan ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT) * factor;

			margins = GetWindowMargins(GUI.hWnd,newWidth);
			newHeight += margins.top + margins.bottom;
			newWidth += margins.left + margins.right;

			SetWindowPos(GUI.hWnd, NULL, 0, 0, newWidth, newHeight, SWP_NOMOVE);
			break;
		case ID_WINDOW_STRETCH:
			GUI.Stretch = !GUI.Stretch;
			WinDisplayApplyChanges();
			WinRefreshDisplay();
			break;
		case ID_WINDOW_ASPECTRATIO:
			GUI.AspectRatio = !GUI.AspectRatio;
			WinDisplayApplyChanges();
			WinRefreshDisplay();
			break;
		case ID_WINDOW_BILINEAR:
			Settings.BilinearFilter = !Settings.BilinearFilter;
			WinDisplayApplyChanges();
			WinRefreshDisplay();
			break;
		case ID_VIDEO_SHOWFRAMERATE:
			Settings.DisplayFrameRate = !Settings.DisplayFrameRate;
			break;
		case ID_SAVESCREENSHOT:
			Settings.TakeScreenshot=true;
			break;
		case ID_FILE_SAVE_SPC_DATA:
			S9xDumpSPCSnapshot();
			S9xMessage(S9X_INFO, 0, INFO_SAVE_SPC);
			break;
		case ID_FILE_SAVE_SRAM_DATA: {
			bool8 success = Memory.SaveSRAM (S9xGetFilename (".srm", SRAM_DIR).c_str());
			if(!success)
				S9xMessage(S9X_ERROR, S9X_FREEZE_FILE_INFO, SRM_SAVE_FAILED);
		}	break;
		case ID_SAVEMEMPACK: {
			std::string filename = S9xGetFilenameInc(".bs", SRAM_DIR);
			bool8 success = Memory.SaveMPAK(filename.c_str());
			if (!success)
				S9xMessage(S9X_ERROR, 0, MPAK_SAVE_FAILED);
			else
			{
				sprintf(String, "Saved Memory Pack %s", filename.c_str());
				S9xMessage(S9X_INFO, 0, String);
			}
		}	break;
		case ID_FILE_RESET:
#ifdef NETPLAY_SUPPORT
			if (Settings.NetPlayServer)
			{
				S9xNPReset ();
				ReInitSound();
			}
			else
				if (!Settings.NetPlay)
#endif
				{
					S9xMovieUpdateOnReset ();
					if(S9xMoviePlaying())
						S9xMovieStop (TRUE);
					S9xSoftReset ();
					ReInitSound();
				}
				if(!S9xMovieRecording())
					Settings.Paused = false;
				break;
		case ID_FILE_PAUSE:
			Settings.Paused = !Settings.Paused;
			Settings.FrameAdvance = false;
			GUI.FrameAdvanceJustPressed = 0;
			break;
		case ID_FILE_LOAD_OOPS:
			FreezeUnfreezeSlot(-1, FALSE);
			break;
        case ID_FILE_LOAD_FILE:
            FreezeUnfreezeDialog(FALSE);
            break;
        case ID_FILE_LOAD_PREVIEW:
            FreezeUnfreezeDialogPreview(FALSE);
            break;
        case ID_FILE_SAVE_FILE:
            FreezeUnfreezeDialog(TRUE);
            break;
        case ID_FILE_SAVE_PREVIEW:
            FreezeUnfreezeDialogPreview(TRUE);
            break;
		case ID_CHEAT_ENTER:
            WinShowCheatEditorDialog();
			break;
		case ID_CHEAT_SEARCH:
            WinShowCheatSearchDialog();
			break;
		case ID_CHEAT_APPLY:
			Settings.ApplyCheats = !Settings.ApplyCheats;
			if (!Settings.ApplyCheats){
				S9xCheatsDisable ();
				S9xMessage (S9X_INFO, S9X_GAME_GENIE_CODE_ERROR, CHEATS_INFO_DISABLED);
			}else{
				S9xCheatsEnable ();
				bool on = false;
				extern struct SCheatData Cheat;
				for (uint32 i = 0; i < Cheat.group.size() && !on; i++)
					if (Cheat.group[i].enabled)
						on = true;
				S9xMessage (S9X_INFO, S9X_GAME_GENIE_CODE_ERROR, on ? CHEATS_INFO_ENABLED : CHEATS_INFO_ENABLED_NONE);
			}
			break;
		case ID_EMULATION_PAUSEWHENINACTIVE:
			GUI.InactivePause = !GUI.InactivePause;
			break;
		case ID_OPTIONS_SETTINGS:
			RestoreGUIDisplay ();
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_EMU_SETTINGS), hWnd, DlgEmulatorProc);
			RestoreSNESDisplay ();
			break;
        case ID_EMULATION_HACKS:
			if (MessageBoxA(hWnd,
				"The settings in this dialog should only be used for compatibility "
				"with old ROM hacks or if you otherwise know what you're doing.\n\n"
				"If any problems occur, click \"Set Defaults\" to reset the options to normal.",
				"Warning: Unsupported",
				MB_ICONWARNING | MB_OKCANCEL) != IDOK)
				break;

            RestoreGUIDisplay();
			i = DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_HACKS), hWnd, DlgEmulatorHacksProc);
            if (i == 1)
                S9xReset();
            else
                RestoreSNESDisplay();
            break;
		case ID_HELP_ABOUT:
			RestoreGUIDisplay ();
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, DlgAboutProc);
			RestoreSNESDisplay ();
			break;
		case ID_FRAME_ADVANCE:
			Settings.Paused = true;
			Settings.FrameAdvance = true;
			break;
#ifdef DEBUGGER
		case ID_DEBUG_TRACE:
			S9xDebugProcessCommand("T");
			break;

		case ID_DEBUG_FRAME_ADVANCE:
			CPU.Flags |= FRAME_ADVANCE_FLAG;
			ICPU.FrameAdvanceCount = 1;
			Settings.Paused = FALSE;
			break;
		case ID_DEBUG_APU_TRACE:
			// TODO: reactivate once APU debugger works again
			//spc_core->debug_toggle_trace();
			break;
#endif
		case IDM_ROM_INFO:
			RestoreGUIDisplay ();
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ROM_INFO), hWnd, DlgInfoProc);
			RestoreSNESDisplay ();
			break;
		default:
			if ((wParam & 0xffff) >= 0xFF00)
			{
				int i = (wParam & 0xffff) - 0xFF00;
				int j = 0;
				{
					if (wParam == 0xFF00 + MAX_RECENT_GAMES_LIST_SIZE)
					{
						WinDeleteRecentGamesList();
						S9xSetRecentGames();
						break;
					}

					while (j < MAX_RECENT_GAMES_LIST_SIZE && j != i)
						j++;
					if (i == j)
					{

						if (!LoadROM(GUI.RecentGames [i])) {
							sprintf (String, ERR_ROM_NOT_FOUND, (char *)_tToChar(GUI.RecentGames [i]));
							S9xMessage (S9X_ERROR, S9X_ROM_NOT_FOUND, String);
							S9xRemoveFromRecentGames(i);
						}
					}
				}
			}
			break;
        }
        break;
	}
	case WM_EXITMENULOOP:
		UpdateWindow(GUI.hWnd);
		DrawMenuBar(GUI.hWnd);
		S9xClearPause (PAUSE_MENU);
		break;

	case WM_ENTERMENULOOP:
		S9xSetPause (PAUSE_MENU);
		CheckMenuStates ();

		SwitchToGDI();
		DrawMenuBar(GUI.hWnd);
		break;

	case WM_CLOSE:
		SaveMainWinPos();
		break;

	case WM_DESTROY:
		Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR).c_str());
		GUI.hWnd = NULL;
		PostQuitMessage (0);
		return (0);
	case WM_PAINT:
        {
			// refresh screen
			WinRefreshDisplay();
            break;
        }
	case WM_SYSCOMMAND:
        {
            // Prevent screen saver from starting if not paused
			//kode54 says add the ! to fix the screensaver pevention.
            if (!(Settings.ForcedPause || Settings.StopEmulation ||
				(Settings.Paused && !Settings.FrameAdvance)) &&
                (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER))
                return (0);
            break;
        }
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			if(GUI.InactivePause)
			{
				S9xSetPause (PAUSE_INACTIVE_WINDOW);
			}
		}
		else
		{
			Settings.TurboMode = false;
///			if(GUI.InactivePause)
			{
				S9xClearPause (PAUSE_INACTIVE_WINDOW);
			}
			IPPU.ColorsChanged = TRUE;
		}
		break;
	case WM_QUERYNEWPALETTE:
		//            if (!GUI.FullScreen && GUI.ScreenDepth == 8)
		//                RealizePalette (GUI.WindowDC);
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED && GUI.InactivePause)
		{
			S9xSetPause(PAUSE_INACTIVE_WINDOW);
		}
		WinChangeWindowSize(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_MOVE:
		//if (!VOODOO_MODE && !GUI.FullScreen && !Settings.ForcedPause)
		//{
		//	GetWindowRect (GUI.hWnd, &GUI.window_size);
		//}

		break;
	case WM_ENTERSIZEMOVE:
		S9xSetPause(PAUSE_MENU);
		break;
	case WM_EXITSIZEMOVE:
		S9xClearPause(PAUSE_MENU);
		break;
	case WM_DISPLAYCHANGE:
		if (!GUI.FullScreen && !(Settings.ForcedPause & PAUSE_TOGGLE_FULL_SCREEN))
		{
			WinDisplayReset();
		}
		break;
	case WM_MOUSEMOVE:
		if(Settings.StopEmulation)
		{
			SetCursor (GUI.Arrow);
			break;
		}
		// Lo-word of lparam is xpos, hi-word is ypos
//		if (!GUI.IgnoreNextMouseMove)
		{
			//POINT p;
			//p.x = GET_X_LPARAM(lParam);
			//p.y = GET_Y_LPARAM(lParam);
			//ClientToScreen (GUI.hWnd, &p);
			if ((!Settings.ForcedPause && !Settings.StopEmulation &&
				!(Settings.Paused && !Settings.FrameAdvance)) &&
				(GUI.ControllerOption==SNES_MOUSE || GUI.ControllerOption==SNES_MOUSE_SWAPPED)
			   )
			{
				CenterCursor();
			}
			else if (GUI.ControllerOption==SNES_SUPERSCOPE || GUI.ControllerOption==SNES_JUSTIFIER || GUI.ControllerOption==SNES_JUSTIFIER_2 || GUI.ControllerOption==SNES_MACSRIFLE)
			{
				RECT size;
				GetClientRect (GUI.hWnd, &size);
				if(!(GUI.Scale)&&!(GUI.Stretch))
				{
					int x,y, startx, starty;
					x=GET_X_LPARAM(lParam);
					y=GET_Y_LPARAM(lParam);

//					int theight;
//					(IPPU.RenderedScreenHeight> 256)? theight= SNES_HEIGHT_EXTENDED<<1: theight = SNES_HEIGHT_EXTENDED;
					int theight = Settings.ShowOverscan ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT;
					if(IPPU.RenderedScreenHeight > SNES_HEIGHT_EXTENDED) theight <<= 1;

					startx= size.right-IPPU.RenderedScreenWidth;
					startx/=2;
					starty= size.bottom-theight;
					starty/=2;

					if(x<startx)
						GUI.MouseX=0;
					else if(x>(startx+IPPU.RenderedScreenWidth))
						GUI.MouseX=IPPU.RenderedScreenWidth;
					else GUI.MouseX=x-startx;

					if(y<starty)
						GUI.MouseY=0;
					else if(y>(starty+theight))
						GUI.MouseY=theight;
					else GUI.MouseY=y-starty;
				}
				else if(!(GUI.Stretch))
				{
					int x,y, startx, starty, sizex, sizey;
					x=GET_X_LPARAM(lParam);
					y=GET_Y_LPARAM(lParam);

					if (IPPU.RenderedScreenWidth>256)
						sizex=IPPU.RenderedScreenWidth;
					else sizex=IPPU.RenderedScreenWidth*2;

					int theight = Settings.ShowOverscan ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT;
					sizey = (IPPU.RenderedScreenHeight > SNES_HEIGHT_EXTENDED) ? theight : (theight << 1);

					startx= size.right-sizex;
					startx/=2;
					starty= size.bottom-sizey;
					starty/=2;
					if(x<startx)
						GUI.MouseX=0;
					else if(x>(startx+sizex))
						GUI.MouseX=sizex;
					else GUI.MouseX=x-startx;

					if(y<starty)
						GUI.MouseY=0;
					else if(y>(starty+sizey))
						GUI.MouseY=sizey;
					else GUI.MouseY=y-starty;

					GUI.MouseX=(GUI.MouseX*IPPU.RenderedScreenWidth)/sizex;
					GUI.MouseY=(GUI.MouseY*IPPU.RenderedScreenHeight)/sizey;
				}
				else
				{
					int sizex = IPPU.RenderedScreenWidth;
					int sizey = Settings.ShowOverscan ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT;
					sizey = (IPPU.RenderedScreenHeight > SNES_HEIGHT_EXTENDED) ? (sizey << 1) : sizey;
					int width = size.right, height = size.bottom, xdiff = 0, ydiff = 0;
					if(GUI.AspectRatio)
					{
						if(width > sizex*height/sizey)
						{
							xdiff = width - sizex*height/sizey;
							width -= xdiff;
							xdiff >>= 1;
						}
						else if(height > sizey*width/sizex)
						{
							ydiff = height - sizey*width/sizex;
							height -= ydiff;
							ydiff >>= 1;
						}
					}
					GUI.MouseX=(GET_X_LPARAM(lParam)-xdiff)*sizex/width;
					GUI.MouseY=(GET_Y_LPARAM(lParam)-ydiff)*sizey/height;
				}
			}
			else
			{
//				GUI.MouseX = p.x;
//				GUI.MouseY = p.y;
			}
		}
//		else
//			GUI.IgnoreNextMouseMove = false;

		if(!GUI.IgnoreNextMouseMove)
			S9xMouseOn ();
		else
			GUI.IgnoreNextMouseMove = false;
		return 0;
	case WM_LBUTTONDOWN:
		S9xMouseOn ();
		GUI.MouseButtons |= 1;
		break;
	case WM_LBUTTONUP:
		S9xMouseOn ();
		GUI.MouseButtons &= ~1;
		break;
	case WM_RBUTTONDOWN:
		S9xMouseOn ();
		GUI.MouseButtons |= 2;
		break;
	case WM_RBUTTONUP:
		S9xMouseOn ();
		GUI.MouseButtons &= ~2;
		if(GUI.ControllerOption==SNES_JUSTIFIER || GUI.ControllerOption==SNES_JUSTIFIER_2)
		{
			RECT size;
			GetClientRect (GUI.hWnd, &size);
			GUI.MouseButtons&=~1;
			GUI.MouseX=(IPPU.RenderedScreenWidth*(lParam & 0xffff))/(size.right-size.left);
			GUI.MouseY=(((lParam >> 16) & 0xffff)*IPPU.RenderedScreenHeight)/(size.bottom-size.top);
		}
		break;
	case WM_MBUTTONDOWN:
		S9xMouseOn ();
		GUI.MouseButtons |= 4;
		break;
	case WM_MBUTTONUP:
		S9xMouseOn ();
		GUI.MouseButtons &= ~4;
		break;
#ifdef NETPLAY_SUPPORT
	case WM_USER + 3:
		NetPlay.Answer = S9xLoadROMImage (_tFromChar((const char *) lParam));
		SetEvent (NetPlay.ReplyEvent);
		break;
	case WM_USER + 2:
		S9xMessage (0, 0, NetPlay.WarningMsg);
		break;
	case WM_USER + 1:
		RestoreGUIDisplay ();
		S9xRestoreWindowTitle ();
		MessageBox (GUI.hWnd, _tFromChar(NetPlay.ErrorMsg),
			SNES9X_NP_ERROR, MB_OK | MB_ICONSTOP);
		RestoreSNESDisplay ();
		break;
	case WM_USER:
		if (NetPlay.ActionMsg [0] == 0)
			S9xRestoreWindowTitle ();
		else
		{
			TCHAR buf [NP_MAX_ACTION_LEN + 10];

			_stprintf (buf, TEXT("%s %3d%%"), (TCHAR *)_tFromChar(NetPlay.ActionMsg), (int) lParam);
			SetWindowText (GUI.hWnd, buf);
		}
#if 0
		if ((int) lParam >= 0)
		{
			RestoreGUIDisplay ();
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NETPLAYPROGRESS), hWnd, DlgNPProgress);
		}
		else
		{
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NETPLAYPROGRESS), hWnd, DlgNPProgress);
			RestoreSNESDisplay ();
		}
#endif
		break;
#endif
	}
    return DefWindowProc (hWnd, uMsg, wParam, lParam);
}

/*****************************************************************************/
/* WinInit                                                                   */
/*****************************************************************************/
BOOL WinInit( HINSTANCE hInstance)
{
    WNDCLASSEX wndclass;
	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize = sizeof(wndclass);

    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.lpfnWndProc = WinProc;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wndclass.hIconSm = NULL;
    wndclass.hCursor = NULL;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = TEXT("Snes9x: WndClass");
	wndclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);

    GUI.hInstance = hInstance;

    if (!RegisterClassEx (&wndclass))
	{
		MessageBox (NULL, TEXT("Failed to register windows class"), TEXT("Internal Error"), MB_OK | MB_ICONSTOP);
        return FALSE;
	}

	GUI.hMenu = LoadMenu(GUI.hInstance, MAKEINTRESOURCE(IDR_MENU_US));
    if (GUI.hMenu == NULL)
	{
		MessageBox (NULL, TEXT("Failed to initialize the menu.\nThis could indicate a failure of your operating system;\ntry closing some other windows or programs, or restart your computer, before opening Snes9x again.\nOr, if you compiled this program yourself, ensure that Snes9x was built with the proper resource files."), TEXT("Snes9x - Menu Initialization Failure"), MB_OK | MB_ICONSTOP);
//        return FALSE; // disabled: try to function without the menu
	}

	// insert bank and slot submenus / menu items
	HMENU parent_menu_save = GetSubMenuById(ID_FILE_SAVE_POPUP);
	HMENU parent_menu_load = GetSubMenuById(ID_FILE_LOAD_POPUP);
	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);

	for (int i = 0; i < NUM_SAVE_BANKS; i++)
	{
		// one bank sub each for save and load
		HMENU bank_menu_save = CreatePopupMenu();
		HMENU bank_menu_load = CreatePopupMenu();
		// fill the slot entries into the sub menus
		for (int j = 0; j < SAVE_SLOTS_PER_BANK; j++)
		{
			TCHAR *slot_string = g_saveMenuItemStrings[i * SAVE_SLOTS_PER_BANK + j];
			_stprintf(slot_string, _T("Slot #&%d"), j);
			mii.fMask = MIIM_STRING | MIIM_ID;
			mii.dwTypeData = slot_string;
			mii.wID = ID_FILE_SAVE0 + i * SAVE_SLOTS_PER_BANK + j;
			InsertMenuItem(bank_menu_save, j, TRUE, &mii);
			mii.wID = ID_FILE_LOAD0 + i * SAVE_SLOTS_PER_BANK + j;
			InsertMenuItem(bank_menu_load, j, TRUE, &mii);
		}

		// add the bank menus to save and load
		_stprintf(g_bankMenuItemStrings[i], _T("Bank #&%d"), i);
		mii.fMask = MIIM_STRING | MIIM_SUBMENU;
		mii.dwTypeData = g_bankMenuItemStrings[i];
		mii.hSubMenu = bank_menu_save;
		InsertMenuItem(parent_menu_save, i, TRUE, &mii);
		mii.hSubMenu = bank_menu_load;
		InsertMenuItem(parent_menu_load, i, TRUE, &mii);
	}
#ifdef DEBUGGER
	if(GUI.hMenu) {
		InsertMenu(GUI.hMenu,ID_OPTIONS_SETTINGS,MF_BYCOMMAND | MF_STRING | MF_ENABLED,ID_DEBUG_FRAME_ADVANCE,TEXT("&Debug Frame Advance"));
		InsertMenu(GUI.hMenu,ID_OPTIONS_SETTINGS,MF_BYCOMMAND | MF_STRING | MF_ENABLED,ID_DEBUG_TRACE,TEXT("&Trace"));
		InsertMenu(GUI.hMenu,ID_OPTIONS_SETTINGS,MF_BYCOMMAND | MF_STRING | MF_ENABLED,ID_DEBUG_APU_TRACE,TEXT("&APU Trace"));
		InsertMenu(GUI.hMenu,ID_OPTIONS_SETTINGS,MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED,NULL,NULL);
	}
#endif

    TCHAR buf [100];
    _stprintf(buf, TEXT("%s %s"), WINDOW_TITLE, TEXT(VERSION));

    DWORD dwExStyle;
    DWORD dwStyle;
    RECT rect;

    rect.left = rect.top = 0;
    rect.right = MAX_SNES_WIDTH;
    rect.bottom = MAX_SNES_HEIGHT;
    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    dwStyle = WS_OVERLAPPEDWINDOW;

    AdjustWindowRectEx (&rect, dwStyle, FALSE, dwExStyle);
    if ((GUI.hWnd = CreateWindowEx (
        dwExStyle,
        TEXT("Snes9x: WndClass"),
        buf,
        WS_CLIPSIBLINGS |
        WS_CLIPCHILDREN |
        dwStyle,
        0, 0,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL,
        GUI.hMenu,
        hInstance,
        NULL)) == NULL)
        return FALSE;

    GUI.hDC = GetDC (GUI.hWnd);
    GUI.GunSight = LoadCursor (hInstance, MAKEINTRESOURCE (IDC_CURSOR_SCOPE));
    GUI.Arrow = LoadCursor (NULL, IDC_ARROW);
    GUI.Accelerators = LoadAccelerators (hInstance, MAKEINTRESOURCE (IDR_SNES9X_ACCELERATORS));
    Settings.ForcedPause = 0;
    Settings.StopEmulation = TRUE;
    Settings.Paused = FALSE;

	GUI.AVIOut = NULL;

    return TRUE;
}



void S9xExtraUsage ()
{
	S9xMessage(S9X_INFO, S9X_USAGE, "Windows port specific:");
	S9xMessage(S9X_INFO, S9X_USAGE, "-fullscreen                     Start in fullscreen mode");
	S9xMessage(S9X_INFO, S9X_USAGE, "-hidemenu                       Initially hide the GUI menu");
	S9xMessage(S9X_INFO, S9X_USAGE, "-restore                        Reset all settings to default");
	S9xMessage(S9X_INFO, S9X_USAGE, "-removeregistrykeys             Remove registry keys");
	S9xMessage(S9X_INFO, S9X_USAGE, "-cartb <filename>               Specify the second cart for multicart, also triggers multicart");
	MessageBox(NULL, _T("Snes9x command line options have been written to stdout.txt in the same folder as snes9x.exe"), _T("Command line options"), MB_OK | MB_ICONINFORMATION);
}

// handles joystick hotkey presses
VOID CALLBACK HotkeyTimer( UINT idEvent, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
//	static int lastTime = timeGetTime();
//	if(timeGetTime() - lastTime > 5)
	{
		bool S9xGetState (WORD KeyIdent);

		if(GUI.JoystickHotkeys)
		{
			if (Settings.StopEmulation || (Settings.Paused && !Settings.FrameAdvance) || Settings.ForcedPause)
			{
				S9xWinScanJoypads();
			}
			static int counter = 0;
			static uint32 joyState[6][53];
            for(int j = 0 ; j < 6 ; j++)
            {
			    for(int i = 0 ; i < 53 ; i++)
			    {
                    if(counter%2 && !joyState[j][i])
					    continue;

                    WPARAM wp = (WPARAM)(0x8000 | (j << 8) | i);
				    bool active = !S9xGetState(wp);
				    if(active)
				    {
					    if(joyState[j][i] < ULONG_MAX) // 0xffffffffUL
						    joyState[j][i]++;
					    if(joyState[j][i] == 1 || joyState[j][i] >= 12)
						    PostMessage(GUI.hWnd, WM_CUSTKEYDOWN, wp,(LPARAM)(NULL));
				    }
				    else
					    if(joyState[j][i])
					    {
						    joyState[j][i] = 0;
						    PostMessage(GUI.hWnd, WM_CUSTKEYUP, wp,(LPARAM)(NULL));
					    }
			    }
            }
            counter++;
		}
		if(GUI.BackgroundInput && !GUI.InactivePause)
		{
			static int counter = 0;
			static uint32 joyState [256];
			for(int i = 0 ; i < 255 ; i++)
			{
				if(counter%2 && !joyState[i])
					continue;

				// special case for escape since S9xGetState ignores it
				bool active;
				if (i == VK_ESCAPE)
					active = (GetKeyState(VK_ESCAPE) & 0x80) != 0;
				else
					active = !S9xGetState(i);
				if(active)
				{
					if(joyState[i] < ULONG_MAX) // 0xffffffffUL
						joyState[i]++;
					if(joyState[i] == 1 || joyState[i] >= 12)
						PostMessage(GUI.hWnd, WM_CUSTKEYDOWN, (WPARAM)(i),(LPARAM)(NULL));
				}
				else
					if(joyState[i])
					{
						joyState[i] = 0;
						PostMessage(GUI.hWnd, WM_CUSTKEYUP, (WPARAM)(i),(LPARAM)(NULL));
					}
			}
			counter++;
		}
//		lastTime = timeGetTime();
	}
}

static void EnsureInputDisplayUpdated()
{
	if(GUI.FrameAdvanceJustPressed==1 && Settings.Paused && Settings.DisplayPressedKeys==2 && GUI.ControllerOption != SNES_JOYPAD && GUI.ControllerOption != SNES_MULTIPLAYER5 && GUI.ControllerOption != SNES_MULTIPLAYER8)
		WinRefreshDisplay();
}

// for "frame advance skips non-input frames" feature
void S9xOnSNESPadRead()
{
	if(!GUI.FASkipsNonInput)
		return;

	if(prevPadReadFrame != IPPU.TotalEmulatedFrames) // we want <= 1 calls per frame
	{
		prevPadReadFrame = IPPU.TotalEmulatedFrames;

		if(Settings.FrameAdvance && Settings.Paused && !skipNextFrameStop)
		{
			Settings.FrameAdvance = false;

			EnsureInputDisplayUpdated();

			// wait until either unpause or next frame advance
			// note: using GUI.hWnd instead of NULL for PeekMessage/GetMessage breaks some non-modal dialogs
			MSG msg;
			while (Settings.StopEmulation || (Settings.Paused && !Settings.FrameAdvance) ||
				Settings.ForcedPause ||
				PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (!GetMessage (&msg, NULL, 0, 0))
				{
					PostMessage(GUI.hWnd, WM_QUIT, 0,0);
					return;
				}

				if (!TranslateAccelerator (GUI.hWnd, GUI.Accelerators, &msg))
				{
					TranslateMessage (&msg);
					DispatchMessage (&msg);
				}
			}

		}
		else
		{
			skipNextFrameStop = false;
		}
	}
}


enum
{
	k_HD = 0x80000000,

	k_JP = 0x01000000,
	k_MO = 0x02000000,
	k_SS = 0x04000000,
	k_LG = 0x08000000,
	k_RF = 0x10000000,

	k_BT = 0x00100000,
	k_PT = 0x00200000,
	k_PS = 0x00400000,

	k_C1 = 0x00000100,
	k_C2 = 0x00000200,
	k_C3 = 0x00000400,
	k_C4 = 0x00000800,
	k_C5 = 0x00001000,
	k_C6 = 0x00002000,
	k_C7 = 0x00004000,
	k_C8 = 0x00008000
};

enum
{
	kWinCMapPad1PX            = k_HD | k_BT | k_JP | k_C1,
	kWinCMapPad1PA,
	kWinCMapPad1PB,
	kWinCMapPad1PY,
	kWinCMapPad1PL,
	kWinCMapPad1PR,
	kWinCMapPad1PSelect,
	kWinCMapPad1PStart,
	kWinCMapPad1PUp,
	kWinCMapPad1PDown,
	kWinCMapPad1PLeft,
	kWinCMapPad1PRight,

	kWinCMapPad2PX            = k_HD | k_BT | k_JP | k_C2,
	kWinCMapPad2PA,
	kWinCMapPad2PB,
	kWinCMapPad2PY,
	kWinCMapPad2PL,
	kWinCMapPad2PR,
	kWinCMapPad2PSelect,
	kWinCMapPad2PStart,
	kWinCMapPad2PUp,
	kWinCMapPad2PDown,
	kWinCMapPad2PLeft,
	kWinCMapPad2PRight,

	kWinCMapPad3PX            = k_HD | k_BT | k_JP | k_C3,
	kWinCMapPad3PA,
	kWinCMapPad3PB,
	kWinCMapPad3PY,
	kWinCMapPad3PL,
	kWinCMapPad3PR,
	kWinCMapPad3PSelect,
	kWinCMapPad3PStart,
	kWinCMapPad3PUp,
	kWinCMapPad3PDown,
	kWinCMapPad3PLeft,
	kWinCMapPad3PRight,

	kWinCMapPad4PX            = k_HD | k_BT | k_JP | k_C4,
	kWinCMapPad4PA,
	kWinCMapPad4PB,
	kWinCMapPad4PY,
	kWinCMapPad4PL,
	kWinCMapPad4PR,
	kWinCMapPad4PSelect,
	kWinCMapPad4PStart,
	kWinCMapPad4PUp,
	kWinCMapPad4PDown,
	kWinCMapPad4PLeft,
	kWinCMapPad4PRight,

	kWinCMapPad5PX            = k_HD | k_BT | k_JP | k_C5,
	kWinCMapPad5PA,
	kWinCMapPad5PB,
	kWinCMapPad5PY,
	kWinCMapPad5PL,
	kWinCMapPad5PR,
	kWinCMapPad5PSelect,
	kWinCMapPad5PStart,
	kWinCMapPad5PUp,
	kWinCMapPad5PDown,
	kWinCMapPad5PLeft,
	kWinCMapPad5PRight,

	kWinCMapPad6PX            = k_HD | k_BT | k_JP | k_C6,
	kWinCMapPad6PA,
	kWinCMapPad6PB,
	kWinCMapPad6PY,
	kWinCMapPad6PL,
	kWinCMapPad6PR,
	kWinCMapPad6PSelect,
	kWinCMapPad6PStart,
	kWinCMapPad6PUp,
	kWinCMapPad6PDown,
	kWinCMapPad6PLeft,
	kWinCMapPad6PRight,

	kWinCMapPad7PX            = k_HD | k_BT | k_JP | k_C7,
	kWinCMapPad7PA,
	kWinCMapPad7PB,
	kWinCMapPad7PY,
	kWinCMapPad7PL,
	kWinCMapPad7PR,
	kWinCMapPad7PSelect,
	kWinCMapPad7PStart,
	kWinCMapPad7PUp,
	kWinCMapPad7PDown,
	kWinCMapPad7PLeft,
	kWinCMapPad7PRight,

	kWinCMapPad8PX            = k_HD | k_BT | k_JP | k_C8,
	kWinCMapPad8PA,
	kWinCMapPad8PB,
	kWinCMapPad8PY,
	kWinCMapPad8PL,
	kWinCMapPad8PR,
	kWinCMapPad8PSelect,
	kWinCMapPad8PStart,
	kWinCMapPad8PUp,
	kWinCMapPad8PDown,
	kWinCMapPad8PLeft,
	kWinCMapPad8PRight,

	kWinCMapMouse1PL          = k_HD | k_BT | k_MO | k_C1,
	kWinCMapMouse1PR,
	kWinCMapMouse2PL          = k_HD | k_BT | k_MO | k_C2,
	kWinCMapMouse2PR,

	kWinCMapScopeOffscreen    = k_HD | k_BT | k_SS | k_C1,
	kWinCMapScopeFire,
	kWinCMapScopeCursor,
	kWinCMapScopeTurbo,
	kWinCMapScopePause,

	kWinCMapLGun1Offscreen    = k_HD | k_BT | k_LG | k_C1,
	kWinCMapLGun1Trigger,
	kWinCMapLGun1Start,
	kWinCMapLGun2Offscreen    = k_HD | k_BT | k_LG | k_C2,
	kWinCMapLGun2Trigger,
	kWinCMapLGun2Start,

	kWinCMapMacsRifleTrigger  = k_HD | k_BT | k_RF | k_C1,

	kWinCMapMouse1Pointer     = k_HD | k_PT | k_MO | k_C1,
	kWinCMapMouse2Pointer     = k_HD | k_PT | k_MO | k_C2,
	kWinCMapSuperscopePointer = k_HD | k_PT | k_SS | k_C1,
	kWinCMapJustifier1Pointer = k_HD | k_PT | k_LG | k_C1,
	kWinCMapMacsRiflePointer  = k_HD | k_PT | k_RF | k_C1,

	kWinCMapPseudoPtrBase     = k_HD | k_PS | k_LG | k_C2	// for Justifier 2P
};



#define	ASSIGN_BUTTONf(n, s)	S9xMapButton (n, cmd = S9xGetCommandT(s), false)
#define	ASSIGN_BUTTONt(n, s)	S9xMapButton (n, cmd = S9xGetCommandT(s), true)
#define	ASSIGN_POINTRf(n, s)	S9xMapPointer(n, cmd = S9xGetCommandT(s), false)
#define	ASSIGN_POINTRt(n, s)	S9xMapPointer(n, cmd = S9xGetCommandT(s), true)

#define KeyIsPressed(km, k)		(1 & (((unsigned char *) km) [(k) >> 3] >> ((k) & 7)))

void S9xSetupDefaultKeymap(void)
{
	s9xcommand_t	cmd;

	ASSIGN_BUTTONf(kWinCMapPad1PX,         "Joypad1 X");
	ASSIGN_BUTTONf(kWinCMapPad1PA,         "Joypad1 A");
	ASSIGN_BUTTONf(kWinCMapPad1PB,         "Joypad1 B");
	ASSIGN_BUTTONf(kWinCMapPad1PY,         "Joypad1 Y");
	ASSIGN_BUTTONf(kWinCMapPad1PL,         "Joypad1 L");
	ASSIGN_BUTTONf(kWinCMapPad1PR,         "Joypad1 R");
	ASSIGN_BUTTONf(kWinCMapPad1PSelect,    "Joypad1 Select");
	ASSIGN_BUTTONf(kWinCMapPad1PStart,     "Joypad1 Start");
	ASSIGN_BUTTONf(kWinCMapPad1PUp,        "Joypad1 Up");
	ASSIGN_BUTTONf(kWinCMapPad1PDown,      "Joypad1 Down");
	ASSIGN_BUTTONf(kWinCMapPad1PLeft,      "Joypad1 Left");
	ASSIGN_BUTTONf(kWinCMapPad1PRight,     "Joypad1 Right");

	ASSIGN_BUTTONf(kWinCMapPad2PX,         "Joypad2 X");
	ASSIGN_BUTTONf(kWinCMapPad2PA,         "Joypad2 A");
	ASSIGN_BUTTONf(kWinCMapPad2PB,         "Joypad2 B");
	ASSIGN_BUTTONf(kWinCMapPad2PY,         "Joypad2 Y");
	ASSIGN_BUTTONf(kWinCMapPad2PL,         "Joypad2 L");
	ASSIGN_BUTTONf(kWinCMapPad2PR,         "Joypad2 R");
	ASSIGN_BUTTONf(kWinCMapPad2PSelect,    "Joypad2 Select");
	ASSIGN_BUTTONf(kWinCMapPad2PStart,     "Joypad2 Start");
	ASSIGN_BUTTONf(kWinCMapPad2PUp,        "Joypad2 Up");
	ASSIGN_BUTTONf(kWinCMapPad2PDown,      "Joypad2 Down");
	ASSIGN_BUTTONf(kWinCMapPad2PLeft,      "Joypad2 Left");
	ASSIGN_BUTTONf(kWinCMapPad2PRight,     "Joypad2 Right");

	ASSIGN_BUTTONf(kWinCMapPad3PX,         "Joypad3 X");
	ASSIGN_BUTTONf(kWinCMapPad3PA,         "Joypad3 A");
	ASSIGN_BUTTONf(kWinCMapPad3PB,         "Joypad3 B");
	ASSIGN_BUTTONf(kWinCMapPad3PY,         "Joypad3 Y");
	ASSIGN_BUTTONf(kWinCMapPad3PL,         "Joypad3 L");
	ASSIGN_BUTTONf(kWinCMapPad3PR,         "Joypad3 R");
	ASSIGN_BUTTONf(kWinCMapPad3PSelect,    "Joypad3 Select");
	ASSIGN_BUTTONf(kWinCMapPad3PStart,     "Joypad3 Start");
	ASSIGN_BUTTONf(kWinCMapPad3PUp,        "Joypad3 Up");
	ASSIGN_BUTTONf(kWinCMapPad3PDown,      "Joypad3 Down");
	ASSIGN_BUTTONf(kWinCMapPad3PLeft,      "Joypad3 Left");
	ASSIGN_BUTTONf(kWinCMapPad3PRight,     "Joypad3 Right");

	ASSIGN_BUTTONf(kWinCMapPad4PX,         "Joypad4 X");
	ASSIGN_BUTTONf(kWinCMapPad4PA,         "Joypad4 A");
	ASSIGN_BUTTONf(kWinCMapPad4PB,         "Joypad4 B");
	ASSIGN_BUTTONf(kWinCMapPad4PY,         "Joypad4 Y");
	ASSIGN_BUTTONf(kWinCMapPad4PL,         "Joypad4 L");
	ASSIGN_BUTTONf(kWinCMapPad4PR,         "Joypad4 R");
	ASSIGN_BUTTONf(kWinCMapPad4PSelect,    "Joypad4 Select");
	ASSIGN_BUTTONf(kWinCMapPad4PStart,     "Joypad4 Start");
	ASSIGN_BUTTONf(kWinCMapPad4PUp,        "Joypad4 Up");
	ASSIGN_BUTTONf(kWinCMapPad4PDown,      "Joypad4 Down");
	ASSIGN_BUTTONf(kWinCMapPad4PLeft,      "Joypad4 Left");
	ASSIGN_BUTTONf(kWinCMapPad4PRight,     "Joypad4 Right");

	ASSIGN_BUTTONf(kWinCMapPad5PX,         "Joypad5 X");
	ASSIGN_BUTTONf(kWinCMapPad5PA,         "Joypad5 A");
	ASSIGN_BUTTONf(kWinCMapPad5PB,         "Joypad5 B");
	ASSIGN_BUTTONf(kWinCMapPad5PY,         "Joypad5 Y");
	ASSIGN_BUTTONf(kWinCMapPad5PL,         "Joypad5 L");
	ASSIGN_BUTTONf(kWinCMapPad5PR,         "Joypad5 R");
	ASSIGN_BUTTONf(kWinCMapPad5PSelect,    "Joypad5 Select");
	ASSIGN_BUTTONf(kWinCMapPad5PStart,     "Joypad5 Start");
	ASSIGN_BUTTONf(kWinCMapPad5PUp,        "Joypad5 Up");
	ASSIGN_BUTTONf(kWinCMapPad5PDown,      "Joypad5 Down");
	ASSIGN_BUTTONf(kWinCMapPad5PLeft,      "Joypad5 Left");
	ASSIGN_BUTTONf(kWinCMapPad5PRight,     "Joypad5 Right");

	ASSIGN_BUTTONf(kWinCMapPad6PX,         "Joypad6 X");
	ASSIGN_BUTTONf(kWinCMapPad6PA,         "Joypad6 A");
	ASSIGN_BUTTONf(kWinCMapPad6PB,         "Joypad6 B");
	ASSIGN_BUTTONf(kWinCMapPad6PY,         "Joypad6 Y");
	ASSIGN_BUTTONf(kWinCMapPad6PL,         "Joypad6 L");
	ASSIGN_BUTTONf(kWinCMapPad6PR,         "Joypad6 R");
	ASSIGN_BUTTONf(kWinCMapPad6PSelect,    "Joypad6 Select");
	ASSIGN_BUTTONf(kWinCMapPad6PStart,     "Joypad6 Start");
	ASSIGN_BUTTONf(kWinCMapPad6PUp,        "Joypad6 Up");
	ASSIGN_BUTTONf(kWinCMapPad6PDown,      "Joypad6 Down");
	ASSIGN_BUTTONf(kWinCMapPad6PLeft,      "Joypad6 Left");
	ASSIGN_BUTTONf(kWinCMapPad6PRight,     "Joypad6 Right");

	ASSIGN_BUTTONf(kWinCMapPad7PX,         "Joypad7 X");
	ASSIGN_BUTTONf(kWinCMapPad7PA,         "Joypad7 A");
	ASSIGN_BUTTONf(kWinCMapPad7PB,         "Joypad7 B");
	ASSIGN_BUTTONf(kWinCMapPad7PY,         "Joypad7 Y");
	ASSIGN_BUTTONf(kWinCMapPad7PL,         "Joypad7 L");
	ASSIGN_BUTTONf(kWinCMapPad7PR,         "Joypad7 R");
	ASSIGN_BUTTONf(kWinCMapPad7PSelect,    "Joypad7 Select");
	ASSIGN_BUTTONf(kWinCMapPad7PStart,     "Joypad7 Start");
	ASSIGN_BUTTONf(kWinCMapPad7PUp,        "Joypad7 Up");
	ASSIGN_BUTTONf(kWinCMapPad7PDown,      "Joypad7 Down");
	ASSIGN_BUTTONf(kWinCMapPad7PLeft,      "Joypad7 Left");
	ASSIGN_BUTTONf(kWinCMapPad7PRight,     "Joypad7 Right");

	ASSIGN_BUTTONf(kWinCMapPad8PX,         "Joypad8 X");
	ASSIGN_BUTTONf(kWinCMapPad8PA,         "Joypad8 A");
	ASSIGN_BUTTONf(kWinCMapPad8PB,         "Joypad8 B");
	ASSIGN_BUTTONf(kWinCMapPad8PY,         "Joypad8 Y");
	ASSIGN_BUTTONf(kWinCMapPad8PL,         "Joypad8 L");
	ASSIGN_BUTTONf(kWinCMapPad8PR,         "Joypad8 R");
	ASSIGN_BUTTONf(kWinCMapPad8PSelect,    "Joypad8 Select");
	ASSIGN_BUTTONf(kWinCMapPad8PStart,     "Joypad8 Start");
	ASSIGN_BUTTONf(kWinCMapPad8PUp,        "Joypad8 Up");
	ASSIGN_BUTTONf(kWinCMapPad8PDown,      "Joypad8 Down");
	ASSIGN_BUTTONf(kWinCMapPad8PLeft,      "Joypad8 Left");
	ASSIGN_BUTTONf(kWinCMapPad8PRight,     "Joypad8 Right");

	ASSIGN_BUTTONt(kWinCMapMouse1PL,       "Mouse1 L");
	ASSIGN_BUTTONt(kWinCMapMouse1PR,       "Mouse1 R");
	ASSIGN_BUTTONt(kWinCMapMouse2PL,       "Mouse2 L");
	ASSIGN_BUTTONt(kWinCMapMouse2PR,       "Mouse2 R");

	ASSIGN_BUTTONt(kWinCMapScopeOffscreen, "Superscope AimOffscreen");
	ASSIGN_BUTTONt(kWinCMapScopeFire,      "Superscope Fire");
	ASSIGN_BUTTONt(kWinCMapScopeCursor,    "Superscope Cursor");
	ASSIGN_BUTTONt(kWinCMapScopeTurbo,     "Superscope ToggleTurbo");
	ASSIGN_BUTTONt(kWinCMapScopePause,     "Superscope Pause");

	ASSIGN_BUTTONt(kWinCMapLGun1Offscreen, "Justifier1 AimOffscreen");
	ASSIGN_BUTTONt(kWinCMapLGun1Trigger,   "Justifier1 Trigger");
	ASSIGN_BUTTONt(kWinCMapLGun1Start,     "Justifier1 Start");
	ASSIGN_BUTTONt(kWinCMapLGun2Offscreen, "Justifier2 AimOffscreen");
	ASSIGN_BUTTONt(kWinCMapLGun2Trigger,   "Justifier2 Trigger");
	ASSIGN_BUTTONt(kWinCMapLGun2Start,     "Justifier2 Start");

	ASSIGN_BUTTONt(kWinCMapMacsRifleTrigger,  "MacsRifle Trigger");

	ASSIGN_POINTRt(kWinCMapMouse1Pointer,     "Pointer Mouse1");
	ASSIGN_POINTRt(kWinCMapMouse2Pointer,     "Pointer Mouse2");
	ASSIGN_POINTRt(kWinCMapSuperscopePointer, "Pointer Superscope");
	ASSIGN_POINTRt(kWinCMapJustifier1Pointer, "Pointer Justifier1");
	ASSIGN_POINTRt(kWinCMapMacsRiflePointer,  "Pointer MacsRifle");

	ASSIGN_POINTRf(PseudoPointerBase,         "Pointer Justifier2");
	ASSIGN_BUTTONf(kWinCMapPseudoPtrBase + 0, "ButtonToPointer 1u Med");
	ASSIGN_BUTTONf(kWinCMapPseudoPtrBase + 1, "ButtonToPointer 1d Med");
	ASSIGN_BUTTONf(kWinCMapPseudoPtrBase + 2, "ButtonToPointer 1l Med");
	ASSIGN_BUTTONf(kWinCMapPseudoPtrBase + 3, "ButtonToPointer 1r Med");
}

void ControlPadFlagsToS9xReportButtons(int n, uint32 p)
{
	uint32	base = k_HD | k_BT | k_JP | (0x100 << n);

	S9xReportButton(base +  0, (p & 0x0040) != 0);
	S9xReportButton(base +  1, (p & 0x0080) != 0);
	S9xReportButton(base +  2, (p & 0x8000) != 0);
	S9xReportButton(base +  3, (p & 0x4000) != 0);
	S9xReportButton(base +  4, (p & 0x0020) != 0);
	S9xReportButton(base +  5, (p & 0x0010) != 0);
	S9xReportButton(base +  6, (p & 0x2000) != 0);
	S9xReportButton(base +  7, (p & 0x1000) != 0);
	S9xReportButton(base +  8, (p & 0x0800) != 0);
	S9xReportButton(base +  9, (p & 0x0400) != 0);
	S9xReportButton(base + 10, (p & 0x0200) != 0);
	S9xReportButton(base + 11, (p & 0x0100) != 0);
}

void ControlPadFlagsToS9xPseudoPointer(uint32 p)
{
	// prevent screwiness caused by trying to move the pointer left+right or up+down
	if((p & 0x0c00) == 0x0c00) p &= ~0x0c00;
	if((p & 0x0300) == 0x0300) p &= ~0x0300;

	// checks added to prevent a lack of right/down movement from breaking left/up movement
	if(!(p & 0x0400))
		S9xReportButton(kWinCMapPseudoPtrBase + 0, (p & 0x0800) != 0);
	if(!(p & 0x0800))
		S9xReportButton(kWinCMapPseudoPtrBase + 1, (p & 0x0400) != 0);
	if(!(p & 0x0100))
		S9xReportButton(kWinCMapPseudoPtrBase + 2, (p & 0x0200) != 0);
	if(!(p & 0x0200))
		S9xReportButton(kWinCMapPseudoPtrBase + 3, (p & 0x0100) != 0);
}

static void ProcessInput(void)
{
#ifdef NETPLAY_SUPPORT
    if (!Settings.NetPlay)
#endif
	S9xWinScanJoypads ();

	extern uint32 joypads [8];
	for(int i = 0 ; i < 8 ; i++)
		ControlPadFlagsToS9xReportButtons(i, joypads[i]);

	if (GUI.ControllerOption==SNES_JUSTIFIER_2)
		ControlPadFlagsToS9xPseudoPointer(joypads[1]);
}

//static void WinDisplayString (const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap);




/*****************************************************************************/
/* WinMain                                                                   */
/*****************************************************************************/
void DeinitS9x(void);

int WINAPI WinMain(
				   HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	Settings.StopEmulation = TRUE;

	SetCurrentDirectory(S9xGetDirectoryT(DEFAULT_DIR));

	// Redirect stderr and stdout to file. It wouldn't go to any commandline anyway.
	FILE* fout = freopen("stdout.txt", "w", stdout);
	if(fout) setvbuf(fout, NULL, _IONBF, 0);
	FILE* ferr = freopen("stderr.txt", "w", stderr);
	if(ferr) setvbuf(ferr, NULL, _IONBF, 0);

	DWORD wSoundTimerRes;

	WinRegisterConfigItems ();

	ConfigFile::SetAlphaSort(false);
	ConfigFile::SetTimeSort(false);

    const TCHAR *rom_filename = WinParseCommandLineAndLoadConfigFile (GetCommandLine());
    WinSaveConfigFile ();
	WinLockConfigFile ();

	LoadExts();

    ControllerOptionsFromControllers();
    ChangeInputDevice();

    WinInit (hInstance);
	if(GUI.HideMenu)
	{
		SetMenu (GUI.hWnd, NULL);
	}

    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;
    ZeroMemory(&notificationFilter, sizeof(notificationFilter));
    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    RegisterDeviceNotification(GUI.hWnd, &notificationFilter, DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);

	InitRenderFilters();

    GUI.ControlForced = 0xff;
    Settings.Rewinding = false;

    S9xSetRecentGames ();

	RestoreMainWinPos();

	void InitSnes9x (void);
	InitSnes9x ();

	if(GUI.FullScreen) {
		GUI.FullScreen = false;
		ToggleFullScreen();
	}

    TIMECAPS tc;
    if (timeGetDevCaps(&tc, sizeof(TIMECAPS))== TIMERR_NOERROR)
    {
#ifdef __MINGW32__
        wSoundTimerRes = min<int>(max<int>(tc.wPeriodMin, 1), tc.wPeriodMax);
#else
        wSoundTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
#endif
        timeBeginPeriod (wSoundTimerRes);
    }
	else
	{
		wSoundTimerRes = 5;
        timeBeginPeriod (wSoundTimerRes);
	}

    Settings.StopEmulation = TRUE;

	if(GUI.JoystickHotkeys || GUI.BackgroundInput)
	    GUI.hHotkeyTimer = timeSetEvent (32, 0, (LPTIMECALLBACK)HotkeyTimer, 0, TIME_PERIODIC);
	else
		GUI.hHotkeyTimer = 0;

    GUI.ServerTimerSemaphore = CreateSemaphore (NULL, 0, 10, NULL);

	if (rom_filename)
	{
		if (Settings.Multi) // we found -cartB parameter
		{
			lstrcpy(multiRomA, rom_filename); // for the mutli cart dialog
			LoadROM(rom_filename, multiRomB);
		}
		else
		{
			LoadROM(rom_filename);
		}

		if (*Settings.InitialSnapshotFilename)
		{
			S9xUnfreezeGame(Settings.InitialSnapshotFilename);
		}
	}

	S9xUnmapAllControls();
	S9xSetupDefaultKeymap();

	DWORD lastTime = timeGetTime();

    MSG msg;

    while (TRUE)
    {
		EnsureInputDisplayUpdated();

		// note: using GUI.hWnd instead of NULL for PeekMessage/GetMessage breaks some non-modal dialogs
        while (Settings.StopEmulation || (Settings.Paused && !Settings.FrameAdvance) ||
			Settings.ForcedPause ||
			PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (!GetMessage (&msg, NULL, 0, 0))
                goto loop_exit; // got WM_QUIT

            if (!TranslateAccelerator (GUI.hWnd, GUI.Accelerators, &msg))
            {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }

			S9xSetSoundMute(GUI.Mute || Settings.ForcedPause || (Settings.Paused && (!Settings.FrameAdvance || GUI.FAMute)));
        }

#ifdef NETPLAY_SUPPORT
        if (!Settings.NetPlay || !NetPlay.PendingWait4Sync ||
            WaitForSingleObject (GUI.ClientSemaphore, 100) != WAIT_TIMEOUT)
        {
            if (NetPlay.PendingWait4Sync)
            {
                NetPlay.PendingWait4Sync = FALSE;
                NetPlay.FrameCount++;
                S9xNPStepJoypadHistory ();
            }
#endif
			if(watches[0].on)
			{
				// copy the memory used by each active watch
				for(unsigned int i = 0 ; i < sizeof(watches)/sizeof(*watches) ; i++)
				{
					if(watches[i].on)
					{
						int address = watches[i].address - 0x7E0000;
						const uint8* source;
						if(address < 0x20000)
							source = Memory.RAM + address ;
						else if(address < 0x30000)
							source = Memory.SRAM + address  - 0x20000;
						else
							source = Memory.FillRAM + address  - 0x30000;

						CopyMemory(Cheat.CWatchRAM + address, source, watches[i].size);
					}
				}
			}
			if(cheatSearchHWND)
			{
				if(timeGetTime() - lastTime >= 100)
				{
					SendMessage(cheatSearchHWND, WM_COMMAND, (WPARAM)(IDC_REFRESHLIST),(LPARAM)(NULL));
					lastTime = timeGetTime();
				}
			}

			if(Settings.FrameAdvance)
			{
				if(GFX.InfoStringTimeout > 4)
					GFX.InfoStringTimeout = 4;

				if(!GUI.FASkipsNonInput)
					Settings.FrameAdvance = false;
			}
			if(GUI.FrameAdvanceJustPressed)
				GUI.FrameAdvanceJustPressed--;

			ProcessInput();

			if (GUI.rewindBufferSize
#ifdef NETPLAY_SUPPORT
				&& !Settings.NetPlay
#endif
				) {
				if (Settings.Rewinding) {
					Settings.Rewinding = stateMan.pop();
				}
				else {
					if (IPPU.TotalEmulatedFrames % GUI.rewindGranularity == 0)
						stateMan.push();
				}
			}

			S9xMainLoop();
			GUI.FrameCount++;
			if (GUI.CursorTimer)
			{
				if (--GUI.CursorTimer == 0)
				{
					if (GUI.ControllerOption != SNES_SUPERSCOPE && GUI.ControllerOption != SNES_JUSTIFIER && GUI.ControllerOption != SNES_JUSTIFIER_2 && GUI.ControllerOption != SNES_MACSRIFLE)
						SetCursor(NULL);
				}
			}
#ifdef NETPLAY_SUPPORT
        }
#endif
        if (CPU.Flags & DEBUG_MODE_FLAG)
        {
            Settings.Paused = TRUE;
            Settings.FrameAdvance = false;
            CPU.Flags &= ~DEBUG_MODE_FLAG;
        }
    }

loop_exit:

	Settings.StopEmulation = TRUE;

	// stop sound playback
	CloseSoundDevice();

    if (GUI.hHotkeyTimer)
        timeKillEvent (GUI.hHotkeyTimer);

    timeEndPeriod(wSoundTimerRes);

    if (!Settings.StopEmulation)
    {
        Memory.SaveSRAM (S9xGetFilename (".srm", SRAM_DIR).c_str());
        S9xSaveCheatFile (S9xGetFilename (".cht", CHEAT_DIR).c_str());
    }
    //if (!VOODOO_MODE && !GUI.FullScreen)
    //    GetWindowRect (GUI.hWnd, &GUI.window_size);


	// this goes here, because the avi
	// recording might have messed with
	// the auto frame skip setting
	// (it needs to come before WinSave)
	DoAVIClose(0);

	S9xMovieShutdown (); // must happen before saving config

	WinUnlockConfigFile ();
    WinSaveConfigFile ();
	WinCleanupConfigData();

	Memory.Deinit();

	ClearExts();

	DeInitRenderFilters();

	S9xGraphicsDeinit();
	S9xDeinitAPU();
	WinDeleteRecentGamesList ();
	DeinitS9x();

	return msg.wParam;
}

void FreezeUnfreezeDialog(bool8 freeze)
{
	if (Settings.StopEmulation)
		return;

    TCHAR filename[MAX_PATH];
    OPENFILENAME ofn;

    *filename = TEXT('\0');
    memset((LPVOID)&ofn, 0, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = GUI.hWnd;
    ofn.lpstrFilter = TEXT("State files\0*.state;*.frz;*.0*\0All Files\0*.*\0\0");
    ofn.lpstrFile = filename;
    ofn.lpstrInitialDir = S9xGetDirectoryT(SNAPSHOT_DIR);
    ofn.lpstrTitle = TEXT("Select State");
    ofn.lpstrDefExt = TEXT("state");
    ofn.nMaxFile = MAX_PATH;

    if (freeze) {
        ofn.Flags = OFN_OVERWRITEPROMPT;
        if (GetSaveFileName(&ofn)) {
            FreezeUnfreeze(_tToChar(ofn.lpstrFile), freeze);
        }
    } else {
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        if (GetOpenFileName(&ofn)) {
            FreezeUnfreeze(_tToChar(ofn.lpstrFile), freeze);
        }
    }
}

void FreezeUnfreezeDialogPreview(bool8 freeze)
{
    if(Settings.StopEmulation)
        return;

    CSaveLoadWithPreviewDlg dlg(freeze);
    int slot = dlg.show();
    if(slot >= 0)
        FreezeUnfreezeSlot(slot, freeze);
}

void GetSlotFilename(int slot, char filename[_MAX_PATH + 1])
{
    char ext[_MAX_EXT + 1];

    if(slot == -1)
        strcpy(ext, ".oops");
    else
        snprintf(ext, _MAX_EXT, ".%03d", slot);
    strcpy(filename, S9xGetFilename(ext, SNAPSHOT_DIR).c_str());
}

void FreezeUnfreezeSlot(int slot, bool8 freeze)
{
    char filename[_MAX_PATH + 1];
    GetSlotFilename(slot, filename);

    FreezeUnfreeze(filename, freeze);
}

void FreezeUnfreeze (const char *filename, bool8 freeze)
{
#ifdef NETPLAY_SUPPORT
    if (!freeze && Settings.NetPlay && !Settings.NetPlayServer)
    {
        S9xMessage (S9X_INFO, S9X_NETPLAY_NOT_SERVER,
			"Only the server is allowed to load freeze files.");
        return;
    }
#endif

    if(Settings.StopEmulation)
        return;

	if (GUI.ConfirmSaveLoad)
	{
		std::string msg, title;
		if (freeze)
		{
			msg = "Are you sure you want to SAVE to\n";
			title = "Confirm SAVE";
		}
		else
		{
			msg = "Are you sure you want to LOAD from\n";
			title = "Confirm LOAD";
		}
		msg += filename;
		int res = MessageBox(GUI.hWnd, _tFromChar(msg.c_str()), _tFromChar(title.c_str()), MB_YESNO | MB_ICONQUESTION);
		if (res != IDYES)
			return;
	}

    S9xSetPause (PAUSE_FREEZE_FILE);

    if (freeze)
	{
//		extern bool diagnostic_freezing;
//		if(GetAsyncKeyState('Q') && GetAsyncKeyState('W') && GetAsyncKeyState('E') && GetAsyncKeyState('R'))
//		{
//			diagnostic_freezing = true;
//		}
        S9xFreezeGame (filename);
//
//		diagnostic_freezing = false;
	}
    else
    {

        if (S9xUnfreezeGame (filename))
        {
//	        S9xMessage (S9X_INFO, S9X_FREEZE_FILE_INFO, S9xBasename (filename));
#ifdef NETPLAY_SUPPORT
            S9xNPServerQueueSendingFreezeFile (filename);
#endif
//            UpdateBackBuffer();
        }

		// fix next frame advance after loading non-skipping state from a skipping state
			skipNextFrameStop = true;
    }

    S9xClearPause (PAUSE_FREEZE_FILE);
}

bool UnfreezeScreenshotSlot(int slot, uint16 **image_buffer, int &width, int &height)
{
    char filename[_MAX_PATH + 1];
    GetSlotFilename(slot, filename);

    return S9xUnfreezeScreenshot(filename, image_buffer, width, height);
}

void CheckDirectoryIsWritable (const char *filename)
{
    FILE *fs = fopen (filename, "w+");

    if (fs == NULL)
	MessageBox (GUI.hWnd, TEXT("The folder where Snes9x saves emulated save RAM files and\ngame save positions (freeze files) is currently set to a\nread-only folder.\n\nIf you do not change the game save folder, Snes9x will be\nunable to save your progress in this game. Change the folder\nfrom the Settings Dialog available from the Options menu.\n\nThe default save folder is called Saves, if no value is set.\n"),
							 TEXT("Snes9x: Unable to save file warning"),
							 MB_OK | MB_ICONINFORMATION);
    else
    {
        fclose (fs);
        remove (filename);
    }
}

static void CheckMenuStates ()
{
    MENUITEMINFO mii;
    unsigned int i;

    memset( &mii, 0, sizeof( mii));
    mii.cbSize = sizeof( mii);
    mii.fMask = MIIM_STATE;

	mii.fState = (GUI.FullScreen||GUI.EmulatedFullscreen) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_WINDOW_FULLSCREEN, FALSE, &mii);

	mii.fState = GUI.Stretch ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_WINDOW_STRETCH, FALSE, &mii);

	mii.fState = GUI.Stretch ? (GUI.AspectRatio ? MFS_CHECKED : MFS_UNCHECKED) : MFS_CHECKED|MFS_DISABLED;
    SetMenuItemInfo (GUI.hMenu, ID_WINDOW_ASPECTRATIO, FALSE, &mii);

	mii.fState = Settings.BilinearFilter ? MFS_CHECKED : MFS_UNCHECKED;
	if(!GUI.Stretch)
		mii.fState |= MFS_DISABLED;
    SetMenuItemInfo (GUI.hMenu, ID_WINDOW_BILINEAR, FALSE, &mii);

	mii.fState = Settings.DisplayFrameRate ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_VIDEO_SHOWFRAMERATE, FALSE, &mii);

	mii.fState = (Settings.Paused && !Settings.StopEmulation) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_FILE_PAUSE, FALSE, &mii);

	mii.fState = (GUI.InactivePause) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_EMULATION_PAUSEWHENINACTIVE, FALSE, &mii);

    mii.fState = MFS_UNCHECKED;
    if (Settings.StopEmulation)
        mii.fState |= MFS_DISABLED;
	SetMenuItemInfo (GUI.hMenu, ID_FILE_SAVE_SPC_DATA, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_FILE_SAVE_SRAM_DATA, FALSE, &mii);

	for(int i = ID_FILE_SAVE0; i <= ID_FILE_SAVE_FILE; i++)
		SetMenuItemInfo (GUI.hMenu, i, FALSE, &mii);

	for (int i = ID_FILE_LOAD0; i <= ID_FILE_LOAD_FILE; i++)
		SetMenuItemInfo(GUI.hMenu, i, FALSE, &mii);

    SetMenuItemInfo(GUI.hMenu, ID_FILE_SAVE_PREVIEW, FALSE, &mii);
    SetMenuItemInfo(GUI.hMenu, ID_FILE_LOAD_PREVIEW, FALSE, &mii);

    SetMenuItemInfo (GUI.hMenu, ID_FILE_RESET, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_CHEAT_ENTER, FALSE, &mii);
	SetMenuItemInfo (GUI.hMenu, IDM_ROM_INFO, FALSE, &mii);

	if (GUI.FullScreen)
        mii.fState |= MFS_DISABLED;
    SetMenuItemInfo (GUI.hMenu, ID_CHEAT_SEARCH, FALSE, &mii);

#ifdef NETPLAY_SUPPORT
    if (Settings.NetPlay && !Settings.NetPlayServer)
        mii.fState = MFS_DISABLED;
    else
        mii.fState = Settings.NetPlayServer ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_NETPLAY_SERVER, FALSE, &mii);

    mii.fState = Settings.NetPlay && !Settings.NetPlayServer ? 0 : MFS_DISABLED;
    SetMenuItemInfo (GUI.hMenu, ID_NETPLAY_DISCONNECT, FALSE, &mii);

    mii.fState = Settings.NetPlay || Settings.NetPlayServer ? MFS_DISABLED : 0;
    SetMenuItemInfo (GUI.hMenu, ID_NETPLAY_CONNECT, FALSE, &mii);

    mii.fState = NPServer.SendROMImageOnConnect ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_NETPLAY_SEND_ROM_ON_CONNECT, FALSE, &mii);

    mii.fState = NPServer.SyncByReset ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_NETPLAY_SYNC_BY_RESET, FALSE, &mii);

    mii.fState = Settings.NetPlayServer ? 0 : MFS_DISABLED;
    SetMenuItemInfo (GUI.hMenu, ID_NETPLAY_SYNC, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_NETPLAY_ROM, FALSE, &mii);
#endif

    mii.fState = Settings.ApplyCheats ? MFS_CHECKED : MFS_UNCHECKED;
    if (Settings.StopEmulation)
        mii.fState |= MFS_DISABLED;
    SetMenuItemInfo( GUI.hMenu, ID_CHEAT_APPLY, FALSE, &mii);

    mii.fState = MFS_UNCHECKED;
	if (GUI.AVIOut)
        mii.fState |= MFS_DISABLED;
    SetMenuItemInfo (GUI.hMenu, ID_SOUND_OPTIONS, FALSE, &mii);

    SetMenuItemInfo (GUI.hMenu, ID_SOUND_NOSOUND, FALSE, &mii);
    for (i = 0; i < COUNT(SoundRates); i++)
        SetMenuItemInfo (GUI.hMenu, SoundRates[i].ident, FALSE, &mii);

    if (Settings.SoundPlaybackRate == 0 || GUI.Mute)
        mii.fState |= MFS_DISABLED;

    SetMenuItemInfo (GUI.hMenu, ID_SOUND_SYNC, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_SOUND_INTERPOLATED, FALSE, &mii);

    SetMenuItemInfo (GUI.hMenu, ID_SOUND_16MS, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_SOUND_32MS, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_SOUND_48MS, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_SOUND_64MS, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_SOUND_80MS, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_SOUND_96MS, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_SOUND_112MS, FALSE, &mii);
	SetMenuItemInfo (GUI.hMenu, ID_SOUND_128MS, FALSE, &mii);
	SetMenuItemInfo (GUI.hMenu, ID_SOUND_144MS, FALSE, &mii);
	SetMenuItemInfo (GUI.hMenu, ID_SOUND_160MS, FALSE, &mii);
	SetMenuItemInfo (GUI.hMenu, ID_SOUND_176MS, FALSE, &mii);
	SetMenuItemInfo (GUI.hMenu, ID_SOUND_192MS, FALSE, &mii);
	SetMenuItemInfo (GUI.hMenu, ID_SOUND_208MS, FALSE, &mii);

    mii.fState = MFS_CHECKED;
	if (GUI.AVIOut)
        mii.fState |= MFS_DISABLED;

    if (Settings.SoundPlaybackRate == 0 || GUI.Mute )
        SetMenuItemInfo (GUI.hMenu, ID_SOUND_NOSOUND, FALSE, &mii);
    else
    {
        for (i = 0; i < COUNT(SoundRates); i++)
        {
            if (SoundRates [i].rate == Settings.SoundPlaybackRate)
            {
                SetMenuItemInfo (GUI.hMenu, SoundRates[i].ident, FALSE, &mii);
                break;
            }
        }
    }
    if (Settings.SoundPlaybackRate == 0 || GUI.Mute)
        mii.fState |= MFS_DISABLED;

	int id;
	switch (GUI.SoundBufferSize)
    {
	case 16:  id = ID_SOUND_16MS; break;
	case 32:  id = ID_SOUND_32MS; break;
	case 48:  id = ID_SOUND_48MS; break;
	case 64:  id = ID_SOUND_64MS; break;
	case 80:  id = ID_SOUND_80MS; break;
	case 96:  id = ID_SOUND_96MS; break;
	case 112: id = ID_SOUND_112MS; break;
	case 128: id = ID_SOUND_128MS; break;
	case 144: id = ID_SOUND_144MS; break;
	case 160: id = ID_SOUND_160MS; break;
	case 176: id = ID_SOUND_176MS; break;
	case 192: id = ID_SOUND_192MS; break;
	case 208: id = ID_SOUND_208MS; break;
    }
    SetMenuItemInfo (GUI.hMenu, id, FALSE, &mii);

    if (Settings.SoundSync)
        SetMenuItemInfo (GUI.hMenu, ID_SOUND_SYNC, FALSE, &mii);

#ifdef DEBUGGER
    mii.fState = (CPU.Flags & TRACE_FLAG) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_DEBUG_TRACE, FALSE, &mii);
	// TODO: reactivate once APU debugger works again
	//mii.fState = (spc_core->debug_is_enabled()) ? MFS_CHECKED : MFS_UNCHECKED;
	mii.fState = MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_DEBUG_APU_TRACE, FALSE, &mii);
#endif

	mii.fState = (!Settings.StopEmulation) ? MFS_ENABLED : MFS_DISABLED;
    SetMenuItemInfo (GUI.hMenu, ID_FILE_MOVIE_PLAY, FALSE, &mii);
    SetMenuItemInfo (GUI.hMenu, ID_FILE_MOVIE_RECORD, FALSE, &mii);

	mii.fState = (S9xMovieActive () && !Settings.StopEmulation) ? MFS_ENABLED : MFS_DISABLED;
    SetMenuItemInfo (GUI.hMenu, ID_FILE_MOVIE_STOP, FALSE, &mii);

	mii.fState = (GUI.SoundChannelEnable & (1 << 0)) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_CHANNELS_CHANNEL1, FALSE, &mii);
	mii.fState = (GUI.SoundChannelEnable & (1 << 1)) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_CHANNELS_CHANNEL2, FALSE, &mii);
	mii.fState = (GUI.SoundChannelEnable & (1 << 2)) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_CHANNELS_CHANNEL3, FALSE, &mii);
	mii.fState = (GUI.SoundChannelEnable & (1 << 3)) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_CHANNELS_CHANNEL4, FALSE, &mii);
	mii.fState = (GUI.SoundChannelEnable & (1 << 4)) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_CHANNELS_CHANNEL5, FALSE, &mii);
	mii.fState = (GUI.SoundChannelEnable & (1 << 5)) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_CHANNELS_CHANNEL6, FALSE, &mii);
	mii.fState = (GUI.SoundChannelEnable & (1 << 6)) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_CHANNELS_CHANNEL7, FALSE, &mii);
	mii.fState = (GUI.SoundChannelEnable & (1 << 7)) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo (GUI.hMenu, ID_CHANNELS_CHANNEL8, FALSE, &mii);

	mii.fState = GUI.BackgroundInput ? MFS_CHECKED : MFS_UNCHECKED;
	SetMenuItemInfo (GUI.hMenu, ID_EMULATION_BACKGROUNDINPUT, FALSE, &mii);

    mii.fState = GUI.BackgroundKeyHotkeys ? MFS_CHECKED : MFS_UNCHECKED;
    if (!GUI.BackgroundInput)
        mii.fState |= MFS_DISABLED;
    SetMenuItemInfo(GUI.hMenu, ID_INPUT_BACKGROUNDKEYBOARDHOTKEYS, FALSE, &mii);

	UINT validFlag;
    ControllerOptionsFromControllers();

	validFlag = (((1<<SNES_JOYPAD) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_JOYPAD ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_SNES_JOYPAD, FALSE, &mii);

	validFlag = (((1<<SNES_MULTIPLAYER5) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_MULTIPLAYER5 ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_ENABLE_MULTITAP, FALSE, &mii);

	validFlag = (((1<<SNES_MULTIPLAYER8) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_MULTIPLAYER8 ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_MULTITAP8, FALSE, &mii);

	validFlag = (((1<<SNES_MOUSE) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_MOUSE ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_MOUSE_TOGGLE, FALSE, &mii);

	validFlag = (((1<<SNES_MOUSE_SWAPPED) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_MOUSE_SWAPPED ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_MOUSE_SWAPPED, FALSE, &mii);

	validFlag = (((1<<SNES_SUPERSCOPE) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_SUPERSCOPE ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_SCOPE_TOGGLE, FALSE, &mii);

	validFlag = (((1<<SNES_JUSTIFIER) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_JUSTIFIER ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_JUSTIFIER, FALSE, &mii);

	validFlag = (((1<<SNES_JUSTIFIER_2) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_JUSTIFIER_2 ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_JUSTIFIERS, FALSE, &mii);

	validFlag = (((1<<SNES_MACSRIFLE) & GUI.ValidControllerOptions) && (!S9xMovieActive() || !S9xMovieGetFrameCounter())) ? MFS_ENABLED : MFS_DISABLED;
    mii.fState = validFlag | (GUI.ControllerOption == SNES_MACSRIFLE ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemInfo (GUI.hMenu, IDM_MACSRIFLE_TOGGLE, FALSE, &mii);

	mii.fState = !Settings.StopEmulation ? MFS_ENABLED : MFS_DISABLED;
	SetMenuItemInfo (GUI.hMenu, ID_FILE_AVI_RECORDING, FALSE, &mii);

	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STRING;

	mii.dwTypeData = (TCHAR *)(!GUI.AVIOut ? TEXT("Start AVI Recording...") : TEXT("Stop AVI Recording"));
	SetMenuItemInfo (GUI.hMenu, ID_FILE_AVI_RECORDING, FALSE, &mii);
}

static void ResetFrameTimer ()
{
	// determines if we can do sound sync
	GUI.AllowSoundSync = Settings.PAL ? Settings.FrameTime == Settings.FrameTimePAL : Settings.FrameTime == Settings.FrameTimeNTSC;
}

static bool LoadROMPlain(const TCHAR *filename)
{
	if (!filename || !*filename)
		return (FALSE);
	SetCurrentDirectory(S9xGetDirectoryT(ROM_DIR));
    if (Memory.LoadROM (_tToChar(filename)))
    {
		S9xStartCheatSearch (&Cheat);
        ReInitSound();
		ResetFrameTimer();
        return (TRUE);
    }
    return (FALSE);
}

static bool LoadROMMulti(const TCHAR *filename, const TCHAR *filename2)
{
	SetCurrentDirectory(S9xGetDirectoryT(ROM_DIR));
	if (Memory.LoadMultiCart(_tToChar(filename), _tToChar(filename2)))
	{
		S9xStartCheatSearch(&Cheat);
		ReInitSound();
		ResetFrameTimer();
		return (TRUE);
	}
	return (FALSE);
}

static bool LoadROM(const TCHAR *filename, const TCHAR *filename2 /*= NULL*/) {

#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay && !Settings.NetPlayServer)
	{
		S9xMessage (S9X_INFO, S9X_NETPLAY_NOT_SERVER,
			WINPROC_DISCONNECT);
		return false;
	}
#endif

	if (!Settings.StopEmulation) {
		Memory.SaveSRAM (S9xGetFilename (".srm", SRAM_DIR).c_str());
		S9xSaveCheatFile (S9xGetFilename (".cht", CHEAT_DIR).c_str());
	}

	if(filename2)
		Settings.StopEmulation = !LoadROMMulti(filename, filename2);
	else
		Settings.StopEmulation = !LoadROMPlain(filename);

	if (!Settings.StopEmulation) {
		bool8 loadedSRAM = Memory.LoadSRAM (S9xGetFilename (".srm", SRAM_DIR).c_str());
		if(!loadedSRAM) // help migration from earlier Snes9x versions by checking ROM directory for savestates
			Memory.LoadSRAM (S9xGetFilename (".srm", ROMFILENAME_DIR).c_str());
		if(!filename2) // no recent for multi cart
			S9xAddToRecentGames (filename);
		CheckDirectoryIsWritable (S9xGetFilename (".---", SNAPSHOT_DIR).c_str());

#ifdef NETPLAY_SUPPORT
		if (NPServer.SendROMImageOnConnect)
			S9xNPServerQueueSendingROMImage ();
		else
			S9xNPServerQueueSendingLoadROMRequest (Memory.ROMName);
#endif
        if(GUI.rewindBufferSize)
		{
		#if !_WIN64
			if (GUI.rewindBufferSize > 1024) GUI.rewindBufferSize = 1024;
		#endif
            stateMan.init(GUI.rewindBufferSize * 1024 * 1024);
		}
	}

	if(GUI.ControllerOption == SNES_SUPERSCOPE || GUI.ControllerOption == SNES_MACSRIFLE)
		SetCursor (GUI.GunSight);
	else {
		SetCursor (GUI.Arrow);
		GUI.CursorTimer = 60;
	}
	Settings.Paused = false;
    S9xRestoreWindowTitle();

	if (GUI.FullscreenOnOpen)
	{
		if (!GUI.FullScreen && !GUI.EmulatedFullscreen)
			ToggleFullScreen();
		SetMenu(GUI.hWnd, NULL);
	}

	return !Settings.StopEmulation;
}

bool8 S9xLoadROMImage (const TCHAR *string)
{
    RestoreGUIDisplay ();
    TCHAR *buf = new TCHAR [200 + lstrlen (string)];
    _stprintf (buf, TEXT("The NetPlay server is requesting you load the following game:\n '%s'"),
		string);

    MessageBox (GUI.hWnd, buf,
		SNES9X_INFO,
		MB_OK | MB_ICONINFORMATION);

    delete buf;

    TCHAR FileName [_MAX_PATH];

	if(DoOpenRomDialog(FileName)) {
        return LoadROM(FileName);
    }
    else
        return (FALSE);

    return (TRUE);
}

/*****************************************************************************/
#ifdef NETPLAY_SUPPORT
void EnableServer (bool8 enable)
{
    if (enable != Settings.NetPlayServer)
    {
        if (Settings.NetPlay && !Settings.NetPlayServer)
        {
            Settings.NetPlay = FALSE;
            S9xNPDisconnect ();
        }

        if (enable)
        {
            S9xSetPause (PAUSE_NETPLAY_CONNECT);
            Settings.NetPlayServer = S9xNPStartServer (Settings.Port);
            Sleep (1000);

            if (!S9xNPConnectToServer ("127.0.0.1", Settings.Port,
				Memory.ROMName))
            {
                S9xClearPause (PAUSE_NETPLAY_CONNECT);
            }
        }
        else
        {
            Settings.NetPlayServer = FALSE;
            S9xNPStopServer ();
        }
    }
}
#endif

void S9xAddToRecentGames (const TCHAR *filename)
{
    // Make sure its not in the list already
    int i;
	for(i = 0; i < MAX_RECENT_GAMES_LIST_SIZE; i++)
        if (!*GUI.RecentGames[i] || lstrcmp (filename, GUI.RecentGames[i]) == 0)
            break;

	const bool underMax = (i < MAX_RECENT_GAMES_LIST_SIZE);
	if(underMax && *GUI.RecentGames[i])
	{
		// It is in the list, move it to the head of the list.
		TCHAR temp [MAX_PATH];
		lstrcpy(temp, GUI.RecentGames[i]);
		for(int j = i; j > 0; j--)
			lstrcpy(GUI.RecentGames[j], GUI.RecentGames[j-1]);

		lstrcpy(GUI.RecentGames[0], temp);
	}
	else
	{
		// Not in the list, add it.
		if(underMax)
			// Extend the recent game list length by 1.
			memmove(&GUI.RecentGames[1], &GUI.RecentGames[0], MAX_PATH*i*sizeof(TCHAR));
		else
			// Throw the last item off the end of the list
			memmove(&GUI.RecentGames[1], &GUI.RecentGames[0], MAX_PATH*(i-1)*sizeof(TCHAR));

		lstrcpy(GUI.RecentGames[0], filename);

		WinSaveConfigFile();
	}

    S9xSetRecentGames();
}

void S9xRemoveFromRecentGames (int i)
{
	if (*GUI.RecentGames [i])
	{
		for (int j = i; j < MAX_RECENT_GAMES_LIST_SIZE-1; j++)
			lstrcpy(GUI.RecentGames [j], GUI.RecentGames [j + 1]);
		*GUI.RecentGames [MAX_RECENT_GAMES_LIST_SIZE-1] = TEXT('\0');

		S9xSetRecentGames ();
	}
}

#ifdef UNICODE

/* OV2: We need dynamic binding to this function as it is not present in
   windows versions < vista. Since the jumplist is a win7 feature this
   is not a problem.
*/
typedef HRESULT (STDAPICALLTYPE *SHCIFPN) (__in PCWSTR pszPath, __in_opt IBindCtx *pbc, __in REFIID riid, __deref_out void **ppv);

HRESULT Win7_JLSetRecentGames(ICustomDestinationList *pcdl, IObjectArray *poaRemoved, UINT maxSlots)
{
    IObjectCollection *poc;

	HMODULE S32dll = GetModuleHandle(TEXT("shell32.dll"));
	SHCIFPN SHCreateIFPN = (SHCIFPN)GetProcAddress(S32dll,"SHCreateItemFromParsingName");
	if(!SHCreateIFPN) {
		return S_FALSE;
	}
	HRESULT hr = CoCreateInstance
                    (CLSID_EnumerableObjectCollection,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&poc));
    if (SUCCEEDED(hr)) {
		UINT max_list = MIN(maxSlots,GUI.MaxRecentGames);
		for (UINT i = 0; i < max_list && *GUI.RecentGames[i]; i++) {
            IShellItem *psi;
			if(SUCCEEDED(SHCreateIFPN(GUI.RecentGames[i],NULL,IID_PPV_ARGS(&psi)))) {
                hr = poc->AddObject(psi);
                psi->Release();
            }
        }

        IObjectArray *poa;
        hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
        if (SUCCEEDED(hr)) {
            hr = pcdl->AppendCategory(TEXT("ROMs"), poa);
            poa->Release();
        }
        poc->Release();
    }
    return hr;
}

void Win7_CreateJumpList()
{
    ICustomDestinationList *pcdl;
    HRESULT hr = CoCreateInstance(
                    CLSID_DestinationList,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&pcdl));
    if (SUCCEEDED(hr)) {
        UINT maxSlots;
        IObjectArray *poaRemoved;
        hr = pcdl->BeginList(&maxSlots, IID_PPV_ARGS(&poaRemoved));
        if (SUCCEEDED(hr)) {
            hr = Win7_JLSetRecentGames(pcdl, poaRemoved,maxSlots);
            if (SUCCEEDED(hr)) {
                hr = pcdl->CommitList();
            }
            poaRemoved->Release();
        }
		pcdl->Release();
    }
}
#endif

void S9xSetRecentGames ()
{
    HMENU file = GetSubMenu (GUI.hMenu, 0);
    if (file)
    {
        HMENU recent = GetSubMenu (file, 1);
        if (recent)
        {
            MENUITEMINFO mii;
            TCHAR name [256 + 10];
            int i;

            // Clear out the menu first
            for (i = GetMenuItemCount (recent) - 1; i >= 0; i--)
                RemoveMenu (recent, i, MF_BYPOSITION);

            mii.cbSize = sizeof (mii);
            mii.fMask = MIIM_TYPE | MIIM_DATA | MIIM_STATE | MIIM_ID;
            mii.fType = MFT_STRING;
            mii.fState = MFS_UNCHECKED;

            for (i = 0; i < MAX_RECENT_GAMES_LIST_SIZE && i < GUI.MaxRecentGames && *GUI.RecentGames [i]; i++)
            {
                // Build up a menu item string in the form:
                // 1. <basename of ROM image name>

                _stprintf (name, TEXT("&%c. "), i < 9 ? '1' + i : 'A' + i - 9);

				// append the game title to name, with formatting modifications as necessary
				{
					TCHAR baseName [256];
					lstrcpy (baseName, _tFromChar(S9xBasename (std::string(_tToChar(GUI.RecentGames [i]))).c_str()));
					int pos = lstrlen (name), baseNameLen = lstrlen (baseName);
					for (int j = 0; j < baseNameLen ; j++)
					{
						TCHAR c = baseName [j];
						name [pos++] = c;

						// & is a special character in Windows menus,
						// so we have to change & to && when copying over the game title
						// otherwise "Pocky & Rocky (U).smc" will show up as "Pocky _Rocky (U).smc", for example
						if(c == TEXT('&'))
							name [pos++] = TEXT('&');
					}
					name [pos] = TEXT('\0');
				}

				mii.dwTypeData = name;
                mii.cch = lstrlen (name) + 1;
                mii.wID = 0xFF00 + i;

                InsertMenuItem (recent, 0xFF00 + i, FALSE, &mii);
            }

			if (i > 0)
			{
				MENUITEMINFO mii_sep = mii;
				mii_sep.fMask = MIIM_TYPE;
				mii_sep.fType = MFT_SEPARATOR;
				InsertMenuItem(recent, 0xFF00 + i + 1, FALSE, &mii_sep);

				mii.dwTypeData = TEXT("Clear List");
				mii.cch = lstrlen(name) + 1;
				mii.wID = 0xFF00 + MAX_RECENT_GAMES_LIST_SIZE;
				InsertMenuItem(recent, mii.wID, FALSE, &mii);
			}
#ifdef UNICODE
			Win7_CreateJumpList();
#endif
        }
    }
}

RECT GetWindowMargins(HWND hwnd, UINT width)
{
	RECT rcMargins = {0,0,0,0};

	AdjustWindowRectEx(&rcMargins, GetWindowStyle(hwnd), !GUI.HideMenu, GetWindowExStyle(hwnd));

	rcMargins.left = abs(rcMargins.left);
	rcMargins.top = abs(rcMargins.top);

	if (!GUI.HideMenu) {
		RECT rcTemp = {0,0,(LONG)width,0x7FFF}; // 0x7FFF="Infinite" height
		SendMessage(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rcTemp);

		// Adjust our previous calculation to compensate for menu
		// wrapping.
		rcMargins.top = rcTemp.top;
	}

	return rcMargins;
}

void WinDeleteRecentGamesList ()
{
	for(int i=0;i<MAX_RECENT_GAMES_LIST_SIZE;i++)
		GUI.RecentGames[i][0]=TEXT('\0');
}

BOOL CreateToolTip(int toolID, HWND hDlg, TCHAR* pText)
{
    // toolID:  the resource ID of the control.
    // hDlg:    the handle of the dialog box.
    // pText:   the text that appears in the tooltip.
    // g_hInst: the global instance handle.

    if (!toolID || !hDlg || !pText)
    {
        return FALSE;
    }
    // Get the window of the tool.
    HWND hwndTool = GetDlgItem(hDlg, toolID);

    // Create the tooltip.
    HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                              WS_POPUP |TTS_ALWAYSTIP | TTS_BALLOON,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              hDlg, NULL,
                              g_hInst, NULL);

   if (!hwndTool || !hwndTip)
   {
       return FALSE;
   }

    // Associate the tooltip with the tool.
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hDlg;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = pText;
    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

    return TRUE;
}

void UpdateAudioDeviceDropdown(HWND hCtl)
{
    std::vector<std::wstring> device_list = GetAvailableSoundDevices();

    ComboBox_ResetContent(hCtl);

    int num_devices = device_list.size();

    if (!num_devices)
    {
        ComboBox_AddString(hCtl, _T("Default"));
    }
    else
    {
        for (int i = 0; i < num_devices; i++)
        {
            ComboBox_AddString(hCtl, device_list[i].c_str());
        }
    }
}

INT_PTR CALLBACK DlgSoundConf(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hTrackbar;
	TCHAR valTxt[10];

    // temporary GUI state for restoring after switching devices (need to actually switch devices to get output devices)
    static int prevDriver;

	switch(msg)
	{
	    case WM_INITDIALOG:
        {
            WinRefreshDisplay();

            prevDriver = GUI.SoundDriver;

            // FIXME: these strings should come from wlanguage.h

            CreateToolTip(IDC_INRATEEDIT, hDlg, TEXT("For each 'Input rate' samples generated by the SNES, 'Playback rate' samples will produced. If you experience crackling you can try to lower this setting."));
            CreateToolTip(IDC_INRATE, hDlg, TEXT("For each 'Input rate' samples generated by the SNES, 'Playback rate' samples will produced. If you experience crackling you can try to lower this setting."));
            CreateToolTip(IDC_DYNRATECONTROL, hDlg, TEXT("Try to dynamically adjust the input rate to never overflow or underflow the sound buffer."));

            HWND output_dropdown = GetDlgItem(hDlg, IDC_OUTPUT_DEVICE);
            UpdateAudioDeviceDropdown(output_dropdown);
            ComboBox_SetCurSel(output_dropdown, FindAudioDeviceIndex(GUI.AudioDevice));

            int pos;
            pos = SendDlgItemMessage(hDlg, IDC_DRIVER, CB_INSERTSTRING, -1, (LPARAM)TEXT("WaveOut"));
            SendDlgItemMessage(hDlg, IDC_DRIVER, CB_SETITEMDATA, pos, WIN_WAVEOUT_DRIVER);
            pos = SendDlgItemMessage(hDlg, IDC_DRIVER, CB_INSERTSTRING, -1, (LPARAM)TEXT("XAudio2"));
            SendDlgItemMessage(hDlg, IDC_DRIVER, CB_SETITEMDATA, pos, WIN_XAUDIO2_SOUND_DRIVER);
            SendDlgItemMessage(hDlg, IDC_DRIVER, CB_SETCURSEL, 0, 0);
            for (pos = 0; pos < SendDlgItemMessage(hDlg, IDC_DRIVER, CB_GETCOUNT, 0, 0); pos++) {
                if (SendDlgItemMessage(hDlg, IDC_DRIVER, CB_GETITEMDATA, pos, 0) == GUI.SoundDriver) {
                    SendDlgItemMessage(hDlg, IDC_DRIVER, CB_SETCURSEL, pos, 0);
                    break;
                }
            }

            if (WinGetAutomaticInputRate() < 1)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_AUTOMATICINPUTRATE), 0);
                GUI.AutomaticInputRate = false;
            }

            EnableWindow(GetDlgItem(hDlg, IDC_INRATEEDIT), !GUI.AutomaticInputRate);
            EnableWindow(GetDlgItem(hDlg, IDC_INRATE), !GUI.AutomaticInputRate);

            SendDlgItemMessage(hDlg, IDC_INRATE, TBM_SETRANGE, TRUE, MAKELONG(0, 20));
            SendDlgItemMessage(hDlg, IDC_INRATE, TBM_SETPOS, TRUE, (Settings.SoundInputRate - 31100) / 50);
            SendDlgItemMessage(hDlg, IDC_INRATE, TBM_SETTICFREQ, 1, 0);
            _sntprintf(valTxt, 10, TEXT("%d"), Settings.SoundInputRate);
            Edit_SetText(GetDlgItem(hDlg, IDC_INRATEEDIT), valTxt);

            // regular volume
            SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME_REGULAR, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
            SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME_REGULAR, TBM_SETPOS, TRUE, 100 - GUI.VolumeRegular);
            SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME_REGULAR, TBM_SETTICFREQ, 10, 0);
            _sntprintf(valTxt, 10, TEXT("%d"), GUI.VolumeRegular);
            Edit_SetText(GetDlgItem(hDlg, IDC_EDIT_VOLUME_REGULAR), valTxt);

            // turbo volume
            SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME_TURBO, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
            SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME_TURBO, TBM_SETPOS, TRUE, 100 - GUI.VolumeTurbo);
            SendDlgItemMessage(hDlg, IDC_SLIDER_VOLUME_TURBO, TBM_SETTICFREQ, 10, 0);
            _sntprintf(valTxt, 10, TEXT("%d"), GUI.VolumeTurbo);
            Edit_SetText(GetDlgItem(hDlg, IDC_EDIT_VOLUME_TURBO), valTxt);


            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 0, (LPARAM)TEXT("8 KHz"));
            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 1, (LPARAM)TEXT("11 KHz"));
            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 2, (LPARAM)TEXT("16 KHz"));
            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 3, (LPARAM)TEXT("22 KHz"));
            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 4, (LPARAM)TEXT("30 KHz"));
            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 5, (LPARAM)TEXT("32 KHz"));
            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 6, (LPARAM)TEXT("35 KHz"));
            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 7, (LPARAM)TEXT("44 KHz"));
            SendDlgItemMessage(hDlg, IDC_RATE, CB_INSERTSTRING, 8, (LPARAM)TEXT("48 KHz"));

            int temp;
            switch (Settings.SoundPlaybackRate)
            {
                case 8000:temp = 0; break;
                case 11025:temp = 1; break;
                case 16000:temp = 2; break;
                case 22050:temp = 3; break;
                case 30000:temp = 4; break;
                case 0:
                default:
                case 32000:temp = 5; break;
                case 35000:temp = 6; break;
                case 44000:
                case 44100:temp = 7; break;
                case 48000:temp = 8; break;
            }
            SendDlgItemMessage(hDlg, IDC_RATE, CB_SETCURSEL, temp, 0);

            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 0, (LPARAM)TEXT("16 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 1, (LPARAM)TEXT("32 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 2, (LPARAM)TEXT("48 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 3, (LPARAM)TEXT("64 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 4, (LPARAM)TEXT("80 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 5, (LPARAM)TEXT("96 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 6, (LPARAM)TEXT("112 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 7, (LPARAM)TEXT("128 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 8, (LPARAM)TEXT("144 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 9, (LPARAM)TEXT("160 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 10, (LPARAM)TEXT("176 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 11, (LPARAM)TEXT("192 ms"));
            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_INSERTSTRING, 12, (LPARAM)TEXT("208 ms"));

            SendDlgItemMessage(hDlg, IDC_BUFLEN, CB_SETCURSEL, ((GUI.SoundBufferSize / 16) - 1), 0);

            if (Settings.DynamicRateControl)
                SendDlgItemMessage(hDlg, IDC_DYNRATECONTROL, BM_SETCHECK, BST_CHECKED, 0);

            if (GUI.Mute)
                SendDlgItemMessage(hDlg, IDC_MUTE, BM_SETCHECK, BST_CHECKED, 0);
            if (GUI.FAMute)
                SendDlgItemMessage(hDlg, IDC_FAMT, BM_SETCHECK, BST_CHECKED, 0);

            if (Settings.SoundSync)
                SendDlgItemMessage(hDlg, IDC_SYNC_TO_SOUND_CPU, BM_SETCHECK, BST_CHECKED, 0);

            if (GUI.AutomaticInputRate)
                SendDlgItemMessage(hDlg, IDC_AUTOMATICINPUTRATE, BM_SETCHECK, BST_CHECKED, 0);

            return true;
        }
		case WM_PAINT:
		{
		    PAINTSTRUCT ps;
		    BeginPaint (hDlg, &ps);

		    EndPaint (hDlg, &ps);
		}
		return true;
		case WM_HSCROLL:
		case WM_VSCROLL:
		{
			WORD loword_w = LOWORD(wParam);
			if (loword_w == TB_THUMBPOSITION || loword_w == TB_THUMBTRACK || loword_w == TB_ENDTRACK) {
				HWND trackHwnd = (HWND)lParam;
				WORD scrollPos = SendMessage(trackHwnd, TBM_GETPOS, 0, 0);
				int trackValue = 100 - scrollPos;
				int editId = 0;
				if (trackHwnd == GetDlgItem(hDlg, IDC_INRATE))
				{
					trackValue = 31100 + 50 * scrollPos;
					editId = IDC_INRATEEDIT;
				}
				else if (trackHwnd == GetDlgItem(hDlg, IDC_SLIDER_VOLUME_REGULAR))
				{
					editId = IDC_EDIT_VOLUME_REGULAR;
				}
				else if (trackHwnd == GetDlgItem(hDlg, IDC_SLIDER_VOLUME_TURBO))
				{
					editId = IDC_EDIT_VOLUME_TURBO;
				}
				_sntprintf(valTxt, 10, TEXT("%d"), trackValue);
				Edit_SetText(GetDlgItem(hDlg, editId), valTxt);
				return true;
			}
		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					GUI.SoundDriver=SendDlgItemMessage(hDlg, IDC_DRIVER, CB_GETITEMDATA,
										SendDlgItemMessage(hDlg, IDC_DRIVER, CB_GETCURSEL, 0,0),0);
					Settings.DynamicRateControl=IsDlgButtonChecked(hDlg, IDC_DYNRATECONTROL);
					Settings.SoundSync=IsDlgButtonChecked(hDlg, IDC_SYNC_TO_SOUND_CPU);
					GUI.Mute=IsDlgButtonChecked(hDlg, IDC_MUTE);
					GUI.FAMute=IsDlgButtonChecked(hDlg, IDC_FAMT)!=0;


					switch(SendDlgItemMessage(hDlg, IDC_RATE,CB_GETCURSEL,0,0))
					{
					case 0: Settings.SoundPlaybackRate=8000;	break;
					case 1: Settings.SoundPlaybackRate=11025;	break;
					case 2: Settings.SoundPlaybackRate=16000;	break;
					case 3: Settings.SoundPlaybackRate=22050;	break;
					case 4: Settings.SoundPlaybackRate=30000;	break;
					case 5: Settings.SoundPlaybackRate=32000;	break;
					case 6: Settings.SoundPlaybackRate=35000;	break;
					case 7: Settings.SoundPlaybackRate=44100;	break;
					case 8: Settings.SoundPlaybackRate=48000;	break;
					}

					GUI.SoundBufferSize=(16*(1+(SendDlgItemMessage(hDlg,IDC_BUFLEN,CB_GETCURSEL,0,0))));

					Edit_GetText(GetDlgItem(hDlg,IDC_INRATEEDIT),valTxt,10);
					int sliderVal=_tstoi(valTxt);
					Settings.SoundInputRate = sliderVal>0?sliderVal:32000;

					GUI.AutomaticInputRate = IsDlgButtonChecked(hDlg, IDC_AUTOMATICINPUTRATE);

					// regular volume
					Edit_GetText(GetDlgItem(hDlg, IDC_EDIT_VOLUME_REGULAR), valTxt, 10);
					sliderVal = _tstoi(valTxt);
					GUI.VolumeRegular = (sliderVal >= 0 && sliderVal <= 100) ? sliderVal : 100;

					// turbo volume
					Edit_GetText(GetDlgItem(hDlg, IDC_EDIT_VOLUME_TURBO), valTxt, 10);
					sliderVal = _tstoi(valTxt);
					GUI.VolumeTurbo = (sliderVal >= 0 && sliderVal <= 100) ? sliderVal : 100;

                    // output device
                    Edit_GetText(GetDlgItem(hDlg, IDC_OUTPUT_DEVICE), GUI.AudioDevice, MAX_AUDIO_NAME_LENGTH);

					WinSaveConfigFile();

					// already done in WinProc on return
					// ReInitSound();

                    EndDialog(hDlg, 1);
                    return true;
				}

				case IDCANCEL:
                    GUI.SoundDriver = prevDriver;
					EndDialog(hDlg, 1);
					return true;

				case IDC_AUTOMATICINPUTRATE:
                {
                    bool temp_auto_input_rate = IsDlgButtonChecked(hDlg, IDC_AUTOMATICINPUTRATE);
                    EnableWindow(GetDlgItem(hDlg, IDC_INRATEEDIT), !temp_auto_input_rate);
                    EnableWindow(GetDlgItem(hDlg, IDC_INRATE), !temp_auto_input_rate);

                    if (temp_auto_input_rate)
                    {
                        int newrate = WinGetAutomaticInputRate();
                        if (newrate)
                        {
                            SendDlgItemMessage(hDlg, IDC_INRATE, TBM_SETPOS, TRUE, (newrate - 31100) / 50);
                            _sntprintf(valTxt, 10, TEXT("%d"), newrate);
                            Edit_SetText(GetDlgItem(hDlg, IDC_INRATEEDIT), valTxt);
                        }
                        else
                        {
                            SendDlgItemMessage(hDlg, IDC_AUTOMATICINPUTRATE, BM_SETCHECK, BST_UNCHECKED, 0);
                        }
                    }

                    return true;
                }
				case IDC_DRIVER:
					if(CBN_SELCHANGE==HIWORD(wParam))
					{
						int driver = SendDlgItemMessage(hDlg, IDC_DRIVER, CB_GETITEMDATA,
										SendDlgItemMessage(hDlg, IDC_DRIVER, CB_GETCURSEL, 0,0),0);

						switch(driver) {
							case WIN_WAVEOUT_DRIVER:
								SendDlgItemMessage(hDlg,IDC_BUFLEN,CB_SETCURSEL,3,0);
								break;
							case WIN_XAUDIO2_SOUND_DRIVER:
								SendDlgItemMessage(hDlg,IDC_BUFLEN,CB_SETCURSEL,3,0);
								break;
							default:
								SendDlgItemMessage(hDlg,IDC_BUFLEN,CB_SETCURSEL,7,0);
								break;
						}

                        // get current selected device name, switch driver, try to select the same
                        HWND output_dropdown = GetDlgItem(hDlg, IDC_OUTPUT_DEVICE);
                        TCHAR selected_device[MAX_AUDIO_NAME_LENGTH];
                        Edit_GetText(output_dropdown, selected_device, MAX_AUDIO_NAME_LENGTH);
                        GUI.SoundDriver = driver;
                        ReInitSound();
                        UpdateAudioDeviceDropdown(output_dropdown);
                        ComboBox_SetCurSel(output_dropdown, FindAudioDeviceIndex(selected_device));
						return true;
					}
					else return false;
				case IDC_INRATEEDIT:
					if(HIWORD(wParam)==EN_UPDATE) {
						Edit_GetText(GetDlgItem(hDlg,IDC_INRATEEDIT),valTxt,10);
						int sliderVal=_tstoi(valTxt);
						SendDlgItemMessage(hDlg, IDC_INRATE, TBM_SETPOS,TRUE,(sliderVal - 31100)/50);
					}
					break;
				default: return false;


		}
	}
	return false;
}

//  SetSelProc
//  Callback procedure to set the initial selection of the (folder) browser.
int CALLBACK SetSelProc( HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    if (uMsg==BFFM_INITIALIZED) {
        ::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData );
    }
    return 0;
}

const char *StaticRAMBitSize ()
{
    static char tmp [20];

    sprintf (tmp, " (%dKbit)", 8*(Memory.SRAMMask + 1) / 1024);
    return (tmp);
}

INT_PTR CALLBACK DlgInfoProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		WinRefreshDisplay();
		{
			char temp[100];
			char romtext[4096];
            sprintf(romtext, "Name: %s\r\n", Memory.ROMName);
			sprintf(temp, "Speed: %02X/%s\r\nROM Map: %s\r\nType: %02x\r\n", Memory.ROMSpeed, ((Memory.ROMSpeed&0x10)!=0)?"FastROM":"SlowROM",(Memory.HiROM)?"HiROM":"LoROM",Memory.ROMType);
			strcat(romtext, temp);
			strcat(romtext, "Kart contents: ");
			strcat(romtext, Memory.KartContents ());
			strcat(romtext, "\r\nHeader ROM Size: ");
			strcat(romtext, Memory.Size());
			sprintf(temp, "\r\nCalculated ROM Size: %d Mbits", Memory.CalculatedSize/0x20000);
			strcat(romtext, temp);

			strcat(romtext, "\r\nSRAM size: ");
			strcat(romtext, Memory.StaticRAMSize ());
			strcat(romtext, StaticRAMBitSize());
			strcat(romtext, "\r\nActual Checksum: ");
			sprintf(temp, "%04X", Memory.CalculatedChecksum);
			strcat(romtext, temp);
			strcat(romtext, "\r\nHeader Checksum: ");
			sprintf(temp, "%04X", Memory.ROMChecksum);
			strcat(romtext, temp);
			strcat(romtext, "\r\nHeader Checksum Complement: ");
			sprintf(temp, "%04X", Memory.ROMComplementChecksum);
			strcat(romtext, temp);
			strcat(romtext, "\r\nOutput: ");
			if(Memory.ROMRegion>12||Memory.ROMRegion<2)
				strcat(romtext, "NTSC 60Hz");
			else strcat(romtext, "PAL 50Hz");

			sprintf(temp, "\r\nCRC32:\t%08X", Memory.ROMCRC32);
			strcat(romtext, temp);


			strcat(romtext, "\r\nLicensee: ");

			int tmp=Memory.CompanyId;
			if(tmp==0)
				tmp=(Memory.HiROM)?Memory.ROM[0x0FFDA]:Memory.ROM[0x7FDA];
			switch(tmp)
				//				switch(((Memory.ROMSpeed&0x0F)!=0)?Memory.ROM[0x0FFDA]:Memory.ROM[0x7FDA])
				//				switch(atoi(Memory.CompanyId))
				//				switch(((Memory.CompanyId[0]-'0')*16)+(Memory.CompanyId[1]-'0'))
			{
			case 0:strcat(romtext, "INVALID COMPANY");break;
			case 1:strcat(romtext, "Nintendo");break;
			case 2:strcat(romtext, "Ajinomoto");break;
			case 3:strcat(romtext, "Imagineer-Zoom");break;
			case 4:strcat(romtext, "Chris Gray Enterprises Inc.");break;
			case 5:strcat(romtext, "Zamuse");break;
			case 6:strcat(romtext, "Falcom");break;
			case 7:strcat(romtext, NOTKNOWN "7");break;
			case 8:strcat(romtext, "Capcom");break;
			case 9:strcat(romtext, "HOT-B");break;
			case 10:strcat(romtext, "Jaleco");break;
			case 11:strcat(romtext, "Coconuts");break;
			case 12:strcat(romtext, "Rage Software");break;
			case 13:strcat(romtext, "Micronet"); break; //Acc. ZFE
			case 14:strcat(romtext, "Technos");break;
			case 15:strcat(romtext, "Mebio Software");break;
			case 16:strcat(romtext, "SHOUEi System"); break; //Acc. ZFE
			case 17:strcat(romtext, "Starfish");break; //UCON 64
			case 18:strcat(romtext, "Gremlin Graphics");break;
			case 19:strcat(romtext, "Electronic Arts");break;
			case 20:strcat(romtext, "NCS / Masaya"); break; //Acc. ZFE
			case 21:strcat(romtext, "COBRA Team");break;
			case 22:strcat(romtext, "Human/Field");break;
			case 23:strcat(romtext, "KOEI");break;
			case 24:strcat(romtext, "Hudson Soft");break;
			case 25:strcat(romtext, "Game Village");break;//uCON64
			case 26:strcat(romtext, "Yanoman");break;
			case 27:strcat(romtext, NOTKNOWN "27");break;
			case 28:strcat(romtext, "Tecmo");break;
			case 29:strcat(romtext, NOTKNOWN "29");break;
			case 30:strcat(romtext, "Open System");break;
			case 31:strcat(romtext, "Virgin Games");break;
			case 32:strcat(romtext, "KSS");break;
			case 33:strcat(romtext, "Sunsoft");break;
			case 34:strcat(romtext, "POW");break;
			case 35:strcat(romtext, "Micro World");break;
			case 36:strcat(romtext, NOTKNOWN "36");break;
			case 37:strcat(romtext, NOTKNOWN "37");break;
			case 38:strcat(romtext, "Enix");break;
			case 39:strcat(romtext, "Loriciel/Electro Brain");break;//uCON64
			case 40:strcat(romtext, "Kemco");break;
			case 41:strcat(romtext, "Seta Co.,Ltd.");break;
			case 42:strcat(romtext, "Culture Brain"); break; //Acc. ZFE
			case 43:strcat(romtext, "Irem Japan");break;//Irem? Gun Force J
			case 44:strcat(romtext, "Pal Soft"); break; //Acc. ZFE
			case 45:strcat(romtext, "Visit Co.,Ltd.");break;
			case 46:strcat(romtext, "INTEC Inc."); break; //Acc. ZFE
			case 47:strcat(romtext, "System Sacom Corp."); break; //Acc. ZFE
			case 48:strcat(romtext, "Viacom New Media");break; //Zoop!
			case 49:strcat(romtext, "Carrozzeria");break;
			case 50:strcat(romtext, "Dynamic");break;
			case 51:strcat(romtext, "Nintendo");break;
			case 52:strcat(romtext, "Magifact");break;
			case 53:strcat(romtext, "Hect");break;
			case 54:strcat(romtext, NOTKNOWN "54");break;
			case 55:strcat(romtext, NOTKNOWN "55");break;
			case 56:strcat(romtext, "Capcom Europe");break;//Capcom? BOF2(E) MM7 (E)
			case 57:strcat(romtext, "Accolade Europe");break;//Accolade?Bubsy 2 (E)
			case 58:strcat(romtext, NOTKNOWN "58");break;
			case 59:strcat(romtext, "Arcade Zone");break;//uCON64
			case 60:strcat(romtext, "Empire Software");break;
			case 61:strcat(romtext, "Loriciel");break;
			case 62:strcat(romtext, "Gremlin Graphics"); break; //Acc. ZFE
			case 63:strcat(romtext, NOTKNOWN "63");break;
			case 64:strcat(romtext, "Seika Corp.");break;
			case 65:strcat(romtext, "UBI Soft");break;
			case 66:strcat(romtext, NOTKNOWN "66");break;
			case 67:strcat(romtext, NOTKNOWN "67");break;
			case 68:strcat(romtext, "LifeFitness Exertainment");break;//?? Exertainment Mountain Bike Rally (U).zip
			case 69:strcat(romtext, NOTKNOWN "69");break;
			case 70:strcat(romtext, "System 3");break;
			case 71:strcat(romtext, "Spectrum Holobyte");break;
			case 72:strcat(romtext, NOTKNOWN "72");break;
			case 73:strcat(romtext, "Irem");break;
			case 74:strcat(romtext, NOTKNOWN "74");break;
			case 75:strcat(romtext, "Raya Systems/Sculptured Software");break;
			case 76:strcat(romtext, "Renovation Products");break;
			case 77:strcat(romtext, "Malibu Games/Black Pearl");break;
			case 78:strcat(romtext, NOTKNOWN "78");break;
			case 79:strcat(romtext, "U.S. Gold");break;
			case 80:strcat(romtext, "Absolute Entertainment");break;
			case 81:strcat(romtext, "Acclaim");break;
			case 82:strcat(romtext, "Activision");break;
			case 83:strcat(romtext, "American Sammy");break;
			case 84:strcat(romtext, "GameTek");break;
			case 85:strcat(romtext, "Hi Tech Expressions");break;
			case 86:strcat(romtext, "LJN Toys");break;
			case 87:strcat(romtext, NOTKNOWN "87");break;
			case 88:strcat(romtext, NOTKNOWN "88");break;
			case 89:strcat(romtext, NOTKNOWN "89");break;
			case 90:strcat(romtext, "Mindscape");break;
			case 91:strcat(romtext, "Romstar, Inc."); break; //Acc. ZFE
			case 92:strcat(romtext, NOTKNOWN "92");break;
			case 93:strcat(romtext, "Tradewest");break;
			case 94:strcat(romtext, NOTKNOWN "94");break;
			case 95:strcat(romtext, "American Softworks Corp.");break;
			case 96:strcat(romtext, "Titus");break;
			case 97:strcat(romtext, "Virgin Interactive Entertainment");break;
			case 98:strcat(romtext, "Maxis");break;
			case 99:strcat(romtext, "Origin/FCI/Pony Canyon");break;//uCON64
			case 100:strcat(romtext, NOTKNOWN "100");break;
			case 101:strcat(romtext, NOTKNOWN "101");break;
			case 102:strcat(romtext, NOTKNOWN "102");break;
			case 103:strcat(romtext, "Ocean");break;
			case 104:strcat(romtext, NOTKNOWN "104");break;
			case 105:strcat(romtext, "Electronic Arts");break;
			case 106:strcat(romtext, NOTKNOWN "106");break;
			case 107:strcat(romtext, "Laser Beam");break;
			case 108:strcat(romtext, NOTKNOWN "108");break;
			case 109:strcat(romtext, NOTKNOWN "109");break;
			case 110:strcat(romtext, "Elite");break;
			case 111:strcat(romtext, "Electro Brain");break;
			case 112:strcat(romtext, "Infogrames");break;
			case 113:strcat(romtext, "Interplay");break;
			case 114:strcat(romtext, "LucasArts");break;
			case 115:strcat(romtext, "Parker Brothers");break;
			case 116:strcat(romtext, "Konami");break;//uCON64
			case 117:strcat(romtext, "STORM");break;
			case 118:strcat(romtext, NOTKNOWN "118");break;
			case 119:strcat(romtext, NOTKNOWN "119");break;
			case 120:strcat(romtext, "THQ Software");break;
			case 121:strcat(romtext, "Accolade Inc.");break;
			case 122:strcat(romtext, "Triffix Entertainment");break;
			case 123:strcat(romtext, NOTKNOWN "123");break;
			case 124:strcat(romtext, "Microprose");break;
			case 125:strcat(romtext, NOTKNOWN "125");break;
			case 126:strcat(romtext, NOTKNOWN "126");break;
			case 127:strcat(romtext, "Kemco");break;
			case 128:strcat(romtext, "Misawa");break;
			case 129:strcat(romtext, "Teichio");break;
			case 130:strcat(romtext, "Namco Ltd.");break;
			case 131:strcat(romtext, "Lozc");break;
			case 132:strcat(romtext, "Koei");break;
			case 133:strcat(romtext, NOTKNOWN "133");break;
			case 134:strcat(romtext, "Tokuma Shoten Intermedia");break;
			case 135:strcat(romtext, "Tsukuda Original"); break; //Acc. ZFE
			case 136:strcat(romtext, "DATAM-Polystar");break;
			case 137:strcat(romtext, NOTKNOWN "137");break;
			case 138:strcat(romtext, NOTKNOWN "138");break;
			case 139:strcat(romtext, "Bullet-Proof Software");break;
			case 140:strcat(romtext, "Vic Tokai");break;
			case 141:strcat(romtext, NOTKNOWN "141");break;
			case 142:strcat(romtext, "Character Soft");break;
			case 143:strcat(romtext, "I\'\'Max");break;
			case 144:strcat(romtext, "Takara");break;
			case 145:strcat(romtext, "CHUN Soft");break;
			case 146:strcat(romtext, "Video System Co., Ltd.");break;
			case 147:strcat(romtext, "BEC");break;
			case 148:strcat(romtext, NOTKNOWN "148");break;
			case 149:strcat(romtext, "Varie");break;
			case 150:strcat(romtext, "Yonezawa / S'Pal Corp."); break; //Acc. ZFE
			case 151:strcat(romtext, "Kaneco");break;
			case 152:strcat(romtext, NOTKNOWN "152");break;
			case 153:strcat(romtext, "Pack in Video");break;
			case 154:strcat(romtext, "Nichibutsu");break;
			case 155:strcat(romtext, "TECMO");break;
			case 156:strcat(romtext, "Imagineer Co.");break;
			case 157:strcat(romtext, NOTKNOWN "157");break;
			case 158:strcat(romtext, NOTKNOWN "158");break;
			case 159:strcat(romtext, NOTKNOWN "159");break;
			case 160:strcat(romtext, "Telenet");break;
			case 161:strcat(romtext, "Hori"); break; //Acc. uCON64
			case 162:strcat(romtext, NOTKNOWN "162");break;
			case 163:strcat(romtext, NOTKNOWN "163");break;
			case 164:strcat(romtext, "Konami");break;
			case 165:strcat(romtext, "K.Amusement Leasing Co.");break;
			case 166:strcat(romtext, NOTKNOWN "166");break;
			case 167:strcat(romtext, "Takara");break;
			case 168:strcat(romtext, NOTKNOWN "168");break;
			case 169:strcat(romtext, "Technos Jap.");break;
			case 170:strcat(romtext, "JVC");break;
			case 171:strcat(romtext, NOTKNOWN "171");break;
			case 172:strcat(romtext, "Toei Animation");break;
			case 173:strcat(romtext, "Toho");break;
			case 174:strcat(romtext, NOTKNOWN "174");break;
			case 175:strcat(romtext, "Namco Ltd.");break;
			case 176:strcat(romtext, "Media Rings Corp."); break; //Acc. ZFE
			case 177:strcat(romtext, "ASCII Co. Activison");break;
			case 178:strcat(romtext, "Bandai");break;
			case 179:strcat(romtext, NOTKNOWN "179");break;
			case 180:strcat(romtext, "Enix America");break;
			case 181:strcat(romtext, NOTKNOWN "181");break;
			case 182:strcat(romtext, "Halken");break;
			case 183:strcat(romtext, NOTKNOWN "183");break;
			case 184:strcat(romtext, NOTKNOWN "184");break;
			case 185:strcat(romtext, NOTKNOWN "185");break;
			case 186:strcat(romtext, "Culture Brain");break;
			case 187:strcat(romtext, "Sunsoft");break;
			case 188:strcat(romtext, "Toshiba EMI");break;
			case 189:strcat(romtext, "Sony Imagesoft");break;
			case 190:strcat(romtext, NOTKNOWN "190");break;
			case 191:strcat(romtext, "Sammy");break;
			case 192:strcat(romtext, "Taito");break;
			case 193:strcat(romtext, NOTKNOWN "193");break;
			case 194:strcat(romtext, "Kemco");break;
			case 195:strcat(romtext, "Square");break;
			case 196:strcat(romtext, "Tokuma Soft");break;
			case 197:strcat(romtext, "Data East");break;
			case 198:strcat(romtext, "Tonkin House");break;
			case 199:strcat(romtext, NOTKNOWN "199");break;
			case 200:strcat(romtext, "KOEI");break;
			case 201:strcat(romtext, NOTKNOWN "201");break;
			case 202:strcat(romtext, "Konami USA");break;
			case 203:strcat(romtext, "NTVIC");break;
			case 204:strcat(romtext, NOTKNOWN "204");break;
			case 205:strcat(romtext, "Meldac");break;
			case 206:strcat(romtext, "Pony Canyon");break;
			case 207:strcat(romtext, "Sotsu Agency/Sunrise");break;
			case 208:strcat(romtext, "Disco/Taito");break;
			case 209:strcat(romtext, "Sofel");break;
			case 210:strcat(romtext, "Quest Corp.");break;
			case 211:strcat(romtext, "Sigma");break;
			case 212:strcat(romtext, "Ask Kodansha Co., Ltd."); break; //Acc. ZFE
			case 213:strcat(romtext, NOTKNOWN "213");break;
			case 214:strcat(romtext, "Naxat");break;
			case 215:strcat(romtext, NOTKNOWN "215");break;
			case 216:strcat(romtext, "Capcom Co., Ltd.");break;
			case 217:strcat(romtext, "Banpresto");break;
			case 218:strcat(romtext, "Tomy");break;
			case 219:strcat(romtext, "Acclaim");break;
			case 220:strcat(romtext, NOTKNOWN "220");break;
			case 221:strcat(romtext, "NCS");break;
			case 222:strcat(romtext, "Human Entertainment");break;
			case 223:strcat(romtext, "Altron");break;
			case 224:strcat(romtext, "Jaleco");break;
			case 225:strcat(romtext, NOTKNOWN "225");break;
			case 226:strcat(romtext, "Yutaka");break;
			case 227:strcat(romtext, NOTKNOWN "227");break;
			case 228:strcat(romtext, "T&ESoft");break;
			case 229:strcat(romtext, "EPOCH Co.,Ltd.");break;
			case 230:strcat(romtext, NOTKNOWN "230");break;
			case 231:strcat(romtext, "Athena");break;
			case 232:strcat(romtext, "Asmik");break;
			case 233:strcat(romtext, "Natsume");break;
			case 234:strcat(romtext, "King Records");break;
			case 235:strcat(romtext, "Atlus");break;
			case 236:strcat(romtext, "Sony Music Entertainment");break;
			case 237:strcat(romtext, NOTKNOWN "237");break;
			case 238:strcat(romtext, "IGS");break;
			case 239:strcat(romtext, NOTKNOWN "239");break;
			case 240:strcat(romtext, NOTKNOWN "240");break;
			case 241:strcat(romtext, "Motown Software");break;
			case 242:strcat(romtext, "Left Field Entertainment");break;
			case 243:strcat(romtext, "Beam Software");break;
			case 244:strcat(romtext, "Tec Magik");break;
			case 245:strcat(romtext, NOTKNOWN "245");break;
			case 246:strcat(romtext, NOTKNOWN "246");break;
			case 247:strcat(romtext, NOTKNOWN "247");break;
			case 248:strcat(romtext, NOTKNOWN "248");break;
			case 249:strcat(romtext, "Cybersoft");break;
			case 250:strcat(romtext, NOTKNOWN "250");break;
			case 251:strcat(romtext, "Psygnosis"); break; //Acc. ZFE
			case 252:strcat(romtext, NOTKNOWN "252");break;
			case 253:strcat(romtext, NOTKNOWN "253");break;
			case 254:strcat(romtext, "Davidson"); break; //Acc. uCON64
			case 255:strcat(romtext, NOTKNOWN "255");break;
			default:strcat(romtext, NOTKNOWN);break;
				}

				strcat(romtext, "\r\nROM Version: ");
				sprintf(temp, "1.%d", (Memory.HiROM)?Memory.ROM[0x0FFDB]:Memory.ROM[0x7FDB]);
				strcat(romtext, temp);
				strcat(romtext, "\r\nRegion: ");
				switch(Memory.ROMRegion)
				{
				case 0:
					strcat(romtext, "Japan");
					break;
				case 1:
					strcat(romtext, "USA/Canada");
					break;
				case 2:
					strcat(romtext, "Oceania, Europe, and Asia");
					break;
				case 3:
					strcat(romtext, "Sweden");
					break;
				case 4:
					strcat(romtext, "Finland");
					break;
				case 5:
					strcat(romtext, "Denmark");
					break;
				case 6:
					strcat(romtext, "France");
					break;
				case 7:
					strcat(romtext, "Holland");
					break;
				case 8:
					strcat(romtext, "Spain");
					break;
				case 9:
					strcat(romtext, "Germany, Austria, and Switzerland");
					break;
				case 10:
					strcat(romtext, "Italy");
					break;
				case 11:
					strcat(romtext, "Hong Kong and China");
					break;
				case 12:
					strcat(romtext, "Indonesia");
					break;
				case 13:
					strcat(romtext, "South Korea");
					break;
				default:
					sprintf(temp, "Unknown region %d", Memory.ROMRegion);
					strcat(romtext, temp);
					break;
				}

				Utf8ToWide romname(Memory.ROMFilename.c_str());
				TCHAR rominfo[4096];
				_stprintf(rominfo, TEXT("File: %s\r\n%s"), (TCHAR *)romname, (TCHAR *)_tFromMS932(romtext));

                SendDlgItemMessage(hDlg, IDC_ROM_DATA, WM_SETTEXT, 0, (LPARAM)((TCHAR *)rominfo));
				break;
			}
			case WM_CTLCOLORSTATIC:

				if(GetDlgCtrlID((HWND)lParam)==IDC_ROM_DATA && GUI.InfoColor!=WIN32_WHITE)
				{
					SetTextColor((HDC)wParam, GUI.InfoColor);
					SetBkColor((HDC)wParam, RGB(0,0,0));
					return (INT_PTR)GetStockObject( BLACK_BRUSH );
				}
				break;
			case WM_PAINT:
				break;

			case WM_COMMAND:
				{
					switch(LOWORD(wParam))
					{
					case IDOK:
					case IDCANCEL:
						EndDialog(hDlg, 0);
						return true;
						break;
					default: break;
					}
				}
			default:
				break;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgAboutProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg)
	{
	case WM_INITDIALOG:
		WinRefreshDisplay();
		{
			TCHAR buf[2048];//find better way of dealing.
			_stprintf(buf,DISCLAIMER_TEXT,TEXT(VERSION));
			SetDlgItemText(hDlg, IDC_DISCLAIMER, buf);
			SetWindowText(hDlg, ABOUT_DIALOG_TITLE APP_NAME);
		}
		return true;
	case WM_PAINT:
		{
		PAINTSTRUCT ps;
		BeginPaint (hDlg, &ps);

		EndPaint (hDlg, &ps);
		}
		return true;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				EndDialog(hDlg, 0);
				return true;
				break;
			default: return false; break;
			}
		}
	default:return false;
	}
}

INT_PTR CALLBACK DlgEmulatorHacksProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UDACCEL accel = { 0, 10 };
    bool must_reset = 0;

    switch (msg)
    {
    case WM_INITDIALOG:

        SendDlgItemMessage(hDlg, IDC_SFX_CLOCK_SPEED_SPIN, UDM_SETRANGE, 0, MAKELPARAM((short)400, (short)50));
        SendDlgItemMessage(hDlg, IDC_SFX_CLOCK_SPEED_SPIN, UDM_SETPOS, 0, Settings.SuperFXClockMultiplier);
        SendDlgItemMessage(hDlg, IDC_SFX_CLOCK_SPEED_SPIN, UDM_SETACCEL, 1, (LPARAM)&accel);

        SendDlgItemMessage(hDlg, IDC_CPU_OVERCLOCK, CB_ADDSTRING, 0, (LPARAM)TEXT("None"));
        SendDlgItemMessage(hDlg, IDC_CPU_OVERCLOCK, CB_ADDSTRING, 0, (LPARAM)TEXT("Low"));
        SendDlgItemMessage(hDlg, IDC_CPU_OVERCLOCK, CB_ADDSTRING, 0, (LPARAM)TEXT("Medium"));
        SendDlgItemMessage(hDlg, IDC_CPU_OVERCLOCK, CB_ADDSTRING, 0, (LPARAM)TEXT("Max"));
        SendDlgItemMessage(hDlg, IDC_CPU_OVERCLOCK, CB_SETCURSEL, Settings.OverclockMode, 0);

        SendDlgItemMessage(hDlg, IDC_SOUND_INTERPOLATION, CB_ADDSTRING, 0, (LPARAM)TEXT("None"));
        SendDlgItemMessage(hDlg, IDC_SOUND_INTERPOLATION, CB_ADDSTRING, 0, (LPARAM)TEXT("Linear"));
        SendDlgItemMessage(hDlg, IDC_SOUND_INTERPOLATION, CB_ADDSTRING, 0, (LPARAM)TEXT("Gaussian (SNES Hardware)"));
        SendDlgItemMessage(hDlg, IDC_SOUND_INTERPOLATION, CB_ADDSTRING, 0, (LPARAM)TEXT("Cubic"));
        SendDlgItemMessage(hDlg, IDC_SOUND_INTERPOLATION, CB_ADDSTRING, 0, (LPARAM)TEXT("Sinc"));
        SendDlgItemMessage(hDlg, IDC_SOUND_INTERPOLATION, CB_SETCURSEL, Settings.InterpolationMethod, 0);

        CheckDlgButton(hDlg, IDC_INVALID_VRAM, !Settings.BlockInvalidVRAMAccessMaster);
        CheckDlgButton(hDlg, IDC_SEPARATE_ECHO_BUFFER, Settings.SeparateEchoBuffer);
        CheckDlgButton(hDlg, IDC_NO_SPRITE_LIMIT, Settings.MaxSpriteTilesPerLine == 128);

        return true;
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:
            if (((Settings.BlockInvalidVRAMAccessMaster != !IsDlgButtonChecked(hDlg, IDC_INVALID_VRAM)) ||
                (Settings.SeparateEchoBuffer != IsDlgButtonChecked(hDlg, IDC_SEPARATE_ECHO_BUFFER)))
                && !Settings.StopEmulation)
            {

                if (MessageBoxA(hDlg, "To make the requested changes, the game will need to be reset.", "Reset Required", MB_OKCANCEL) != IDOK)
                    return false;
                must_reset = true;
            }

            Settings.SuperFXClockMultiplier = SendDlgItemMessage(hDlg, IDC_SFX_CLOCK_SPEED_SPIN, UDM_GETPOS, 0, 0);
            Settings.OverclockMode = SendDlgItemMessage(hDlg, IDC_CPU_OVERCLOCK, CB_GETCURSEL, 0, 0);
            Settings.InterpolationMethod = SendDlgItemMessage(hDlg, IDC_SOUND_INTERPOLATION, CB_GETCURSEL, 0, 0);
            Settings.BlockInvalidVRAMAccessMaster = !IsDlgButtonChecked(hDlg, IDC_INVALID_VRAM);
            Settings.SeparateEchoBuffer = IsDlgButtonChecked(hDlg, IDC_SEPARATE_ECHO_BUFFER);
            Settings.MaxSpriteTilesPerLine = IsDlgButtonChecked(hDlg, IDC_NO_SPRITE_LIMIT) ? 128 : 34;

            switch (Settings.OverclockMode)
            {
            default:
            case 0:
                Settings.OneClockCycle = 6;
                Settings.OneSlowClockCycle = 8;
                Settings.TwoClockCycles = 12;
                break;
            case 1:
                Settings.OneClockCycle = 6;
                Settings.OneSlowClockCycle = 6;
                Settings.TwoClockCycles = 12;
                break;
            case 2:
                Settings.OneClockCycle = 4;
                Settings.OneSlowClockCycle = 6;
                Settings.TwoClockCycles = 8;
                break;
            case 3:
                Settings.OneClockCycle = 3;
                Settings.OneSlowClockCycle = 4;
                Settings.TwoClockCycles = 6;
                break;
            }

            EndDialog(hDlg, must_reset);
            return true;

        case IDCANCEL:
            EndDialog(hDlg, 0);
            return true;

		case IDC_SET_DEFAULTS:
			SendDlgItemMessage(hDlg, IDC_SFX_CLOCK_SPEED_SPIN, UDM_SETPOS, 0, 100);
			SendDlgItemMessage(hDlg, IDC_CPU_OVERCLOCK, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(hDlg, IDC_SOUND_INTERPOLATION, CB_SETCURSEL, 2, 0);
			CheckDlgButton(hDlg, IDC_INVALID_VRAM, false);
			CheckDlgButton(hDlg, IDC_SEPARATE_ECHO_BUFFER, false);
			CheckDlgButton(hDlg, IDC_NO_SPRITE_LIMIT, false);
			break;
        default:
            break;
        }

        return true;
        break;

    default:
        return false;
    }
}

INT_PTR CALLBACK DlgEmulatorProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR paths[10][MAX_PATH];
	static int which = 0;
	switch(msg)
	{
	case WM_INITDIALOG:
		WinRefreshDisplay();
		{
			SetWindowText(hDlg, EMUSET_TITLE);
			SetDlgItemText(hDlg, IDC_LABEL_FREEZE, EMUSET_LABEL_DIRECTORY);
			SetDlgItemText(hDlg, IDOK, BUTTON_OK);
			SetDlgItemText(hDlg, IDCANCEL, BUTTON_CANCEL);
			SetDlgItemText(hDlg, IDC_LABEL_ASRAM, EMUSET_LABEL_ASRAM);
			SetDlgItemText(hDlg, IDC_LABEL_ASRAM_TEXT, EMUSET_LABEL_ASRAM_TEXT);
			SetDlgItemText(hDlg, IDC_LABEL_SMAX, EMUSET_LABEL_SMAX);
			SetDlgItemText(hDlg, IDC_LABEL_SMAX_TEXT, EMUSET_LABEL_SMAX_TEXT);
			SetDlgItemText(hDlg, IDC_LABEL_STURBO_TEXT, EMUSET_LABEL_STURBO_TEXT);
			SetDlgItemText(hDlg, IDC_LABEL_STURBO, EMUSET_LABEL_STURBO);
			SetDlgItemText(hDlg, IDC_BROWSE, EMUSET_BROWSE);
			SetDlgItemText(hDlg, IDC_CUSTOM_FOLDER_FIELD, GUI.FreezeFileDir);
			SetDlgItemText(hDlg, IDC_CONFIG_NAME_BOX, TEXT(S9X_CONF_FILE_NAME));
			SendDlgItemMessage(hDlg, IDC_SRAM_SPIN, UDM_SETRANGE, 0, MAKELPARAM((short)99, (short)0));
			SendDlgItemMessage(hDlg, IDC_SRAM_SPIN,UDM_SETPOS,0, Settings.AutoSaveDelay);
			SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP, UDM_SETRANGE, 0, MAKELPARAM((short)59, (short)0));
			SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP,UDM_SETPOS,0, Settings.AutoMaxSkipFrames);
			SendDlgItemMessage(hDlg, IDC_SPIN_TURBO_SKIP, UDM_SETRANGE, 0, MAKELPARAM((short)600, (short)0));
			SendDlgItemMessage(hDlg, IDC_SPIN_TURBO_SKIP,UDM_SETPOS,0, Settings.TurboSkipFrames);
            SendDlgItemMessage(hDlg, IDC_REWIND_BUFFER_SPIN, UDM_SETRANGE, 0, MAKELPARAM((short)4000, (short)0));
            SendDlgItemMessage(hDlg, IDC_REWIND_BUFFER_SPIN,UDM_SETPOS,0, GUI.rewindBufferSize);
            SendDlgItemMessage(hDlg, IDC_REWIND_GRANULARITY_SPIN, UDM_SETRANGE, 0, MAKELPARAM((short)300, (short)1));
            SendDlgItemMessage(hDlg, IDC_REWIND_GRANULARITY_SPIN,UDM_SETPOS,0, GUI.rewindGranularity);
			SendDlgItemMessage(hDlg, IDC_SFXSPEED_SPIN, UDM_SETRANGE, 0, MAKELPARAM((short)400, (short)50));
			SendDlgItemMessage(hDlg, IDC_SFXSPEED_SPIN, UDM_SETPOS, 0, Settings.SuperFXClockMultiplier);
			CheckDlgButton(hDlg,IDC_INACTIVE_PAUSE,GUI.InactivePause ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg,IDC_CUSTOMROMOPEN,GUI.CustomRomOpen ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_ADD_REGISTRY, GUI.AddToRegistry ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg,IDC_HIRESAVI,GUI.AVIHiRes ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CONFIRMSAVELOAD, GUI.ConfirmSaveLoad ? BST_CHECKED : BST_UNCHECKED);

			int inum = 0;
			lstrcpy(paths[inum++],GUI.RomDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_ROMS);
			lstrcpy(paths[inum++],GUI.ScreensDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_SCREENS);
			lstrcpy(paths[inum++],GUI.MovieDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_MOVIES);
			lstrcpy(paths[inum++],GUI.SPCDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_SPCS);
			lstrcpy(paths[inum++],GUI.FreezeFileDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_SAVES);
			lstrcpy(paths[inum++],GUI.SRAMFileDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_SRAM);
			lstrcpy(paths[inum++],GUI.PatchDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_PATCHES);
			lstrcpy(paths[inum++], GUI.CheatDir);
			SendDlgItemMessage(hDlg, IDC_DIRCOMBO, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_CHEATS);
			lstrcpy(paths[inum++],GUI.BiosDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_BIOS);
			lstrcpy(paths[inum++],GUI.SatDir);
			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)SETTINGS_OPTION_DIRECTORY_SATDATA);

			SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_SETCURSEL,(WPARAM)0,0);
 			SetDlgItemText(hDlg, IDC_CUSTOM_FOLDER_FIELD, paths[0]);
			which = 0;

			SetCurrentDirectory(S9xGetDirectoryT(DEFAULT_DIR));
		}
		case WM_PAINT:
		{
		PAINTSTRUCT ps;
		BeginPaint (hDlg, &ps);

		EndPaint (hDlg, &ps);
		}
		return true;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BROWSE:
				{
					LPMALLOC lpm=NULL;
					LPITEMIDLIST iidl=NULL;
					BROWSEINFO bi;
					memset(&bi, 0, sizeof(BROWSEINFO));
					TCHAR path[MAX_PATH];
					_tfullpath(path, paths[which], MAX_PATH);
					TCHAR title[]=SETTINGS_TITLE_SELECTFOLDER;
					bi.hwndOwner=hDlg;
					bi.pszDisplayName=path;
					bi.lpszTitle=title;
					bi.lpfn = SetSelProc;
					bi.lParam = (LPARAM)(LPCSTR) path;
                    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
					iidl=SHBrowseForFolder(&bi);
					if(iidl) SHGetPathFromIDList(iidl, path);

					SHGetMalloc(&lpm);
					lpm->Free(iidl);
					absToRel(paths[which], path, S9xGetDirectoryT(DEFAULT_DIR));
 					SetDlgItemText(hDlg, IDC_CUSTOM_FOLDER_FIELD, paths[which]);
				}
				break;
			case IDC_CUSTOM_FOLDER_FIELD:
				which = SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_GETCURSEL,0,0);
				GetDlgItemText(hDlg, IDC_CUSTOM_FOLDER_FIELD, paths[which], MAX_PATH);
				break;
			case IDC_DIRCOMBO:
				which = SendDlgItemMessage(hDlg,IDC_DIRCOMBO,CB_GETCURSEL,0,0);
 				SetDlgItemText(hDlg, IDC_CUSTOM_FOLDER_FIELD, paths[which]);
				break;
			case IDOK:
				{
					int inum = 0;
					lstrcpy(GUI.RomDir,paths[inum++]);
					lstrcpy(GUI.ScreensDir,paths[inum++]);
					lstrcpy(GUI.MovieDir,paths[inum++]);
					lstrcpy(GUI.SPCDir,paths[inum++]);
					lstrcpy(GUI.FreezeFileDir,paths[inum++]);
					lstrcpy(GUI.SRAMFileDir,paths[inum++]);
					lstrcpy(GUI.PatchDir,paths[inum++]);
					lstrcpy(GUI.CheatDir, paths[inum++]);
					lstrcpy(GUI.BiosDir,paths[inum++]);
					lstrcpy(GUI.SatDir,paths[inum++]);

					GUI.InactivePause = (BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_INACTIVE_PAUSE));
					GUI.CustomRomOpen = (BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_CUSTOMROMOPEN));
					GUI.AVIHiRes = (BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_HIRESAVI));
					GUI.ConfirmSaveLoad = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CONFIRMSAVELOAD));
					bool AddRegistryChecked = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ADD_REGISTRY));
					if (GUI.AddToRegistry && !AddRegistryChecked)
						S9xWinRemoveRegistryKeys();
					GUI.AddToRegistry = AddRegistryChecked;

					Settings.TurboSkipFrames=SendDlgItemMessage(hDlg, IDC_SPIN_TURBO_SKIP, UDM_GETPOS, 0,0);
					Settings.AutoMaxSkipFrames=SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP, UDM_GETPOS, 0,0);
					Settings.AutoSaveDelay=SendDlgItemMessage(hDlg, IDC_SRAM_SPIN, UDM_GETPOS, 0,0);
                    GUI.rewindGranularity = SendDlgItemMessage(hDlg, IDC_REWIND_GRANULARITY_SPIN, UDM_GETPOS, 0,0);
					Settings.SuperFXClockMultiplier = SendDlgItemMessage(hDlg, IDC_SFXSPEED_SPIN, UDM_GETPOS, 0, 0);
                    if(GUI.rewindGranularity==0) GUI.rewindGranularity = 1;
                    unsigned int newRewindBufSize = SendDlgItemMessage(hDlg, IDC_REWIND_BUFFER_SPIN, UDM_GETPOS, 0,0);
                    if(GUI.rewindBufferSize != newRewindBufSize) {
                        GUI.rewindBufferSize = newRewindBufSize;
                        if(!Settings.StopEmulation) stateMan.init(GUI.rewindBufferSize * 1024 * 1024);
                    }

					WinSaveConfigFile();
				}
				/* fall through */
			case IDCANCEL:
				EndDialog(hDlg, 0);
				return true;
				break;
			default: return false; break;
			}
		}
	default:return false;
	}
}

#define SKIP_FLOPPY

static bool ExtensionIsValid(const TCHAR * filename)
{
	ExtList* curr=valid_ext;
	while(curr!=NULL)
	{
		if(curr->extension==NULL)
		{
			if(NULL==_tcsstr(filename, TEXT(".")))
				return true;
		}
		else if(filename[(lstrlen(filename)-1)-lstrlen(curr->extension)]=='.')
		{
			if(0==_tcsncicmp(&filename[(lstrlen(filename))-lstrlen(curr->extension)],
				curr->extension, lstrlen(curr->extension)))
				return true;
		}
		curr=curr->next;
	}
	return false;
}

bool IsCompressed(const TCHAR* filename)
{
	ExtList* curr=valid_ext;
	while(curr!=NULL)
	{
		if(curr->extension==NULL)
		{
			if(NULL==_tcsstr(filename, TEXT(".")))
				return curr->compressed;
		}
		else if(filename[(lstrlen(filename)-1)-lstrlen(curr->extension)]=='.')
		{
			if(0==_tcsncicmp(&filename[(lstrlen(filename))-lstrlen(curr->extension)],
				curr->extension, lstrlen(curr->extension)))
				return curr->compressed;
		}
		curr=curr->next;
	}
	return false;
}

inline bool AllASCII(char *b, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (b[i] < 32 || b[i] > 126)
		{
			return(false);
		}
	}
	return(true);
}

inline int InfoScore(char *Buffer)
{
	int score = 0;
	if (Buffer[28] + (Buffer[29] << 8) +
		Buffer[30] + (Buffer[31] << 8) == 0xFFFF)
	{	score += 3; }

	if (Buffer[26] == 0x33) { score += 2; }
	if ((Buffer[21] & 0xf) < 4) {	score += 2; }
	if (!(Buffer[61] & 0x80)) { score -= 4; }
	if ((1 << (Buffer[23] - 7)) > 48) { score -= 1; }
	if (Buffer[25] < 14) { score += 1; }
	if (!AllASCII(Buffer, 20)) { score -= 1; }

	return (score);
}

inline unsigned short sum(unsigned char *array, unsigned int size = HEADER_SIZE)
{
	unsigned short theSum = 0;
	for (unsigned int i = 0; i < size; i++)
	{
		theSum += array[i];
	}
	return(theSum);
}

void rominfo(const TCHAR *filename, TCHAR *namebuffer, TCHAR *sizebuffer)
{
	lstrcpy(namebuffer, ROM_ITEM_DESCNOTAVAILABLE);
	lstrcpy(sizebuffer, TEXT("? Mbits"));

#ifdef UNZIP_SUPPORT
	if(IsCompressed(filename))
	{
		unzFile uf = unzOpen(_tToChar(filename));
		if(uf)
		{
			unz_file_info info;
			if(UNZ_OK == unzGetCurrentFileInfo(uf, &info, 0,0,0,0,0,0))
			{
				if (info.uncompressed_size < 0x8000) // Smaller than a block
					lstrcpy(namebuffer, ROM_ITEM_NOTAROM);
				else
					lstrcpy(namebuffer, ROM_ITEM_COMPRESSEDROMDESCRIPTION);

				// should subtract header size, so this may be slightly off, but it's better than "? MBits"
				double MBitD = (double)(info.uncompressed_size - 0) / 0x100000 * 8;
				int MBitI = (int)MBitD;
				int sizeIndex;
				if(0!=(MBitI / 10))
				{
					sizebuffer[0] = MBitI / 10 + TEXT('0');
					sizeIndex = 1;
				}
				else
					sizeIndex = 0;
				sizebuffer[sizeIndex+0] = MBitI % 10 + TEXT('0');
				sizebuffer[sizeIndex+1] = TEXT('.');
				sizebuffer[sizeIndex+2] = (char)((MBitD - MBitI) * 10) + TEXT('0');
				sizebuffer[sizeIndex+3] = (char)((int)((MBitD - MBitI) * 100) % 10) + TEXT('0');
				sizebuffer[sizeIndex+4] = TEXT(' ');
				sizebuffer[sizeIndex+5] = TEXT('M');
				sizebuffer[sizeIndex+6] = TEXT('b');
				sizebuffer[sizeIndex+7] = TEXT('i');
				sizebuffer[sizeIndex+8] = TEXT('t');
				sizebuffer[sizeIndex+9] = TEXT('\0');
			}
			unzClose(uf);
		}
		return;
	}
#endif

	struct _stat filestats;
	_tstat(filename, &filestats);

	int HeaderSize = 0;

	if (filestats.st_size >= 0x8000)
	{
		ifstream ROMFile(filename, ios::in | ios::binary);
		if (ROMFile)
		{
			int HasHeadScore = 0, NoHeadScore = 0,
				HeadRemain = filestats.st_size & 0x7FFF;

			switch(HeadRemain)
			{
			case 0:
				NoHeadScore += 3;
				break;

			case HEADER_SIZE:
				HasHeadScore += 2;
				break;
			}

			unsigned char HeaderBuffer[HEADER_SIZE];
			ROMFile.read((char *)HeaderBuffer, HEADER_SIZE);

			if (sum(HeaderBuffer) < 2500) { HasHeadScore += 2; }

			//SMC/SWC Header
			if (HeaderBuffer[8] == 0xAA &&
				HeaderBuffer[9] == 0xBB &&
				HeaderBuffer[10]== 4)
			{ HasHeadScore += 3; }
			//FIG Header
			else if ((HeaderBuffer[4] == 0x77 && HeaderBuffer[5] == 0x83) ||
				(HeaderBuffer[4] == 0xDD && HeaderBuffer[5] == 0x82) ||
				(HeaderBuffer[4] == 0xDD && HeaderBuffer[5] == 2) ||
				(HeaderBuffer[4] == 0xF7 && HeaderBuffer[5] == 0x83) ||
				(HeaderBuffer[4] == 0xFD && HeaderBuffer[5] == 0x82) ||
				(HeaderBuffer[4] == 0x00 && HeaderBuffer[5] == 0x80) ||
				(HeaderBuffer[4] == 0x47 && HeaderBuffer[5] == 0x83) ||
				(HeaderBuffer[4] == 0x11 && HeaderBuffer[5] == 2))
			{ HasHeadScore += 2; }
			else if (!strncmp("GAME DOCTOR SF 3", (char *)HeaderBuffer, 16))
			{ HasHeadScore += 5; }

			HeaderSize = HasHeadScore > NoHeadScore ? HEADER_SIZE : 0;

			bool EHi = false;
			if (filestats.st_size - HeaderSize >= 0x500000)
			{
				ROMFile.seekg(0x40FFC0 + HeaderSize, ios::beg);
				ROMFile.read((char *)HeaderBuffer, INFO_LEN);
				if (InfoScore((char *)HeaderBuffer) > 1)
				{
					EHi = true;
					_tcsncpy(namebuffer, _tFromMS932((char *)HeaderBuffer), 21);
				}
			}

			if (!EHi)
			{
				if (filestats.st_size - HeaderSize >= 0x10000)
				{
					char LoHead[INFO_LEN], HiHead[INFO_LEN];

					ROMFile.seekg(0x7FC0 + HeaderSize, ios::beg);
					ROMFile.read(LoHead, INFO_LEN);
					int LoScore = InfoScore(LoHead);

					ROMFile.seekg(0xFFC0 + HeaderSize, ios::beg);
					ROMFile.read(HiHead, INFO_LEN);
					int HiScore = InfoScore(HiHead);

					_tcsncpy(namebuffer, _tFromMS932(LoScore > HiScore ? LoHead : HiHead), 21);

					if (filestats.st_size - HeaderSize >= 0x20000)
					{
						ROMFile.seekg((filestats.st_size - HeaderSize) / 2 + 0x7FC0 + HeaderSize, ios::beg);
						ROMFile.read(LoHead, INFO_LEN);
						int IntLScore = InfoScore(LoHead) / 2;

						if (IntLScore > LoScore && IntLScore > HiScore)
						{
							_tcsncpy(namebuffer, _tFromMS932(LoHead), 21);
						}
					}
				}
				else //ROM only has one block
				{
					char buf[21];
					ROMFile.seekg(0x7FC0 + HeaderSize, ios::beg);
					ROMFile.read(buf, 21);
					_tcsncpy(namebuffer,_tFromMS932(buf),21);
				}
			}
			ROMFile.close();
		}
		else //Couldn't open file
		{
			lstrcpy(namebuffer, ROM_ITEM_CANTOPEN);
		}
	}
	else //Smaller than a block
	{
		lstrcpy(namebuffer, ROM_ITEM_NOTAROM);
	}

	double MBitD = (double)(filestats.st_size - HeaderSize) / 0x100000 * 8;
	int MBitI = (int)MBitD;
	int sizeIndex;
	if(0!=(MBitI / 10))
	{
		sizebuffer[0] = MBitI / 10 + TEXT('0');
		sizeIndex = 1;
	}
	else
		sizeIndex = 0;
	sizebuffer[sizeIndex+0] = MBitI % 10 + TEXT('0');
	sizebuffer[sizeIndex+1] = TEXT('.');
	sizebuffer[sizeIndex+2] = (char)((MBitD - MBitI) * 10) + TEXT('0');
	sizebuffer[sizeIndex+3] = (char)((int)((MBitD - MBitI) * 100) % 10) + TEXT('0');
	sizebuffer[sizeIndex+4] = TEXT(' ');
	sizebuffer[sizeIndex+5] = TEXT('M');
	sizebuffer[sizeIndex+6] = TEXT('b');
	sizebuffer[sizeIndex+7] = TEXT('i');
	sizebuffer[sizeIndex+8] = TEXT('t');
	sizebuffer[sizeIndex+9] = TEXT('\0');
	namebuffer[21] = TEXT('\0');
}

void GetPathFromTree( HWND hDlg, UINT tree, TCHAR* selected, HTREEITEM hItem)
{
	TVITEM tv;
	TCHAR temp[MAX_PATH];
	temp[0]=('\0');
	memset(&tv, 0, sizeof(TVITEM));
	HTREEITEM hTreeTemp=hItem;

	if(tv.iImage==7)
	{
		tv.mask=TVIF_HANDLE|TVIF_IMAGE;
		tv.hItem=hTreeTemp;
		tv.iImage=6;
		TreeView_SetItem(GetDlgItem(hDlg, tree),&tv);
		memset(&tv, 0, sizeof(TVITEM));
	}

	tv.mask=TVIF_HANDLE|TVIF_TEXT;
	tv.hItem=hTreeTemp;
	tv.pszText=temp;
	tv.cchTextMax =MAX_PATH;
	TreeView_GetItem(GetDlgItem(hDlg, tree), &tv);

	_stprintf(selected, TEXT("%s"), temp);
	while(TreeView_GetParent(GetDlgItem(hDlg, tree), hTreeTemp))
	{
		temp[0]=TEXT('\0');
		hTreeTemp=TreeView_GetParent(GetDlgItem(hDlg, tree), hTreeTemp);
		tv.mask=TVIF_HANDLE|TVIF_TEXT;
		tv.hItem=hTreeTemp;
		tv.pszText=temp;
		tv.cchTextMax =MAX_PATH;
		TreeView_GetItem(GetDlgItem(hDlg, tree), &tv);
		_stprintf(temp, TEXT("%s\\%s"),temp, selected);
		lstrcpy(selected, temp);
	}
}

typedef struct RomDataCacheNode
{
	TCHAR* fname;
	TCHAR* rname;
	TCHAR* rmbits;
	struct RomDataCacheNode* next;
} RomDataList;

void ClearCacheList(RomDataList* rdl)
{
	RomDataList* temp=rdl;
	RomDataList* temp2=NULL;
	if(rdl==NULL)
		return;
	do
	{
		temp2=temp->next;
		if(temp->fname)
			delete [] temp->fname;
		if(temp->rmbits)
			delete [] temp->rmbits;
		if(temp->rname)
			delete [] temp->rname;
		delete temp;
		temp=temp2;
	}
	while(temp!=NULL);
}


void ExpandDir(TCHAR * selected, HTREEITEM hParent, HWND hDlg)
{
	TCHAR temp[MAX_PATH];
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(WIN32_FIND_DATA));
	lstrcat(selected, TEXT("\\*"));
	HANDLE hFind=FindFirstFile(selected,&wfd);
	selected[(lstrlen(selected)-1)]=TEXT('\0');

	do
	{
		if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if(lstrcmp(wfd.cFileName, TEXT("."))&&lstrcmp(wfd.cFileName, TEXT("..")))
			{
				//skip these, add the rest.
				TV_INSERTSTRUCT tvis;
				memset(&tvis, 0, sizeof(TV_INSERTSTRUCT));
				tvis.hParent=hParent;
				tvis.hInsertAfter=TVI_SORT;
				tvis.item.mask = TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvis.item.pszText=wfd.cFileName;
				tvis.item.cchTextMax=MAX_PATH;
				const bool locked = (wfd.dwFileAttributes&(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_OFFLINE))!=0;
				const bool hidden = (wfd.dwFileAttributes&(FILE_ATTRIBUTE_HIDDEN))!=0;
				tvis.item.iImage=hidden?9:(locked?8:7);
				tvis.item.iSelectedImage=locked?8:6;
				HTREEITEM hNewTree=TreeView_InsertItem(GetDlgItem(hDlg, IDC_ROM_DIR),&tvis);

				lstrcpy(temp, selected);
				lstrcat(temp, wfd.cFileName);
				lstrcat(temp, TEXT("\\*"));

				bool subdir=false;
				WIN32_FIND_DATA wfd2;
				memset(&wfd2, 0, sizeof(WIN32_FIND_DATA));
				HANDLE hFind2=FindFirstFile(temp,&wfd2);
				do
				{
					if(wfd2.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					{
						if(lstrcmp(wfd2.cFileName, TEXT("."))&&lstrcmp(wfd2.cFileName, TEXT("..")))
						{
							subdir=true;
						}
					}
				}
				while(FindNextFile(hFind2, &wfd2)&&!subdir);

				if(subdir)
				{
					TV_INSERTSTRUCT tvis;
					memset(&tvis, 0, sizeof(TV_INSERTSTRUCT));
					tvis.hParent=hNewTree;
					tvis.hInsertAfter=TVI_SORT;
					TreeView_InsertItem(GetDlgItem(hDlg, IDC_ROM_DIR),&tvis);

				}
				FindClose(hFind2);

			}
		}
	}
	while(FindNextFile(hFind, &wfd));

	FindClose(hFind);
	//scan for folders
}



void ListFilesFromFolder(HWND hDlg, RomDataList** prdl)
{
	RomDataList* rdl= *prdl;
	RomDataList* current=NULL;
	int count=0;
	TVITEM tv;
	TCHAR temp[MAX_PATH];
	TCHAR selected[MAX_PATH]; // directory path
	temp[0]='\0';
	memset(&tv, 0, sizeof(TVITEM));
	HTREEITEM hTreeItem=TreeView_GetSelection(GetDlgItem(hDlg, IDC_ROM_DIR));

	GetPathFromTree(hDlg, IDC_ROM_DIR, selected, hTreeItem);

	SendDlgItemMessage(hDlg, IDC_ROMLIST, WM_SETREDRAW, FALSE, 0);
	ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_ROMLIST));
	ClearCacheList(rdl);
	rdl=NULL;
	//Add items here.

	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(WIN32_FIND_DATA));

	lstrcat(selected, TEXT("\\*"));

	HANDLE hFind=FindFirstFile(selected, &wfd);
	selected[(lstrlen(selected)-1)]=TEXT('\0');
	do
	{
		if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			continue;
		if(ExtensionIsValid(wfd.cFileName))
		{
			RomDataList* newitem=new RomDataList;
			memset(newitem, 0, sizeof(RomDataList));
			newitem->fname=new TCHAR[1+lstrlen(wfd.cFileName)];
			lstrcpy(newitem->fname, wfd.cFileName);

			// hide ntldr and no-name files
			if(!newitem->fname || !*newitem->fname || (!lstrcmp(newitem->fname, TEXT("ntldr")) && lstrlen(selected)<4))
				continue;

			// too small to be a ROM
			if (wfd.nFileSizeLow < 0x8000 && !IsCompressed(wfd.cFileName))
				continue;

			count++;

			if(!rdl)
				rdl=newitem;
			else
			{
				if(0>_tcsicmp(newitem->fname,rdl->fname))
				{
					newitem->next=rdl;
					rdl=newitem;
				}
				else
				{
					RomDataList* trail=rdl;
					current=rdl->next;
					while(current!=NULL&&0<_tcsicmp(newitem->fname,current->fname))
					{
						trail=current;
						current=current->next;
					}
					newitem->next=current;
					trail->next=newitem;
				}
			}
		}
	}
	while(FindNextFile(hFind, &wfd));

	FindClose(hFind);

		SendDlgItemMessage(hDlg, IDC_ROMLIST, WM_SETREDRAW, TRUE, 0);
	*prdl=rdl;
	ListView_SetItemCountEx (GetDlgItem(hDlg, IDC_ROMLIST), count, 0);
	ListView_SetItemState (GetDlgItem(hDlg,IDC_ROMLIST), 0, LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
}

// load multicart rom dialog
INT_PTR CALLBACK DlgMultiROMProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:{
		WinRefreshDisplay();
		TCHAR path[MAX_PATH];
		SetCurrentDirectory(S9xGetDirectoryT(BIOS_DIR));
		_tfullpath(path, TEXT("stbios.bin"), MAX_PATH);
		SetDlgItemText(hDlg, IDC_MULTICART_BIOSEDIT, path);
		FILE* ftemp = _tfopen(path, TEXT("rb"));
		if(ftemp)
		{
			fclose(ftemp);
			SetDlgItemText(hDlg, IDC_MULTICART_BIOSNOTFOUND, MULTICART_BIOS_FOUND);
		}
		else
			SetDlgItemText(hDlg, IDC_MULTICART_BIOSNOTFOUND, MULTICART_BIOS_NOT_FOUND);
		SetDlgItemText(hDlg, IDC_MULTICART_EDITA, multiRomA);
		SetDlgItemText(hDlg, IDC_MULTICART_EDITB, multiRomB);
		break;}
	case WM_COMMAND:
		{
			TCHAR rom1[MAX_PATH]={0}, rom2[MAX_PATH]={0};
			SetCurrentDirectory(S9xGetDirectoryT(ROM_DIR));
			switch(LOWORD(wParam))
			{
			case IDOK:
				GetDlgItemText(hDlg, IDC_MULTICART_EDITA, multiRomA, MAX_PATH);
				GetDlgItemText(hDlg, IDC_MULTICART_EDITB, multiRomB, MAX_PATH);
				EndDialog(hDlg, 1);
				return true;
			case IDCANCEL:
				EndDialog(hDlg, 0);
				return true;
			case IDC_MULTICART_SWAP:
				GetDlgItemText(hDlg, IDC_MULTICART_EDITA, rom2, MAX_PATH);
				GetDlgItemText(hDlg, IDC_MULTICART_EDITB, rom1, MAX_PATH);
				SetDlgItemText(hDlg, IDC_MULTICART_EDITA, rom1);
				SetDlgItemText(hDlg, IDC_MULTICART_EDITB, rom2);
				break;
			case IDC_MULTICART_BROWSEA:
				if(!DoOpenRomDialog(rom1, true))
					break;
				SetDlgItemText(hDlg, IDC_MULTICART_EDITA, rom1);
				break;
			case IDC_MULTICART_BROWSEB:
				if(!DoOpenRomDialog(rom2, true))
					break;
				SetDlgItemText(hDlg, IDC_MULTICART_EDITB, rom2);
				break;
			case IDC_MULTICART_CLEARA:
				rom1[0] = TEXT('\0');
				SetDlgItemText(hDlg, IDC_MULTICART_EDITA, rom1);
				break;
			case IDC_MULTICART_CLEARB:
				rom1[1] = TEXT('\0');
				SetDlgItemText(hDlg, IDC_MULTICART_EDITB, rom2);
				break;
			}
		}
	}
	return false;
}

void MoveControlY(HWND hDlg, HWND hCtrl, unsigned int newY)
{
	RECT ctrlRect;
	GetWindowRect(hCtrl, &ctrlRect);
	POINT pt;
	pt.x = ctrlRect.left;
	pt.y = ctrlRect.top;
	ScreenToClient(hDlg, &pt);
	MoveWindow(hCtrl, pt.x, newY, ctrlRect.right - ctrlRect.left, ctrlRect.bottom - ctrlRect.top, FALSE);
}

void MoveOpenRomWindows(HWND hDlg, unsigned int newWidth, int newHeight)
{
	RECT rectRomList, rectDirList, rectSplitter, rectStatic, rectCombo, rectOk, rectCancel;

	HWND romList = GetDlgItem(hDlg, IDC_ROMLIST);
	HWND dirList = GetDlgItem(hDlg, IDC_ROM_DIR);
	HWND splitter = GetDlgItem(hDlg, IDC_ROM_SPLITTER);
	HWND memStatic = GetDlgItem(hDlg, IDC_STATIC_MEMORY_TYPE);
	HWND memCombo = GetDlgItem(hDlg, IDC_MEM_TYPE);
	HWND okButton = GetDlgItem(hDlg, IDOK);
	HWND cancelButton = GetDlgItem(hDlg, IDCANCEL);

	GetWindowRect(romList, &rectRomList);
	GetWindowRect(dirList, &rectDirList);
	GetWindowRect(splitter, &rectSplitter);
	GetWindowRect(memStatic, &rectStatic);
	GetWindowRect(memCombo, &rectCombo);
	GetWindowRect(okButton, &rectOk);
	GetWindowRect(cancelButton, &rectCancel);

	unsigned int comboTop = newHeight - (rectCombo.bottom - rectCombo.top) - 5;
	unsigned int staticTop = comboTop - (rectStatic.bottom - rectStatic.top);
	unsigned int newListHeight = staticTop - 15;
	POINT pt;
	pt.x = rectRomList.left;
	pt.y = rectRomList.top;
	ScreenToClient(hDlg, &pt);
	unsigned int newListWidth = newWidth - pt.x - 5;

	unsigned int buttonTop = newHeight - (rectOk.bottom - rectOk.top) - 5;
	unsigned int buttonLeft = newWidth - (rectOk.right - rectOk.left) - (rectCancel.right - rectCancel.left) - 5;
	unsigned int buttonCancelLeft = buttonLeft + (rectOk.right - rectOk.left);

	// only change width / height
	SetWindowPos(romList, 0, 0, 0, newListWidth, newListHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
	SetWindowPos(dirList, 0, 0, 0, rectDirList.right - rectDirList.left, newListHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
	SetWindowPos(splitter, 0, 0, 0, rectSplitter.right - rectSplitter.left, newListHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);

	// only change y pos
	MoveControlY(hDlg, memStatic, staticTop);
	MoveControlY(hDlg, GetDlgItem(hDlg, IDC_STATIC_INTERLEAVE_MODE), staticTop);
	MoveControlY(hDlg, GetDlgItem(hDlg, IDC_STATIC_VIDEO_SYSTEM), staticTop);
	MoveControlY(hDlg, GetDlgItem(hDlg, IDC_STATIC_HEADER), staticTop);

	MoveControlY(hDlg, memCombo, comboTop);
	MoveControlY(hDlg, GetDlgItem(hDlg, IDC_INTERLEAVE), comboTop);
	MoveControlY(hDlg, GetDlgItem(hDlg, IDC_VIDEO_MODE), comboTop);
	MoveControlY(hDlg, GetDlgItem(hDlg, IDC_HEADER), comboTop);

	// only change position
	SetWindowPos(okButton, 0, buttonLeft, buttonTop, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);
	SetWindowPos(cancelButton, 0, buttonCancelLeft, buttonTop, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);
}

void MoveSplitterRelativeDirList(HWND hDlg, int offset_x)
{
	HWND hTree, hList, hSplitter;
	RECT treeRect;
	RECT listRect;
	hTree = GetDlgItem(hDlg, IDC_ROM_DIR);
	hList = GetDlgItem(hDlg, IDC_ROMLIST);
	hSplitter = GetDlgItem(hDlg, IDC_ROM_SPLITTER);
	GetWindowRect(hTree, &treeRect);

	POINT p;
	p.x = offset_x + treeRect.right;

	p.y = treeRect.top;
	GetWindowRect(hList, &listRect);

	if (p.x > (listRect.right - 50))
	{
		offset_x -= (short)(p.x - (listRect.right - 50));
		p.x = listRect.right - 50;
	}


	ScreenToClient(hDlg, &p);

	if (p.x < 50)
	{
		offset_x += (short)(50 - p.x);
		p.x = 50;
	}

	MoveWindow(hSplitter, p.x, p.y, listRect.left - treeRect.right, listRect.bottom - listRect.top, FALSE);
	MoveWindow(hList, p.x + (listRect.left - treeRect.right), p.y, listRect.right - listRect.left - offset_x, listRect.bottom - listRect.top, TRUE);
	p.x = treeRect.left;
	p.y = treeRect.top;
	ScreenToClient(hDlg, &p);
	MoveWindow(hTree, p.x, p.y, treeRect.right - treeRect.left + offset_x, treeRect.bottom - treeRect.top, true);
	InvalidateRect(hSplitter, NULL, true);
}

void SaveCustomDialogSettings(HWND hDlg)
{
	WINDOWPLACEMENT wndPlacement = { 0 };
	wndPlacement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hDlg, &wndPlacement);
	GUI.customRomDlgSettings.window_maximized = wndPlacement.showCmd == SW_SHOWMAXIMIZED;
	GUI.customRomDlgSettings.window_size = wndPlacement.rcNormalPosition;

	HWND dirList = GetDlgItem(hDlg, IDC_ROM_DIR);
	HWND romList = GetDlgItem(hDlg, IDC_ROMLIST);
	RECT dirRect;
	GetWindowRect(dirList, &dirRect);
	GUI.customRomDlgSettings.folderPaneWidth = dirRect.right - dirRect.left;

	LVCOLUMN col;
	memset(&col, 0, sizeof(LVCOLUMN));
	col.mask = LVCF_WIDTH;

	ListView_GetColumn(romList, 0, &col);
	GUI.customRomDlgSettings.columnFilename = col.cx;

	ListView_GetColumn(romList, 1, &col);
	GUI.customRomDlgSettings.columnDescription = col.cx;

	ListView_GetColumn(romList, 2, &col);
	GUI.customRomDlgSettings.columnSize = col.cx;
}

INT_PTR CALLBACK DlgOpenROMProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int rv=0;
	static HWND hSplit;
	static HIMAGELIST hIcons;
	static TCHAR *filename;
	static RomDataList* rdl;
	static int selectionMarkOverride = -1;
	static bool initDone = false;
	static RomDataList* nextInvalidatedROM = NULL;
	static int nextInvalidatedROMCounter = 0;
	static HWND romList = NULL;
	static HWND dirList = NULL;
	switch(msg)
	{
	case WM_SIZE:
		MoveOpenRomWindows(hDlg, LOWORD(lParam), HIWORD(lParam));
		RedrawWindow(hDlg, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		break;
	case WM_INITDIALOG:
		WinRefreshDisplay();
		{
			initDone = false;

			// suppress annoying "no disk in drive" errors
			SetErrorMode(SEM_FAILCRITICALERRORS);

			romList = GetDlgItem(hDlg,IDC_ROMLIST);
			dirList = GetDlgItem(hDlg,IDC_ROM_DIR);
			filename=(TCHAR*)lParam;
			RECT treeRect;
			RECT listRect;
			WNDCLASSEX wcex;
			TCHAR tempclassname[]=TEXT("S9xSplitter");
			memset(&wcex, 0, sizeof(WNDCLASSEX));
			wcex.cbSize=sizeof(WNDCLASSEX);
			wcex.hInstance=g_hInst;
			wcex.lpfnWndProc=DlgChildSplitProc;
			wcex.lpszClassName=tempclassname;
			wcex.hbrBackground=(HBRUSH)GetStockObject(LTGRAY_BRUSH);
			wcex.hCursor=LoadCursor(NULL, IDC_SIZEWE);
			ATOM aSplitter=RegisterClassEx(&wcex);
			GetWindowRect(dirList, &treeRect);
			GetWindowRect(romList, &listRect);
			POINT p;

			ListView_SetExtendedListViewStyle(romList, LVS_EX_FULLROWSELECT);

			p.x=treeRect.right;
			p.y=treeRect.top;
			ScreenToClient(hDlg, &p);
			hSplit=CreateWindow(TEXT("S9xSplitter"), TEXT(""),WS_CHILD|WS_VISIBLE , p.x, p.y, listRect.left-treeRect.right , listRect.bottom-listRect.top, hDlg, (HMENU)IDC_ROM_SPLITTER, g_hInst,0);

			LVCOLUMN col;
			static const LPTSTR temp1 = ROM_COLUMN_FILENAME;
			memset(&col, 0, sizeof(LVCOLUMN));
			col.mask=LVCF_FMT|LVCF_ORDER|LVCF_TEXT|LVCF_WIDTH;
			col.fmt=LVCFMT_LEFT;
			col.iOrder=0;
			col.cx= GUI.customRomDlgSettings.columnFilename;
			col.cchTextMax=5;
			col.pszText=temp1;

			ListView_InsertColumn(romList,    0,   &col);

			static const LPTSTR temp2 = ROM_COLUMN_DESCRIPTION;
			memset(&col, 0, sizeof(LVCOLUMN));
			col.mask=LVCF_FMT|LVCF_ORDER|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
			col.fmt=LVCFMT_LEFT;
			col.iOrder=1;
			col.cx= GUI.customRomDlgSettings.columnDescription;
			col.cchTextMax=32;
			col.pszText=temp2;
			col.iSubItem=1;

			ListView_InsertColumn(romList,    1,   &col);


			static const LPTSTR temp3 = ROM_COLUMN_SIZE;
			memset(&col, 0, sizeof(LVCOLUMN));
			col.mask=LVCF_FMT|LVCF_ORDER|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
			col.fmt=LVCFMT_LEFT;
			col.iOrder=2;
			col.cx= GUI.customRomDlgSettings.columnSize;
			col.cchTextMax=32;
			col.pszText=temp3;
			col.iSubItem=2;

			ListView_InsertColumn(romList,    2,   &col);


			SendDlgItemMessage(hDlg, IDC_MEM_TYPE,CB_INSERTSTRING,0,(LPARAM)ROM_OPTION_AUTODETECT);
			SendDlgItemMessage(hDlg, IDC_MEM_TYPE,CB_INSERTSTRING,1,(LPARAM)ROM_OPTION_FORCEHIROM);
			SendDlgItemMessage(hDlg, IDC_MEM_TYPE,CB_INSERTSTRING,2,(LPARAM)ROM_OPTION_FORCELOROM);
			SendDlgItemMessage(hDlg, IDC_MEM_TYPE,CB_SETCURSEL,0,0);

			SendDlgItemMessage(hDlg, IDC_VIDEO_MODE,CB_INSERTSTRING,0,(LPARAM)ROM_OPTION_AUTODETECT);
			SendDlgItemMessage(hDlg, IDC_VIDEO_MODE,CB_INSERTSTRING,1,(LPARAM)ROM_OPTION_FORCEPAL);
			SendDlgItemMessage(hDlg, IDC_VIDEO_MODE,CB_INSERTSTRING,2,(LPARAM)ROM_OPTION_FORCENTSC);
			SendDlgItemMessage(hDlg, IDC_VIDEO_MODE,CB_SETCURSEL,0,0);

			SendDlgItemMessage(hDlg, IDC_HEADER,CB_INSERTSTRING,0,(LPARAM)ROM_OPTION_AUTODETECT);
			SendDlgItemMessage(hDlg, IDC_HEADER,CB_INSERTSTRING,1,(LPARAM)ROM_OPTION_FORCEHEADER);
			SendDlgItemMessage(hDlg, IDC_HEADER,CB_INSERTSTRING,2,(LPARAM)ROM_OPTION_FORCENOHEADER);
			SendDlgItemMessage(hDlg, IDC_HEADER,CB_SETCURSEL,0,0);

			SendDlgItemMessage(hDlg, IDC_INTERLEAVE,CB_INSERTSTRING,0,(LPARAM)ROM_OPTION_AUTODETECT);
			SendDlgItemMessage(hDlg, IDC_INTERLEAVE,CB_INSERTSTRING,1,(LPARAM)ROM_OPTION_NONINTERLEAVED);
			SendDlgItemMessage(hDlg, IDC_INTERLEAVE,CB_INSERTSTRING,2,(LPARAM)ROM_OPTION_MODE1);
			SendDlgItemMessage(hDlg, IDC_INTERLEAVE,CB_INSERTSTRING,3,(LPARAM)ROM_OPTION_MODE2);
			SendDlgItemMessage(hDlg, IDC_INTERLEAVE,CB_INSERTSTRING,4,(LPARAM)ROM_OPTION_GD24);
			SendDlgItemMessage(hDlg, IDC_INTERLEAVE,CB_SETCURSEL,0,0);

			hIcons=ImageList_Create(16,16,ILC_COLOR24,10,10);

			HANDLE hBitmap;

#define ADD_IMAGE(IDB_NAME) \
			hBitmap=LoadImage(g_hInst, MAKEINTRESOURCE(IDB_NAME), IMAGE_BITMAP, 0,0, LR_DEFAULTCOLOR|LR_CREATEDIBSECTION); \
			ImageList_Add(hIcons, (HBITMAP)hBitmap, NULL); \
			DeleteObject(hBitmap);

			ADD_IMAGE(IDB_HARDDRIVE); // 0
			ADD_IMAGE(IDB_CDDRIVE); // 1
			ADD_IMAGE(IDB_NETDRIVE); // 2
			ADD_IMAGE(IDB_REMOVABLE); // 3
			ADD_IMAGE(IDB_RAMDISK); // 4
			ADD_IMAGE(IDB_UNKNOWN); // 5
			ADD_IMAGE(IDB_OPENFOLDER); // 6
			ADD_IMAGE(IDB_CLOSEDFOLDER); // 7
			ADD_IMAGE(IDB_LOCKEDFOLDER); // 8
			ADD_IMAGE(IDB_HIDDENFOLDER); // 9

			TreeView_SetImageList(dirList, hIcons, TVSIL_NORMAL);

//			DWORD dw;
			TCHAR buffer[MAX_PATH];
			TCHAR blah[MAX_PATH];
			long result=ERROR_SUCCESS/*-1*/;
			HTREEITEM hTreeDrive=NULL;

			TCHAR drive [_MAX_DRIVE + 1];
			lstrcpy (drive,TEXT("C:\\"));


			_tfullpath (buffer, S9xGetDirectoryT(ROM_DIR), MAX_PATH);
			_tsplitpath (buffer, drive, NULL, NULL, NULL);

			DWORD driveMask=GetLogicalDrives();

#ifndef SKIP_FLOPPY
			for (int i=0;i<26;i++)
#else
				for (int i=2;i<26;i++)
#endif
				{
					if(driveMask&(1<<i))
					{
						TCHAR driveName[4];
						driveName[0]='A'+i;
						driveName[1]=':';
						driveName[2]='\\';
						driveName[3]='\0';
						UINT driveType=GetDriveType(driveName);
						driveName[2]='\0';

						TVINSERTSTRUCT tvis;
						memset(&tvis, 0, sizeof(TVINSERTSTRUCT));

						tvis.hParent=NULL;
						tvis.hInsertAfter=TVI_ROOT;
						tvis.item.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;

						switch(driveType)
						{
							case DRIVE_FIXED:     tvis.item.iSelectedImage=tvis.item.iImage=0; break;
							case DRIVE_CDROM:     tvis.item.iSelectedImage=tvis.item.iImage=1; break;
							case DRIVE_REMOTE:    tvis.item.iSelectedImage=tvis.item.iImage=2; break;
							case DRIVE_REMOVABLE: tvis.item.iSelectedImage=tvis.item.iImage=3; break;
							case DRIVE_RAMDISK:   tvis.item.iSelectedImage=tvis.item.iImage=4; break;
							default:              tvis.item.iSelectedImage=tvis.item.iImage=5; break;
						}

						tvis.item.pszText=driveName;

						HTREEITEM hTwee=TreeView_InsertItem(dirList,&tvis);

						if(result==ERROR_SUCCESS && !_tcsnicmp(drive, driveName, 2))
							hTreeDrive=hTwee;

						TCHAR temp[10];
						lstrcpy(temp, driveName);
						lstrcat(temp, TEXT("\\*"));
						bool subdir=false;

						if(driveType==DRIVE_REMOVABLE || driveType == DRIVE_CDROM || driveType == DRIVE_UNKNOWN)
						{
								TV_INSERTSTRUCT tvis;
								memset(&tvis, 0, sizeof(TV_INSERTSTRUCT));
								tvis.hParent=hTwee;
								tvis.hInsertAfter=TVI_SORT;
								TreeView_InsertItem(dirList,&tvis);

						}
						else
						{
							WIN32_FIND_DATA wfd2;
							memset(&wfd2, 0, sizeof(WIN32_FIND_DATA));
							HANDLE hFind2=FindFirstFile(temp,&wfd2);
							do
							{
								if(wfd2.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
								{
									if(lstrcmp(wfd2.cFileName, TEXT("."))&&lstrcmp(wfd2.cFileName, TEXT("..")))
									{
										subdir=true;
									}
								}
							}
							while(FindNextFile(hFind2, &wfd2)&&!subdir);

							if(subdir)
							{
								TV_INSERTSTRUCT tvis;
								memset(&tvis, 0, sizeof(TV_INSERTSTRUCT));
								tvis.hParent=hTwee;
								tvis.hInsertAfter=TVI_SORT;
								TreeView_InsertItem(dirList,&tvis);
							}
							FindClose(hFind2);
						}
					}
				}

				SendDlgItemMessage(hDlg, IDC_ROM_DIR, WM_SETREDRAW, FALSE, 0);

				if(result==ERROR_SUCCESS)
				{
					HTREEITEM hTreePrev;//,hTreeRoot;
				//	hTreePrev=TreeView_GetRoot(dirList);
				//	hTreeRoot=hTreeDrive;
					hTreePrev=hTreeDrive;
					HTREEITEM hTemp=hTreePrev;
					TCHAR* temp=buffer;
					TCHAR* temp2, * temp3;

					do
					{
						temp2=_tcsstr(temp, TEXT("\\"));
						temp3=_tcsstr(temp, TEXT("/"));
						if(temp3 && temp3 < temp2)
							temp2 = temp3;

						TVITEM tvi;
						memset(&tvi, 0, sizeof(TVITEM));
						tvi.mask=TVIF_TEXT;
						tvi.pszText=blah;
						tvi.cchTextMax=MAX_PATH;
						blah[0]=TEXT('\0');

						if(temp2)
							*temp2=TEXT('\0');

						tvi.hItem=hTemp;
						TreeView_GetItem(dirList, &tvi);

						if(_tcsicmp(blah, temp) != 0)
						{
							do
							{
								tvi.mask=TVIF_TEXT;
								tvi.pszText=blah;
								tvi.cchTextMax=MAX_PATH;
								hTemp=TreeView_GetNextSibling(dirList, hTemp);
								tvi.hItem=hTemp;
								TreeView_GetItem(dirList, &tvi);
							}
							while((hTemp != NULL) && (_tcsicmp(blah, temp) != 0));

							if(hTemp!=NULL)
							{
								hTreePrev=hTemp;

								TreeView_SelectItem(dirList, hTreePrev);
								TreeView_EnsureVisible(dirList, hTreePrev);
								if(temp2)
									TreeView_Expand(dirList, hTreePrev, TVE_EXPAND);

								hTemp=TreeView_GetChild(dirList, hTreePrev);
							}
						}
						else
						{
							TreeView_SelectItem(dirList, hTemp);
							TreeView_EnsureVisible(dirList, hTemp);
							if(temp2)
								TreeView_Expand(dirList, hTemp, TVE_EXPAND);

							hTemp=TreeView_GetChild(dirList, hTemp);
						}
						if(temp2)
							temp=temp2+1;
						else
							temp=NULL;
					}
					while(temp);

					if(Memory.ROMFilename[0]!='\0')
					{
						LVFINDINFO lvfi;
						memset(&lvfi, 0, sizeof(LVFINDINFO));
						TCHAR filename[_MAX_PATH];
						TCHAR *tmp, *tmp2;
						lstrcpy(filename,_tFromChar(Memory.ROMFilename.c_str()));
						tmp = filename;
						while(tmp2=_tcsstr(tmp, TEXT("\\")))
							tmp=tmp2+sizeof(TCHAR);

						lvfi.flags=LVFI_STRING;
						lvfi.psz=tmp2;

						int idx=ListView_FindItem(romList, -1, &lvfi);
						ListView_SetSelectionMark(romList, idx);
						ListView_SetItemState(romList, idx, LVIS_SELECTED|LVIS_FOCUSED,LVIS_FOCUSED|LVIS_SELECTED);
						ListView_EnsureVisible(romList, idx, FALSE);

					}
					SendDlgItemMessage(hDlg, IDC_ROM_DIR, WM_SETREDRAW, TRUE, 0);
				}
				initDone = true;

				ListView_EnsureVisible (romList, (int)SendMessage(romList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED), FALSE);

				// start up the WM_TIMER event
				nextInvalidatedROM = rdl;
				nextInvalidatedROMCounter = 0;
				SetTimer(hDlg,42,600,NULL);

				WINDOWPLACEMENT wndPlacement = { 0 };
				wndPlacement.length = sizeof(WINDOWPLACEMENT);
				wndPlacement.showCmd = GUI.customRomDlgSettings.window_maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
				wndPlacement.rcNormalPosition = GUI.customRomDlgSettings.window_size;
				SetWindowPlacement(hDlg, &wndPlacement);

				RECT clientRect;
				GetClientRect(hDlg, &clientRect);
				MoveOpenRomWindows(hDlg, clientRect.right, clientRect.bottom);
				MoveSplitterRelativeDirList(hDlg, GUI.customRomDlgSettings.folderPaneWidth - (treeRect.right - treeRect.left));

				return true; //true sets the keyboard focus, in case we need this elsewhere
		}
		case WM_TIMER:
			{
				if(!initDone || !nextInvalidatedROM || !rdl)
					return false;

				// see if current selection needs filling in, and skip to that if so
				int selected = (int)SendMessage(romList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
				if(selected>=0)
				{
					RomDataList* curr=rdl;
					for(int i=0;i<selected;i++)
						if(curr) curr=curr->next;
					if(curr && !curr->rname)
					{
						nextInvalidatedROM = curr;
						nextInvalidatedROMCounter = selected;
					}
				}

				LVHITTESTINFO lvhi;
				lvhi.flags = LVHT_ONITEM;
				lvhi.iItem = 0;
				lvhi.iSubItem = 0;
				lvhi.pt.x = 0;
				lvhi.pt.y = 0;
				ListView_HitTest(romList, &lvhi);
				int firstVisibleItem = lvhi.iItem+1;
				int lastVisibleItem = firstVisibleItem+20;

				// skip up to 100 things that don't need updating
				bool enteredValid = false;
				if(nextInvalidatedROM->rname || nextInvalidatedROMCounter<firstVisibleItem || nextInvalidatedROMCounter>lastVisibleItem)
				for(int i = 0 ; i < 100 ; i++)
					if(nextInvalidatedROM->rname || (!enteredValid && (nextInvalidatedROMCounter<firstVisibleItem || nextInvalidatedROMCounter>lastVisibleItem)))
					{
						if(!enteredValid && nextInvalidatedROMCounter>=firstVisibleItem && nextInvalidatedROMCounter<lastVisibleItem)
							enteredValid = true;
						nextInvalidatedROM = nextInvalidatedROM->next;
						if(nextInvalidatedROM)
							nextInvalidatedROMCounter++;
						else
						{
							nextInvalidatedROM = rdl;
							nextInvalidatedROMCounter = 0;
						}
					}

				// update 1 item, if it needs updating
				if(!nextInvalidatedROM->rname)
				{
					TCHAR path[MAX_PATH];
					TCHAR buffer[32];
					TCHAR buffer2[32];
					GetPathFromTree(hDlg, IDC_ROM_DIR, path, TreeView_GetSelection(dirList));
					lstrcat(path, TEXT("\\"));
					lstrcat(path, nextInvalidatedROM->fname);
					rominfo(path, buffer, buffer2);
					nextInvalidatedROM->rname=new TCHAR[lstrlen(buffer)+1];
					lstrcpy(nextInvalidatedROM->rname, buffer);
					nextInvalidatedROM->rmbits=new TCHAR[lstrlen(buffer2)+1];
					lstrcpy(nextInvalidatedROM->rmbits, buffer2);

					ListView_RedrawItems(romList,nextInvalidatedROMCounter,nextInvalidatedROMCounter);
				}

				// next timer
				nextInvalidatedROM = nextInvalidatedROM->next;
				if(nextInvalidatedROM)
					nextInvalidatedROMCounter++;
				else
				{
					nextInvalidatedROM = rdl;
					nextInvalidatedROMCounter = 0;
				}
				SetTimer(hDlg,42,600,NULL);
				return true;
			}
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint (hDlg, &ps);

		EndPaint (hDlg, &ps);
		}
		return true;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					LVITEM lvi;
					memset(&lvi, 0, sizeof(LVITEM));
					//get selections
					int list_index = selectionMarkOverride == -1 ? ListView_GetSelectionMark(romList) : selectionMarkOverride;
					if(list_index!=-1 && (int)SendMessage(romList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED)!=-1)
					{
						rv=1;
						TCHAR temp[MAX_PATH];
						temp[0]='\0';
						lvi.iItem=list_index;
						lvi.mask=LVIF_TEXT;
						lvi.pszText=filename;
						lvi.cchTextMax=MAX_PATH;
						ListView_GetItem(romList, &lvi);

						lstrcpy(temp, filename);

						HTREEITEM hTreeTemp=TreeView_GetSelection(dirList);
						TVITEM tv;
						memset(&tv, 0, sizeof(TVITEM));

						tv.mask=TVIF_HANDLE|TVIF_TEXT;
						tv.hItem=hTreeTemp;
						tv.pszText=temp;
						tv.cchTextMax =MAX_PATH;
						TreeView_GetItem(dirList, &tv);
						_stprintf(temp, TEXT("%s\\%s"), temp, filename);

						lstrcpy(filename, temp);

						while(TreeView_GetParent(dirList, hTreeTemp)!=NULL)
						{
							temp[0]=TEXT('\0');
							hTreeTemp=TreeView_GetParent(dirList, hTreeTemp);
							tv.mask=TVIF_HANDLE|TVIF_TEXT;
							tv.hItem=hTreeTemp;
							tv.pszText=temp;
							tv.cchTextMax =MAX_PATH;
							TreeView_GetItem(dirList, &tv);
							_stprintf(temp, TEXT("%s\\%s"),temp, filename);
							lstrcpy(filename, temp);
						}

						int iTemp=SendDlgItemMessage(hDlg, IDC_MEM_TYPE,CB_GETCURSEL,0,0);

						Settings.ForceHiROM=Settings.ForceLoROM=FALSE;
						if(iTemp==1)
							Settings.ForceHiROM=TRUE;
						else if(iTemp==2)
							Settings.ForceLoROM=TRUE;

						iTemp=SendDlgItemMessage(hDlg, IDC_INTERLEAVE,CB_GETCURSEL,0,0);

						Settings.ForceNotInterleaved=Settings.ForceInterleaved=Settings.ForceInterleaved2=Settings.ForceInterleaveGD24=FALSE;
						if(iTemp==1)
							Settings.ForceNotInterleaved=TRUE;
						else if(iTemp==2)
							Settings.ForceInterleaved=TRUE;
						else if(iTemp==3)
							Settings.ForceInterleaved2=TRUE;
						else if(iTemp==4)
							Settings.ForceInterleaveGD24=TRUE;

						iTemp=SendDlgItemMessage(hDlg, IDC_VIDEO_MODE,CB_GETCURSEL,0,0);

						Settings.ForceNTSC=Settings.ForcePAL=FALSE;
						if(iTemp==1)
							Settings.ForcePAL=TRUE;
						else if(iTemp==2)
							Settings.ForceNTSC=TRUE;


						iTemp=SendDlgItemMessage(hDlg, IDC_HEADER,CB_GETCURSEL,0,0);

						Settings.ForceNoHeader=Settings.ForceHeader=FALSE;
						if(iTemp==1)
							Settings.ForceHeader=TRUE;
						else if(iTemp==2)
							Settings.ForceNoHeader=TRUE;

						lstrcpy(temp, filename);
						int i=lstrlen(temp);
						while(temp[i]!=TEXT('\\') && temp[i]!=TEXT('/'))
						{
							temp[i]=TEXT('\0');
							i--;
						}
						temp[i]=TEXT('\0');

						if(!GUI.LockDirectories)
							absToRel(GUI.RomDir, temp, S9xGetDirectoryT(DEFAULT_DIR));
					}
					else
					{
						return false;
					}
				}
			case IDCANCEL:
				SaveCustomDialogSettings(hDlg);
				EndDialog(hDlg, rv);
				ClearCacheList(rdl);
				rdl=NULL;
				DestroyWindow(hSplit);
				UnregisterClass(TEXT("S9xSplitter"), g_hInst);
				TreeView_DeleteAllItems(dirList);
				ListView_DeleteAllItems(romList);
				return true;
				break;
			default: return false; break;
			}
		}
	case WM_NOTIFY:
		{
			if(lParam == 0)
				return false;
			NMHDR* pNmh=(NMHDR*)lParam;
			static int foundItemOverride = -1;
			switch(pNmh->idFrom)
			{
			case IDC_ROMLIST:
				{
					switch(pNmh->code)
					{
					// allow typing in a ROM filename (or part of it) to jump to it
					// necessary to implement ourselves because Windows doesn't provide
					// this functionality for virtual (owner data) lists such as this
					case LVN_ODFINDITEM:
						{
							LRESULT pResult;

							// pNMHDR has information about the item we should find
							// In pResult we should save which item that should be selected
							NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)lParam;

							/* pFindInfo->iStart is from which item we should search.
							We search to bottom, and then restart at top and will stop
							at pFindInfo->iStart, unless we find an item that match
							*/

							// Set the default return value to -1
							// That means we didn't find any match.
							pResult = -1;

							//Is search NOT based on string?
							if( (pFindInfo->lvfi.flags & LVFI_STRING) == 0 )
							{
								//This will probably never happend...
								return pResult;
							}

							//This is the string we search for
							LPCTSTR searchstr = pFindInfo->lvfi.psz;

							int startPos = pFindInfo->iStart;
							//Is startPos outside the list (happens if last item is selected)
							if(startPos >= ListView_GetItemCount(romList))
								startPos = 0;

							if(rdl==NULL)
								return pResult;

							RomDataList* curr=rdl;
							for(int i=0;i<startPos;i++)
								curr=curr->next;

							int currentPos=startPos;
							pResult=startPos;

							bool looped = false;

							// perform search
							do
							{
								// does this word begin with all characters in searchstr?
								if( _tcsnicmp(curr->fname, searchstr, lstrlen(searchstr)) == 0)
								{
									// select this item and stop search
									pResult = currentPos;
									break;
								}
								else if( _tcsnicmp(curr->fname, searchstr, lstrlen(searchstr)) > 0)
								{
									if(looped)
									{
										pResult = currentPos;
										break;
									}

									// optimization: the items are ordered alphabetically, so go back to the top since we know it can't be anything further down
									curr=rdl;
									currentPos = 0;
									looped = true;
									continue;
								}

								//Go to next item
								currentPos++;
								curr=curr->next;

								//Need to restart at top?
								if(currentPos >= ListView_GetItemCount(romList))
								{
									currentPos = 0;
									curr = rdl;
								}

							//Stop if back to start
							}while(currentPos != startPos);

							foundItemOverride = pResult;

							// in case previously-selected item is 0
							ListView_SetItemState (romList, 1, LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);

							return pResult; // HACK: for some reason this selects the first item instead of what it's returning... current workaround is to manually re-select this return value upon the next changed event
						}
						break;
					case LVN_ITEMCHANGED:
						{
							// hack - see note directly above
							LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lParam;
							if(lpnmlv->uNewState & (LVIS_SELECTED|LVIS_FOCUSED))
							{
								if(foundItemOverride != -1 && lpnmlv->iItem == 0)
								{
									ListView_SetItemState (romList, foundItemOverride, LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
									ListView_EnsureVisible (romList, foundItemOverride, FALSE);
									selectionMarkOverride = foundItemOverride;
									foundItemOverride = -1;
								}
								else
								{
									selectionMarkOverride = lpnmlv->iItem;
								}
							}
						}
						break;
					case LVN_GETDISPINFO:
						{
							if(!initDone)
								return false;
							int i, j;
							RomDataList* curr=rdl;
							if(rdl==NULL)
								return false;
							NMLVDISPINFO * nmlvdi=(NMLVDISPINFO*)lParam;
							j=nmlvdi->item.iItem;
							for(i=0;i<j;i++)
								if(curr) curr=curr->next;
							if(curr==NULL)
								return false;
							//if(curr->rname==NULL && j==(int)SendMessage(romList, LVM_GETNEXTITEM, -1, LVNI_SELECTED))
							//{
							//	TCHAR path[MAX_PATH];
							//	TCHAR buffer[32];
							//	TCHAR buffer2[32];
							//	GetPathFromTree(hDlg, IDC_ROM_DIR, path, TreeView_GetSelection(dirList));
							//	strcat(path, "\\");
							//	strcat(path, curr->fname);
							//	rominfo(path, buffer, buffer2);
							//	curr->rname=new char[strlen(buffer)+1];
							//	strcpy(curr->rname, buffer);
							//	curr->rmbits=new char[strlen(buffer2)+1];
							//	strcpy(curr->rmbits, buffer2);
							//}

							if(nmlvdi->item.iSubItem==0)
							{
								nmlvdi->item.pszText = (TCHAR *)(curr->fname?curr->fname:TEXT(""));
								nmlvdi->item.cchTextMax=MAX_PATH;
							}
							if(nmlvdi->item.iSubItem==1)
							{
								nmlvdi->item.pszText = (TCHAR *)(curr->rname?curr->rname:TEXT(""));
								nmlvdi->item.cchTextMax=24;
							}

							if(nmlvdi->item.iSubItem==2)
							{
								nmlvdi->item.pszText = (TCHAR *)(curr->rmbits?curr->rmbits:TEXT(""));
								nmlvdi->item.cchTextMax=11;
							}
							// nmlvdi->item.mask=LVIF_TEXT; // This is bad as wine relies on this to not change.
						}
						break;
					case NM_DBLCLK:
						{
							PostMessage(hDlg, WM_COMMAND, (WPARAM)(IDOK),(LPARAM)(NULL));
						}
					default:break;
					}
				}
				break;
			case IDC_ROM_DIR:
				{
					switch(pNmh->code)
					{
					case TVN_ITEMEXPANDING:
						{
							TCHAR selected[MAX_PATH];
							NMTREEVIEW* nmTv=(NMTREEVIEW*)lParam;

							while(TreeView_GetChild(dirList,nmTv->itemNew.hItem))
							{
								TreeView_DeleteItem(dirList, TreeView_GetChild(dirList,nmTv->itemNew.hItem));
							}

							if(nmTv->action&TVE_EXPAND)
							{

								GetPathFromTree(hDlg, IDC_ROM_DIR, selected,nmTv->itemNew.hItem);
								ExpandDir(selected, nmTv->itemNew.hItem, hDlg);
							}
							else
							{
								TVITEM tv;
								memset(&tv, 0, sizeof(TVITEM));
								HTREEITEM hTreeTemp=nmTv->itemNew.hItem;

								if(tv.iImage==6)
								{
									tv.mask=TVIF_HANDLE|TVIF_IMAGE;
									tv.hItem=hTreeTemp;
									tv.iImage=7;
									TreeView_SetItem(dirList,&tv);
								}


								TV_INSERTSTRUCT tvis;
								memset(&tvis, 0, sizeof(TV_INSERTSTRUCT));
								tvis.hParent=nmTv->itemNew.hItem;
								tvis.hInsertAfter=TVI_SORT;
								TreeView_InsertItem(dirList,&tvis);

							}
						}
						return false;
						break;
					case TVN_SELCHANGED:
						{
							ListFilesFromFolder(hDlg, &rdl);
							nextInvalidatedROM = rdl;
							nextInvalidatedROMCounter = 0;
						}
					default:return false;
					}
				}
			default:return false;
			}
		}
	default:return false;
	}
	return false;
}

LRESULT CALLBACK DlgChildSplitProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool PaintSpecial;
	static short drag_x;
	short temp_x;
	switch(msg)
	{
	case WM_CREATE:
		return 0;
	case WM_SIZE:
        return 0;
    case WM_PAINT:
		PAINTSTRUCT ps;
		GetUpdateRect (hWnd, &ps.rcPaint, true);
		ps.hdc=GetDC(hWnd);
		ps.fErase=true;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		ReleaseDC(hWnd, ps.hdc);
        return 0;
    case WM_LBUTTONDOWN:
		PaintSpecial=true;
		drag_x=GET_X_LPARAM(lParam);
		SetCapture(hWnd);
        return 0;
    case WM_LBUTTONUP:
	{
		PaintSpecial = false;
		temp_x = (GET_X_LPARAM(lParam) - drag_x);
		HWND hDlg = GetParent(hWnd);
		MoveSplitterRelativeDirList(hDlg, temp_x);
		ReleaseCapture();
		return 0;
	}
    case WM_MOUSEMOVE:
	{
		if (wParam & MK_LBUTTON)
		{
			//move paint location
			PaintSpecial = true;
			temp_x = (GET_X_LPARAM(lParam) - drag_x);
			HWND hDlg = GetParent(hWnd);
			MoveSplitterRelativeDirList(hDlg, temp_x);
		}
		return 0;
	}
    case WM_CAPTURECHANGED:
		PaintSpecial=false;
		ReleaseCapture();
		return 0;
    case WM_DESTROY:
        return 0;
	default:return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}




#ifdef NETPLAY_SUPPORT
INT_PTR CALLBACK DlgNetConnect(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	HKEY hKey;
	TCHAR defPort[5];
	TCHAR portTemp[5];
	TCHAR temp[100];
//	char temp2[5];
	static TCHAR* hostname;
//	unsigned long cbData;
//	static int i;
	if(Settings.Port==0)
	{
		_itot(1996,defPort,10);
	}
	else
	{
		_itot(Settings.Port,defPort,10);
	}

	WORD chkLength;
//	if(RegCreateKeyEx(HKEY_CURRENT_USER,MY_REG_KEY "\\1.x\\NetPlayServerHistory",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS, NULL, &hKey,NULL) == ERROR_SUCCESS){}

	switch (msg)
	{
	case WM_INITDIALOG:
		WinRefreshDisplay();
		SetWindowText(hDlg,NPCON_TITLE);
		SetDlgItemText(hDlg,IDC_LABEL_SERVERADDY,NPCON_LABEL_SERVERADDY);
		SetDlgItemText(hDlg,IDC_LABEL_PORTNUM,NPCON_LABEL_PORTNUM);
		SetDlgItemText(hDlg,IDC_CLEARHISTORY, NPCON_CLEARHISTORY);
		SetDlgItemText(hDlg,IDOK,BUTTON_OK);
		SetDlgItemText(hDlg,IDCANCEL,BUTTON_CANCEL);
		hostname = (TCHAR *)lParam;
		{
			for(int i=0; i<MAX_RECENT_HOSTS_LIST_SIZE && *GUI.RecentHostNames[i]; i++)
				SendDlgItemMessage(hDlg, IDC_HOSTNAME, CB_INSERTSTRING,i,(LPARAM)GUI.RecentHostNames[i]);
		}

		SendDlgItemMessage(hDlg, IDC_PORTNUMBER, WM_SETTEXT, 0, (LPARAM)defPort);

		SendDlgItemMessage(hDlg, IDC_HOSTNAME, WM_SETTEXT, 0, (LPARAM)NPCON_ENTERHOST);

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CLEARHISTORY:
			{
				{
					SendDlgItemMessage(hDlg,IDC_HOSTNAME,CB_RESETCONTENT,0,0);
					SendDlgItemMessage(hDlg,IDC_HOSTNAME,CB_INSERTSTRING,0,(LPARAM)GUI.RecentHostNames[0]);
					for(int i=1; i<MAX_RECENT_HOSTS_LIST_SIZE; i++)
						*GUI.RecentHostNames[i] = TEXT('\0');
				}
				break;
			}
		case IDOK:
			{

				chkLength = (WORD) SendDlgItemMessage(hDlg,IDC_PORTNUMBER,EM_LINELENGTH,0,0);
				*((LPWORD)portTemp) = chkLength;
				SendDlgItemMessage(hDlg,IDC_PORTNUMBER,EM_GETLINE,0,(LPARAM)(LPCTSTR)portTemp);

				if(_ttoi(portTemp)>65535||_ttoi(portTemp)<1024)
				{
					MessageBox(hDlg,TEXT("Port Number needs to be between 1024 and 65535"),TEXT("Error"),MB_OK);
					break;
				}
				else
				{
					Settings.Port = _ttoi(portTemp);
				}
				//chkLength = (WORD) SendDlgItemMessage(hDlg,IDC_HOSTNAME,EM_LINELENGTH,0,0);
				//if(chkLength > 0)
				//{
				//SendDlgItemMessage(hDlg,IDC_HOSTNAME,EM_GETLINE,0,(LPARAM)hostname);
				SendDlgItemMessage(hDlg,IDC_HOSTNAME,WM_GETTEXT,100,(LPARAM)temp);
				if(!lstrcmp(temp, NPCON_ENTERHOST))
				{
					MessageBox(hDlg,NPCON_PLEASE_ENTERHOST,TEXT("Error"),MB_OK);
					break;
				}
				lstrcpy(hostname,temp);
				//MessageBox(hDlg,temp,"hola",MB_OK);

				// save hostname in recent list
				{
					int i;
					for(i=0; i<MAX_RECENT_HOSTS_LIST_SIZE; i++)
					{
						if(!*GUI.RecentHostNames[i])
						{
							lstrcpy(GUI.RecentHostNames[i], hostname);
							break;
						}
						else if(!_tcsicmp(GUI.RecentHostNames[i], hostname))
							break;
					}
					if(i == MAX_RECENT_HOSTS_LIST_SIZE)
						lstrcpy(GUI.RecentHostNames[1+(rand()%(MAX_RECENT_HOSTS_LIST_SIZE-1))], hostname);
				}

				unsigned long len;
				len = lstrlen(temp);
				if(len > 0)
				{
					EndDialog(hDlg,1);
					return TRUE;
				}
				else
				{
					EndDialog(hDlg,0);
					return TRUE;
				}

				break;
				//}
			}
		case IDCANCEL:
			{
				EndDialog(hDlg, 0);
				return TRUE;
			}
		default:break;
		}
	}
	return FALSE;
}
#endif
void SetInfoDlgColor(unsigned char r, unsigned char g, unsigned char b)
{
	GUI.InfoColor=RGB(r,g,b);
}

#define SNES9XWPROGID TEXT("Snes9x.Win32")
#define SNES9XWPROGIDDESC TEXT("Snes9x ROM")
#define SNES9XWAPPDESC TEXT("Snes9x is a portable, freeware Super Nintendo Entertainment System (SNES) emulator.")
#define SNES9XWAPPNAME TEXT("Snes9x")
#define REGCREATEKEY(key,subkey) \
	if(regResult=RegCreateKeyEx(key, subkey,\
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE , NULL , &hKey, NULL ) != ERROR_SUCCESS){\
		return false;\
	}
#define REGSETVALUE(key,name,type,data,size) \
	if(regResult=RegSetValueEx(key, name, NULL, type, (BYTE *)data, size) != ERROR_SUCCESS){\
		RegCloseKey(hKey);\
		return false;\
	}

void S9xWinRemoveRegistryKeys() {
	TCHAR szRegKey[4096] = {};

    if(!valid_ext)
        LoadExts();

	_stprintf_s(szRegKey, 4095, TEXT("Software\\Classes\\%s"), SNES9XWPROGID);
	SHDeleteKey(HKEY_CURRENT_USER, szRegKey);
	_stprintf_s(szRegKey, 4095, TEXT("Software\\RegisteredApplications\\%s"), SNES9XWPROGID);
	SHDeleteKey(HKEY_CURRENT_USER, szRegKey);
	_stprintf_s(szRegKey, 4095, TEXT("Software\\Snes9x"), SNES9XWPROGID);
	SHDeleteKey(HKEY_CURRENT_USER, szRegKey);

	const TCHAR* szExeNames[] = { TEXT("snes9x.exe"), TEXT("snes9x-debug.exe"), TEXT("snes9x-x64.exe"), TEXT("snes9x-debug-x64.exe") };
	for (auto& szExeName : szExeNames)
	{
		_stprintf_s(szRegKey, 4095, TEXT("Software\\Classes\\Applications\\%s"), szExeName);
		SHDeleteKey(HKEY_CURRENT_USER, szRegKey);

		ExtList* curr = valid_ext;
		while (curr->next != NULL) {
			auto ext = curr->extension;
			_stprintf(szRegKey, TEXT("Software\\Classes\\.%s\\OpenWithList\\%s"), ext, szExeName);
			SHDeleteKey(HKEY_CURRENT_USER, szRegKey);
			curr = curr->next;
		}
	}
}

bool RegisterProgid() {
	LONG	regResult;
	TCHAR	szRegKey[PATH_MAX];
	TCHAR	szExePath[PATH_MAX];
    TCHAR   *szExeName;
	HKEY	hKey;

	szRegKey[PATH_MAX-1]=TEXT('\0');
	GetModuleFileName(NULL, szExePath, PATH_MAX);
    szExeName = PathFindFileName(szExePath);

    /* Register ProgID (for use in Default Programs)
    */
    _stprintf_s(szRegKey,PATH_MAX-1,TEXT("Software\\Classes\\%s"),SNES9XWPROGID);
    REGCREATEKEY(HKEY_CURRENT_USER, szRegKey)
    REGSETVALUE(hKey,NULL,REG_SZ,SNES9XWPROGIDDESC,(lstrlen(SNES9XWPROGIDDESC) + 1) * sizeof(TCHAR))
    RegCloseKey(hKey);

    _stprintf_s(szRegKey,PATH_MAX-1,TEXT("Software\\Classes\\%s\\DefaultIcon"),SNES9XWPROGID);
    REGCREATEKEY(HKEY_CURRENT_USER,szRegKey)
    _stprintf_s(szRegKey,PATH_MAX-1,TEXT("%s,0"), szExePath);
    REGSETVALUE(hKey,NULL,REG_SZ,szRegKey,(lstrlen(szRegKey) + 1) * sizeof(TCHAR))
    RegCloseKey(hKey);

    _stprintf_s(szRegKey,PATH_MAX-1,TEXT("Software\\Classes\\%s\\shell"),SNES9XWPROGID);
    REGCREATEKEY(HKEY_CURRENT_USER,szRegKey)
    REGSETVALUE(hKey,NULL,REG_SZ,TEXT("open"),5 * sizeof(TCHAR))
    RegCloseKey(hKey);

    _stprintf_s(szRegKey,PATH_MAX-1,TEXT("Software\\Classes\\%s\\shell\\open\\command"),SNES9XWPROGID);
    REGCREATEKEY(HKEY_CURRENT_USER,szRegKey)
    _stprintf_s(szRegKey,PATH_MAX-1,TEXT("\"%s\" \"%%L\""),szExePath);
    REGSETVALUE(hKey,NULL,REG_SZ,szRegKey,(lstrlen(szRegKey) + 1) * sizeof(TCHAR))
    RegCloseKey(hKey);

    /* Register Capabilities (for Default Programs)
    */
    REGCREATEKEY(HKEY_CURRENT_USER,TEXT("SOFTWARE\\Snes9x\\Capabilities"))
    REGSETVALUE(hKey,TEXT("ApplicationDescription"),REG_SZ,SNES9XWAPPDESC,(lstrlen(SNES9XWAPPDESC) + 1) * sizeof(TCHAR))
    REGSETVALUE(hKey,TEXT("ApplicationName"),REG_SZ,SNES9XWAPPNAME,(lstrlen(SNES9XWAPPNAME) + 1) * sizeof(TCHAR))
    RegCloseKey(hKey);

    REGCREATEKEY(HKEY_CURRENT_USER,TEXT("SOFTWARE\\RegisteredApplications"))
    REGSETVALUE(hKey,TEXT("Snes9x"),REG_SZ,TEXT("SOFTWARE\\Snes9x\\Capabilities"),(lstrlen(TEXT("SOFTWARE\\Snes9x\\Capabilities")) + 1) * sizeof(TCHAR))
    RegCloseKey(hKey);

    /* Register under Applications for use in OpenWithList
    */
    _stprintf_s(szRegKey,PATH_MAX-1,TEXT("Software\\Classes\\Applications\\%s"),szExeName);
	REGCREATEKEY(HKEY_CURRENT_USER,szRegKey)
	RegCloseKey(hKey);

    _stprintf_s(szRegKey,PATH_MAX-1,TEXT("Software\\Classes\\Applications\\%s\\shell\\open\\command"),szExeName);
	REGCREATEKEY(HKEY_CURRENT_USER,szRegKey)
	_stprintf_s(szRegKey,PATH_MAX-1,TEXT("\"%s\" \"%%L\""),szExePath);
	REGSETVALUE(hKey,NULL,REG_SZ,szRegKey,(lstrlen(szRegKey) + 1) * sizeof(TCHAR))
	RegCloseKey(hKey);

	return true;
}

bool RegisterExt(TCHAR *ext) {
	LONG	regResult;
	TCHAR	szRegKey[PATH_MAX];
    TCHAR	szExePath[PATH_MAX];
    TCHAR   *szExeName;
	HKEY	hKey;

    if(!ext || ext==TEXT('\0'))
        return false;

    GetModuleFileName(NULL, szExePath, PATH_MAX);
    szExeName = PathFindFileName(szExePath);

    /* Register .ext as S9x capability (for Default Programs)
    */
    REGCREATEKEY(HKEY_CURRENT_USER,TEXT("SOFTWARE\\Snes9x\\Capabilities\\FileAssociations"))
	_stprintf_s(szRegKey,PATH_MAX-1,TEXT(".%s"),ext);
	REGSETVALUE(hKey,szRegKey,REG_SZ,SNES9XWPROGID,(lstrlen(SNES9XWPROGID) + 1) * sizeof(TCHAR))
	RegCloseKey(hKey);

    /* Register in OpenWithList - will not be taken as default icon, necessary for Jump List
    */
	_stprintf(szRegKey,TEXT("Software\\Classes\\.%s\\OpenWithList\\%s"),ext,szExeName);
	REGCREATEKEY(HKEY_CURRENT_USER,szRegKey)
	RegCloseKey(hKey);

	return true;
}

void RegisterExts(void) {
	ExtList *curr=valid_ext;
	while(curr->next!=NULL) {
		RegisterExt(curr->extension);
		curr=curr->next;
	}
}

void ClearExts(void)
{
	ExtList* temp;
	ExtList* curr=valid_ext;
	while(curr!=NULL)
	{
		temp=curr->next;
		if(curr->extension)
			delete [] curr->extension;
		delete curr;
		curr=temp;
	}
	valid_ext=NULL;

}

void LoadExts(void)
{
	char buffer[MAX_PATH+2];
	if(valid_ext!=NULL)
	{
		ClearExts();
	}
	ExtList* curr;
	valid_ext=new ExtList;
	curr=valid_ext;
	memset(curr, 0, sizeof(ExtList));
	ifstream in;

#if (((defined(_MSC_VER) && _MSC_VER >= 1300)) || defined(__MINGW32__))
	in.open("Valid.Ext", ios::in);
#else
	in.open("Valid.Ext", ios::in|ios::nocreate);
#endif
	if (!in.is_open())
	{
		in.clear();
		MakeExtFile();
	#if (((defined(_MSC_VER) && _MSC_VER >= 1300)) || defined(__MINGW32__))
		in.open("Valid.Ext", ios::in);
	#else
		in.open("Valid.Ext", ios::in|ios::nocreate);
	#endif
		if(!in.is_open())
		{
			MessageBox(GUI.hWnd, TEXT("Fatal Error: The File \"Valid.Ext\" could not be found or created."), TEXT("Error"), MB_ICONERROR|MB_OK);
			PostQuitMessage(-1);
		}
	}

	do
	{
		buffer[0]='\0';
		in.getline(buffer,MAX_PATH+2);
		if((*buffer)!='\0')
		{
			curr->next=new ExtList;
			curr=curr->next;
			memset(curr, 0, sizeof(ExtList));
			if(_strnicmp(buffer+strlen(buffer)-1, "Y", 1)==0)
				curr->compressed=true;
			if(strlen(buffer)>1)
			{
				curr->extension=new TCHAR[strlen(buffer)];
				_tcsncpy(curr->extension, _tFromChar(buffer), strlen(buffer)-1);
				curr->extension[strlen(buffer)-1]='\0';
			}
			else curr->extension=NULL;
		}
	}
	while(!in.eof());
	in.close();
	curr=valid_ext;
	valid_ext=valid_ext->next;
	delete curr;

	if (GUI.AddToRegistry)
	{
		RegisterProgid();
		RegisterExts();
	}
}

void MakeExtFile(void)
{
	ofstream out;
	out.open("Valid.Ext");

	out<<"smcN"<<endl;
#ifdef UNZIP_SUPPORT
	out<<"zipY"<<endl;
	out<<"msu1Y"<<endl;
#endif
	out<<"gzY"<<endl;
	out<<"swcN"<<endl;
	out<<"figN"<<endl;
	out<<"sfcN"<<endl;
	out<<"bsN"<<endl;
	out<<"jmaY"<<endl;
	out << "stN" << endl;
	out.close();
	SetFileAttributes(TEXT("Valid.Ext"), FILE_ATTRIBUTE_ARCHIVE|FILE_ATTRIBUTE_READONLY);
};
#ifdef NETPLAY_SUPPORT
INT_PTR CALLBACK DlgNPOptions(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR defPort[5];
	WORD chkLength;
	if(Settings.Port==0)
	{
		_itot(1996,defPort,10);
	}
	else
	{
		_itot(Settings.Port,defPort,10);
	}

	switch (msg)
	{
	case WM_INITDIALOG:
		WinRefreshDisplay();
		SetWindowText(hDlg,NPOPT_TITLE);
		SetDlgItemText(hDlg,IDC_LABEL_PORTNUM,NPOPT_LABEL_PORTNUM);
		SetDlgItemText(hDlg,IDC_LABEL_PAUSEINTERVAL,NPOPT_LABEL_PAUSEINTERVAL);
		SetDlgItemText(hDlg,IDC_LABEL_PAUSEINTERVAL_TEXT,NPOPT_LABEL_PAUSEINTERVAL_TEXT);
		SetDlgItemText(hDlg,IDC_LABEL_MAXSKIP,NPOPT_LABEL_MAXSKIP);
		SetDlgItemText(hDlg,IDC_SYNCBYRESET,NPOPT_SYNCBYRESET);
		SetDlgItemText(hDlg,IDC_SENDROM,NPOPT_SENDROM);
		SetDlgItemText(hDlg,IDC_ACTASSERVER,NPOPT_ACTASSERVER);
		SetDlgItemText(hDlg,IDC_PORTNUMBLOCK,NPOPT_PORTNUMBLOCK);
		SetDlgItemText(hDlg,IDC_CLIENTSETTINGSBLOCK,NPOPT_CLIENTSETTINGSBLOCK);
		SetDlgItemText(hDlg,IDC_SERVERSETTINGSBLOCK,NPOPT_SERVERSETTINGSBLOCK);
		SetDlgItemText(hDlg,IDOK,BUTTON_OK);
		SetDlgItemText(hDlg,IDCANCEL,BUTTON_CANCEL);

		SendDlgItemMessage(hDlg, IDC_PORTNUMBERA, WM_SETTEXT, 0, (LPARAM)defPort);
		if(Settings.NetPlayServer)
		{
			SendDlgItemMessage(hDlg, IDC_ACTASSERVER, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		}
		if(NPServer.SendROMImageOnConnect)
		{
			SendDlgItemMessage(hDlg, IDC_SENDROM, BM_SETCHECK, BST_CHECKED,0);
		}

		if(NPServer.SyncByReset)
		{
			SendDlgItemMessage(hDlg, IDC_SYNCBYRESET, BM_SETCHECK, BST_CHECKED,0);
		}
		SendDlgItemMessage(hDlg, IDC_MAXSPIN, UDM_SETRANGE,0,MAKELPARAM((short)60,(short)0));
		SendDlgItemMessage(hDlg, IDC_MAXSPIN, UDM_SETPOS,0,MAKELONG(NetPlay.MaxFrameSkip,0));
		SendDlgItemMessage(hDlg, IDC_PAUSESPIN, UDM_SETRANGE,0,MAKELONG(30,0));
		SendDlgItemMessage(hDlg, IDC_PAUSESPIN, UDM_SETPOS,0,MAKELONG(NetPlay.MaxBehindFrameCount,0));
		return TRUE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint (hDlg, &ps);

			EndPaint (hDlg, &ps);
		}
		return true;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				NetPlay.MaxFrameSkip=(uint32)SendDlgItemMessage(hDlg, IDC_MAXSPIN, UDM_GETPOS,0,0);
				NetPlay.MaxBehindFrameCount=(uint32)SendDlgItemMessage(hDlg, IDC_PAUSESPIN, UDM_GETPOS,0,0);
				chkLength=(WORD)SendDlgItemMessage(hDlg,IDC_PORTNUMBERA,EM_LINELENGTH,0,0);
				*((LPWORD)defPort) = chkLength;
				SendDlgItemMessage(hDlg,IDC_PORTNUMBERA,EM_GETLINE,0,(LPARAM)defPort);
				if(_ttoi(defPort)<1024||_ttoi(defPort)>65535)
				{
					MessageBox(hDlg,TEXT("Port Number needs to be betweeb 1024 and 65535"),TEXT("Error"),MB_OK);
					break;
				}
				else
				{
					Settings.Port = _ttoi(defPort);
				}
				//MessageBox(hDlg,defPort,defPort,MB_OK);
				Settings.NetPlayServer = IsDlgButtonChecked(hDlg,IDC_ACTASSERVER);
				NPServer.SendROMImageOnConnect = IsDlgButtonChecked(hDlg,IDC_SENDROM);
				NPServer.SyncByReset = IsDlgButtonChecked(hDlg,IDC_SYNCBYRESET);

				EndDialog(hDlg,0);
				WinSaveConfigFile();
				return TRUE;
			}
		case IDCANCEL:
			{
				EndDialog(hDlg,0);
				return TRUE;
			}
		}
	}
	return FALSE;
}
#endif

void EnableDisableKeyFields (int index, HWND hDlg)
{
	bool enableUnTurboable;
	if(index < 5)
	{
		SetDlgItemText(hDlg,IDC_LABEL_RIGHT,INPUTCONFIG_LABEL_RIGHT);
		SetDlgItemText(hDlg,IDC_LABEL_UPLEFT,INPUTCONFIG_LABEL_UPLEFT);
		SetDlgItemText(hDlg,IDC_LABEL_UPRIGHT,INPUTCONFIG_LABEL_UPRIGHT);
		SetDlgItemText(hDlg,IDC_LABEL_DOWNRIGHT,INPUTCONFIG_LABEL_DOWNRIGHT);
		SetDlgItemText(hDlg,IDC_LABEL_UP,INPUTCONFIG_LABEL_UP);
		SetDlgItemText(hDlg,IDC_LABEL_LEFT,INPUTCONFIG_LABEL_LEFT);
		SetDlgItemText(hDlg,IDC_LABEL_DOWN,INPUTCONFIG_LABEL_DOWN);
		SetDlgItemText(hDlg,IDC_LABEL_DOWNLEFT,INPUTCONFIG_LABEL_DOWNLEFT);
		enableUnTurboable = true;
	}
	else
	{
		SetDlgItemText(hDlg,IDC_LABEL_UP,INPUTCONFIG_LABEL_MAKE_TURBO);
		SetDlgItemText(hDlg,IDC_LABEL_LEFT,INPUTCONFIG_LABEL_MAKE_HELD);
		SetDlgItemText(hDlg,IDC_LABEL_DOWN,INPUTCONFIG_LABEL_MAKE_TURBO_HELD);
		SetDlgItemText(hDlg,IDC_LABEL_RIGHT,INPUTCONFIG_LABEL_CLEAR_TOGGLES_AND_TURBO);
		SetDlgItemText(hDlg,IDC_LABEL_UPLEFT,INPUTCONFIG_LABEL_UNUSED);
		SetDlgItemText(hDlg,IDC_LABEL_UPRIGHT,INPUTCONFIG_LABEL_UNUSED);
		SetDlgItemText(hDlg,IDC_LABEL_DOWNLEFT,INPUTCONFIG_LABEL_UNUSED);
		SetDlgItemText(hDlg,IDC_LABEL_DOWNRIGHT,INPUTCONFIG_LABEL_UNUSED);
		SetDlgItemText(hDlg,IDC_UPLEFT,INPUTCONFIG_LABEL_UNUSED);
		SetDlgItemText(hDlg,IDC_UPRIGHT,INPUTCONFIG_LABEL_UNUSED);
		SetDlgItemText(hDlg,IDC_DWNLEFT,INPUTCONFIG_LABEL_UNUSED);
		SetDlgItemText(hDlg,IDC_DWNRIGHT,INPUTCONFIG_LABEL_UNUSED);
		enableUnTurboable = false;
	}

	EnableWindow(GetDlgItem(hDlg,IDC_UPLEFT), enableUnTurboable);
	EnableWindow(GetDlgItem(hDlg,IDC_UPRIGHT), enableUnTurboable);
	EnableWindow(GetDlgItem(hDlg,IDC_DWNRIGHT), enableUnTurboable);
	EnableWindow(GetDlgItem(hDlg,IDC_DWNLEFT), enableUnTurboable);
}

void UpdateModeComboBox(HWND hComboBox)
{
	TCHAR modeTxt[80];
	bool foundCurMode=false;
	dMode curMode;

	dm.clear();
	ComboBox_ResetContent(hComboBox);
	WinEnumDisplayModes(&dm);

	for(int i=0;i<dm.size();i++) {
		_stprintf( modeTxt, TEXT("%dx%d %dbit %dhz"), dm[i].width , dm[i].height, dm[i].depth ,dm[i].rate);
		ComboBox_AddString(hComboBox,modeTxt);
		if(GUI.FullscreenMode.width==dm[i].width&&GUI.FullscreenMode.height==dm[i].height&&GUI.FullscreenMode.rate==dm[i].rate&&GUI.FullscreenMode.depth==dm[i].depth) {
			ComboBox_SetCurSel(hComboBox,i);
			foundCurMode=true;
		}
	}
	if(!foundCurMode) {
		dm.push_back(GUI.FullscreenMode);
		_stprintf( modeTxt, TEXT("Unsupported mode selected"));
		ComboBox_AddString(hComboBox,modeTxt);
		ComboBox_SetCurSel(hComboBox,ComboBox_GetCount(hComboBox)-1);
	}

}

static WNDPROC lpfnOldWndProc;

LRESULT CALLBACK GroupBoxCheckBoxTitle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT checkBoxRect;
	if(message==WM_PAINT) {
		HWND checkHwnd=GetDlgItem(GetParent(hWnd),IDC_SHADER_ENABLED);
		GetClientRect(checkHwnd,&checkBoxRect);
		MapWindowPoints(checkHwnd,hWnd,(LPPOINT)&checkBoxRect,2);
		ValidateRect(hWnd,&checkBoxRect);
	}
	return CallWindowProc(lpfnOldWndProc, hWnd, message, wParam, lParam);
}

void SelectOutputMethodInVideoDropdown(HWND hDlg, OutputMethod method)
{
	// select item with corresponding itemdata
	int item_count = SendDlgItemMessage(hDlg, IDC_OUTPUTMETHOD, CB_GETCOUNT, 0, 0);
	for (int index = 0; index < item_count; index++) {
		int item_data = SendDlgItemMessage(hDlg, IDC_OUTPUTMETHOD, CB_GETITEMDATA, index, 0);
		if (item_data == method)
			SendDlgItemMessage(hDlg, IDC_OUTPUTMETHOD, CB_SETCURSEL, index, 0);
	}
}

INT_PTR CALLBACK DlgFunky(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int index;
	TCHAR s[80];
	char temp[80];
	TCHAR openFileName[MAX_PATH];
	OPENFILENAME ofn;

	// temporary GUI state for restoring after previewing while selecting options
	static int prevScale, prevScaleHiRes, prevPPL;
	static bool prevStretch, prevAspectRatio, prevHeightExtend, prevAutoDisplayMessages, prevBilinearFilter, prevShaderEnabled, prevBlendHires, prevIntegerScaling, prevNTSCScanlines;
	static int prevAspectWidth;
	static OutputMethod prevOutputMethod;
	static TCHAR prevD3DShaderFile[MAX_PATH],prevOGLShaderFile[MAX_PATH];

	switch(msg)
	{

	case WM_INITDIALOG:
    {
        WinRefreshDisplay();

        CreateToolTip(IDC_EMUFULLSCREEN, hDlg, TEXT("Emulate fullscreen by creating a window that spans the entire screen when going fullscreen"));
        CreateToolTip(IDC_AUTOFRAME, hDlg, TEXT("Try to achieve 50/60 fps by limiting the speed and skipping at most 'max skipped frames'"));
        CreateToolTip(IDC_MAXSKIP, hDlg, TEXT("Try to achieve 50/60 fps by limiting the speed and skipping at most 'max skipped frames'"));
        CreateToolTip(IDC_FIXEDSKIP, hDlg, TEXT("Always skip a fixed number of frames - no speed limit"));
        CreateToolTip(IDC_SKIPCOUNT, hDlg, TEXT("Always skip a fixed number of frames - no speed limit"));
        CreateToolTip(IDC_HIRES, hDlg, TEXT("Support the hi-res mode that a few games use, otherwise render them in low-res"));
        CreateToolTip(IDC_HEIGHT_EXTEND, hDlg, TEXT("Display an extra 15 pixels at the bottom, which few games use. Also increases AVI output size from 256x224 to 256x240"));
        CreateToolTip(IDC_MESSAGES_IN_IMAGE, hDlg, TEXT("Draw text inside the SNES image (will get into AVIs, screenshots, and filters)"));
		CreateToolTip(IDC_MESSAGES_SCALE, hDlg, TEXT("Try to scale messages with EPX instead of Simple, only works for 2x and 3x and when displaying after filters"));

        prevOutputMethod = GUI.outputMethod;
        prevScale = GUI.Scale;
        prevScaleHiRes = GUI.ScaleHiRes;
        prevPPL = GFX.RealPPL;
        prevStretch = GUI.Stretch;
        prevBilinearFilter = Settings.BilinearFilter;
        prevAspectRatio = GUI.AspectRatio;
        prevAspectWidth = GUI.AspectWidth;
		prevIntegerScaling = GUI.IntegerScaling;
		prevHeightExtend = Settings.ShowOverscan;
        prevAutoDisplayMessages = Settings.AutoDisplayMessages != 0;
        prevShaderEnabled = GUI.shaderEnabled;
        prevBlendHires = GUI.BlendHiRes;
		prevNTSCScanlines = GUI.NTSCScanlines;

        lstrcpy(prevD3DShaderFile, GUI.D3DshaderFileName);
        lstrcpy(prevOGLShaderFile, GUI.OGLshaderFileName);

        _stprintf(s, TEXT("Current: %dx%d %dbit %dHz"), GUI.FullscreenMode.width, GUI.FullscreenMode.height, GUI.FullscreenMode.depth, GUI.FullscreenMode.rate);
        SendDlgItemMessage(hDlg, IDC_CURRMODE, WM_SETTEXT, 0, (LPARAM)s);

        if (GUI.DoubleBuffered)
            SendDlgItemMessage(hDlg, IDC_DBLBUFFER, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (GUI.Vsync)
            SendDlgItemMessage(hDlg, IDC_VSYNC, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		if (GUI.ReduceInputLag)
			SendDlgItemMessage(hDlg, IDC_REDUCEINPUTLAG, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		if (GUI.NTSCScanlines)
			SendDlgItemMessage(hDlg, IDC_NTSCSCANLINES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        SendDlgItemMessage(hDlg, IDC_FRAMERATESKIPSLIDER, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, 9));
        if (Settings.SkipFrames != AUTO_FRAMERATE)
            SendDlgItemMessage(hDlg, IDC_FRAMERATESKIPSLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)Settings.SkipFrames);
        EnableWindow(GetDlgItem(hDlg, IDC_TRANS), TRUE);
        if (Settings.Transparency)
            SendDlgItemMessage(hDlg, IDC_TRANS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

        if (GUI.BlendHiRes)
            SendDlgItemMessage(hDlg, IDC_HIRESBLEND, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (Settings.ShowOverscan)
            SendDlgItemMessage(hDlg, IDC_HEIGHT_EXTEND, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (Settings.AutoDisplayMessages)
            SendDlgItemMessage(hDlg, IDC_MESSAGES_IN_IMAGE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (Settings.SkipFrames == AUTO_FRAMERATE)
            SendDlgItemMessage(hDlg, IDC_AUTOFRAME, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (GUI.Stretch)
        {
            SendDlgItemMessage(hDlg, IDC_STRETCH, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        }

		if (GUI.AspectRatio)
			SendDlgItemMessage(hDlg, IDC_ASPECT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

		EnableWindow(GetDlgItem(hDlg, IDC_INTEGERSCALING), GUI.AspectRatio);

		if (GUI.IntegerScaling)
			SendDlgItemMessage(hDlg, IDC_INTEGERSCALING, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

		SendDlgItemMessage(hDlg, IDC_ASPECTDROP, CB_ADDSTRING, 0, (LPARAM)TEXT("8:7"));
        SendDlgItemMessage(hDlg, IDC_ASPECTDROP, CB_ADDSTRING, 0, (LPARAM)TEXT("4:3"));
        switch (GUI.AspectWidth) {
        case ASPECT_WIDTH_4_3:
            SendDlgItemMessage(hDlg, IDC_ASPECTDROP, CB_SETCURSEL, (WPARAM)0, 0);
            break;
        case ASPECT_WIDTH_8_7:
            SendDlgItemMessage(hDlg, IDC_ASPECTDROP, CB_SETCURSEL, (WPARAM)1, 0);
            break;
        default:
            SendDlgItemMessage(hDlg, IDC_ASPECTDROP, CB_ADDSTRING, 0, (LPARAM)TEXT("Custom"));
            SendDlgItemMessage(hDlg, IDC_ASPECTDROP, CB_SETCURSEL, (WPARAM)2, 0);
            break;
        }

        if (GUI.FullScreen || GUI.EmulatedFullscreen)
            SendDlgItemMessage(hDlg, IDC_FULLSCREEN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (GUI.EmulateFullscreen)
            SendDlgItemMessage(hDlg, IDC_EMUFULLSCREEN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (GUI.FullscreenOnOpen)
            SendDlgItemMessage(hDlg, IDC_FULLSCREEN_ON_OPEN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (Settings.BilinearFilter)
            SendDlgItemMessage(hDlg, IDC_BILINEAR, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        if (Settings.DisplayFrameRate)
            SendDlgItemMessage(hDlg, IDC_SHOWFPS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

        if (Settings.SkipFrames == AUTO_FRAMERATE) {
            SendDlgItemMessage(hDlg, IDC_AUTOFRAME, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
            EnableWindow(GetDlgItem(hDlg, IDC_SKIPCOUNT), FALSE);
        }
        else {
            SendDlgItemMessage(hDlg, IDC_FIXEDSKIP, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
            EnableWindow(GetDlgItem(hDlg, IDC_MAXSKIP), FALSE);
        }
        SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP_DISP, UDM_SETRANGE, 0, MAKELPARAM((short)59, (short)0));
        SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP_DISP, UDM_SETPOS, 0, Settings.AutoMaxSkipFrames);
        SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP_DISP_FIXED, UDM_SETRANGE, 0, MAKELPARAM((short)59, (short)0));
        SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP_DISP_FIXED, UDM_SETPOS, 0, Settings.SkipFrames == AUTO_FRAMERATE ? 0 : Settings.SkipFrames);
		SendDlgItemMessage(hDlg, IDC_SPIN_OSD_SIZE, UDM_SETRANGE, 0, MAKELPARAM(160, 24));
		SendDlgItemMessage(hDlg, IDC_SPIN_OSD_SIZE, UDM_SETPOS, 0, GUI.OSDSize);

        if (GUI.shaderEnabled) {
            SendDlgItemMessage(hDlg, IDC_SHADER_ENABLED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
            EnableWindow(GetDlgItem(hDlg, IDC_SHADER_HLSL_FILE), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_SHADER_HLSL_BROWSE), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_SHADER_GLSL_FILE), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_SHADER_GLSL_BROWSE), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_SHADER_GLSL_PARAMETERS), TRUE);
        }
        SetDlgItemText(hDlg, IDC_SHADER_HLSL_FILE, GUI.D3DshaderFileName);
        SetDlgItemText(hDlg, IDC_SHADER_GLSL_FILE, GUI.OGLshaderFileName);

        lpfnOldWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_SHADER_GROUP), GWLP_WNDPROC, (LONG_PTR)GroupBoxCheckBoxTitle);

        EnableWindow(GetDlgItem(hDlg, IDC_ASPECT), GUI.Stretch);

        // add output method to droplist with itemdata set to their enum value
        auto InsertOutputMethod = [&](LPARAM text, int value) {
            auto inserted_index = SendDlgItemMessage(hDlg, IDC_OUTPUTMETHOD, CB_ADDSTRING, 0, text);
            SendDlgItemMessage(hDlg, IDC_OUTPUTMETHOD, CB_SETITEMDATA, inserted_index, value);
        };
#if DIRECTDRAW_SUPPORT
        InsertOutputMethod((LPARAM)TEXT("DirectDraw"), DIRECTDRAW);
#endif
        InsertOutputMethod((LPARAM)TEXT("Direct3D"), DIRECT3D);
        InsertOutputMethod((LPARAM)TEXT("OpenGL"), OPENGL);
        InsertOutputMethod((LPARAM)TEXT("Vulkan"), VULKAN);

        SelectOutputMethodInVideoDropdown(hDlg, GUI.outputMethod);

        // add all the GUI.Scale filters to the combo box
        for (int filter = 0; filter < (int)NUM_FILTERS; filter++)
        {
            strcpy(temp, GetFilterName((RenderFilter)filter));
            SendDlgItemMessageA(hDlg, IDC_FILTERBOX, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)temp);
        }
        for (int filter = 0, hiResPos = 0; filter < (int)NUM_FILTERS; filter++)
        {
            if (GetFilterHiResSupport((RenderFilter)filter))
            {
                strcpy(temp, GetFilterName((RenderFilter)filter));
                SendDlgItemMessageA(hDlg, IDC_FILTERBOX2, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)temp);
                if (GUI.ScaleHiRes == filter)
                    SendDlgItemMessage(hDlg, IDC_FILTERBOX2, CB_SETCURSEL, (WPARAM)hiResPos, 0);
                hiResPos++;
            }
        }

        SendDlgItemMessage(hDlg, IDC_FILTERBOX, CB_SETCURSEL, (WPARAM)GUI.Scale, 0);

        UpdateModeComboBox(GetDlgItem(hDlg, IDC_RESOLUTION));

        // have to start focus on something like this or Escape won't exit the dialog
        SetFocus(hDlg);

        break;
    }
	case WM_CLOSE:
	case WM_DESTROY:
		break;
	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDC_STRETCH:
			// for some reason this screws up the fullscreen mode clipper if it happens before the refresh
			if(IsDlgButtonChecked(hDlg, IDC_STRETCH)==BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_ASPECT), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_ASPECT), FALSE);
			}

			GUI.Stretch = (bool)(IsDlgButtonChecked(hDlg,IDC_STRETCH)==BST_CHECKED);
			// refresh screen, so the user can see the new mode
			// (assuming the dialog box isn't completely covering the game window)
			WinDisplayApplyChanges();

			WinRefreshDisplay();
			break;

		case IDC_BILINEAR:

			Settings.BilinearFilter = (bool)(IsDlgButtonChecked(hDlg,IDC_BILINEAR)==BST_CHECKED);

			//// refresh screen, so the user can see the new stretch mode
			WinDisplayApplyChanges();

			WinRefreshDisplay();
			break;

		case IDC_MESSAGES_IN_IMAGE:
		case IDC_MESSAGES_SCALE:
			Settings.AutoDisplayMessages = (bool)(IsDlgButtonChecked(hDlg,IDC_MESSAGES_IN_IMAGE)==BST_CHECKED);
			if(Settings.AutoDisplayMessages)
			{
				if(GFX.InfoString.empty()) {
					GFX.InfoString = "Test message!";
					GFX.InfoStringTimeout = 1;
				}
				S9xDisplayMessages(GFX.Screen, GFX.RealPPL, IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight, 1);
				EnableWindow(GetDlgItem(hDlg, IDC_MESSAGES_SCALE), FALSE);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_MESSAGES_SCALE), TRUE);
			}
			// refresh screen, so the user can see the new mode
			WinRefreshDisplay();
			break;

		case IDC_INTEGERSCALING:
			GUI.IntegerScaling = (bool)(IsDlgButtonChecked(hDlg, IDC_INTEGERSCALING) == BST_CHECKED);
			WinDisplayApplyChanges();
			WinRefreshDisplay();
			break;

		case IDC_ASPECT:
			GUI.AspectRatio = (bool)(IsDlgButtonChecked(hDlg,IDC_ASPECT)==BST_CHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_INTEGERSCALING), GUI.AspectRatio);
			// refresh screen, so the user can see the new mode
			WinDisplayApplyChanges();

			WinRefreshDisplay();
			break;
		case IDC_ASPECTDROP:
			if(HIWORD(wParam)==CBN_SELCHANGE) {
				int newsel = SendDlgItemMessage(hDlg,IDC_ASPECTDROP,CB_GETCURSEL,0,0);
				switch(newsel) {
					case 0:
						GUI.AspectWidth = ASPECT_WIDTH_4_3;
						break;
					case 1:
						GUI.AspectWidth = ASPECT_WIDTH_8_7;
						break;
					default:
						GUI.AspectWidth = prevAspectWidth;
						break;
				}
				WinDisplayApplyChanges();
				WinRefreshDisplay();
			}
			break;

		case IDC_HEIGHT_EXTEND:
			Settings.ShowOverscan = (bool)(IsDlgButtonChecked(hDlg,IDC_HEIGHT_EXTEND)==BST_CHECKED);
			// refresh screen, so the user can see the new mode
			WinDisplayApplyChanges();

			WinRefreshDisplay();
			break;

		case IDC_NTSCSCANLINES:
			GUI.NTSCScanlines = (bool)(IsDlgButtonChecked(hDlg, IDC_NTSCSCANLINES) == BST_CHECKED);
			WinDisplayApplyChanges();
			WinRefreshDisplay();
			break;

		case IDC_HIRESBLEND:
			GUI.BlendHiRes = (bool)(IsDlgButtonChecked(hDlg,IDC_HIRESBLEND)==BST_CHECKED);
			WinRefreshDisplay();
			break;

		case IDC_AUTOFRAME:
			if(BN_CLICKED==HIWORD(wParam)||BN_DBLCLK==HIWORD(wParam))
			{
				EnableWindow(GetDlgItem(hDlg, IDC_MAXSKIP), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_SKIPCOUNT), FALSE);
				return true;

			}
			break;
		case IDC_FIXEDSKIP:
			if(BN_CLICKED==HIWORD(wParam)||BN_DBLCLK==HIWORD(wParam))
			{
				EnableWindow(GetDlgItem(hDlg, IDC_MAXSKIP), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_SKIPCOUNT), TRUE);
				return true;

			}
			break;
		case IDC_OUTPUTMETHOD:
			if(HIWORD(wParam)==CBN_SELCHANGE) {
                int index = SendDlgItemMessage(hDlg, IDC_OUTPUTMETHOD, CB_GETCURSEL, 0, 0);
				OutputMethod newOut = (OutputMethod)SendDlgItemMessage(hDlg, IDC_OUTPUTMETHOD, CB_GETITEMDATA, index, 0);
				if(GUI.outputMethod==newOut)
					break;
				if(GUI.FullScreen)
					ToggleFullScreen();
				GUI.outputMethod=newOut;

				WinDisplayReset();
				UpdateModeComboBox(GetDlgItem(hDlg,IDC_RESOLUTION));
				WinRefreshDisplay();
				UpdateWindow(GUI.hWnd);
			}
			break;
		case IDC_SHADER_ENABLED:
			GUI.shaderEnabled = (bool)(IsDlgButtonChecked(hDlg,IDC_SHADER_ENABLED)==BST_CHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_SHADER_GROUP),GUI.shaderEnabled);
			EnableWindow(GetDlgItem(hDlg, IDC_SHADER_HLSL_FILE),GUI.shaderEnabled);
			EnableWindow(GetDlgItem(hDlg, IDC_SHADER_HLSL_BROWSE),GUI.shaderEnabled);
			EnableWindow(GetDlgItem(hDlg, IDC_SHADER_GLSL_FILE),GUI.shaderEnabled);
			EnableWindow(GetDlgItem(hDlg, IDC_SHADER_GLSL_BROWSE),GUI.shaderEnabled);
			EnableWindow(GetDlgItem(hDlg, IDC_SHADER_GLSL_PARAMETERS), GUI.shaderEnabled);

			GetDlgItemText(hDlg,IDC_SHADER_HLSL_FILE,GUI.D3DshaderFileName,MAX_PATH);
			GetDlgItemText(hDlg,IDC_SHADER_GLSL_FILE,GUI.OGLshaderFileName,MAX_PATH);
			WinDisplayApplyChanges();
			WinRefreshDisplay();
			break;
		case IDC_SHADER_HLSL_BROWSE:
			GetDlgItemText(hDlg,IDC_SHADER_HLSL_FILE,openFileName,MAX_PATH);
			memset((LPVOID)&ofn, 0, sizeof(OPENFILENAME));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = TEXT("Shader Files\0*.shader;*.cg;*.cgp\0All Files\0*.*\0\0");
			ofn.lpstrFile = openFileName;
			ofn.lpstrTitle = TEXT("Select Shader");
			ofn.lpstrDefExt = TEXT("shader");
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			if(GetOpenFileName(&ofn)) {
				SetDlgItemText(hDlg,IDC_SHADER_HLSL_FILE,openFileName);
				lstrcpy(GUI.D3DshaderFileName,openFileName);
				WinDisplayApplyChanges();
				WinRefreshDisplay();
			}
			break;
		case IDC_SHADER_GLSL_BROWSE:
			GetDlgItemText(hDlg,IDC_SHADER_GLSL_FILE,openFileName,MAX_PATH);
			memset((LPVOID)&ofn, 0, sizeof(OPENFILENAME));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = TEXT("Shader Files\0*.shader;*.cg;*.cgp;*.glsl;*.glslp;*.slang;*.slangp\0All Files\0*.*\0\0");
			ofn.lpstrFile = openFileName;
			ofn.lpstrTitle = TEXT("Select Shader");
			ofn.lpstrDefExt = TEXT("shader");
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			if(GetOpenFileName(&ofn)) {
				SetDlgItemText(hDlg,IDC_SHADER_GLSL_FILE,openFileName);
				lstrcpy(GUI.OGLshaderFileName,openFileName);
				WinDisplayApplyChanges();
				WinRefreshDisplay();
			}
			break;
        case IDC_SHADER_GLSL_PARAMETERS:
        {
            GetDlgItemText(hDlg, IDC_SHADER_GLSL_FILE, GUI.OGLshaderFileName, MAX_PATH);
			int len = lstrlen(GUI.OGLshaderFileName);
            if((len < 6 || _tcsncicmp(&GUI.OGLshaderFileName[len - 6], TEXT(".glslp"), 6)) &&
			   (len < 7 || _tcsncicmp(&GUI.OGLshaderFileName[len - 7], TEXT(".slangp"), 7))) {
                MessageBox(GUI.hWnd, TEXT("Parameters are only supported for .glslp and .slangp shaders"), TEXT("No Parameters"), MB_OK | MB_ICONINFORMATION);
                break;
            }
			ShowWindow(hDlg, SW_HIDE);
			WinDisplayApplyChanges();
			WinRefreshDisplay();
			auto shader_parameters = WinGetShaderParameters();
			if (shader_parameters) {
				auto save_function = WinGetShaderSaveFunction();
				CShaderParamDlg dlg(*shader_parameters, save_function);
				if (dlg.show()) {
					SetDlgItemText(hDlg, IDC_SHADER_GLSL_FILE, GUI.OGLshaderFileName);
					WinDisplayApplyChanges();
					WinRefreshDisplay();
				}
			}
			ShowWindow(hDlg, SW_SHOW);
            break;
        }
		case IDC_FILTERBOX:
			if(HIWORD(wParam)==CBN_SELCHANGE) {
				int scale = (int)SendDlgItemMessage(hDlg,IDC_FILTERBOX,CB_GETCURSEL,0,0);
				if(scale == GUI.Scale)
					break;

				const int oldScaleScale = max(GetFilterScale(GUI.Scale), GetFilterScale(GUI.ScaleHiRes));

				GUI.Scale = (RenderFilter)scale;

				const int newScaleScale = max(GetFilterScale(GUI.Scale), GetFilterScale(GUI.ScaleHiRes));

				if(oldScaleScale != newScaleScale)
					RestoreSNESDisplay();

				// refresh screen, so the user can see the new filter
				// (assuming the dialog box isn't completely covering the game window)
				WinRefreshDisplay();
			}
			break;
		case IDC_FILTERBOX2: // hi-res
			{
				char text [256];
				text[0] = '\0';
				SendMessageA(GetDlgItem(hDlg, IDC_FILTERBOX2), WM_GETTEXT, 256,(LPARAM)text);

				int scale = GUI.Scale;
				for(int i=0; i<NUM_FILTERS; i++)
					if(!stricmp(GetFilterName((RenderFilter)i), text))
						scale = i;

				if(scale == GUI.ScaleHiRes)
					break;

				const int oldScaleScale = max(GetFilterScale(GUI.Scale), GetFilterScale(GUI.ScaleHiRes));

				GUI.ScaleHiRes = (RenderFilter)scale;

				const int newScaleScale = max(GetFilterScale(GUI.Scale), GetFilterScale(GUI.ScaleHiRes));

				if(oldScaleScale != newScaleScale)
					RestoreSNESDisplay();

				// refresh screen, so the user can see the new filter
				// (assuming the dialog box isn't completely covering the game window)
				WinRefreshDisplay();
			}
			break;

		case IDOK:
			bool fullscreenWanted;
 			Settings.Transparency = IsDlgButtonChecked(hDlg, IDC_TRANS);
			Settings.BilinearFilter = (bool)(IsDlgButtonChecked(hDlg,IDC_BILINEAR)==BST_CHECKED);
			Settings.ShowOverscan = IsDlgButtonChecked(hDlg, IDC_HEIGHT_EXTEND)!=0;
			GUI.DoubleBuffered = (bool)(IsDlgButtonChecked(hDlg, IDC_DBLBUFFER)==BST_CHECKED);
			GUI.ReduceInputLag = (bool)(IsDlgButtonChecked(hDlg, IDC_REDUCEINPUTLAG) == BST_CHECKED);
			GUI.Vsync = (bool)(IsDlgButtonChecked(hDlg, IDC_VSYNC

				)==BST_CHECKED);
			{
				int newOSDSize = SendDlgItemMessage(hDlg, IDC_SPIN_OSD_SIZE, UDM_GETPOS, 0, 0);
				bool need_reset = ((bool)Settings.AutoDisplayMessages != prevAutoDisplayMessages || newOSDSize != GUI.OSDSize);
				GUI.OSDSize = newOSDSize;
				if (need_reset)
				{
					WinDisplayReset();
					WinRefreshDisplay();
					UpdateWindow(GUI.hWnd);
				}
			}
			if(IsDlgButtonChecked(hDlg, IDC_AUTOFRAME))
			{
				Settings.SkipFrames=AUTO_FRAMERATE;
				Settings.AutoMaxSkipFrames=SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP_DISP, UDM_GETPOS, 0,0);
			}
			else
			{
				Settings.SkipFrames=SendDlgItemMessage(hDlg, IDC_SPIN_MAX_SKIP_DISP_FIXED, UDM_GETPOS, 0,0);
			}


			GUI.Stretch = (bool)(IsDlgButtonChecked(hDlg, IDC_STRETCH)==BST_CHECKED);
			GUI.AspectRatio = (bool)(IsDlgButtonChecked(hDlg, IDC_ASPECT)==BST_CHECKED);
			GUI.IntegerScaling = (bool)(IsDlgButtonChecked(hDlg, IDC_INTEGERSCALING) == BST_CHECKED);
			fullscreenWanted = (bool)(IsDlgButtonChecked(hDlg, IDC_FULLSCREEN)==BST_CHECKED);
			GUI.EmulateFullscreen = (bool)(IsDlgButtonChecked(hDlg, IDC_EMUFULLSCREEN)==BST_CHECKED);
			GUI.FullscreenOnOpen = (bool)(IsDlgButtonChecked(hDlg, IDC_FULLSCREEN_ON_OPEN) == BST_CHECKED);
			Settings.DisplayFrameRate = IsDlgButtonChecked(hDlg, IDC_SHOWFPS);
			GUI.BlendHiRes = (bool)(IsDlgButtonChecked(hDlg, IDC_HIRESBLEND)==BST_CHECKED);

			index = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_RESOLUTION));

			GUI.FullscreenMode = dm[index];

			GetDlgItemText(hDlg,IDC_SHADER_HLSL_FILE,GUI.D3DshaderFileName,MAX_PATH);


			// we might've changed the region that the game draws over
			// (by turning on "maintain aspect ratio", or turning on "extend height" when "maintain aspect ratio" is already on),
			// so we must invalidate the window to redraw black
			// behind the possibly-newly-revealed areas of the window
			RedrawWindow((HWND)GetParent(hDlg),NULL,NULL, RDW_INVALIDATE | RDW_INTERNALPAINT | RDW_ERASENOW);

			WinSaveConfigFile();

			if(GUI.FullScreen && (GUI.FullscreenMode.width < 512 || GUI.FullscreenMode.height < 478))
				GUI.Scale = FILTER_NONE;

			if(GUI.FullScreen && (GUI.FullscreenMode.width < 512 || GUI.FullscreenMode.height < 478))
				GUI.ScaleHiRes = FILTER_SIMPLE1X;

			EndDialog(hDlg,0);
			WinDisplayApplyChanges();

			// if we are emulating fullscreen and now want to change to regular fullscreen
			// we want to switch back to the last window size
			if(fullscreenWanted && GUI.EmulatedFullscreen && !GUI.EmulateFullscreen) {
				GUI.EmulateFullscreen = true;
				ToggleFullScreen();
				GUI.EmulateFullscreen = false;
			}
			if(fullscreenWanted != GUI.EmulatedFullscreen) {
				ToggleFullScreen();
			}

			return false;

		case IDCANCEL:
			SelectOutputMethodInVideoDropdown(hDlg, prevOutputMethod);
			SendMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_OUTPUTMETHOD,CBN_SELCHANGE),0);

			{
				//UpdateScale(GUI.Scale, prevScale);
				GUI.Scale = (RenderFilter)prevScale;
				GUI.ScaleHiRes = (RenderFilter)prevScaleHiRes;
				GUI.Stretch = prevStretch;
				Settings.AutoDisplayMessages = prevAutoDisplayMessages;
				Settings.BilinearFilter = prevBilinearFilter;
				GUI.AspectRatio = prevAspectRatio;
				GUI.AspectWidth = prevAspectWidth;
				GUI.IntegerScaling = prevIntegerScaling;
				Settings.ShowOverscan = prevHeightExtend;
				GUI.shaderEnabled = prevShaderEnabled;
				GUI.BlendHiRes = prevBlendHires;
				GUI.NTSCScanlines = prevNTSCScanlines;
				lstrcpy(GUI.D3DshaderFileName,prevD3DShaderFile);
				lstrcpy(GUI.OGLshaderFileName,prevOGLShaderFile);
			}

			EndDialog(hDlg,0);

			return false;
		}


	}

	return false;
}

static void set_buttoninfo(int index, HWND hDlg)
{
	SendDlgItemMessage(hDlg,IDC_UP,WM_USER+44,Joypad[index].Up,0);
	SendDlgItemMessage(hDlg,IDC_LEFT,WM_USER+44,Joypad[index].Left,0);
	SendDlgItemMessage(hDlg,IDC_DOWN,WM_USER+44,Joypad[index].Down,0);
	SendDlgItemMessage(hDlg,IDC_RIGHT,WM_USER+44,Joypad[index].Right,0);
	SendDlgItemMessage(hDlg,IDC_A,WM_USER+44,Joypad[index].A,0);
	SendDlgItemMessage(hDlg,IDC_B,WM_USER+44,Joypad[index].B,0);
	SendDlgItemMessage(hDlg,IDC_X,WM_USER+44,Joypad[index].X,0);
	SendDlgItemMessage(hDlg,IDC_Y,WM_USER+44,Joypad[index].Y,0);
	SendDlgItemMessage(hDlg,IDC_L,WM_USER+44,Joypad[index].L,0);
	SendDlgItemMessage(hDlg,IDC_R,WM_USER+44,Joypad[index].R,0);
	SendDlgItemMessage(hDlg,IDC_START,WM_USER+44,Joypad[index].Start,0);
	SendDlgItemMessage(hDlg,IDC_SELECT,WM_USER+44,Joypad[index].Select,0);
	if(index < 5)
	{
		SendDlgItemMessage(hDlg,IDC_UPLEFT,WM_USER+44,Joypad[index].Left_Up,0);
		SendDlgItemMessage(hDlg,IDC_UPRIGHT,WM_USER+44,Joypad[index].Right_Up,0);
		SendDlgItemMessage(hDlg,IDC_DWNLEFT,WM_USER+44,Joypad[index].Left_Down,0);
		SendDlgItemMessage(hDlg,IDC_DWNRIGHT,WM_USER+44,Joypad[index].Right_Down,0);
	}
}

void TranslateKey(WORD keyz,char *out);
//HWND funky;
SJoyState JoystickF [16];


#ifdef NETPLAY_SUPPORT
INT_PTR CALLBACK DlgNPProgress(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SendDlgItemMessage(hDlg,IDC_NPPROGRESS,PBM_SETRANGE,0,(LPARAM)MAKELPARAM (0, 100));
	SendDlgItemMessage(hDlg,IDC_NPPROGRESS,PBM_SETPOS,(WPARAM)(int)NetPlay.PercentageComplete,0);

	return false;
}
#endif
INT_PTR CALLBACK DlgInputConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR temp[256];
	short C;
	int i, which;
	static int index=0;


	static SJoypad pads[10];


	//HBRUSH g_hbrBackground;

	InitInputCustomControl();
switch(msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint (hDlg, &ps);

			EndPaint (hDlg, &ps);
		}
		return true;
	case WM_INITDIALOG:
		WinRefreshDisplay();
		SetWindowText(hDlg,INPUTCONFIG_TITLE);
		SetDlgItemText(hDlg,IDC_JPTOGGLE,INPUTCONFIG_JPTOGGLE);
		SetDlgItemText(hDlg,IDC_OK,BUTTON_OK);
		SetDlgItemText(hDlg,IDC_CANCEL,BUTTON_CANCEL);
		SetDlgItemText(hDlg,IDC_LABEL_UP,INPUTCONFIG_LABEL_UP);
		SetDlgItemText(hDlg,IDC_LABEL_DOWN,INPUTCONFIG_LABEL_DOWN);
		SetDlgItemText(hDlg,IDC_LABEL_LEFT,INPUTCONFIG_LABEL_LEFT);
		SetDlgItemText(hDlg,IDC_LABEL_A,INPUTCONFIG_LABEL_A);
		SetDlgItemText(hDlg,IDC_LABEL_B,INPUTCONFIG_LABEL_B);
		SetDlgItemText(hDlg,IDC_LABEL_X,INPUTCONFIG_LABEL_X);
		SetDlgItemText(hDlg,IDC_LABEL_Y,INPUTCONFIG_LABEL_Y);
		SetDlgItemText(hDlg,IDC_LABEL_L,INPUTCONFIG_LABEL_L);
		SetDlgItemText(hDlg,IDC_LABEL_R,INPUTCONFIG_LABEL_R);
		SetDlgItemText(hDlg,IDC_LABEL_START,INPUTCONFIG_LABEL_START);
		SetDlgItemText(hDlg,IDC_LABEL_SELECT,INPUTCONFIG_LABEL_SELECT);
		SetDlgItemText(hDlg,IDC_LABEL_UPRIGHT,INPUTCONFIG_LABEL_UPRIGHT);
		SetDlgItemText(hDlg,IDC_LABEL_UPLEFT,INPUTCONFIG_LABEL_UPLEFT);
		SetDlgItemText(hDlg,IDC_LABEL_DOWNRIGHT,INPUTCONFIG_LABEL_DOWNRIGHT);
		SetDlgItemText(hDlg,IDC_LABEL_DOWNLEFT,INPUTCONFIG_LABEL_DOWNLEFT);
		SetDlgItemText(hDlg,IDC_LABEL_BLUE,INPUTCONFIG_LABEL_BLUE);

		for(i=5;i<10;i++)
			Joypad[i].Left_Up = Joypad[i].Right_Up = Joypad[i].Left_Down = Joypad[i].Right_Down = 0;

		memcpy(pads, Joypad, 10*sizeof(SJoypad));

		for( i=0;i<256;i++)
			GetAsyncKeyState(i);

		for( C = 0; C != 16; C ++)
	        JoystickF[C].Attached = joyGetDevCaps( JOYSTICKID1+C, &JoystickF[C].Caps, sizeof( JOYCAPS)) == JOYERR_NOERROR;

		for(i=1;i<6;i++)
		{
			_stprintf(temp,INPUTCONFIG_JPCOMBO,i);
			SendDlgItemMessage(hDlg,IDC_JPCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)temp);
		}

		for(i=6;i<11;i++)
		{
			_stprintf(temp,INPUTCONFIG_JPCOMBO INPUTCONFIG_LABEL_CONTROLLER_TURBO_PANEL_MOD,i-5);
			SendDlgItemMessage(hDlg,IDC_JPCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)temp);
		}

		SendDlgItemMessage(hDlg,IDC_JPCOMBO,CB_SETCURSEL,(WPARAM)0,0);

		SendDlgItemMessage(hDlg,IDC_JPTOGGLE,BM_SETCHECK, Joypad[index].Enabled ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg,IDC_ALLOWLEFTRIGHT,BM_SETCHECK, Settings.UpAndDown ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);

		set_buttoninfo(index,hDlg);

		EnableDisableKeyFields(index,hDlg);

		PostMessage(hDlg,WM_COMMAND, CBN_SELCHANGE<<16, 0);

		SetFocus(GetDlgItem(hDlg,IDC_JPCOMBO));

		return true;
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_USER+46:
		// refresh command, for clicking away from a selected field
		index = SendDlgItemMessage(hDlg,IDC_JPCOMBO,CB_GETCURSEL,0,0);
		if(index > 4) index += 3; // skip controllers 6, 7, and 8 in the input dialog
		set_buttoninfo(index,hDlg);
		return TRUE;
	case WM_USER+43:
		//MessageBox(hDlg,"USER+43 CAUGHT","moo",MB_OK);
		which = GetDlgCtrlID((HWND)lParam);
		switch(which)
		{
		case IDC_UP:
			Joypad[index].Up = wParam;

			break;
		case IDC_DOWN:
			Joypad[index].Down = wParam;

			break;
		case IDC_LEFT:
			Joypad[index].Left = wParam;

			break;
		case IDC_RIGHT:
			Joypad[index].Right = wParam;

			break;
		case IDC_A:
			Joypad[index].A = wParam;

			break;
		case IDC_B:
			Joypad[index].B = wParam;

			break;
		case IDC_X:
			Joypad[index].X = wParam;

			break;
		case IDC_Y:
			Joypad[index].Y = wParam;

			break;
		case IDC_L:
			Joypad[index].L = wParam;
			break;

		case IDC_R:
			Joypad[index].R = wParam;

			break;
		case IDC_SELECT:
			Joypad[index].Select = wParam;

			break;
		case IDC_START:
			Joypad[index].Start = wParam;

			break;
		case IDC_UPLEFT:
			Joypad[index].Left_Up = wParam;

			break;
		case IDC_UPRIGHT:
			Joypad[index].Right_Up = wParam;

			break;
		case IDC_DWNLEFT:
			Joypad[index].Left_Down = wParam;

            break;
		case IDC_DWNRIGHT:
			Joypad[index].Right_Down = wParam;

			break;

		}

		set_buttoninfo(index,hDlg);

		PostMessage(hDlg,WM_NEXTDLGCTL,0,0);
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			memcpy(Joypad, pads, 10*sizeof(SJoypad));
			EndDialog(hDlg,0);
			break;

		case IDOK:
			Settings.UpAndDown = IsDlgButtonChecked(hDlg, IDC_ALLOWLEFTRIGHT);
			WinSaveConfigFile();
			EndDialog(hDlg,0);
			break;

		case IDC_JPTOGGLE: // joypad Enable toggle
			index = SendDlgItemMessage(hDlg,IDC_JPCOMBO,CB_GETCURSEL,0,0);
			if(index > 4) index += 3; // skip controllers 6, 7, and 8 in the input dialog
			Joypad[index].Enabled=IsDlgButtonChecked(hDlg,IDC_JPTOGGLE);
			set_buttoninfo(index, hDlg); // update display of conflicts
			break;

		}
		switch(HIWORD(wParam))
		{
			case CBN_SELCHANGE:
				index = SendDlgItemMessage(hDlg,IDC_JPCOMBO,CB_GETCURSEL,0,0);
				SendDlgItemMessage(hDlg,IDC_JPCOMBO,CB_SETCURSEL,(WPARAM)index,0);
				if(index > 4) index += 3; // skip controllers 6, 7, and 8 in the input dialog
				if(index < 8)
				{
					SendDlgItemMessage(hDlg,IDC_JPTOGGLE,BM_SETCHECK, Joypad[index].Enabled ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);
					EnableWindow(GetDlgItem(hDlg,IDC_JPTOGGLE),TRUE);
				}
				else
				{
					SendDlgItemMessage(hDlg,IDC_JPTOGGLE,BM_SETCHECK, Joypad[index-8].Enabled ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);
					EnableWindow(GetDlgItem(hDlg,IDC_JPTOGGLE),FALSE);
				}

				set_buttoninfo(index,hDlg);

				EnableDisableKeyFields(index,hDlg);

				break;
		}
		return FALSE;

	}

	return FALSE;
}

struct hotkey_dialog_item {
    SCustomKey *key_entry;
    TCHAR *description;
};

// this structure defines the four sub pages in the hotkey config dialog
// to keep an entry blank, set the SCustomKey pointer to NULL and the text to an empty string
static hotkey_dialog_item hotkey_dialog_items[MAX_SWITCHABLE_HOTKEY_DIALOG_PAGES][MAX_SWITCHABLE_HOTKEY_DIALOG_ITEMS] = {
    {
        { &CustomKeys.SpeedUp, HOTKEYS_LABEL_1_1 },
        { &CustomKeys.SpeedDown, HOTKEYS_LABEL_1_2 },
        { &CustomKeys.Pause, HOTKEYS_LABEL_1_3 },
        { &CustomKeys.FastForwardToggle, HOTKEYS_LABEL_1_4 },
        { &CustomKeys.FastForward, HOTKEYS_LABEL_1_5 },
        { &CustomKeys.Rewind, HOTKEYS_LABEL_1_6 },
        { &CustomKeys.SkipUp, HOTKEYS_LABEL_1_7 },
        { &CustomKeys.SkipDown, HOTKEYS_LABEL_1_8 },
        { &CustomKeys.Mute, HOTKEYS_LABEL_1_9 },
        { &CustomKeys.ToggleCheats, HOTKEYS_LABEL_1_10 },
        { &CustomKeys.QuitS9X, HOTKEYS_LABEL_1_11 },
        { &CustomKeys.ResetGame, HOTKEYS_LABEL_1_12 },
        { &CustomKeys.SaveScreenShot, HOTKEYS_LABEL_1_13 },
		{ &CustomKeys.FrameAdvance, HOTKEYS_LABEL_1_14 },
    },
    {
        { &CustomKeys.BGL1, HOTKEYS_LABEL_2_1 },
        { &CustomKeys.BGL2, HOTKEYS_LABEL_2_2 },
        { &CustomKeys.BGL3, HOTKEYS_LABEL_2_3 },
        { &CustomKeys.BGL4, HOTKEYS_LABEL_2_4 },
        { &CustomKeys.BGL5, HOTKEYS_LABEL_2_5 },
        { &CustomKeys.ClippingWindows, HOTKEYS_LABEL_2_6 },
        { &CustomKeys.Transparency, HOTKEYS_LABEL_2_7 },
		{ &CustomKeys.ToggleBackdrop, HOTKEYS_LABEL_2_8 },
        { &CustomKeys.ScopePause, HOTKEYS_LABEL_2_9 },
        { &CustomKeys.SwitchControllers, HOTKEYS_LABEL_2_10 },
        { &CustomKeys.JoypadSwap, HOTKEYS_LABEL_2_11 },
        { &CustomKeys.ShowPressed, HOTKEYS_LABEL_2_12 },
        { &CustomKeys.FrameCount, HOTKEYS_LABEL_2_13 },
		{ &CustomKeys.ReadOnly, HOTKEYS_LABEL_2_14 },
    },
    {
        { &CustomKeys.TurboA, HOTKEYS_LABEL_3_1 },
        { &CustomKeys.TurboB, HOTKEYS_LABEL_3_2 },
        { &CustomKeys.TurboY, HOTKEYS_LABEL_3_3 },
        { &CustomKeys.TurboX, HOTKEYS_LABEL_3_4 },
        { &CustomKeys.TurboL, HOTKEYS_LABEL_3_5 },
        { &CustomKeys.TurboR, HOTKEYS_LABEL_3_6 },
        { &CustomKeys.TurboStart, HOTKEYS_LABEL_3_7 },
        { &CustomKeys.TurboSelect, HOTKEYS_LABEL_3_8 },
        { &CustomKeys.TurboLeft, HOTKEYS_LABEL_3_9 },
        { &CustomKeys.TurboUp, HOTKEYS_LABEL_3_10 },
        { &CustomKeys.TurboRight, HOTKEYS_LABEL_3_11 },
        { &CustomKeys.TurboDown, HOTKEYS_LABEL_3_12 },
		{ &CustomKeys.ScopeTurbo, HOTKEYS_LABEL_3_13 },
		{ NULL, _T("") },
    },
    {
        { &CustomKeys.SelectSave[0], HOTKEYS_LABEL_4_1 },
        { &CustomKeys.SelectSave[1], HOTKEYS_LABEL_4_2 },
        { &CustomKeys.SelectSave[2], HOTKEYS_LABEL_4_3 },
        { &CustomKeys.SelectSave[3], HOTKEYS_LABEL_4_4 },
        { &CustomKeys.SelectSave[4], HOTKEYS_LABEL_4_5 },
        { &CustomKeys.SelectSave[5], HOTKEYS_LABEL_4_6 },
        { &CustomKeys.SelectSave[6], HOTKEYS_LABEL_4_7 },
        { &CustomKeys.SelectSave[7], HOTKEYS_LABEL_4_8 },
        { &CustomKeys.SelectSave[8], HOTKEYS_LABEL_4_9 },
        { &CustomKeys.SelectSave[9], HOTKEYS_LABEL_4_10 },
        { &CustomKeys.SaveFileSelect, HOTKEYS_LABEL_4_11 },
        { &CustomKeys.LoadFileSelect, HOTKEYS_LABEL_4_12 },
		{ NULL, _T("") },
		{ NULL, _T("") },
    },
    {
        { &CustomKeys.AspectRatio, HOTKEYS_SWITCH_ASPECT_RATIO },
        { &CustomKeys.CheatEditorDialog, HOTKEYS_CHEAT_EDITOR_DIALOG },
        { &CustomKeys.CheatSearchDialog, HOTKEYS_CHEAT_SEARCH_DIALOG },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
        { NULL, _T("") },
    },
};

static void set_hotkeyinfo(HWND hDlg)
{
	int index = SendDlgItemMessage(hDlg,IDC_HKCOMBO,CB_GETCURSEL,0,0);

    for (int i = 0; i < MAX_SWITCHABLE_HOTKEY_DIALOG_ITEMS; i++)
    {
        int flags = SW_SHOWNOACTIVATE;
        if (hotkey_dialog_items[index][i].key_entry == NULL)
            flags = SW_HIDE;
        ShowWindow(GetDlgItem(hDlg, IDC_HOTKEY1 + i), flags);
        ShowWindow(GetDlgItem(hDlg, IDC_LABEL_HK1 + i), flags);

        int wParam = 0, lParam = 0;
        if (hotkey_dialog_items[index][i].key_entry)
        {
            wParam = hotkey_dialog_items[index][i].key_entry->key;
            lParam = hotkey_dialog_items[index][i].key_entry->modifiers;
        }
        SendDlgItemMessage(hDlg, IDC_HOTKEY1 + i, WM_USER + 44, wParam, lParam);
    }

	SendDlgItemMessage(hDlg,IDC_SLOTPLUS,WM_USER+44,CustomKeys.SlotPlus.key,CustomKeys.SlotPlus.modifiers);
	SendDlgItemMessage(hDlg,IDC_SLOTMINUS,WM_USER+44,CustomKeys.SlotMinus.key,CustomKeys.SlotMinus.modifiers);
	SendDlgItemMessage(hDlg,IDC_SLOTSAVE,WM_USER+44,CustomKeys.SlotSave.key,CustomKeys.SlotSave.modifiers);
	SendDlgItemMessage(hDlg,IDC_SLOTLOAD,WM_USER+44,CustomKeys.SlotLoad.key,CustomKeys.SlotLoad.modifiers);
    SendDlgItemMessage(hDlg, IDC_DIALOGSAVE, WM_USER + 44, CustomKeys.DialogSave.key, CustomKeys.DialogSave.modifiers);
    SendDlgItemMessage(hDlg, IDC_DIALOGLOAD, WM_USER + 44, CustomKeys.DialogLoad.key, CustomKeys.DialogLoad.modifiers);
	SendDlgItemMessage(hDlg, IDC_BANKPLUS, WM_USER + 44, CustomKeys.BankPlus.key, CustomKeys.BankPlus.modifiers);
	SendDlgItemMessage(hDlg, IDC_BANKMINUS, WM_USER + 44, CustomKeys.BankMinus.key, CustomKeys.BankMinus.modifiers);
	int i;
	for(i = 0 ; i < SAVE_SLOTS_PER_BANK; i++) SendDlgItemMessage(hDlg,IDC_SAVE1+i,WM_USER+44,CustomKeys.Save[i].key,CustomKeys.Save[i].modifiers);
	for(i = 0 ; i < SAVE_SLOTS_PER_BANK; i++) SendDlgItemMessage(hDlg,IDC_SAVE11+i,WM_USER+44,CustomKeys.Load[i].key,CustomKeys.Load[i].modifiers);

    for(int i = 0; i < MAX_SWITCHABLE_HOTKEY_DIALOG_ITEMS; i++)
    {
        SetDlgItemText(hDlg, IDC_LABEL_HK1 + i, hotkey_dialog_items[index][i].description);
    }
}

// DlgHotkeyConfig
INT_PTR CALLBACK DlgHotkeyConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i, which;
	static int index=0;


	static SCustomKeys keys;


	//HBRUSH g_hbrBackground;
	InitKeyCustomControl();
switch(msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint (hDlg, &ps);

			EndPaint (hDlg, &ps);
		}
		return true;
	case WM_INITDIALOG:
		WinRefreshDisplay();
		SetWindowText(hDlg,HOTKEYS_TITLE);

		// insert hotkey page list items
		for(i=1 ; i <= MAX_SWITCHABLE_HOTKEY_DIALOG_PAGES; i++)
		{
			TCHAR temp[256];
			_stprintf(temp,HOTKEYS_HKCOMBO,i);
			SendDlgItemMessage(hDlg,IDC_HKCOMBO,CB_ADDSTRING,0,(LPARAM)(LPCTSTR)temp);
		}

		SendDlgItemMessage(hDlg,IDC_HKCOMBO,CB_SETCURSEL,(WPARAM)0,0);

		memcpy(&keys, &CustomKeys, sizeof(SCustomKeys));
		for( i=0;i<256;i++)
		{
			GetAsyncKeyState(i);
		}

		SetDlgItemText(hDlg,IDC_LABEL_BLUE,HOTKEYS_LABEL_BLUE);

		set_hotkeyinfo(hDlg);

		PostMessage(hDlg,WM_COMMAND, CBN_SELCHANGE<<16, 0);

		SetFocus(GetDlgItem(hDlg,IDC_HKCOMBO));


		return true;
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_USER+46:
		// refresh command, for clicking away from a selected field
		index = SendDlgItemMessage(hDlg,IDC_HKCOMBO,CB_GETCURSEL,0,0);
		set_hotkeyinfo(hDlg);
		return TRUE;
	case WM_USER+43:
	{
		//MessageBox(hDlg,"USER+43 CAUGHT","moo",MB_OK);
		int modifiers = 0;
		if(GetAsyncKeyState(VK_MENU) || wParam == VK_MENU)
			modifiers |= CUSTKEY_ALT_MASK;
		if(GetAsyncKeyState(VK_CONTROL) || wParam == VK_CONTROL)
			modifiers |= CUSTKEY_CTRL_MASK;
		if(GetAsyncKeyState(VK_SHIFT) || wParam == VK_SHIFT)
			modifiers |= CUSTKEY_SHIFT_MASK;

		int index = SendDlgItemMessage(hDlg,IDC_HKCOMBO,CB_GETCURSEL,0,0);

		which = GetDlgCtrlID((HWND)lParam);

		switch(which)
		{
		case IDC_SLOTPLUS:
			CustomKeys.SlotPlus.key = wParam;
			CustomKeys.SlotPlus.modifiers = modifiers;
			break;

		case IDC_SLOTMINUS:
			CustomKeys.SlotMinus.key = wParam;
			CustomKeys.SlotMinus.modifiers = modifiers;
			break;

		case IDC_SLOTLOAD:
			CustomKeys.SlotLoad.key = wParam;
			CustomKeys.SlotLoad.modifiers = modifiers;
			break;

		case IDC_SLOTSAVE:
			CustomKeys.SlotSave.key = wParam;
			CustomKeys.SlotSave.modifiers = modifiers;
			break;

        case IDC_DIALOGSAVE:
            CustomKeys.DialogSave.key = wParam;
            CustomKeys.DialogSave.modifiers = modifiers;
            break;

        case IDC_DIALOGLOAD:
            CustomKeys.DialogLoad.key = wParam;
            CustomKeys.DialogLoad.modifiers = modifiers;
            break;

		case IDC_BANKPLUS:
			CustomKeys.BankPlus.key = wParam;
			CustomKeys.BankPlus.modifiers = modifiers;
			break;

		case IDC_BANKMINUS:
			CustomKeys.BankMinus.key = wParam;
			CustomKeys.BankMinus.modifiers = modifiers;
			break;
		}

        if(which >= IDC_HOTKEY1 && which <= IDC_HOTKEY14)
        {
            int offset = which - IDC_HOTKEY1;
            hotkey_dialog_items[index][offset].key_entry->key = wParam;
            hotkey_dialog_items[index][offset].key_entry->modifiers = modifiers;
        }

		if(which >= IDC_SAVE1 && which <= IDC_SAVE10)
		{
			CustomKeys.Save[which-IDC_SAVE1].key = wParam;
			CustomKeys.Save[which-IDC_SAVE1].modifiers = modifiers;
		}
		if(which >= IDC_SAVE11 && which <= IDC_SAVE20)
		{
			CustomKeys.Load[which-IDC_SAVE11].key = wParam;
			CustomKeys.Load[which-IDC_SAVE11].modifiers = modifiers;
		}

		set_hotkeyinfo(hDlg);
		PostMessage(hDlg,WM_NEXTDLGCTL,0,0);
//		PostMessage(hDlg,WM_KILLFOCUS,0,0);
	}
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			memcpy(&CustomKeys, &keys, sizeof(SCustomKeys));
			EndDialog(hDlg,0);
			break;
		case IDOK:
			WinSaveConfigFile();
			EndDialog(hDlg,0);
			break;
		}
		switch(HIWORD(wParam))
		{
			case CBN_SELCHANGE:
				index = SendDlgItemMessage(hDlg,IDC_HKCOMBO,CB_GETCURSEL,0,0);
				SendDlgItemMessage(hDlg,IDC_HKCOMBO,CB_SETCURSEL,(WPARAM)index,0);

				set_hotkeyinfo(hDlg);

				SetFocus(GetDlgItem(hDlg,IDC_HKCOMBO));

				break;
		}
		return FALSE;

	}

	return FALSE;
}


template<typename IntType>
int ScanAddress(const TCHAR* str, IntType& value)
{
	int ret = 0;
	if(_totlower(*str) == TEXT('s'))
	{
		ret = _stscanf(str+1, TEXT("%x"), &value);
		value += 0x7E0000 + 0x20000;
	}
	else if(_totlower(*str) == TEXT('i'))
	{
		ret = _stscanf(str+1, TEXT("%x"), &value);
		value += 0x7E0000 + 0x30000;
	}
	else
	{
		int plus = (*str == TEXT('0') && _totlower(str[1]) == TEXT('x')) ? 2 : 0;
		ret = _stscanf(str+plus, TEXT("%x"), &value);
	}
	return ret;
}

enum CheatStatus{
	Untouched,
	Deleted,
	Modified
};
typedef struct{
	std::vector<DWORD> state;
}CheatTracker;

int WinSearchCheatDatabase()
{
	std::string filename;
	int result;
	int reason = 0;

	filename = S9xGetDirectory(CHEAT_DIR);
	filename += "\\cheats.bml";
	if (!(result = S9xImportCheatsFromDatabase(filename.c_str())))
	{
		return result;
	}

	if (result < reason)
		reason = result;

	filename = S9xGetDirectory(HOME_DIR);
	filename += "\\cheats.bml";
	if (!(result = S9xImportCheatsFromDatabase(filename.c_str())))
	{
		return result;
	}

	if (result < reason)
		reason = result;

	filename = S9xGetDirectory(ROM_DIR);
	filename += "\\cheats.bml";
	if (!(result = S9xImportCheatsFromDatabase(filename.c_str())))
	{
		return result;
	}

	if (result < reason)
		reason = result;

	MessageBox(GUI.hWnd,
		reason == -1 ?
			TEXT("The database file cheats.bml was not found. It is normally installed with "
				"Snes9x, but you may also place a custom copy in your configuration or cheats directory.") :
			TEXT("No matching game was found in the databases. If you are using a non-official "
				"translation or modified copy, you may be able to find and manually enter the codes."),
		reason == -1 ? TEXT("Couldn't Find Cheats Database") : TEXT("No Matching Game Found"),
		MB_OK | MB_ICONEXCLAMATION);

	return reason;
}

#define ITEM_QUERY(a, b, c, d, e)  memset(&a, 0, sizeof(LV_ITEM)); \
						a.iItem= ListView_GetSelectionMark(GetDlgItem(hDlg, b)); \
						a.iSubItem=c; \
						a.mask=LVIF_TEXT; \
						a.pszText=d; \
						a.cchTextMax=e; \
						ListView_GetItem(GetDlgItem(hDlg, b), &a);
#define CHEAT_SIZE 1024

/* return a vector of all selected list items with their index in the listview
*  as first element and their lparam as second element of a pair
*/
static std::vector<std::pair<int, int>> get_all_selected_listitems(HWND lView)
{
    std::vector<std::pair<int, int>> result;
    LVITEM lvitem = { 0 };
    lvitem.mask = LVIF_PARAM;
    int index = ListView_GetNextItem(lView, -1, LVNI_SELECTED);
    do
    {
        lvitem.iItem = index;
        ListView_GetItem(lView, &lvitem);
        result.push_back(std::make_pair(index, (int)lvitem.lParam));
    } while ((index = ListView_GetNextItem(lView, index, LVNI_SELECTED)) != -1);

    return result;
}

INT_PTR CALLBACK DlgCheater(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool internal_change;
	static bool has_sel;
	static int  sel_idx;
	static uint8 new_sel;
	static CheatTracker ct;
	switch(msg)
	{
	case WM_INITDIALOG:
			WinRefreshDisplay();

			ListView_SetExtendedListViewStyle(GetDlgItem(hDlg, IDC_CHEAT_LIST), LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);

			SendDlgItemMessage(hDlg, IDC_CHEAT_CODE, EM_LIMITTEXT, CHEAT_SIZE, 0);
			SendDlgItemMessage(hDlg, IDC_CHEAT_DESCRIPTION, EM_LIMITTEXT, CHEAT_SIZE, 0);

			LVCOLUMN col;
			TCHAR temp[32];
			lstrcpy(temp, SEARCH_COLUMN_CODE);
			memset(&col, 0, sizeof(LVCOLUMN));
			col.mask=LVCF_FMT|LVCF_ORDER|LVCF_TEXT|LVCF_WIDTH;
			col.fmt=LVCFMT_LEFT;
			col.iOrder=0;
			col.cx=100;
			col.pszText=temp;

			ListView_InsertColumn(GetDlgItem(hDlg,IDC_CHEAT_LIST),    0,   &col);

			lstrcpy(temp,SEARCH_COLUMN_DESCRIPTION);
			memset(&col, 0, sizeof(LVCOLUMN));
			col.mask=LVCF_FMT|LVCF_ORDER|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
			col.fmt=LVCFMT_LEFT;
			col.iOrder=2;
			col.cx=300;
			col.pszText=temp;
			col.iSubItem=2;

			ListView_InsertColumn(GetDlgItem(hDlg,IDC_CHEAT_LIST),    1,   &col);

			ct.state.resize(Cheat.group.size());

			for(uint32 counter =0; counter < Cheat.group.size(); counter++)
			{
				std::string code_string;
				int curr_idx = -1;
				code_string = S9xCheatGroupToText(counter);
				Utf8ToWide wstring(code_string.c_str());

				LVITEM lvi;
				memset(&lvi, 0, sizeof(LVITEM));
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.cchTextMax = CHEAT_SIZE;
				lvi.pszText = wstring;
				lvi.lParam = counter; // we save the internal cheat index as lParam of list entry
				lvi.iItem = counter;
				curr_idx = ListView_InsertItem(GetDlgItem(hDlg,IDC_CHEAT_LIST), &lvi);

				ct.state[counter] = Untouched;

				Utf8ToWide wstring_name(Cheat.group[counter].name.c_str());
				memset(&lvi, 0, sizeof(LVITEM));
				lvi.iItem = curr_idx;
				lvi.iSubItem = 1;
				lvi.mask = LVIF_TEXT;
				lvi.cchTextMax = CHEAT_SIZE;
				lvi.pszText = wstring_name;
				SendDlgItemMessage(hDlg,IDC_CHEAT_LIST, LVM_SETITEM, 0, (LPARAM)&lvi);

				ListView_SetCheckState(GetDlgItem(hDlg,IDC_CHEAT_LIST), curr_idx, Cheat.group[counter].enabled);

			}
		return true;
	case WM_PAINT:
		{
		PAINTSTRUCT ps;
		BeginPaint (hDlg, &ps);

		EndPaint (hDlg, &ps);
		}
		return true;
	case WM_NOTIFY:
		{
			switch(LOWORD(wParam))
			{
            case IDC_CHEAT_LIST:
            {
                // react only to item changes (we are interested in selection or checkbox)
                if (((LPNMHDR)lParam)->code == LVN_ITEMCHANGED)
                {
                    NMLISTVIEW* listview_notify = (NMLISTVIEW*)lParam;
                    if (listview_notify->uChanged & LVIF_STATE)
                    {
                        HWND lView = GetDlgItem(hDlg, IDC_CHEAT_LIST);
                        int sel_count = ListView_GetSelectedCount(lView);
                        // selection change, update button states and selection tracking variable
                        if ((listview_notify->uOldState & LVIS_SELECTED) != (listview_notify->uNewState & LVIS_SELECTED))
                        {
                            if (0 == sel_count)
                            {
                                EnableWindow(GetDlgItem(hDlg, IDC_DELETE_CHEAT), false);
                                EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_CHEAT), false);
                                has_sel = false;
                                sel_idx = -1;
                            }
                            else
                            {
                                EnableWindow(GetDlgItem(hDlg, IDC_DELETE_CHEAT), true);
                                if (!has_sel || sel_idx != ListView_GetSelectionMark(lView))
                                {
                                    new_sel = 3;
                                    //change
                                    TCHAR buf[CHEAT_SIZE];
                                    LV_ITEM lvi;

                                    internal_change = true; // do not enable update button

                                    /* Code */
                                    ITEM_QUERY(lvi, IDC_CHEAT_LIST, 0, buf, CHEAT_SIZE);
                                    SetDlgItemText(hDlg, IDC_CHEAT_CODE, lvi.pszText);

                                    /* Description */
                                    ITEM_QUERY(lvi, IDC_CHEAT_LIST, 1, buf, CHEAT_SIZE);
                                    SetDlgItemText(hDlg, IDC_CHEAT_DESCRIPTION, lvi.pszText);

                                    internal_change = false;
                                }
                                sel_idx = ListView_GetSelectionMark(lView);
                                has_sel = true;
                            }
                        }

                        // multi-select and change of checkbox state - set same state to all selected items
                        if (sel_count > 1 && (listview_notify->uOldState & LVIS_STATEIMAGEMASK) != (listview_notify->uNewState & LVIS_STATEIMAGEMASK))
                        {
                            auto selected_items = get_all_selected_listitems(lView);
                            for (const auto &item : selected_items)
                            {
                                ListView_SetItemState(lView, item.first, listview_notify->uNewState, LVIS_STATEIMAGEMASK);
                            }
                        }

                        return true;
                    }
                }
            }
			default: return false;
			}
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_CHEAT_DESCRIPTION:
				{
					switch(HIWORD(wParam))
					{
					case EN_CHANGE:

						if(internal_change)
						{
							return false;
						}
						if(!has_sel)
							return true;

						EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_CHEAT), true);
						return true;
					}
					break;

				}
			case IDC_ADD_CHEAT:
				{
					char temp[CHEAT_SIZE];
					TCHAR tempDesc[CHEAT_SIZE];
					GetDlgItemTextA(hDlg, IDC_CHEAT_CODE, temp, CHEAT_SIZE);
					GetDlgItemText(hDlg, IDC_CHEAT_DESCRIPTION, tempDesc, CHEAT_SIZE);

					std::string valid_cheat = S9xCheatValidate(temp);

					if(!valid_cheat.empty())
					{
						Utf8ToWide wstring(valid_cheat.c_str());

						int curr_idx = -1;

						LVITEM lvi;
						memset(&lvi, 0, sizeof(LVITEM));
						lvi.mask = LVIF_TEXT | LVIF_PARAM;
						lvi.pszText = wstring;
						lvi.iItem = ListView_GetItemCount(GetDlgItem(hDlg,IDC_CHEAT_LIST));
						lvi.lParam = -1; // -1 signals this is a new cheat
						curr_idx = ListView_InsertItem(GetDlgItem(hDlg,IDC_CHEAT_LIST), &lvi);

						SetDlgItemText(hDlg, IDC_CHEAT_CODE, wstring);

						memset(&lvi, 0, sizeof(LVITEM));
						lvi.iItem = curr_idx;
						lvi.iSubItem = 1;
						lvi.mask = LVIF_TEXT;
						lvi.pszText = tempDesc;
						lvi.cchTextMax = CHEAT_SIZE;
						ListView_SetItem(GetDlgItem(hDlg, IDC_CHEAT_LIST), &lvi);

						// unselect all items, select new item and make sure it is visible
						ListView_SetItemState(GetDlgItem(hDlg, IDC_CHEAT_LIST), -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
						ListView_SetItemState(GetDlgItem(hDlg, IDC_CHEAT_LIST), curr_idx, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
						ListView_EnsureVisible(GetDlgItem(hDlg, IDC_CHEAT_LIST), curr_idx, FALSE);
					}
				}
				break;
			case IDC_UPDATE_CHEAT:
				{
					unsigned int j;
					TCHAR temp[CHEAT_SIZE];
					char code[CHEAT_SIZE];
					GetDlgItemTextA(hDlg, IDC_CHEAT_CODE, code, CHEAT_SIZE);

					std::string valid_cheat = S9xCheatValidate(code);

					if(!valid_cheat.empty())
					{
						Utf8ToWide wstring(code);

						LVITEM lvi;

						// get index in internal cheat list, if present mark as modified
						memset(&lvi, 0, sizeof(LVITEM));
						lvi.mask = LVIF_PARAM;
						lvi.iItem = sel_idx;
						ListView_GetItem(GetDlgItem(hDlg, IDC_CHEAT_LIST), &lvi);
						if (lvi.lParam >= 0)
							ct.state[lvi.lParam] = Modified;

						memset(&lvi, 0, sizeof(LVITEM));
						lvi.mask=LVIF_TEXT;
						lvi.pszText=wstring;
						lvi.iItem=sel_idx;
						lvi.cchTextMax = CHEAT_SIZE;
						ListView_SetItem(GetDlgItem(hDlg,IDC_CHEAT_LIST), &lvi);
						SetDlgItemText(hDlg, IDC_CHEAT_CODE, wstring);

						GetDlgItemText(hDlg, IDC_CHEAT_DESCRIPTION, temp, CHEAT_SIZE);

						memset(&lvi, 0, sizeof(LVITEM));
						lvi.iItem=sel_idx;
						lvi.iSubItem = 1;
						lvi.mask=LVIF_TEXT;
						lvi.pszText=temp;
						lvi.cchTextMax = CHEAT_SIZE;
						SendDlgItemMessage(hDlg,IDC_CHEAT_LIST, LVM_SETITEM, 0, (LPARAM)&lvi);

						// update done, disable update button
						EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_CHEAT), false);
					}
				}

				break;
			case IDC_DELETE_CHEAT:
				{
                    // save index, deleting item removes selection
                    int old_sel = sel_idx;
                    HWND lView = GetDlgItem(hDlg, IDC_CHEAT_LIST);
                    auto deleted_items = get_all_selected_listitems(lView);

                    // we delete in reverse order so that our item indexes stay valid
                    std::reverse(deleted_items.begin(), deleted_items.end());
                    for (const auto &item : deleted_items)
                    {
                        // get index in internal cheat list, if present mark as deleted
                        if(item.second >= 0)
                            ct.state[item.second] = Deleted;
                        ListView_DeleteItem(lView, item.first);
                    }

					ListView_SetItemState(lView, old_sel, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				}

				break;
			case IDC_DELETE_ALL:
				if (MessageBox(hDlg,
					TEXT("This will remove all cheats for this game."),
					TEXT("Remove all"),
					MB_OKCANCEL | MB_ICONWARNING) != IDOK)
				{
					break;
				}
				ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_CHEAT_LIST));
				for (unsigned int j = 0; j < Cheat.group.size(); j++)
				{
					ct.state[j] = Deleted;
				}
			case IDC_CLEAR_CHEATS:
				internal_change = true;
				SetDlgItemText(hDlg,IDC_CHEAT_CODE,TEXT(""));
				SetDlgItemText(hDlg,IDC_CHEAT_DESCRIPTION,TEXT(""));
				internal_change = false;
				ListView_SetItemState(GetDlgItem(hDlg, IDC_CHEAT_LIST),sel_idx, 0, LVIS_SELECTED|LVIS_FOCUSED);
				ListView_SetSelectionMark(GetDlgItem(hDlg, IDC_CHEAT_LIST), -1);
				sel_idx=-1;
				has_sel=false;
				break;
			case IDC_SEARCH_DB:
				if (MessageBox(hDlg,
					TEXT("This will apply current changes from the cheat dialog, try to import cheats from the cheat database, then reopen the dialog."),
					TEXT("Search Database"),
					MB_OKCANCEL | MB_ICONINFORMATION) == IDOK)
				{
					PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDOK, NC_SEARCHDB), (LPARAM)GetDlgItem(hDlg, IDOK));
				}
				break;
			case IDC_CHEAT_CODE:
				{
					uint32 j, k;
					long index;
					char buffer[15];
					char buffer2[15];
					POINT point;
					switch(HIWORD(wParam))
					{
					case EN_CHANGE:
						if (internal_change)
						{
							return true;
						}
						char temp[CHEAT_SIZE];
						char *valid_cheat = NULL;
						GetDlgItemTextA(hDlg, IDC_CHEAT_CODE, temp, CHEAT_SIZE);

						if (temp && temp[0] && (!S9xCheatValidate(temp).empty()))
						{
							if (has_sel)
								EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_CHEAT), true);
							else
								EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_CHEAT), false);
							EnableWindow(GetDlgItem(hDlg, IDC_ADD_CHEAT), true);
							delete[] valid_cheat;
						}
						else
						{
							EnableWindow(GetDlgItem(hDlg, IDC_ADD_CHEAT), false);
							EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_CHEAT), false);
						}

						break;
					}
					break;
				}
			case IDOK:
				{
					bool hit;
					unsigned int scanned;
					for(int k = 0; k < ListView_GetItemCount(GetDlgItem(hDlg, IDC_CHEAT_LIST)); k++)
					{
						LVITEM lvi;

						// get index in internal cheat list, if present mark as deleted
						memset(&lvi, 0, sizeof(LVITEM));
						lvi.mask = LVIF_PARAM;
						lvi.iItem = k;
						ListView_GetItem(GetDlgItem(hDlg, IDC_CHEAT_LIST), &lvi);

						int internal_index = lvi.lParam;

						if(internal_index >= 0)
						{
							if(ct.state[internal_index]==(unsigned long)Modified) // modified in GUI, change in core
							{
								TCHAR wcode[CHEAT_SIZE];
								TCHAR wdescription[CHEAT_SIZE];

								memset(&lvi, 0, sizeof(LV_ITEM));
								lvi.iItem = k;
								lvi.mask = LVIF_TEXT;
								lvi.pszText = wcode;
								lvi.cchTextMax = CHEAT_SIZE;
								ListView_GetItem(GetDlgItem(hDlg, IDC_CHEAT_LIST), &lvi);

								memset(&lvi, 0, sizeof(LV_ITEM));
								lvi.iItem = k;
								lvi.iSubItem = 1;
								lvi.mask=LVIF_TEXT;
								lvi.pszText = wdescription;
								lvi.cchTextMax = CHEAT_SIZE;
								ListView_GetItem(GetDlgItem(hDlg, IDC_CHEAT_LIST), &lvi);

								WideToUtf8 code(wcode);
								WideToUtf8 description(wdescription);

								S9xModifyCheatGroup(internal_index, std::string(description), std::string(code));
							}

							// set core state according to checkbox
							if (ListView_GetCheckState(GetDlgItem(hDlg, IDC_CHEAT_LIST), k))
								S9xEnableCheatGroup(internal_index);
							else
								S9xDisableCheatGroup(internal_index);
						}
						else // new cheat, add
						{
							TCHAR wcode[CHEAT_SIZE];
							TCHAR wdescription[CHEAT_SIZE];

							LV_ITEM lvi;
							memset(&lvi, 0, sizeof(LV_ITEM));
							lvi.iItem = k;
							lvi.mask = LVIF_TEXT;
							lvi.pszText = wcode;
							lvi.cchTextMax = CHEAT_SIZE;
							ListView_GetItem(GetDlgItem(hDlg, IDC_CHEAT_LIST), &lvi);

							memset(&lvi, 0, sizeof(LV_ITEM));
							lvi.iItem = k;
							lvi.iSubItem = 1;
							lvi.mask = LVIF_TEXT;
							lvi.pszText = wdescription;
							lvi.cchTextMax = CHEAT_SIZE;
							ListView_GetItem(GetDlgItem(hDlg, IDC_CHEAT_LIST), &lvi);

							WideToUtf8 code(wcode);
							WideToUtf8 description(wdescription);

							int index = S9xAddCheatGroup(std::string(description), std::string(code));

							if (index >= 0)
								if (ListView_GetCheckState(GetDlgItem(hDlg, IDC_CHEAT_LIST), k))
									S9xEnableCheatGroup(index);
						}
					}

					for(int l = ct.state.size() - 1; l >= 0; l-- )
					{
						if(ct.state[l]==Deleted)
						{
							S9xDeleteCheatGroup(l);
						}
					}
				}
			case IDCANCEL:
				EndDialog(hDlg, HIWORD(wParam) == NC_SEARCHDB ? NC_SEARCHDB : 0);
				return true;
			default:
				return false;
			}
		}

	default: return false;
	}
}


#define TEST_BIT(a,v) \
((a)[(v) >> 5] & (1 << ((v) & 31)))

static inline int CheatCount(int byteSub)
{
	int a, b=0;
//	for(a=0;a<0x32000-byteSub;a++)
	for(a=0;a<0x30000-byteSub;a++) // hide IRAM from cheat dialog (it seems not to be searhed correctly)
	{
		if(TEST_BIT(Cheat.ALL_BITS, a))
			b++;
	}
	return b;
}


struct ICheat
{
    uint32  address;
    uint32  new_val;
    uint32  saved_val;
	int		size;
    bool8   enabled;
    bool8   saved;
    char    name [22];
	int format;
};

bool TestRange(int val_type, S9xCheatDataSize bytes,  uint32 value)
{
	if(val_type!=2)
	{
		if(bytes==S9X_8_BITS)
		{
			if(value<256)
				return true;
			else return false;
		}
		if(bytes==S9X_16_BITS)
		{
			if(value<65536)
				return true;
			else return false;
		}
		if(bytes==S9X_24_BITS)
		{
			if(value<16777216)
				return true;
			else return false;
		}
		//if it reads in, it's a valid 32-bit unsigned!
		return true;
	}
	else
	{
		if(bytes==S9X_8_BITS)
		{
			if((int32)value<128 && (int32)value >= -128)
				return true;
			else return false;
		}
		if(bytes==S9X_16_BITS)
		{
			if((int32)value<32768 && (int32)value >= -32768)
				return true;
			else return false;
		}
		if(bytes==S9X_24_BITS)
		{
			if((int32)value<8388608 && (int32)value >= -8388608)
				return true;
			else return false;
		}
		//should be handled by sscanf
		return true;
	}
}


INT_PTR CALLBACK DlgCheatSearch(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static S9xCheatDataSize bytes;
	static int val_type;
	static int use_entered;
	static S9xCheatComparisonType comp_type;
	switch(msg)
	{
	case WM_INITDIALOG:
		{
		WinRefreshDisplay();
			if(val_type==0)
				val_type=1;
			ListView_SetExtendedListViewStyle(GetDlgItem(hDlg, IDC_ADDYS), LVS_EX_FULLROWSELECT);

			//defaults
			SendDlgItemMessage(hDlg, IDC_LESS_THAN, BM_SETCHECK, BST_CHECKED, 0);
			if(!use_entered)
				SendDlgItemMessage(hDlg, IDC_PREV, BM_SETCHECK, BST_CHECKED, 0);
			else if(use_entered==1)
			{
				SendDlgItemMessage(hDlg, IDC_ENTERED, BM_SETCHECK, BST_CHECKED, 0);
				EnableWindow(GetDlgItem(hDlg, IDC_VALUE_ENTER), true);
				EnableWindow(GetDlgItem(hDlg, IDC_ENTER_LABEL), true);
			}
			else if(use_entered==2)
			{
				SendDlgItemMessage(hDlg, IDC_ENTEREDADDRESS, BM_SETCHECK, BST_CHECKED, 0);
				EnableWindow(GetDlgItem(hDlg, IDC_VALUE_ENTER), true);
				EnableWindow(GetDlgItem(hDlg, IDC_ENTER_LABEL), true);
			}
			SendDlgItemMessage(hDlg, IDC_UNSIGNED, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_1_BYTE, BM_SETCHECK, BST_CHECKED, 0);

			if(comp_type==S9X_GREATER_THAN)
			{
				SendDlgItemMessage(hDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_GREATER_THAN, BM_SETCHECK, BST_CHECKED, 0);
			}
			else if(comp_type==S9X_LESS_THAN_OR_EQUAL)
			{
				SendDlgItemMessage(hDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_LESS_THAN_EQUAL, BM_SETCHECK, BST_CHECKED, 0);

			}
			else if(comp_type==S9X_GREATER_THAN_OR_EQUAL)
			{
				SendDlgItemMessage(hDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_GREATER_THAN_EQUAL, BM_SETCHECK, BST_CHECKED, 0);

			}
			else if(comp_type==S9X_EQUAL)
			{
				SendDlgItemMessage(hDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_EQUAL, BM_SETCHECK, BST_CHECKED, 0);

			}
			else if(comp_type==S9X_NOT_EQUAL)
			{
				SendDlgItemMessage(hDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_NOT_EQUAL, BM_SETCHECK, BST_CHECKED, 0);

			}

			if(val_type==2)
			{
				SendDlgItemMessage(hDlg, IDC_UNSIGNED, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_SIGNED, BM_SETCHECK, BST_CHECKED, 0);

			}
			else if(val_type==3)
			{
				SendDlgItemMessage(hDlg, IDC_UNSIGNED, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_HEX, BM_SETCHECK, BST_CHECKED, 0);
			}

			if(bytes==S9X_16_BITS)
			{
				SendDlgItemMessage(hDlg, IDC_1_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_2_BYTE, BM_SETCHECK, BST_CHECKED, 0);
			}
			else if(bytes==S9X_24_BITS)
			{
				SendDlgItemMessage(hDlg, IDC_1_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_3_BYTE, BM_SETCHECK, BST_CHECKED, 0);
			}
			else if(bytes==S9X_32_BITS)
			{
				SendDlgItemMessage(hDlg, IDC_1_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);
				SendDlgItemMessage(hDlg, IDC_4_BYTE, BM_SETCHECK, BST_CHECKED, 0);
			}

			LVCOLUMN col;
			TCHAR temp[32];
			lstrcpy(temp,TEXT("Address"));
			memset(&col, 0, sizeof(LVCOLUMN));
			col.mask=LVCF_FMT|LVCF_ORDER|LVCF_TEXT|LVCF_WIDTH;
			col.fmt=LVCFMT_LEFT;
			col.iOrder=0;
			col.cx=70;
			col.cchTextMax=7;
			col.pszText=temp;

			ListView_InsertColumn(GetDlgItem(hDlg,IDC_ADDYS),   0,   &col);

			lstrcpy(temp,TEXT("Curr. Value"));
			memset(&col, 0, sizeof(LVCOLUMN));
			col.mask=LVCF_FMT|LVCF_ORDER|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
			col.fmt=LVCFMT_LEFT;
			col.iOrder=1;
			col.cx=104;
			col.cchTextMax=3;
			col.pszText=temp;
			col.iSubItem=1;

			ListView_InsertColumn(GetDlgItem(hDlg,IDC_ADDYS),    1,   &col);

			lstrcpy(temp,TEXT("Prev. Value"));
			memset(&col, 0, sizeof(LVCOLUMN));
			col.mask=LVCF_FMT|LVCF_ORDER|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
			col.fmt=LVCFMT_LEFT;
			col.iOrder=2;
			col.cx=104;
			col.cchTextMax=32;
			col.pszText=temp;
			col.iSubItem=2;

			ListView_InsertColumn(GetDlgItem(hDlg,IDC_ADDYS),    2,   &col);

			{
					int l = CheatCount(bytes);
					ListView_SetItemCount (GetDlgItem(hDlg, IDC_ADDYS), l);
			}

		}
		return true;

		case WM_DESTROY:
			{
				cheatSearchHWND = NULL;
				S9xSaveCheatFile (S9xGetFilename (".cht", CHEAT_DIR));
				break;
			}

		case WM_PAINT:
		{
		PAINTSTRUCT ps;
		BeginPaint (hDlg, &ps);

		EndPaint (hDlg, &ps);
		}
		return true;
	case WM_NOTIFY:
		{
			static int selectionMarkOverride = -1;
			static int foundItemOverride = -1;
			if(wParam == IDC_ADDYS)
			{
				NMHDR * nmh=(NMHDR*)lParam;
				if(nmh->hwndFrom == GetDlgItem(hDlg, IDC_ADDYS) && nmh->code == LVN_GETDISPINFO)
				{
					static TCHAR buf[12]; // the following code assumes this variable is static
					int i, j;
					NMLVDISPINFO * nmlvdi=(NMLVDISPINFO*)lParam;
					j=nmlvdi->item.iItem;
					j++;
					for(i=0;i<(0x32000-bytes)&& j>0;i++)
					{
						if(TEST_BIT(Cheat.ALL_BITS, i))
							j--;
					}
					if (i>=0x32000 && j!=0)
					{
						return false;
					}
					i--;
					if(j=nmlvdi->item.iSubItem==0)
					{
						if(i < 0x20000)
							_stprintf(buf, TEXT("%06X"), i+0x7E0000);
						else if(i < 0x30000)
							_stprintf(buf, TEXT("s%05X"), i-0x20000);
						else
							_stprintf(buf, TEXT("i%05X"), i-0x30000);
						nmlvdi->item.pszText=buf;
						nmlvdi->item.cchTextMax=8;
					}
					if(j=nmlvdi->item.iSubItem==1)
					{
						int q=0, r=0;
						if(i < 0x20000)
							for(r=0;r<=bytes;r++)
								q+=(Cheat.RAM[i+r])<<(8*r);
						else if(i < 0x30000)
							for(r=0;r<=bytes;r++)
								q+=(Cheat.SRAM[(i-0x20000)+r])<<(8*r);
						else
							for(r=0;r<=bytes;r++)
								q+=(Cheat.FillRAM[(i-0x30000)+r])<<(8*r);
						//needs to account for size
						switch(val_type)
						{
						case 1:
							_stprintf(buf, TEXT("%u"), q);
							break;
						case 3:
							{
								switch(bytes)
								{
									default:
									case S9X_8_BITS:  _stprintf(buf, TEXT("%02X"), q&0xFF);break;
									case S9X_16_BITS: _stprintf(buf, TEXT("%04X"), q&0xFFFF); break;
									case S9X_24_BITS: _stprintf(buf, TEXT("%06X"), q&0xFFFFFF);break;
									case S9X_32_BITS: _stprintf(buf, TEXT("%08X"), q);break;
								}
							}
							break;
						case 2:
							default:
								switch(bytes)
								{
									default:
									case S9X_8_BITS:
										if((q-128)<0)
											_stprintf(buf, TEXT("%d"), q&0xFF);
										else _stprintf(buf, TEXT("%d"), q-256);
										break;
									case S9X_16_BITS:
										if((q-32768)<0)
											_stprintf(buf, TEXT("%d"), q&0xFFFF);
										else _stprintf(buf, TEXT("%d"), q-65536);
										break;
									case S9X_24_BITS:
										if((q-0x800000)<0)
											_stprintf(buf, TEXT("%d"), q&0xFFFFFF);
										else _stprintf(buf, TEXT("%d"), q-0x1000000);
										break;

									case S9X_32_BITS: _stprintf(buf, TEXT("%d"), q);break;
								}
								break;
						}
						nmlvdi->item.pszText=buf;
						nmlvdi->item.cchTextMax=4;
					}
					if(j=nmlvdi->item.iSubItem==2)
					{
						int q=0, r=0;
						if(i < 0x20000)
							for(r=0;r<=bytes;r++)
								q+=(Cheat.CWRAM[i+r])<<(8*r);
						else if(i < 0x30000)
							for(r=0;r<=bytes;r++)
								q+=(Cheat.CSRAM[(i-0x20000)+r])<<(8*r);
						else
							for(r=0;r<=bytes;r++)
								q+=(Cheat.CIRAM[(i-0x30000)+r])<<(8*r);
						//needs to account for size
						switch(val_type)
						{
						case 1:
							_stprintf(buf, TEXT("%u"), q);
							break;
						case 3:
							{
								switch(bytes)
								{
									default:
									case S9X_8_BITS:_stprintf(buf, TEXT("%02X"), q&0xFF);break;
									case S9X_16_BITS: _stprintf(buf, TEXT("%04X"), q&0xFFFF); break;
									case S9X_24_BITS: _stprintf(buf, TEXT("%06X"), q&0xFFFFFF);break;
									case S9X_32_BITS: _stprintf(buf, TEXT("%08X"), q);break;
								}
								break;
							}
						case 2:
							default:
								switch(bytes)
								{
									default:
									case S9X_8_BITS:
										if((q-128)<0)
											_stprintf(buf, TEXT("%d"), q&0xFF);
										else _stprintf(buf, TEXT("%d"), q-256);
										break;
									case S9X_16_BITS:
										if((q-32768)<0)
											_stprintf(buf, TEXT("%d"), q&0xFFFF);
										else _stprintf(buf, TEXT("%d"), q-65536);
										break;
									case S9X_24_BITS:
										if((q-0x800000)<0)
											_stprintf(buf, TEXT("%d"), q&0xFFFFFF);
										else _stprintf(buf, TEXT("%d"), q-0x1000000);
										break;

									case S9X_32_BITS: _stprintf(buf, TEXT("%d"), q);break;
								}
								break;
						}
						nmlvdi->item.pszText=buf;
						nmlvdi->item.cchTextMax=4;
					}
					// nmlvdi->item.mask=LVIF_TEXT; // This is bad as wine relies on this to not change.

				}
				else if(nmh->hwndFrom == GetDlgItem(hDlg, IDC_ADDYS) && (nmh->code == (UINT)LVN_ITEMACTIVATE||nmh->code == (UINT)NM_CLICK))
				{
					bool enable=true;
					if(-1==ListView_GetSelectionMark(nmh->hwndFrom))
					{
						enable=false;
					}
					EnableWindow(GetDlgItem(hDlg, IDC_C_ADD), enable);
                    EnableWindow(GetDlgItem(hDlg, IDC_C_WATCH), enable);
				}
				// allow typing in an address to jump to it
				else if(nmh->hwndFrom == GetDlgItem(hDlg, IDC_ADDYS) && nmh->code == (UINT)LVN_ODFINDITEM)
				{
					LRESULT pResult;

					// pNMHDR has information about the item we should find
					// In pResult we should save which item that should be selected
					NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)lParam;

					/* pFindInfo->iStart is from which item we should search.
					We search to bottom, and then restart at top and will stop
					at pFindInfo->iStart, unless we find an item that match
					*/

					// Set the default return value to -1
					// That means we didn't find any match.
					pResult = -1;

					//Is search NOT based on string?
					if( (pFindInfo->lvfi.flags & LVFI_STRING) == 0 )
					{
						//This will probably never happend...
						return pResult;
					}

					//This is the string we search for
					const TCHAR *searchstr = pFindInfo->lvfi.psz;

					int startPos = pFindInfo->iStart;
					//Is startPos outside the list (happens if last item is selected)
					if(startPos >= ListView_GetItemCount(GetDlgItem(hDlg,IDC_ADDYS)))
						startPos = 0;

					int currentPos, addrPos;
					for(addrPos=0,currentPos=0;addrPos<(0x32000-bytes)&&currentPos<startPos;addrPos++)
					{
						if(TEST_BIT(Cheat.ALL_BITS, addrPos))
							currentPos++;
					}

					pResult=currentPos;

					if (addrPos>=0x32000 && addrPos!=0)
						break;

					// ignore leading 0's
					while(searchstr[0] == '0' && searchstr[1] != '\0')
						searchstr++;

					int searchNum = 0;

					ScanAddress(searchstr, searchNum);


//					if (searchstr[0] != '7')
//						break; // all searchable addresses begin with a 7

					bool looped = false;

					// perform search
					do
					{

						if(addrPos == searchNum)
						{
							// select this item and stop search
							pResult = currentPos;
							break;
						}
						else if(addrPos > searchNum)
						{
							if(looped)
							{
								pResult = currentPos;
								break;
							}

							// optimization: the items are ordered alphabetically, so go back to the top since we know it can't be anything further down
							currentPos = 0;
							addrPos = 0;
							while(!TEST_BIT(Cheat.ALL_BITS, addrPos))
								addrPos++;
							looped = true;
							continue;
						}

						//Go to next item
						addrPos++;
						while(!TEST_BIT(Cheat.ALL_BITS, addrPos))
							addrPos++;
						currentPos++;

						//Need to restart at top?
						if(currentPos >= ListView_GetItemCount(GetDlgItem(hDlg,IDC_ADDYS)))
						{
							currentPos = 0;
							addrPos = 0;
							while(!TEST_BIT(Cheat.ALL_BITS, addrPos))
								addrPos++;
						}

					//Stop if back to start
					}while(currentPos != startPos);

					foundItemOverride = pResult;

					// in case previously-selected item is 0
					ListView_SetItemState (GetDlgItem(hDlg,IDC_ADDYS), 1, LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);

					return pResult; // HACK: for some reason this selects the first item instead of what it's returning... current workaround is to manually re-select this return value upon the next changed event
				}
				else if(nmh->hwndFrom == GetDlgItem(hDlg, IDC_ADDYS) && nmh->code == LVN_ITEMCHANGED)
				{
					// hack - see note directly above
					LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lParam;
					if(lpnmlv->uNewState & (LVIS_SELECTED|LVIS_FOCUSED))
					{
						if(foundItemOverride != -1 && lpnmlv->iItem == 0)
						{
							ListView_SetItemState (GetDlgItem(hDlg,IDC_ADDYS), foundItemOverride, LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
							ListView_EnsureVisible (GetDlgItem(hDlg,IDC_ADDYS), foundItemOverride, FALSE);
							selectionMarkOverride = foundItemOverride;
							foundItemOverride = -1;
						}
						else
						{
							selectionMarkOverride = lpnmlv->iItem;
						}
					}
				}
			}
		}
		break;
	case WM_ACTIVATE:
		ListView_RedrawItems(GetDlgItem(hDlg, IDC_ADDYS),0, 0x32000);
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_LESS_THAN:
			case IDC_GREATER_THAN:
			case IDC_LESS_THAN_EQUAL:
			case IDC_GREATER_THAN_EQUAL:
			case IDC_EQUAL:
			case IDC_NOT_EQUAL:
				if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_LESS_THAN))
					comp_type=S9X_LESS_THAN;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_GREATER_THAN))
					comp_type=S9X_GREATER_THAN;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_LESS_THAN_EQUAL))
					comp_type=S9X_LESS_THAN_OR_EQUAL;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_GREATER_THAN_EQUAL))
					comp_type=S9X_GREATER_THAN_OR_EQUAL;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_EQUAL))
					comp_type=S9X_EQUAL;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_NOT_EQUAL))
					comp_type=S9X_NOT_EQUAL;

				break;
			case IDC_1_BYTE:
			case IDC_2_BYTE:
			case IDC_3_BYTE:
			case IDC_4_BYTE:
				if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_1_BYTE))
					bytes=S9X_8_BITS;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_2_BYTE))
					bytes=S9X_16_BITS;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_3_BYTE))
					bytes=S9X_24_BITS;
				else bytes=S9X_32_BITS;
				{
					int l = CheatCount(bytes);
					ListView_SetItemCount (GetDlgItem(hDlg, IDC_ADDYS), l);
				}

				break;

			case IDC_SIGNED:
			case IDC_UNSIGNED:
			case IDC_HEX:
				if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_UNSIGNED))
					val_type=1;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_SIGNED))
					val_type=2;
				else val_type=3;
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ADDYS),0, 0x32000);
				break;
			case IDC_C_ADD:
				{
					// account for size
					struct ICheat cht;
//					int idx=-1;
					LVITEM lvi;
					static TCHAR buf[12]; // the following code assumes this variable is static, I think
					memset(&cht, 0, sizeof(struct SCheat));

					//retrieve and convert to SCheat

					if(bytes==S9X_8_BITS)
						cht.size=1;
					else if (bytes==S9X_16_BITS)
						cht.size=2;
					else if (bytes==S9X_24_BITS)
						cht.size=3;
					else if (bytes==S9X_32_BITS)
						cht.size=4;


					ITEM_QUERY(lvi, IDC_ADDYS, 0, buf, 7);


					ScanAddress(buf, cht.address);

					memset(buf, 0, sizeof(TCHAR) * 7);
					if(val_type==1)
					{
						ITEM_QUERY(lvi, IDC_ADDYS, 1, buf, 12);
						_stscanf(buf, TEXT("%u"), &cht.new_val);
						memset(buf, 0, sizeof(TCHAR) * 7);
						ITEM_QUERY(lvi, IDC_ADDYS, 2, buf, 12);
						_stscanf(buf, TEXT("%u"), &cht.saved_val);
					}
					else if(val_type==3)
					{
						ITEM_QUERY(lvi, IDC_ADDYS, 1, buf, 12);
						_stscanf(buf, TEXT("%x"), &cht.new_val);
						memset(buf, 0, sizeof(TCHAR) * 7);
						ITEM_QUERY(lvi, IDC_ADDYS, 2, buf, 12);
						_stscanf(buf, TEXT("%x"), &cht.saved_val);
					}
					else
					{
						ITEM_QUERY(lvi, IDC_ADDYS, 1, buf, 12);
						_stscanf(buf, TEXT("%d"), &cht.new_val);
						memset(buf, 0, sizeof(TCHAR) * 7);
						ITEM_QUERY(lvi, IDC_ADDYS, 2, buf, 12);
						_stscanf(buf, TEXT("%d"), &cht.saved_val);
					}
					cht.format=val_type;
					//invoke dialog
					DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CHEAT_FROM_SEARCH), hDlg, DlgCheatSearchAdd, (LPARAM)&cht);
				}
				break;
			case IDC_C_RESET:
				S9xStartCheatSearch(&Cheat);
				{
					int l = CheatCount(bytes);
					ListView_SetItemCount (GetDlgItem(hDlg, IDC_ADDYS), l);
				}
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ADDYS),0, 0x32000);
				//val_type=1;
				//SendDlgItemMessage(hDlg, IDC_UNSIGNED, BM_SETCHECK, BST_CHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_SIGNED, BM_SETCHECK, BST_UNCHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_HEX, BM_SETCHECK, BST_UNCHECKED, 0);

				//bytes=S9X_8_BITS;
				//SendDlgItemMessage(hDlg, IDC_1_BYTE, BM_SETCHECK, BST_CHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_2_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_3_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_4_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);


				//use_entered=0;
				//SendDlgItemMessage(hDlg, IDC_PREV, BM_SETCHECK, BST_CHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_ENTERED, BM_SETCHECK, BST_UNCHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_ENTEREDADDRESS, BM_SETCHECK, BST_UNCHECKED, 0);
				//EnableWindow(GetDlgItem(hDlg, IDC_VALUE_ENTER), false);
				//EnableWindow(GetDlgItem(hDlg, IDC_ENTER_LABEL), false);

				//comp_type=S9X_LESS_THAN;
				//SendDlgItemMessage(hDlg, IDC_LESS_THAN, BM_SETCHECK, BST_CHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_GREATER_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_LESS_THAN_EQUAL, BM_SETCHECK, BST_UNCHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_GREATER_THAN_EQUAL, BM_SETCHECK, BST_UNCHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_EQUAL, BM_SETCHECK, BST_UNCHECKED, 0);
				//SendDlgItemMessage(hDlg, IDC_NOT_EQUAL, BM_SETCHECK, BST_UNCHECKED, 0);
				return true;
			case IDC_C_WATCH:
				{
					uint32 address = (uint32)-1;
					TCHAR buf [12];
					LVITEM lvi;
					ITEM_QUERY(lvi, IDC_ADDYS, 0, buf, 7);
					ScanAddress(buf, address);
					{
						memset(buf, 0, sizeof(TCHAR) * 7);

						unsigned int i;
						for(i = 0 ; i < sizeof(watches)/sizeof(*watches) ; i++)
							if(!watches[i].on || watches[i].address == address)
								break;
						if(i >= sizeof(watches)/sizeof(*watches))
							i = (unsigned int)(sizeof(watches)/sizeof(*watches)-1);

						watches[i].on = true;

						// account for size
						if(bytes==S9X_8_BITS)
							watches[i].size=1;
						else if (bytes==S9X_16_BITS)
							watches[i].size=2;
						else if (bytes==S9X_24_BITS)
							watches[i].size=3;
						else if (bytes==S9X_32_BITS)
							watches[i].size=4;

						watches[i].format=val_type;
						watches[i].address=address;
						strncpy(watches[i].buf,_tToChar(buf),12);
						if(address < 0x7E0000 + 0x20000)
							sprintf(watches[i].desc, "%6X", address);
						else if(address < 0x7E0000 + 0x30000)
							sprintf(watches[i].desc, "s%05X", address - 0x7E0000 - 0x20000);
						else
							sprintf(watches[i].desc, "i%05X", address - 0x7E0000 - 0x30000);
					}
					{
						RECT rect;
						GetClientRect (GUI.hWnd, &rect);
						InvalidateRect (GUI.hWnd, &rect, true);
					}
				}
				break;
			case IDC_C_CLEARWATCH:
				{
					for(unsigned int i = 0 ; i < sizeof(watches)/sizeof(*watches) ; i++)
						watches[i].on = false;
					{
						RECT rect;
						GetClientRect (GUI.hWnd, &rect);
						InvalidateRect (GUI.hWnd, &rect, true);
					}
				}
				break;
			case IDC_C_LOADWATCH:
				{
					OPENFILENAME  ofn;
					TCHAR  szFileName[MAX_PATH];
					TCHAR  szPathName[MAX_PATH];
					lstrcpy(szFileName, TEXT("watches"));
					_tfullpath(szPathName, S9xGetDirectoryT(CHEAT_DIR), MAX_PATH);

					memset( (LPVOID)&ofn, 0, sizeof(OPENFILENAME) );
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = GUI.hWnd;
					ofn.lpstrFilter = FILE_INFO_TXT_FILE_TYPE TEXT("\0*.txt\0") FILE_INFO_ANY_FILE_TYPE TEXT("\0*.*\0\0");
					ofn.lpstrFile = szFileName;
					ofn.lpstrDefExt = TEXT("txt");
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
					ofn.lpstrInitialDir = szPathName;
					if(GetOpenFileName( &ofn ))
					{
						FILE *file = _tfopen(szFileName, TEXT("r"));
						if(file)
						{
							unsigned int i;
							for(i = 0 ; i < sizeof(watches)/sizeof(*watches) ; i++)
								watches[i].on = false;
							for(i = 0 ; i < sizeof(watches)/sizeof(*watches) ; i++)
							{
								char nameStr [32];
								nameStr[0]='?'; nameStr[1]='\0';
								fscanf(file, " address = 0x%x, name = \"%31[^\"]\", size = %d, format = %d\n", &watches[i].address, nameStr, &watches[i].size, &watches[i].format);
								if(nameStr[0] == '\0' || nameStr[0] == '?')
								{
									if(watches[i].address < 0x7E0000 + 0x20000)
										sprintf(nameStr, "%06X", watches[i].address);
									else if(watches[i].address < 0x7E0000 + 0x30000)
										sprintf(nameStr, "s%05X", watches[i].address - 0x7E0000 - 0x20000);
									else
										sprintf(nameStr, "i%05X", watches[i].address - 0x7E0000 - 0x30000);
								}
								nameStr[31] = '\0';
								if(!ferror(file))
								{
									watches[i].on = true;
									watches[i].buf[0] = '\0';
									strncpy(watches[i].desc, nameStr, sizeof(watches[i].desc)); watches[i].desc[sizeof(watches[i].desc)-1]='\0';
								}
								if(ferror(file) || feof(file))
									break;
							}
							fclose(file);
						}
					}
					{
						RECT rect;
						GetClientRect (GUI.hWnd, &rect);
						InvalidateRect (GUI.hWnd, &rect, true);
					}
				}
				break;
			case IDC_C_SAVEWATCH:
				{
					OPENFILENAME  ofn;
					TCHAR  szFileName[MAX_PATH];
					TCHAR  szPathName[MAX_PATH];
					lstrcpy(szFileName, TEXT("watches"));
					_tfullpath(szPathName, S9xGetDirectoryT(CHEAT_DIR), MAX_PATH);

					memset( (LPVOID)&ofn, 0, sizeof(OPENFILENAME) );
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = GUI.hWnd;
					ofn.lpstrFilter = FILE_INFO_TXT_FILE_TYPE TEXT("\0*.txt\0") FILE_INFO_ANY_FILE_TYPE TEXT("\0*.*\0\0");
					ofn.lpstrFile = szFileName;
					ofn.lpstrDefExt = TEXT("txt");
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
					ofn.lpstrInitialDir = szPathName;
					if(GetSaveFileName( &ofn ))
					{
						FILE *file = _tfopen(szFileName, TEXT("w"));
						if(file)
						{
							for(unsigned int i = 0 ; i < sizeof(watches)/sizeof(*watches) ; i++)
								if(watches[i].on)
//									fprintf(file, "address = 0x%x, name = \"%6X\", size = %d, format = %d\n", watches[i].address, watches[i].address, watches[i].size, watches[i].format);
									fprintf(file, "address = 0x%x, name = \"?\", size = %d, format = %d\n", watches[i].address, watches[i].size, watches[i].format);
							fclose(file);
						}
					}
					{
						RECT rect;
						GetClientRect (GUI.hWnd, &rect);
						InvalidateRect (GUI.hWnd, &rect, true);
					}
				}
				break;

			case IDC_REFRESHLIST:
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ADDYS),0, 0x32000);
				break;

			case IDC_ENTERED:
			case IDC_ENTEREDADDRESS:
			case IDC_PREV:
				if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_ENTERED))
				{
					use_entered=1;
					EnableWindow(GetDlgItem(hDlg, IDC_VALUE_ENTER), true);
					EnableWindow(GetDlgItem(hDlg, IDC_ENTER_LABEL), true);
				}
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_ENTEREDADDRESS))
				{
					use_entered=2;
					EnableWindow(GetDlgItem(hDlg, IDC_VALUE_ENTER), true);
					EnableWindow(GetDlgItem(hDlg, IDC_ENTER_LABEL), true);
				}
				else
				{
					use_entered=0;
					EnableWindow(GetDlgItem(hDlg, IDC_VALUE_ENTER), false);
					EnableWindow(GetDlgItem(hDlg, IDC_ENTER_LABEL), false);
				}
				return true;
				break;
			case IDC_C_SEARCH:
				{
				val_type=0;

				if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_LESS_THAN))
					comp_type=S9X_LESS_THAN;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_GREATER_THAN))
					comp_type=S9X_GREATER_THAN;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_LESS_THAN_EQUAL))
					comp_type=S9X_LESS_THAN_OR_EQUAL;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_GREATER_THAN_EQUAL))
					comp_type=S9X_GREATER_THAN_OR_EQUAL;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_EQUAL))
					comp_type=S9X_EQUAL;
				else comp_type=S9X_NOT_EQUAL;

				if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_UNSIGNED))
					val_type=1;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_SIGNED))
					val_type=2;
				else val_type=3;



				if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_1_BYTE))
					bytes=S9X_8_BITS;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_2_BYTE))
					bytes=S9X_16_BITS;
				else if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_3_BYTE))
					bytes=S9X_24_BITS;
				else bytes=S9X_32_BITS;


				if(BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_ENTERED) ||
				   BST_CHECKED==IsDlgButtonChecked(hDlg, IDC_ENTEREDADDRESS))
				{
					TCHAR buf[20];
					GetDlgItemText(hDlg, IDC_VALUE_ENTER, buf, 20);
					uint32 value;
					int ret;
					if(use_entered==2)
					{
						ret = ScanAddress(buf, value);
						value -= 0x7E0000;
						S9xSearchForAddress (&Cheat, comp_type, bytes, value, FALSE);
					}
					else
					{
						if(val_type==1)
							ret=_stscanf(buf, TEXT("%ul"), &value);
						else if (val_type==2)
							ret=_stscanf(buf, TEXT("%d"), &value);
						else ret=_stscanf(buf, TEXT("%x"), &value);


						if(ret!=1||!TestRange(val_type, bytes, value))
						{
							MessageBox(hDlg, SEARCH_ERR_INVALIDSEARCHVALUE, SEARCH_TITLE_CHEATERROR, MB_OK);
							return true;
						}

						S9xSearchForValue (&Cheat, comp_type,
							bytes, value,
							(val_type==2), FALSE);
					}

				}
				else
				{
					S9xSearchForChange (&Cheat, comp_type,
                         bytes, (val_type==2), FALSE);
				}
				int l = CheatCount(bytes);
				ListView_SetItemCount (GetDlgItem(hDlg, IDC_ADDYS), l);
				}

				// if non-modal, update "Prev. Value" column after Search
				if(cheatSearchHWND)
				{
					CopyMemory(Cheat.CWRAM, Cheat.RAM, 0x20000);
					CopyMemory(Cheat.CSRAM, Cheat.SRAM, 0x10000);
					CopyMemory(Cheat.CIRAM, Cheat.FillRAM, 0x2000);
				}


				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ADDYS),0, 0x32000);
				return true;
				break;
			case IDOK:
				CopyMemory(Cheat.CWRAM, Cheat.RAM, 0x20000);
				CopyMemory(Cheat.CSRAM, Cheat.SRAM, 0x10000);
				CopyMemory(Cheat.CIRAM, Cheat.FillRAM, 0x2000);
				/* fall through */
			case IDCANCEL:
                if(cheatSearchHWND)
					DestroyWindow(hDlg);
				else
					EndDialog(hDlg, 0);
				return true;
			default: break;
			}
		}
	case WM_MENUCHAR:
	    // get rid of asterisk sound when pressing non existing menu hotkey - would play when opening
		// with default alt + a hotkey
        SetWindowLong(hDlg, DWLP_MSGRESULT, (MNC_CLOSE << 16));
        return true;
    default: return false;
	}
	return false;
}

INT_PTR CALLBACK DlgCheatSearchAdd(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static struct ICheat* new_cheat;
	int ret=-1;
	switch(msg)
	{
	case WM_INITDIALOG:
		WinRefreshDisplay();
		{
			TCHAR buf [12];
			new_cheat=(struct ICheat*)lParam;
			_stprintf(buf, TEXT("%06X"), new_cheat->address);
			SetDlgItemText(hDlg, IDC_NC_ADDRESS, buf);
			switch(new_cheat->format)
			{
			default:
			case 1://UNSIGNED
				memset(buf,0,sizeof(TCHAR) * 12);
				_stprintf(buf, TEXT("%u"), new_cheat->new_val);
				SetDlgItemText(hDlg, IDC_NC_CURRVAL, buf);
                SetDlgItemText(hDlg, IDC_NC_NEWVAL, buf);
				memset(buf,0,sizeof(TCHAR) * 12);
				_stprintf(buf, TEXT("%u"), new_cheat->saved_val);
				SetDlgItemText(hDlg, IDC_NC_PREVVAL, buf);
				SetWindowLongPtr(GetDlgItem(hDlg, IDC_NC_NEWVAL), GWL_STYLE, ES_NUMBER |GetWindowLongPtr(GetDlgItem(hDlg, IDC_NC_NEWVAL),GWL_STYLE));
				SetWindowLongPtr(GetDlgItem(hDlg, IDC_NC_CURRVAL), GWL_STYLE, ES_NUMBER |GetWindowLongPtr(GetDlgItem(hDlg, IDC_NC_CURRVAL),GWL_STYLE));
				SetWindowLongPtr(GetDlgItem(hDlg, IDC_NC_PREVVAL), GWL_STYLE, ES_NUMBER |GetWindowLongPtr(GetDlgItem(hDlg, IDC_NC_PREVVAL),GWL_STYLE));
				if(new_cheat->size==1)
				{
					SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, 3, 0);
					SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, 3, 0);
					SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, 3, 0);

				}
				if(new_cheat->size==2)
				{
					SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, 5, 0);
					SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, 5, 0);
					SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, 5, 0);

				}
				if(new_cheat->size==3)
				{
					SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, 8, 0);
					SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, 8, 0);
					SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, 8, 0);

				}
				if(new_cheat->size==4)
				{
					SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, 10, 0);
					SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, 10, 0);
					SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, 10, 0);

				}
				break;
			case 3:
				{
					TCHAR formatstring[10];
					_stprintf(formatstring, TEXT("%%%02dX"),new_cheat->size*2);
					memset(buf,0,sizeof(TCHAR) * 12);
					_stprintf(buf, formatstring, new_cheat->new_val);
					SetDlgItemText(hDlg, IDC_NC_CURRVAL, buf);
					memset(buf,0,sizeof(TCHAR) * 12);
					_stprintf(buf, formatstring, new_cheat->saved_val);
					SetDlgItemText(hDlg, IDC_NC_PREVVAL, buf);
					SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, new_cheat->size*2, 0);
					SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, new_cheat->size*2, 0);
					SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, new_cheat->size*2, 0);

				}
				break; //HEX
			case 2:
			memset(buf,0,sizeof(TCHAR) * 12);
			_stprintf(buf, TEXT("%d"), new_cheat->new_val);
			SetDlgItemText(hDlg, IDC_NC_CURRVAL, buf);
			memset(buf,0,sizeof(TCHAR) * 12);
			_stprintf(buf, TEXT("%d"), new_cheat->saved_val);
			SetDlgItemText(hDlg, IDC_NC_PREVVAL, buf);
			if(new_cheat->size==1)
			{
				//-128
				SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, 4, 0);
				SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, 4, 0);
				SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, 4, 0);
			}
			if(new_cheat->size==2)
			{
				//-32768
				SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, 6, 0);
				SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, 6, 0);
				SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, 6, 0);
			}
			if(new_cheat->size==3)
			{
				//-8388608
				SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, 8, 0);
				SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, 8, 0);
				SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, 8, 0);
			}
			if(new_cheat->size==4)
			{
				//-2147483648
				SendDlgItemMessage(hDlg, IDC_NC_CURRVAL,EM_SETLIMITTEXT, 11, 0);
				SendDlgItemMessage(hDlg, IDC_NC_PREVVAL,EM_SETLIMITTEXT, 11, 0);
				SendDlgItemMessage(hDlg, IDC_NC_NEWVAL,EM_SETLIMITTEXT, 11, 0);
			}
			break;
			}
		}
			return true;
		case WM_PAINT:
		{
		PAINTSTRUCT ps;
		BeginPaint (hDlg, &ps);

		EndPaint (hDlg, &ps);
		}
		return true;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					int ret = 0;
					TCHAR buf[23];
					int temp=new_cheat->size;
					S9xCheatDataSize tmp = S9X_8_BITS;
					memset(new_cheat, 0, sizeof(struct SCheat));
					new_cheat->size=temp;
					GetDlgItemText(hDlg, IDC_NC_ADDRESS, buf, 7);
					ScanAddress(buf, new_cheat->address);

					if(temp==1)
						tmp=S9X_8_BITS;
					if(temp==2)
						tmp=S9X_16_BITS;
					if(temp==3)
						tmp=S9X_24_BITS;
					if(temp==4)
						tmp=S9X_32_BITS;


					if(0!=GetDlgItemText(hDlg, IDC_NC_NEWVAL, buf, 12))
					{
						if(new_cheat->format==2)
							ret=_stscanf(buf, TEXT("%d"), &new_cheat->new_val);
						else if(new_cheat->format==1)
							ret=_stscanf(buf, TEXT("%u"), &new_cheat->new_val);
						else if(new_cheat->format==3)
							ret=_stscanf(buf, TEXT("%x"), &new_cheat->new_val);

						if(ret!=1 || !TestRange(new_cheat->format, tmp, new_cheat->new_val))
						{
							MessageBox(hDlg, SEARCH_ERR_INVALIDNEWVALUE, SEARCH_TITLE_RANGEERROR, MB_OK);
							return true;
						}


						if(0==GetDlgItemText(hDlg, IDC_NC_CURRVAL, buf, 12))
							new_cheat->saved=FALSE;
						else
						{
							int i;
							if(new_cheat->format==2)
								ret=_stscanf(buf, TEXT("%d"), &i);
							else if(new_cheat->format==1)
								ret=_stscanf(buf, TEXT("%u"), &i);
							else if(new_cheat->format==3)
								ret=_stscanf(buf, TEXT("%x"), &i);

							if(ret!=1 || !TestRange(new_cheat->format, tmp, i))
							{
								MessageBox(hDlg, SEARCH_ERR_INVALIDCURVALUE, SEARCH_TITLE_RANGEERROR, MB_OK);
								return true;
							}


							new_cheat->saved_val=i;
							new_cheat->saved=TRUE;
						}
						TCHAR tempBuf[22];
						GetDlgItemText(hDlg, IDC_NC_DESC, tempBuf, 22);
						strncpy(new_cheat->name,_tToChar(tempBuf),22);

						std::string code_string;
						char code[10];
						for (int byteIndex = 0; byteIndex < new_cheat->size; byteIndex++)
						{
							if (byteIndex > 0)
								code_string += '+';
							snprintf(code, 10, "%x=%x", new_cheat->address + byteIndex, (new_cheat->new_val >> (8 * byteIndex)) & 0xFF);
							code_string += code;
						}

						int index = S9xAddCheatGroup(new_cheat->name, code_string.c_str());
						if(index >= 0)
							S9xEnableCheatGroup(index);

						ret=0;
					}
                    else
                    {
                        MessageBox(hDlg, SEARCH_ERR_INVALIDNEWVALUE, SEARCH_TITLE_RANGEERROR, MB_OK);
                        return true;
                    }
				}

			case IDCANCEL:
				EndDialog(hDlg, ret);
				return true;
			default: break;
			}
		}
	default: return false;
	}
}

static void set_movieinfo(const TCHAR* path, HWND hDlg)
{
	MovieInfo m;
	int i;
	int getInfoResult=FILE_NOT_FOUND;

	if(lstrlen(path))
		getInfoResult = S9xMovieGetInfo(_tToChar(path), &m);

	if(getInfoResult!=FILE_NOT_FOUND)
	{
		TCHAR* p;
		TCHAR tmpstr[128];
		lstrcpyn(tmpstr, _tctime(&m.TimeCreated), 127);
		tmpstr[127]=TEXT('\0');
		if((p=_tcsrchr(tmpstr, TEXT('\n'))))
			*p=TEXT('\0');
		SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_DATE), tmpstr);

		uint32 div = Memory.ROMFramesPerSecond;
		if(!div) div = 60;
		uint32 l=(m.LengthFrames+(div>>1))/div;
		uint32 seconds=l%60;
		l/=60;
		uint32 minutes=l%60;
		l/=60;
		uint32 hours=l%60;
		_stprintf(tmpstr, TEXT("%02d:%02d:%02d"), hours, minutes, seconds);
		SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_LENGTH), tmpstr);
		_stprintf(tmpstr, TEXT("%u"), m.LengthFrames);
		SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_FRAMES), tmpstr);
		_stprintf(tmpstr, TEXT("%u"), m.RerecordCount);
		SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_RERECORD), tmpstr);
	}
	else
	{
		SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_DATE), TEXT(""));
		SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_LENGTH), TEXT(""));
		SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_FRAMES), TEXT(""));
		SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_RERECORD), TEXT(""));
	}

	if(getInfoResult==SUCCESS)
	{
		// set author comment:
		{
///			SetWindowTextW(GetDlgItem(hDlg, IDC_MOVIE_METADATA), m.Metadata); // won't work, because of & symbol

			wchar_t metadata [MOVIE_MAX_METADATA];
			int j, pos = 0, len = wcslen(m.Metadata);
			for (j = 0; j < len ; j++)
			{
				wchar_t c = m.Metadata [j];
				metadata [pos++] = c;

				// & is a special character in Windows fields,
				// so we have to change & to && when copying over the game title
				// otherwise "Pocky & Rocky" will show up as "Pocky  Rocky", for example
				if(c == (wchar_t)'&')
					metadata [pos++] = (wchar_t)'&';
			}
			metadata [pos] = (wchar_t)'\0';

			SetWindowTextW(GetDlgItem(hDlg, IDC_MOVIE_METADATA), metadata);
		}
		SetWindowText(GetDlgItem(hDlg, IDC_LABEL_MOVIEINFOBOX), MOVIE_LABEL_AUTHORINFO);

		if(m.ReadOnly)
		{
			EnableWindow(GetDlgItem(hDlg, IDC_READONLY), FALSE);
			SendDlgItemMessage(hDlg,IDC_READONLY,BM_SETCHECK,BST_CHECKED,0);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_READONLY), TRUE);
///			SendDlgItemMessage(hDlg,IDC_READONLY,BM_SETCHECK,BST_UNCHECKED,0);
		}
		EnableWindow(GetDlgItem(hDlg, IDC_DISPLAY_INPUT), TRUE);

		for(i=0; i<5; ++i)
		{
			SendDlgItemMessage(hDlg,IDC_JOY1+i,BM_SETCHECK,(m.ControllersMask & (1<<i)) ? BST_CHECKED : BST_UNCHECKED,0);
		}

		if(m.Opts & MOVIE_OPT_FROM_RESET)
		{
			SendDlgItemMessage(hDlg,IDC_RECORD_NOW,BM_SETCHECK,BST_UNCHECKED,0);
			SendDlgItemMessage(hDlg,IDC_RECORD_RESET,BM_SETCHECK,BST_CHECKED,0);
		}
		else
		{
			SendDlgItemMessage(hDlg,IDC_RECORD_RESET,BM_SETCHECK,BST_UNCHECKED,0);
			SendDlgItemMessage(hDlg,IDC_RECORD_NOW,BM_SETCHECK,BST_CHECKED,0);
		}


//		if(m.SyncFlags & MOVIE_SYNC_DATA_EXISTS)
		{
	//		SetWindowText(GetDlgItem(hDlg, IDC_LOADEDFROMMOVIE), _T(MOVIE_LABEL_SYNC_DATA_FROM_MOVIE));
		}

		{
			TCHAR str [256];

			if(m.SyncFlags & MOVIE_SYNC_HASROMINFO)
			{
				_stprintf(str, MOVIE_INFO_MOVIEROM MOVIE_INFO_ROMINFO, m.ROMCRC32, (TCHAR *)_tFromChar(m.ROMName));
				SetWindowText(GetDlgItem(hDlg, IDC_MOVIEROMINFO), str);
			}
			else
			{
				_stprintf(str, MOVIE_INFO_MOVIEROM MOVIE_INFO_ROMNOTSTORED);
				SetWindowText(GetDlgItem(hDlg, IDC_MOVIEROMINFO), str);
			}

			bool mismatch = (m.SyncFlags & MOVIE_SYNC_HASROMINFO) && m.ROMCRC32 != Memory.ROMCRC32;
			_stprintf(str, MOVIE_INFO_CURRENTROM MOVIE_INFO_ROMINFO TEXT("%s"), Memory.ROMCRC32, (TCHAR *)_tFromChar(Memory.ROMName), mismatch?MOVIE_INFO_MISMATCH:TEXT(""));
			SetWindowText(GetDlgItem(hDlg, IDC_CURRENTROMINFO), str);

			_stprintf(str, TEXT("%s"), mismatch?MOVIE_WARNING_MISMATCH:MOVIE_WARNING_OK);
			SetWindowText(GetDlgItem(hDlg, IDC_PLAYWARN), str);
		}

		EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
	}
	else
	{
		// get the path of (where we think) the movie is
		TCHAR tempPathStr [512];
		lstrcpyn(tempPathStr, path, 512);
		tempPathStr[511] = TEXT('\0');
		TCHAR* slash = _tcsrchr(tempPathStr, TEXT('\\'));
		slash = max(slash, _tcsrchr(tempPathStr, TEXT('/')));
		if(slash) *slash = TEXT('\0');
		TCHAR tempDirStr [512];
		TCHAR dirStr [768];
		_tfullpath(tempDirStr, tempPathStr, 512);
		TCHAR* documeStr = _tcsstr(tempDirStr, TEXT("Documents and Settings"));
		if(documeStr) { // abbreviate
			lstrcpy(documeStr, documeStr+14);
			lstrcpyn(documeStr, TEXT("docume~1"), 8);
		}
		_stprintf(dirStr, MOVIE_INFO_DIRECTORY, tempDirStr);

		switch(getInfoResult)
		{
			default:
				SetWindowText(GetDlgItem(hDlg, IDC_PLAYWARN), TEXT(""));
				SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_METADATA), TEXT(""));
				break;
			case FILE_NOT_FOUND:
				SetWindowText(GetDlgItem(hDlg, IDC_PLAYWARN), MOVIE_ERR_NOT_FOUND_SHORT);
				SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_METADATA), MOVIE_ERR_NOT_FOUND);
				break;
			case WRONG_FORMAT:
				SetWindowText(GetDlgItem(hDlg, IDC_PLAYWARN), MOVIE_ERR_WRONG_FORMAT_SHORT);
				SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_METADATA), MOVIE_ERR_WRONG_FORMAT);
				break;
			case WRONG_VERSION:
				SetWindowText(GetDlgItem(hDlg, IDC_PLAYWARN), MOVIE_ERR_WRONG_VERSION_SHORT);
				SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_METADATA), MOVIE_ERR_WRONG_VERSION);
				break;
		}
		SetWindowText(GetDlgItem(hDlg, IDC_LABEL_MOVIEINFOBOX), MOVIE_LABEL_ERRORINFO);

		EnableWindow(GetDlgItem(hDlg, IDC_READONLY), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_DISPLAY_INPUT), FALSE);
		SendDlgItemMessage(hDlg,IDC_READONLY,BM_SETCHECK,BST_UNCHECKED,0);
		SendDlgItemMessage(hDlg,IDC_DISPLAY_INPUT,BM_SETCHECK,BST_UNCHECKED,0);


		{
			SendDlgItemMessage(hDlg,IDC_ALLOWLEFTRIGHT,BM_SETCHECK, Settings.UpAndDown ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg,IDC_SYNC_TO_SOUND_CPU,BM_SETCHECK, Settings.SoundSync ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);
		}

		{
			TCHAR str [256];

			// no movie loaded
			SetWindowText(GetDlgItem(hDlg, IDC_MOVIEROMINFO), dirStr);

			_stprintf(str, MOVIE_INFO_CURRENTROM MOVIE_INFO_ROMINFO, Memory.ROMCRC32, (TCHAR *)_tFromChar(Memory.ROMName));
			SetWindowText(GetDlgItem(hDlg, IDC_CURRENTROMINFO), str);
		}

		EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
	}

}

INT_PTR CALLBACK DlgOpenMovie(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static OpenMovieParams* op=NULL;
	static TCHAR movieDirectory [MAX_PATH];

	switch(msg)
	{
	case WM_INITDIALOG:
		WinRefreshDisplay();
		{
			SetCurrentDirectory(S9xGetDirectoryT(DEFAULT_DIR));
			_tfullpath (movieDirectory, GUI.MovieDir, MAX_PATH);
			_tmkdir(movieDirectory);
			SetCurrentDirectory(movieDirectory);

			op=(OpenMovieParams*)lParam;

			// get default filename
			if(Memory.ROMFilename[0]!='\0')
			{
				static TCHAR filename [_MAX_PATH + 1];
				TCHAR drive [_MAX_DRIVE + 1];
				TCHAR dir [_MAX_DIR + 1];
				TCHAR fname [_MAX_FNAME + 1];
				TCHAR ext [_MAX_EXT + 1];
				_tsplitpath (_tFromChar(Memory.ROMFilename.c_str()), drive, dir, fname, ext);
				_tmakepath (filename, TEXT(""), TEXT(""), fname, TEXT("smv"));
				SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_PATH), filename);
				set_movieinfo(filename, hDlg);
			}
			else
			{
				set_movieinfo(TEXT(""), hDlg);
			}

			SendDlgItemMessage(hDlg,IDC_READONLY,BM_SETCHECK,GUI.MovieReadOnly ? BST_CHECKED : BST_UNCHECKED,0);

			SetDlgItemText(hDlg,IDC_LABEL_STARTSETTINGS, MOVIE_LABEL_STARTSETTINGS);
			SetDlgItemText(hDlg,IDC_LABEL_CONTROLLERSETTINGS, MOVIE_LABEL_CONTSETTINGS);
			SetDlgItemText(hDlg,IDC_LABEL_SYNCSETTINGS, MOVIE_LABEL_SYNCSETTINGS);
		}
		return true;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BROWSE_MOVIE:
				{
					OPENFILENAME  ofn;
					TCHAR  szFileName[MAX_PATH];

					szFileName[0] = TEXT('\0');

					memset( (LPVOID)&ofn, 0, sizeof(OPENFILENAME) );
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFilter = MOVIE_FILETYPE_DESCRIPTION TEXT("\0*.smv\0") FILE_INFO_ANY_FILE_TYPE TEXT("\0*.*\0\0");
					ofn.lpstrFile = szFileName;
					ofn.lpstrDefExt = TEXT("smv");
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST; // hide previously-ignored read-only checkbox (the real read-only box is in the open-movie dialog itself)
					ofn.lpstrInitialDir = movieDirectory;
					if(GetOpenFileName( &ofn ))
					{
						SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_PATH), szFileName);

						if(!GUI.LockDirectories)
						{
							TCHAR temp [256];
							GetCurrentDirectory(MAX_PATH, temp);
							absToRel(GUI.MovieDir, temp, S9xGetDirectoryT(DEFAULT_DIR));
						}

						set_movieinfo(szFileName, hDlg);
					}
					SetCurrentDirectory(movieDirectory);
				}
				return true;

			case IDC_MOVIE_PATH:
				{
					TCHAR  szFileName[MAX_PATH];
					GetWindowText(GetDlgItem(hDlg, IDC_MOVIE_PATH), szFileName, MAX_PATH);
					set_movieinfo(szFileName, hDlg);
				}
				break;

			case IDOK:
				{
					if(BST_CHECKED==SendDlgItemMessage(hDlg, IDC_READONLY, BM_GETCHECK,0,0))
					{
						op->ReadOnly=TRUE;
						GUI.MovieReadOnly=TRUE;
					}
					else
						GUI.MovieReadOnly=FALSE;
					if(BST_CHECKED==SendDlgItemMessage(hDlg, IDC_DISPLAY_INPUT, BM_GETCHECK,0,0))
						op->DisplayInput=TRUE;
					GetDlgItemText(hDlg, IDC_MOVIE_PATH, op->Path, MAX_PATH);
					SetCurrentDirectory(movieDirectory);
				}
				EndDialog(hDlg, 1);
				return true;

			case IDCANCEL:
				EndDialog(hDlg, 0);
				return true;

			default:
				break;
			}
		}

	default:
		return false;
	}
}

// checks if the currently loaded ROM has an SRAM file in the saves directory that we have write access to
static bool existsSRAM ()
{
  return(!access(S9xGetFilename(".srm", SRAM_DIR).c_str(), R_OK | W_OK));
}

INT_PTR CALLBACK DlgCreateMovie(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static OpenMovieParams* op=NULL;
	static TCHAR movieDirectory [MAX_PATH];

	switch(msg)
	{
	case WM_INITDIALOG:
		{
			WinRefreshDisplay();
			SetCurrentDirectory(S9xGetDirectoryT(DEFAULT_DIR));
			_tfullpath (movieDirectory, GUI.MovieDir, MAX_PATH);
			_tmkdir(movieDirectory);
			SetCurrentDirectory(movieDirectory);

			// have to save here or the SRAM file might not exist when we check for it
			// (which would cause clear SRAM option to not work)
			Memory.SaveSRAM(S9xGetFilename (".srm", SRAM_DIR).c_str());


			op=(OpenMovieParams*)lParam;

			SendDlgItemMessage(hDlg,IDC_RECORD_RESET,BM_SETCHECK,BST_UNCHECKED,0);

			int i;
			for(i=1; i<5; ++i)
			{
				SendDlgItemMessage(hDlg,IDC_JOY1+i,BM_SETCHECK,BST_UNCHECKED,0);
			}
			SendDlgItemMessage(hDlg,IDC_JOY1,BM_SETCHECK,BST_CHECKED,0);

			// get default filename
			if(Memory.ROMFilename[0]!='\0')
			{
				static TCHAR filename [_MAX_PATH + 1];
				TCHAR drive [_MAX_DRIVE + 1];
				TCHAR dir [_MAX_DIR + 1];
				TCHAR fname [_MAX_FNAME + 1];
				TCHAR ext [_MAX_EXT + 1];
				_tsplitpath (_tFromChar(Memory.ROMFilename.c_str()), drive, dir, fname, ext);
				_tmakepath (filename, TEXT(""), TEXT(""), fname, TEXT("smv"));
				SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_PATH), filename);
			}

			//EnableWindow(GetDlgItem(hDlg, IDC_SYNC_TO_SOUND_CPU), Settings.SoundDriver<1||Settings.SoundDriver>3); // can't sync sound to CPU unless using "Snes9x DirectSound" driver

			SendDlgItemMessage(hDlg,IDC_RECORD_RESET,BM_SETCHECK, (WPARAM)(GUI.MovieStartFromReset ? BST_CHECKED : BST_UNCHECKED), 0);
			SendDlgItemMessage(hDlg,IDC_RECORD_NOW,BM_SETCHECK, (WPARAM)(GUI.MovieStartFromReset ? BST_UNCHECKED : BST_CHECKED), 0);
			if(existsSRAM())
			{
				EnableWindow(GetDlgItem(hDlg, IDC_CLEARSRAM), GUI.MovieStartFromReset);
				SendDlgItemMessage(hDlg,IDC_CLEARSRAM,BM_SETCHECK, (WPARAM)(GUI.MovieClearSRAM ? BST_CHECKED : BST_UNCHECKED), 0);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_CLEARSRAM), false);
				SendDlgItemMessage(hDlg,IDC_CLEARSRAM,BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}

			SetDlgItemText(hDlg,IDC_LABEL_STARTSETTINGS, MOVIE_LABEL_STARTSETTINGS);
			SetDlgItemText(hDlg,IDC_LABEL_CONTROLLERSETTINGS, MOVIE_LABEL_CONTSETTINGS);
			SetDlgItemText(hDlg,IDC_LABEL_SYNCSETTINGS, MOVIE_LABEL_SYNCSETTINGS);
		}
		return true;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BROWSE_MOVIE:
				{
					OPENFILENAME  ofn;
					TCHAR  szFileName[MAX_PATH];

					szFileName[0] = TEXT('\0');

					memset( (LPVOID)&ofn, 0, sizeof(OPENFILENAME) );
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFilter = MOVIE_FILETYPE_DESCRIPTION TEXT("\0*.smv\0") FILE_INFO_ANY_FILE_TYPE TEXT("\0*.*\0\0");
					ofn.lpstrFile = szFileName;
					ofn.lpstrDefExt = TEXT("smv");
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
					ofn.lpstrInitialDir = movieDirectory;
					if(GetSaveFileName( &ofn ))
					{
						SetWindowText(GetDlgItem(hDlg, IDC_MOVIE_PATH), szFileName);

						if(!GUI.LockDirectories)
						{
							TCHAR temp [256];
							GetCurrentDirectory(MAX_PATH, temp);
							absToRel(GUI.MovieDir, temp, S9xGetDirectoryT(DEFAULT_DIR));
						}
					}
					SetCurrentDirectory(movieDirectory);
				}
				return true;

			case IDOK:
				{
					GetDlgItemText(hDlg, IDC_MOVIE_PATH, op->Path, MAX_PATH);
					GetDlgItemTextW(hDlg, IDC_MOVIE_METADATA, op->Metadata, MOVIE_MAX_METADATA);
					int i;
					for(i=wcslen(op->Metadata); i<32; i++)
						wcscat(op->Metadata, L" ");
					op->ControllersMask=0;
					op->Opts=0;
					for(i=0; i<5; ++i) // TODO: should we even bother with 8-controller recording? right now there are only 5 controller buttons in the dialog, so...
					{
						if(BST_CHECKED==SendDlgItemMessage(hDlg, IDC_JOY1+i, BM_GETCHECK,0,0))
							op->ControllersMask |= (1<<i);
					}
					if(BST_CHECKED==SendDlgItemMessage(hDlg, IDC_RECORD_RESET, BM_GETCHECK,0,0))
					{
						op->Opts |= MOVIE_OPT_FROM_RESET;
						GUI.MovieStartFromReset = TRUE;
					}
					else
						GUI.MovieStartFromReset = FALSE;

					op->SyncFlags = MOVIE_SYNC_DATA_EXISTS | MOVIE_SYNC_HASROMINFO;

					if(IsDlgButtonChecked(hDlg, IDC_CLEARSRAM) && IsDlgButtonChecked(hDlg, IDC_RECORD_RESET) && existsSRAM())
					{
						GUI.MovieClearSRAM = TRUE;
						remove(S9xGetFilename (".srm", SRAM_DIR).c_str()); // delete SRAM if it exists (maybe unnecessary?)
						remove(S9xGetFilename (".srm", ROMFILENAME_DIR).c_str());
						Memory.LoadSRAM(S9xGetFilename (".srm", SRAM_DIR).c_str()); // refresh memory (hard reset)
					}
					else if(!IsDlgButtonChecked(hDlg, IDC_CLEARSRAM) && IsDlgButtonChecked(hDlg, IDC_RECORD_RESET) && existsSRAM())
					{
						GUI.MovieClearSRAM = FALSE;
					}
					SetCurrentDirectory(movieDirectory);
				}
				EndDialog(hDlg, 1);
				return true;

			case IDCANCEL:
				EndDialog(hDlg, 0);
				return true;

			case IDC_RECORD_NOW:
				if(existsSRAM())
				{
					EnableWindow(GetDlgItem(hDlg, IDC_CLEARSRAM), false);
				}
				break;

			case IDC_RECORD_RESET:
				if(existsSRAM())
				{
					EnableWindow(GetDlgItem(hDlg, IDC_CLEARSRAM), true);
				}
				break;

			default:
				break;
			}
		}

	default:
		return false;
	}
}



// MYO
void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2)
{
	return;
}

const char * S9xStringInput(const char *msg)
{
	return NULL;
}

void S9xToggleSoundChannel (int c)
{
	if (c == 8)
		GUI.SoundChannelEnable = 255;
    else
		GUI.SoundChannelEnable ^= 1 << c;

	S9xSetSoundControl(GUI.SoundChannelEnable);
}

bool S9xPollButton(uint32 id, bool *pressed){
	if(S9xMoviePlaying())
		return false;

	*pressed = false;

#define CHECK_KEY(controller, button) (!S9xGetState(Joypad[controller].button) || (ToggleJoypadStorage[controller].button && !TurboToggleJoypadStorage[controller].button) || (IPPU.TotalEmulatedFrames%2 == ToggleJoypadStorage[controller].button && TurboToggleJoypadStorage[controller].button))

	extern bool S9xGetState (WORD KeyIdent);
	if (id & k_MO)	// mouse
	{
		switch (id & 0xFF)
		{
			case 0: *pressed = GUI.MouseButtons & 1 /* Left */ || ((id & k_C1) && (CHECK_KEY(0,A) || CHECK_KEY(0,L))) || ((id & k_C2) && (CHECK_KEY(1,A) || CHECK_KEY(1,L))); break;
			case 1: *pressed = GUI.MouseButtons & 2 /* Right */ || ((id & k_C1) && (CHECK_KEY(0,B) || CHECK_KEY(0,R))) || ((id & k_C2) && (CHECK_KEY(1,B) || CHECK_KEY(1,R))); break;
		}
	}
	else
	if (id & k_SS)	// superscope
	{
		switch (id & 0xFF)
		{
			case 0:	*pressed = GUI.MouseX <= 0 || GUI.MouseY <= 0 || GUI.MouseX >= IPPU.RenderedScreenWidth || GUI.MouseY >= ((IPPU.RenderedScreenHeight> 256) ? SNES_HEIGHT_EXTENDED<<1 : SNES_HEIGHT_EXTENDED) || CHECK_KEY(1,X); break;
			case 2:	*pressed = (GUI.MouseButtons & 2) /* Right */ || CHECK_KEY(1,B) || CHECK_KEY(1,R) ; break;
			case 3:	*pressed = (GUI.MouseButtons & 4) /* Middle */ || GUI.superscope_turbo || CHECK_KEY(1,Y);	GUI.superscope_turbo=0; GUI.MouseButtons &= ~4; break;
			case 4:	*pressed =                                        GUI.superscope_pause || CHECK_KEY(1,Start) || CHECK_KEY(1,Select);	break;
			case 1:	*pressed = (GUI.MouseButtons & 1) /* Left */ || CHECK_KEY(1,A) || CHECK_KEY(1,L); break;
		}
	}
	else
	if (id & k_LG)	// justifier
	{
		if (id & k_C1)
		{
			switch (id & 0xFF)
			{
				case 0:	*pressed = GUI.MouseX <= 0 || GUI.MouseY <= 0 || GUI.MouseX >= IPPU.RenderedScreenWidth || GUI.MouseY >= ((IPPU.RenderedScreenHeight> 256) ? SNES_HEIGHT_EXTENDED<<1 : SNES_HEIGHT_EXTENDED) || CHECK_KEY(0,X) || CHECK_KEY(0,Start); break;
				case 1:	*pressed = GUI.MouseButtons & 1 /* Left */  || CHECK_KEY(0,A) || CHECK_KEY(0,L); break;
				case 2: *pressed = GUI.MouseButtons & 2 /* Right */  || CHECK_KEY(1,B) || CHECK_KEY(1,R); break;
			}
		}
		else
		{
			switch (id & 0xFF)
			{
				case 0: *pressed = CHECK_KEY(1,Start) /* 2p Start */  || CHECK_KEY(1,X); break;
				case 1:	*pressed = CHECK_KEY(1,A) /* 2p A */ || CHECK_KEY(1,L); break;
				case 2: *pressed = CHECK_KEY(1,B) /* 2p B */ || CHECK_KEY(1,R); break;
			}
		}
	}
	else
	if (id & k_RF)	// macsrifle
	{
		switch (id & 0xFF)
		{
			case 0:	*pressed = (GUI.MouseButtons & 1) /* Left */ || CHECK_KEY(1,A) || CHECK_KEY(1,L); break;
		}
	}

	return (true);
}

// ??? NYI
bool S9xPollAxis(uint32 id, int16 *value){
    return false;
}

bool S9xPollPointer(uint32 id, int16 *x, int16 *y){
	if(S9xMoviePlaying())
		return false;

	if (id & k_PT)
	{
		*x = GUI.MouseX;
		*y = GUI.MouseY;
	}
	else
		*x = *y = 0;
	return (true);
}

// adjusts settings based on ROM that was just loaded
void S9xPostRomInit()
{
	// "Cheats are on" message if cheats are on and active,
	// to make it less likely that someone will think there is some bug because of
	// a lingering cheat they don't realize is on
	if (Settings.ApplyCheats)
	{
		S9xCheatsEnable();
		extern struct SCheatData Cheat;
	    for (uint32 i = 0; i < Cheat.group.size(); i++)
		{
	        if (Cheat.group[i].enabled)
			{
				char String2 [1024];
				sprintf(String2, "(CHEATS ARE ON!) %s", String);
				strncpy(String, String2, 512);
				break;
			}
		}
	}

	if(!S9xMovieActive() && !startingMovie)
	{
		// revert previously forced control
        if(GUI.ControlForced!=0xff) {
			GUI.ControllerOption = GUI.ControlForced;
            ChangeInputDevice();
        }
		int prevController = GUI.ControllerOption;
		GUI.ValidControllerOptions = 0xFFFF;

		// auto-joypad2 creates fast menu flicker
		if (!Settings.DisableGameSpecificHacks)
		{
			if(strncmp(Memory.ROMName, "MAC:Basic Rifle", 15) == 0)
			{
				GUI.ControllerOption = SNES_MACSRIFLE;

				ChangeInputDevice();
			}
		}

		// NSRT controller settings
		if (!strncmp((const char *)Memory.NSRTHeader+24, "NSRT", 4))
		{
			switch(Memory.NSRTHeader[29])
			{
				default: // unknown or unsupported
					break;
				case 0x00: // Gamepad / Gamepad
					GUI.ControllerOption = SNES_JOYPAD;
					GUI.ValidControllerOptions = (1<<SNES_JOYPAD);
					break;
				case 0x10: // Mouse / Gamepad
					GUI.ControllerOption = SNES_MOUSE;
					GUI.ValidControllerOptions = (1<<SNES_MOUSE);
					break;
				case 0x20: // Mouse_or_Gamepad / Gamepad
					if(GUI.ControllerOption == SNES_MOUSE_SWAPPED)
						GUI.ControllerOption = SNES_MOUSE;
					if(GUI.ControllerOption != SNES_MOUSE)
						GUI.ControllerOption = SNES_JOYPAD;
					GUI.ValidControllerOptions = (1<<SNES_JOYPAD) | (1<<SNES_MOUSE);
					break;
				case 0x01: // Gamepad / Mouse
					GUI.ControllerOption = SNES_MOUSE_SWAPPED;
					GUI.ValidControllerOptions = (1<<SNES_MOUSE_SWAPPED);
					break;
				case 0x22: // Mouse_or_Gamepad / Mouse_or_Gamepad
					if(GUI.ControllerOption != SNES_MOUSE && GUI.ControllerOption != SNES_MOUSE_SWAPPED)
						GUI.ControllerOption = SNES_JOYPAD;
					GUI.ValidControllerOptions = (1<<SNES_JOYPAD) | (1<<SNES_MOUSE) | (1<<SNES_MOUSE_SWAPPED);
					break;
				case 0x03: // Gamepad / Superscope
					GUI.ControllerOption = SNES_SUPERSCOPE;
					GUI.ValidControllerOptions = (1<<SNES_SUPERSCOPE);
					break;
				case 0x04: // Gamepad / Gamepad_or_Superscope
					if(GUI.ControllerOption == SNES_JUSTIFIER || GUI.ControllerOption == SNES_JUSTIFIER_2)
						GUI.ControllerOption = SNES_SUPERSCOPE;
					if(GUI.ControllerOption != SNES_SUPERSCOPE)
						GUI.ControllerOption = SNES_JOYPAD;
					GUI.ValidControllerOptions = (1<<SNES_JOYPAD) | (1<<SNES_SUPERSCOPE);
					break;
				case 0x05: // Gamepad / Justifier
					if(GUI.ControllerOption != SNES_JUSTIFIER_2)
						GUI.ControllerOption = SNES_JUSTIFIER;
					GUI.ValidControllerOptions = (1<<SNES_JUSTIFIER) | (1<<SNES_JUSTIFIER_2);
					break;
				case 0x06: // Gamepad / Multitap_or_Gamepad
					GUI.ControllerOption = SNES_MULTIPLAYER5;
					GUI.ValidControllerOptions = (1<<SNES_MULTIPLAYER5) | (1<<SNES_JOYPAD);
					break;
				case 0x66: // Multitap_or_Gamepad / Multitap_or_Gamepad
					GUI.ControllerOption = SNES_MULTIPLAYER8;
					GUI.ValidControllerOptions = (1<<SNES_MULTIPLAYER8) | (1<<SNES_MULTIPLAYER5) | (1<<SNES_JOYPAD);
					break;
				case 0x24: // Gamepad_or_Mouse / Gamepad_or_Superscope
					if(GUI.ControllerOption == SNES_JUSTIFIER || GUI.ControllerOption == SNES_JUSTIFIER_2)
						GUI.ControllerOption = SNES_SUPERSCOPE;
					if(GUI.ControllerOption != SNES_SUPERSCOPE && GUI.ControllerOption != SNES_MOUSE)
						GUI.ControllerOption = SNES_JOYPAD;
					GUI.ValidControllerOptions = (1<<SNES_JOYPAD) | (1<<SNES_MOUSE) | (1<<SNES_SUPERSCOPE);
					break;
				case 0x27: // Gamepad_or_Mouse / Gamepad_or_Mouse_or_Superscope
					if(GUI.ControllerOption == SNES_JUSTIFIER || GUI.ControllerOption == SNES_JUSTIFIER_2)
						GUI.ControllerOption = SNES_SUPERSCOPE;
					if(GUI.ControllerOption != SNES_SUPERSCOPE && GUI.ControllerOption != SNES_MOUSE && GUI.ControllerOption != SNES_MOUSE_SWAPPED)
						GUI.ControllerOption = SNES_JOYPAD;
					GUI.ValidControllerOptions = (1<<SNES_JOYPAD) | (1<<SNES_MOUSE) | (1<<SNES_MOUSE_SWAPPED) | (1<<SNES_SUPERSCOPE);
					break;
				case 0x08: // Gamepad / Mouse_or_Multitap_or_Gamepad
					if(GUI.ControllerOption == SNES_MOUSE)
						GUI.ControllerOption = SNES_MOUSE_SWAPPED;
					if(GUI.ControllerOption == SNES_MULTIPLAYER8)
						GUI.ControllerOption = SNES_MULTIPLAYER5;
					if(GUI.ControllerOption != SNES_MULTIPLAYER5 && GUI.ControllerOption != SNES_MOUSE_SWAPPED)
						GUI.ControllerOption = SNES_JOYPAD;
					GUI.ValidControllerOptions = (1<<SNES_MOUSE_SWAPPED) | (1<<SNES_MULTIPLAYER5) | (1<<SNES_JOYPAD);
					break;
			}
			ChangeInputDevice();
		}

		// update menu and remember what (if anything) the control was forced from
		GUI.ControlForced = prevController;
	}

	// reset fast-forward and other input-related GUI state
	Settings.TurboMode = FALSE;
	GUI.superscope_turbo = 0;
	GUI.superscope_pause = 0;
	GUI.MouseButtons = 0;
	GUI.MouseX = 0;
	GUI.MouseY = 0;
	GUI.TurboMask = 0;
	GUI.FrameAdvanceJustPressed = 0;

	// black out the screen
 	for (uint32 y = 0; y < (uint32)IPPU.RenderedScreenHeight; y++)
		memset(GFX.Screen + y * GFX.RealPPL, 0, GFX.RealPPL*2);
}
