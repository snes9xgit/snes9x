#ifdef HAVE_LUA

#include "../port.h"
#include "_tfwopen.h"
#include "rsrc/resource.h"
#include <assert.h>
#include <process.h>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include "wsnes9x.h"
#endif
#include "lua-engine.h"

bool DemandLua()
{
	HMODULE mod = LoadLibrary(TEXT("lua51.dll"));
	if(!mod)
	{
		MessageBox(NULL, TEXT("lua51.dll was not found. Please get it into your PATH or in the same directory as snes9x.exe"), TEXT("Snes9x"), MB_OK | MB_ICONERROR);
		return false;
	}
	FreeLibrary(mod);
	return true;
}
 
#define g_hWnd GUI.hWnd
#define g_hInstance GUI.hInstance

#define MAX_RECENT_SCRIPTS 15

static TCHAR Str_Tmp [1024]; // shadow added because the global one is completely unreliable

TCHAR Recent_Scripts[MAX_RECENT_SCRIPTS][1024];

struct ControlLayoutInfo
{
	int controlID;
	
	enum LayoutType // what to do when the containing window resizes
	{
		NONE, // leave the control where it was
		RESIZE_END, // resize the control
		MOVE_START, // move the control
	};
	LayoutType horizontalLayout;
	LayoutType verticalLayout;
};
struct ControlLayoutState
{
	int x,y,width,height;
	bool valid;
	ControlLayoutState() : valid(false) {}
};

static ControlLayoutInfo controlLayoutInfos [] = {
	{IDC_LUACONSOLE, ControlLayoutInfo::RESIZE_END, ControlLayoutInfo::RESIZE_END},
	{IDC_EDIT_LUAPATH, ControlLayoutInfo::RESIZE_END, ControlLayoutInfo::NONE},
	{IDC_BUTTON_LUARUN, ControlLayoutInfo::MOVE_START, ControlLayoutInfo::NONE},
	{IDC_BUTTON_LUASTOP, ControlLayoutInfo::MOVE_START, ControlLayoutInfo::NONE},
};
static const int numControlLayoutInfos = sizeof(controlLayoutInfos)/sizeof(*controlLayoutInfos);


extern std::vector<HWND> LuaScriptHWnds;
struct LuaPerWindowInfo {
	std::string filename;
	HANDLE fileWatcherThread;
	bool started;
	bool closeOnStop;
	bool subservient;
	int width; int height;
	ControlLayoutState layoutState [numControlLayoutInfos];
	LuaPerWindowInfo() : fileWatcherThread(NULL), started(false), closeOnStop(false), subservient(false), width(405), height(244) {}
};
std::map<HWND, LuaPerWindowInfo> LuaWindowInfo;
static TCHAR Lua_Dir[1024]=TEXT("");
#undef max
int WINAPI FileSysWatcher (LPVOID arg)
{
	HWND hDlg = (HWND)arg;
	LuaPerWindowInfo& info = LuaWindowInfo[hDlg];

	while(true)
	{
		TCHAR filename [1024], directory [1024];

		lstrcpyn(filename, _tFromChar(info.filename.c_str()), 1024);
		filename[1023] = 0;
		lstrcpy(directory, filename);
		TCHAR* slash = _tcsrchr(directory, TEXT('/'));
		slash = std::max(slash, _tcsrchr(directory, TEXT('\\')));
		if(slash)
			*slash = 0;

		TCHAR* bar = _tcschr(filename, TEXT('|'));
		if(bar) *bar = TEXT('\0');

		WIN32_FILE_ATTRIBUTE_DATA origData;
		GetFileAttributesEx (filename,  GetFileExInfoStandard,  (LPVOID)&origData);

		HANDLE hNotify = FindFirstChangeNotification(directory, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

		if(hNotify)
		{
			DWORD dwWaitResult = WaitForSingleObject(hNotify, 500);

			if(dwWaitResult != STATUS_TIMEOUT)
			{
				if(dwWaitResult == WAIT_ABANDONED)
					return dwWaitResult;

				WIN32_FILE_ATTRIBUTE_DATA data;
				GetFileAttributesEx (filename,  GetFileExInfoStandard,  (LPVOID)&data);

				// at this point it could be any file in the directory that changed
				// so check to make sure it was the file we care about
				if(memcmp(&origData.ftLastWriteTime, &data.ftLastWriteTime, sizeof(FILETIME)))
				{
					RequestAbortLuaScript((int)hDlg, "terminated to reload the script");
					PostMessage(hDlg, WM_COMMAND, IDC_BUTTON_LUARUN, 0);
				}
			}

			//FindNextChangeNotification(hNotify); // let's not try to reuse it...
			FindCloseChangeNotification(hNotify); // but let's at least make sure to release it!
		}
		else
		{
			Sleep(500);
		}
	}

	return 0;
}

void RegisterWatcherThread (HWND hDlg)
{
	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) FileSysWatcher, (LPVOID) hDlg, CREATE_SUSPENDED, NULL);
	SetThreadPriority(thread, THREAD_PRIORITY_LOWEST);

	LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
	info.fileWatcherThread = thread;

	ResumeThread(thread);
}
void KillWatcherThread (HWND hDlg)
{
	LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
	TerminateThread(info.fileWatcherThread, 0);
	info.fileWatcherThread = NULL;
}


// some extensions that might commonly be near lua files that almost certainly aren't lua files.
static const TCHAR* s_nonLuaExtensions [] = {TEXT("txt"), TEXT("nfo"), TEXT("htm"), TEXT("html"), TEXT("jpg"), TEXT("jpeg"), TEXT("png"), TEXT("bmp"), TEXT("gif"), TEXT("mp3"), TEXT("wav"), TEXT("lnk"), TEXT("exe"), TEXT("bat"), TEXT("gmv"), TEXT("gm2"), TEXT("luasav"), TEXT("sav"), TEXT("srm"), TEXT("brm"), TEXT("cfg"), TEXT("wch"), TEXT("gs*"),  TEXT("bin"),TEXT("smd"),TEXT("gen"),TEXT("32x"),TEXT("cue"),TEXT("iso"),TEXT("raw")};


void Update_Recent_Script(const TCHAR* Path, bool dontPutAtTop)
{
//	TCHAR LogicalName[1024], PhysicalName[1024];
//	bool exists = ObtainFile(Path, LogicalName, PhysicalName, TEXT("luacheck"), s_nonLuaExtensions, sizeof(s_nonLuaExtensions)/sizeof(*s_nonLuaExtensions));
//	ReleaseTempFileCategory(TEXT("luacheck")); // delete the temporary (physical) file if any

//	if(!exists)
//		return;

	int i;

	for(i = 0; i < MAX_RECENT_SCRIPTS; i++)
	{
		if (!(_tcscmp(Recent_Scripts[i], Path)))
		{
			// move recent item to the top of the list
			if(i == 0 || dontPutAtTop)
				return;
			TCHAR temp [1024];
			lstrcpy(temp, Recent_Scripts[i]);
			int j;
			for(j = i; j > 0; j--)
				lstrcpy(Recent_Scripts[j], Recent_Scripts[j-1]);
			lstrcpy(Recent_Scripts[0], temp);
//			MustUpdateMenu = 1;
			return;
		}
	}
	
	if(!dontPutAtTop)
	{
		// add to start of recent list
		for(i = MAX_RECENT_SCRIPTS-1; i > 0; i--)
			lstrcpy(Recent_Scripts[i], Recent_Scripts[i - 1]);

		lstrcpy(Recent_Scripts[0], Path);
	}
	else
	{
		// add to end of recent list
		for(i = 0; i < MAX_RECENT_SCRIPTS; i++)
		{
			if(!*Recent_Scripts[i])
			{
				lstrcpy(Recent_Scripts[i], Path);
				break;
			}
		}
	}

//	MustUpdateMenu = 1;
}

HWND IsScriptFileOpen(const TCHAR* Path)
{
	for(std::map<HWND, LuaPerWindowInfo>::iterator iter = LuaWindowInfo.begin(); iter != LuaWindowInfo.end(); ++iter)
	{
		LuaPerWindowInfo& info = iter->second;
		TCHAR filenameString[1024];
		lstrcpyn(filenameString, _tFromChar(info.filename.c_str()), 1024);
		const TCHAR *filename = filenameString;
		const TCHAR* pathPtr = Path;

		// case-insensitive slash-direction-insensitive compare
		bool same = true;
		while(*filename || *pathPtr)
		{
			if((*filename == TEXT('/') || *filename == TEXT('\\')) && (*pathPtr == TEXT('/') || *pathPtr == TEXT('\\')))
			{
				do {filename++;} while(*filename == TEXT('/') || *filename == TEXT('\\'));
				do {pathPtr++;} while(*pathPtr == TEXT('/') || *pathPtr == TEXT('\\'));
			}
			else if(tolower(*filename) != tolower(*pathPtr))
			{
				same = false;
				break;
			}
			else
			{
				filename++;
				pathPtr++;
			}
		}

		if(same)
			return iter->first;
	}
	return NULL;
}


void PrintToWindowConsole(int hDlgAsInt, const char* str)
{
	HWND hDlg = (HWND)hDlgAsInt;
	HWND hConsole = GetDlgItem(hDlg, IDC_LUACONSOLE);

	int length = GetWindowTextLength(hConsole);
	if(length >= 250000)
	{
		// discard first half of text if it's getting too long
		SendMessage(hConsole, EM_SETSEL, 0, length/2);
		SendMessage(hConsole, EM_REPLACESEL, false, (LPARAM)TEXT(""));
		length = GetWindowTextLength(hConsole);
	}
	SendMessage(hConsole, EM_SETSEL, length, length);

	LuaPerWindowInfo& info = LuaWindowInfo[hDlg];

	{
		SendMessage(hConsole, EM_REPLACESEL, false, (LPARAM)((const TCHAR*)_tFromChar(str)));
	}
}

extern int Show_Genesis_Screen(HWND hWnd);
void OnStart(int hDlgAsInt)
{
	HWND hDlg = (HWND)hDlgAsInt;
	LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
	info.started = true;
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_LUABROWSE), false); // disable browse while running because it misbehaves if clicked in a frameadvance loop
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_LUASTOP), true);
	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_LUARUN), TEXT("Restart"));
	SetWindowText(GetDlgItem(hDlg, IDC_LUACONSOLE), TEXT("")); // clear the console
//	Show_Genesis_Screen(HWnd); // otherwise we might never show the first thing the script draws
}

void OnStop(int hDlgAsInt, bool statusOK)
{
	HWND hDlg = (HWND)hDlgAsInt;
	LuaPerWindowInfo& info = LuaWindowInfo[hDlg];

	HWND prevWindow = GetActiveWindow();
	SetActiveWindow(hDlg); // bring to front among other script/secondary windows, since a stopped script will have some message for the user that would be easier to miss otherwise
	if(prevWindow == g_hWnd) SetActiveWindow(prevWindow);

	info.started = false;
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_LUABROWSE), true);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_LUASTOP), false);
	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_LUARUN), TEXT("Run"));
//	if(statusOK)
//		Show_Genesis_Screen(MainWindow->getHWnd()); // otherwise we might never show the last thing the script draws
	if(info.closeOnStop)
		PostMessage(hDlg, WM_CLOSE, 0, 0);
}

const TCHAR* MakeScriptPathAbsolute(const TCHAR* filename, const TCHAR* extraDirToCheck);

void UpdateFileEntered(HWND hDlg)
{
	TCHAR local_str_tmp [1024];
	SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,WM_GETTEXT,(WPARAM)512,(LPARAM)local_str_tmp);

	// if it exists, make sure we're using an absolute path to it
	const TCHAR* filename = local_str_tmp;
	FILE* file = _tfopen(filename, TEXT("rb"));
	if(file)
	{
		fclose(file);
		filename = MakeScriptPathAbsolute(local_str_tmp, NULL);
		if(filename != local_str_tmp && _tcsicmp(filename, local_str_tmp))
		{
			SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,WM_SETTEXT,(WPARAM)512,(LPARAM)filename);
			SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,EM_SETSEL,0,-1);
			SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,EM_SETSEL,-1,-1);
			return;
		}
	}

	// use ObtainFile to support opening files within archives
//	TCHAR LogicalName[1024], PhysicalName[1024];
//	bool exists = ObtainFile(filename, LogicalName, PhysicalName, TEXT("luacheck"), s_nonLuaExtensions, sizeof(s_nonLuaExtensions)/sizeof(*s_nonLuaExtensions));
//	bool readonly = exists ? ((GetFileAttributes(PhysicalName) & FILE_ATTRIBUTE_READONLY) != 0) : (strchr(LogicalName, TEXT('|')) != NULL || strchr(filename, TEXT('|')) != NULL);
//	ReleaseTempFileCategory(TEXT("luacheck")); // delete the temporary (physical) file if any

	bool exists = true;
	bool readonly = false;
	TCHAR LogicalName[1024];
	_sntprintf(LogicalName, MAX_PATH, TEXT("%s"), filename);

	if(exists)
	{
		LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
		info.filename = _tToChar(LogicalName);

		TCHAR* slash = _tcsrchr(LogicalName, TEXT('/'));
		slash = std::max(slash, _tcsrchr(LogicalName, TEXT('\\')));
		if(slash)
			slash++;
		else
			slash = LogicalName;
		SetWindowText(hDlg, slash);
//		Build_Main_Menu();

		PostMessage(hDlg, WM_COMMAND, IDC_BUTTON_LUARUN, 0);
	}

	const TCHAR* ext = _tcsrchr(LogicalName, TEXT('.'));
	bool isLuaFile = ext && !_tcsicmp(ext, TEXT(".lua"));
	if(exists)
	{
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_LUAEDIT), isLuaFile ? (readonly ? TEXT("View") : TEXT("Edit")) : TEXT("Open"));
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_LUAEDIT), true);
	}
	else
	{
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_LUAEDIT), TEXT("Create"));
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_LUAEDIT), isLuaFile && !readonly);
	}
}

//extern TEXT("C") int Clear_Sound_Buffer(void);

static int Change_File_L(TCHAR *Dest, TCHAR *Dir, TCHAR *Titre, TCHAR *Filter, TCHAR *Ext, HWND hwnd)
{
	OPENFILENAME ofn;

//	SetCurrentDirectory(Emu_Path);

	if (!_tcscmp(Dest, TEXT("")))
	{
		lstrcpy(Dest, TEXT("default."));
		lstrcat(Dest, Ext);
	}

	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = g_hInstance;
	ofn.lpstrFile = Dest;
	ofn.nMaxFile = 2047;
	ofn.lpstrFilter = Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = Dir;
	ofn.lpstrTitle = Titre;
	ofn.lpstrDefExt = Ext;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn)) return 1;

	return 0;
}

LRESULT CALLBACK LuaScriptProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	RECT r2;
	int dx1, dy1, dx2, dy2;

	switch(uMsg)
	{
		case WM_INITDIALOG: {
			if(std::find(LuaScriptHWnds.begin(), LuaScriptHWnds.end(), hDlg) == LuaScriptHWnds.end())
			{
				LuaScriptHWnds.push_back(hDlg);
//				Build_Main_Menu();
			}
//			if (Full_Screen)
//			{
//				while (ShowCursor(false) >= 0);
//				while (ShowCursor(true) < 0);
//			}

//			HANDLE hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LUA));
//			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

			// remove the 30000 character limit from the console control
			SendMessage(GetDlgItem(hDlg, IDC_LUACONSOLE),EM_LIMITTEXT,0,0);

			GetWindowRect(g_hWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			int windowIndex = std::find(LuaScriptHWnds.begin(), LuaScriptHWnds.end(), hDlg) - LuaScriptHWnds.begin();
			int staggerOffset = windowIndex * 24;
			r.left += staggerOffset;
			r.right += staggerOffset;
			r.top += staggerOffset;
			r.bottom += staggerOffset;

			// push it away from the main window if we can
			const int width = (r.right-r.left); 
			const int width2 = (r2.right-r2.left); 
			if(r.left+width2 + width < GetSystemMetrics(SM_CXSCREEN))
			{
				r.right += width;
				r.left += width;
			}
			else if((int)r.left - (int)width2 > 0)
			{
				r.right -= width2;
				r.left -= width2;
			}

			SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

			LuaPerWindowInfo info;
			{
				RECT r3;
				GetClientRect(hDlg, &r3);
				info.width = r3.right - r3.left;
				info.height = r3.bottom - r3.top;
			}
			LuaWindowInfo[hDlg] = info;
			RegisterWatcherThread(hDlg);

			OpenLuaContext((int)hDlg, PrintToWindowConsole, OnStart, OnStop);

			DragAcceptFiles(hDlg, TRUE);

			return true;
		}	break;

		case WM_MENUSELECT:
 		case WM_ENTERSIZEMOVE:
//			Clear_Sound_Buffer();
			break;

		case WM_SIZING:
			{
				// enforce a minimum window size

				LPRECT r = (LPRECT) lParam;
				int minimumWidth = 333;
				int minimumHeight = 117;
				if(r->right - r->left < minimumWidth)
					if(wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
						r->left = r->right - minimumWidth;
					else
						r->right = r->left + minimumWidth;
				if(r->bottom - r->top < minimumHeight)
					if(wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
						r->top = r->bottom - minimumHeight;
					else
						r->bottom = r->top + minimumHeight;
			}
			return TRUE;

		case WM_SIZE:
			{
				// resize or move controls in the window as necessary when the window is resized

				LuaPerWindowInfo& windowInfo = LuaWindowInfo[hDlg];
				int prevDlgWidth = windowInfo.width;
				int prevDlgHeight = windowInfo.height;

				int dlgWidth = LOWORD(lParam);
				int dlgHeight = HIWORD(lParam);

				int deltaWidth = dlgWidth - prevDlgWidth;
				int deltaHeight = dlgHeight - prevDlgHeight;

				for(int i = 0; i < numControlLayoutInfos; i++)
				{
					ControlLayoutInfo layoutInfo = controlLayoutInfos[i];
					ControlLayoutState& layoutState = windowInfo.layoutState[i];

					HWND hCtrl = GetDlgItem(hDlg,layoutInfo.controlID);

					int x,y,width,height;
					if(layoutState.valid)
					{
						x = layoutState.x;
						y = layoutState.y;
						width = layoutState.width;
						height = layoutState.height;
					}
					else
					{
						RECT r;
						GetWindowRect(hCtrl, &r);
						POINT p = {r.left, r.top};
						ScreenToClient(hDlg, &p);
						x = p.x;
						y = p.y;
						width = r.right - r.left;
						height = r.bottom - r.top;
					}

					switch(layoutInfo.horizontalLayout)
					{
						case ControlLayoutInfo::RESIZE_END: width += deltaWidth; break;
						case ControlLayoutInfo::MOVE_START: x += deltaWidth; break;
						default: break;
					}
					switch(layoutInfo.verticalLayout)
					{
						case ControlLayoutInfo::RESIZE_END: height += deltaHeight; break;
						case ControlLayoutInfo::MOVE_START: y += deltaHeight; break;
						default: break;
					}

					SetWindowPos(hCtrl, 0, x,y, width,height, 0);

					layoutState.x = x;
					layoutState.y = y;
					layoutState.width = width;
					layoutState.height = height;
					layoutState.valid = true;
				}

				windowInfo.width = dlgWidth;
				windowInfo.height = dlgHeight;

				RedrawWindow(hDlg, NULL, NULL, RDW_INVALIDATE);
			}
			break;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_BUTTON_LUABROWSE:
				{
					LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
					TCHAR Str_Tmp [1024]; // shadow added because the global one is unreliable
					lstrcpy(Str_Tmp,_tFromChar(info.filename.c_str()));
					SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,WM_GETTEXT,(WPARAM)512,(LPARAM)Str_Tmp);
					TCHAR* bar = _tcschr(Str_Tmp, TEXT('|'));
					if(bar) *bar = TEXT('\0');
//					DialogsOpen++;
//					Clear_Sound_Buffer();
					if(Change_File_L(Str_Tmp, Lua_Dir, TEXT("Load Lua Script"), TEXT("Lua Script\0*.lua*\0All Files\0*.*\0\0"), TEXT("lua"), hDlg))
					{
						SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,WM_SETTEXT,0,(LPARAM)Str_Tmp);
					}
//					DialogsOpen--;

				}	break;
				case IDC_BUTTON_LUAEDIT:
				{
					LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
					TCHAR Str_Tmp [1024]; // shadow added because the global one is unreliable
					lstrcpy(Str_Tmp,_tFromChar(info.filename.c_str()));
					SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,WM_GETTEXT,(WPARAM)512,(LPARAM)Str_Tmp);
					TCHAR LogicalName[1024], PhysicalName[1024];
//					bool exists = ObtainFile(Str_Tmp, LogicalName, PhysicalName, TEXT("luaview"), s_nonLuaExtensions, sizeof(s_nonLuaExtensions)/sizeof(*s_nonLuaExtensions));
					_sntprintf(LogicalName, MAX_PATH, TEXT("%s"), Str_Tmp);
					lstrcpy(PhysicalName, Str_Tmp);
					bool exists = true;
					bool created = false;
					if(!exists)
					{
						FILE* file = _tfopen(Str_Tmp, TEXT("r"));
						if(!file)
						{
							file = _tfopen(Str_Tmp, TEXT("w"));
							if(file)
							{
								created = true;
								exists = true;
								lstrcpy(PhysicalName, Str_Tmp);
							}
						}
						if(file)
							fclose(file);
					}
					if(exists)
					{
						// tell the OS to open the file with its associated editor,
						// without blocking on it or leaving a command window open.
						if((int)ShellExecute(NULL, TEXT("edit"), PhysicalName, NULL, NULL, SW_SHOWNORMAL) == SE_ERR_NOASSOC)
							if((int)ShellExecute(NULL, TEXT("open"), PhysicalName, NULL, NULL, SW_SHOWNORMAL) == SE_ERR_NOASSOC)
								ShellExecute(NULL, NULL, TEXT("notepad"), PhysicalName, NULL, SW_SHOWNORMAL);
					}
					if(created)
					{
						UpdateFileEntered(hDlg);
					}
				}	break;
				case IDC_EDIT_LUAPATH:
				{
					switch(HIWORD(wParam))
					{
						case EN_CHANGE:
						{
							UpdateFileEntered(hDlg);
						}	break;
					}
				}	break;
				case IDC_BUTTON_LUARUN:
				{
					HWND focus = GetFocus();
					HWND textbox = GetDlgItem(hDlg, IDC_EDIT_LUAPATH);
					if(focus != textbox)
						SetActiveWindow(g_hWnd);

					LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
					lstrcpy(Str_Tmp,_tFromChar(info.filename.c_str()));
					TCHAR LogicalName[1024], PhysicalName[1024];
//					bool exists = ObtainFile(Str_Tmp, LogicalName, PhysicalName, TEXT("luarun"), s_nonLuaExtensions, sizeof(s_nonLuaExtensions)/sizeof(*s_nonLuaExtensions));
					_sntprintf(LogicalName, MAX_PATH, TEXT("%s"), Str_Tmp);
					_sntprintf(PhysicalName, MAX_PATH, TEXT("%s"), Str_Tmp);
					Update_Recent_Script(LogicalName, info.subservient);
					if(DemandLua())
						RunLuaScriptFile((int)hDlg, _tToChar(PhysicalName));
				}	break;
				case IDC_BUTTON_LUASTOP:
				{
					PrintToWindowConsole((int)hDlg, "user clicked stop button\r\n");
					SetActiveWindow(g_hWnd);
					if(DemandLua())
						StopLuaScript((int)hDlg);
				}	break;
				case IDC_NOTIFY_SUBSERVIENT:
				{
					LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
					info.subservient = lParam ? true : false;
				}	break;
				//case IDOK:
				case IDCANCEL:
				{	LuaPerWindowInfo& info = LuaWindowInfo[hDlg];
					if(info.filename.empty())
					{
//						if (Full_Screen)
//						{
//							while (ShowCursor(true) < 0);
//							while (ShowCursor(false) >= 0);
//						}
//						DialogsOpen--;
						DragAcceptFiles(hDlg, FALSE);
						KillWatcherThread(hDlg);
						LuaScriptHWnds.erase(std::remove(LuaScriptHWnds.begin(), LuaScriptHWnds.end(), hDlg), LuaScriptHWnds.end());
						LuaWindowInfo.erase(hDlg);
						CloseLuaContext((int)hDlg);
//						Build_Main_Menu();
						EndDialog(hDlg, true);
					}
				}	return true;
			}

			return false;
		}	break;

		case WM_CLOSE:
		{
			LuaPerWindowInfo& info = LuaWindowInfo[hDlg];

			PrintToWindowConsole((int)hDlg, "user closed script window\r\n");
			StopLuaScript((int)hDlg);
			if(info.started)
			{
				// not stopped yet, wait to close until we are, otherwise we'll crash
				info.closeOnStop = true;
				return false;
			}

//			if (Full_Screen)
//			{
//				while (ShowCursor(true) < 0);
//				while (ShowCursor(false) >= 0);
//			}
//			DialogsOpen--;
			DragAcceptFiles(hDlg, FALSE);
			KillWatcherThread(hDlg);
			LuaScriptHWnds.erase(std::remove(LuaScriptHWnds.begin(), LuaScriptHWnds.end(), hDlg), LuaScriptHWnds.end());
			LuaWindowInfo.erase(hDlg);
			CloseLuaContext((int)hDlg);
//			Build_Main_Menu();
			EndDialog(hDlg, true);
		}	return true;

		case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, Str_Tmp, 1024);
			DragFinish(hDrop);
			SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,WM_SETTEXT,0,(LPARAM)Str_Tmp );
			UpdateFileEntered(hDlg);
		}	return true;

	}

	return false;
}

TCHAR Emu_Path [1024];

static const TCHAR* PathWithoutPrefixDotOrSlash(const TCHAR* path)
{
	while(*path &&
	  ((*path == TEXT('.') && (path[1] == TEXT('\\') || path[1] == TEXT('/'))) ||
	  *path == TEXT('\\') || *path == TEXT('/') || *path == TEXT(' ')))
		path++;
	return path;
}

const TCHAR* MakeScriptPathAbsolute(const TCHAR* filename, const TCHAR* extraDirToCheck)
{
	static TCHAR filename2 [1024];
	if(filename[0] && filename[1] != TEXT(':'))
	{
		TCHAR tempFile [1024], curDir [1024];
		lstrcpyn(tempFile, filename, 1024);
		tempFile[1023] = 0;
		const TCHAR* tempFilePtr = PathWithoutPrefixDotOrSlash(tempFile);
		for(int i=0; i<=4; i++)
		{
			if((!*tempFilePtr || tempFilePtr[1] != TEXT(':')) && i != 2)
				lstrcpy(curDir, i!=1 ? ((i!=3||!extraDirToCheck) ? Lua_Dir : extraDirToCheck) : Emu_Path);
			else
				curDir[0] = 0;
			_sntprintf(filename2, 1024, TEXT("%s%s"), curDir, tempFilePtr);
			TCHAR* bar = _tcschr(filename2, TEXT('|'));
			if(bar) *bar = 0;
			FILE* file = _tfopen(filename2, TEXT("rb"));
			if(bar) *bar = TEXT('|');
			if(file || i==4)
				filename = filename2;
			if(file)
			{
				fclose(file);
				break;
			}
		}
	}
	return filename;
}

extern void RequestAbortLuaScript(int uid, const char* message);

const char* OpenLuaScript(const char* filename, const char* extraDirToCheck, bool makeSubservient)
{
	if(LuaScriptHWnds.size() < 16)
	{
		// make the filename absolute before loading
		TCHAR tfilenameString[1024];
		TCHAR *tfilename = tfilenameString;
		lstrcpyn(tfilename, _tFromChar(filename), 1024);
		tfilename = (TCHAR*)MakeScriptPathAbsolute(tfilename, _tFromChar(extraDirToCheck));

		// now check if it's already open and load it if it isn't
		HWND IsScriptFileOpen(const TCHAR* Path);
		HWND scriptHWnd = IsScriptFileOpen(tfilename);
		if(!scriptHWnd)
		{
			HWND prevWindow = GetActiveWindow();

			HWND hDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_LUA), g_hWnd, (DLGPROC) LuaScriptProc);
			SendMessage(hDlg,WM_COMMAND,IDC_NOTIFY_SUBSERVIENT,TRUE);
			SendDlgItemMessage(hDlg,IDC_EDIT_LUAPATH,WM_SETTEXT,0,(LPARAM)((const TCHAR*)_tFromChar(filename)));
//			DialogsOpen++;

			SetActiveWindow(prevWindow);
		}
		else
		{
			RequestAbortLuaScript((int)scriptHWnd, "terminated to restart because of a call to emu.openscript");
			SendMessage(scriptHWnd, WM_COMMAND, IDC_BUTTON_LUARUN, 0);
		}
	}
	else return "Too many script windows are already open.";

	return NULL;
}

#endif // HAVE_LUA
