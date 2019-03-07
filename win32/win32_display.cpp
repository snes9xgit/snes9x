/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/* win32_display.cpp
	Contains most of the display related functions of the win32 port.
	Delegates relevant function calls to the currently selected IS9xDisplayOutput object.
*/

#include "../snes9x.h"
#include "../ppu.h"
#include "../font.h"
#include "wsnes9x.h"
#include "win32_display.h"
#include "CDirect3D.h"
#if DIRECTDRAW_SUPPORT
#include "CDirectDraw.h"
#endif
#include "COpenGL.h"
#include "IS9xDisplayOutput.h"

#include "../filter/hq2x.h"
#include "../filter/2xsai.h"
#include "../apu/apu.h"

// available display output methods
CDirect3D Direct3D;
#if DIRECTDRAW_SUPPORT
CDirectDraw DirectDraw;
#endif
COpenGL OpenGL;
SSurface Src = {0};
extern BYTE *ScreenBufferBlend;

typedef HRESULT (*DWMFLUSHPROC)();
typedef HRESULT (*DWMISCOMPOSITIONENABLEDPROC)(BOOL *);
DWMFLUSHPROC DwmFlushProc = NULL;
DWMISCOMPOSITIONENABLEDPROC DwmIsCompositionEnabledProc = NULL;

// Interface used to access the display output
IS9xDisplayOutput *S9xDisplayOutput=&Direct3D;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

bool8 S9xDeinitUpdate (int, int);
void DoAVIVideoFrame();

// cut off both top and bottom if overscan is disabled and game outputs extended height,
// center image if overscan is enabled and game outputs regular height
static void CheckOverscanOffset()
{
	int lines_to_skip = 0;
	if (!GUI.HeightExtend)
	{
		if (Src.Height == SNES_HEIGHT_EXTENDED)
			lines_to_skip = 7;
		else if (Src.Height == SNES_HEIGHT_EXTENDED << 1)
			lines_to_skip = 14;
	}
	else
	{
		if (Src.Height == SNES_HEIGHT)
			lines_to_skip = -8;
		else if (Src.Height == SNES_HEIGHT << 1)
			lines_to_skip = -16;
	}

	Src.Surface = (BYTE*)(GFX.Screen + lines_to_skip * (int)GFX.RealPPL);
}

/*  WinRefreshDisplay
repeats the last rendered frame
*/
void WinRefreshDisplay(void)
{
	if(!Src.Width)
		return;

	CheckOverscanOffset();

	SelectRenderMethod ();

	S9xDisplayOutput->Render(Src);
	GUI.FlipCounter++;
}

void WinChangeWindowSize(unsigned int newWidth, unsigned int newHeight)
{
	S9xDisplayOutput->ChangeRenderSize(newWidth,newHeight);
}

/*  WinDisplayReset
initializes the currently selected display output and
reinitializes the core graphics rendering
-----
returns true if successful, false otherwise
*/
bool WinDisplayReset(void)
{
	S9xDisplayOutput->DeInitialize();
	switch(GUI.outputMethod) {
		default:
		case DIRECT3D:
			S9xDisplayOutput = &Direct3D;
			break;
#if DIRECTDRAW_SUPPORT
		case DIRECTDRAW:
			S9xDisplayOutput = &DirectDraw;
			break;
#endif
		case OPENGL:
			S9xDisplayOutput = &OpenGL;
			break;
	}
	if(S9xDisplayOutput->Initialize(GUI.hWnd)) {
		S9xGraphicsDeinit();
		S9xSetWinPixelFormat ();
		S9xGraphicsInit();

        if (GUI.DWMSync)
        {
            HMODULE dwmlib = LoadLibrary(TEXT("dwmapi"));
            DwmFlushProc = (DWMFLUSHPROC)GetProcAddress(dwmlib, "DwmFlush");
            DwmIsCompositionEnabledProc = (DWMISCOMPOSITIONENABLEDPROC)GetProcAddress(dwmlib, "DwmIsCompositionEnabled");

            if (!DwmFlushProc || !DwmIsCompositionEnabledProc)
            {
                MessageBox(GUI.hWnd, TEXT("Couldn't load DWM functions. DWM Sync is disabled."), TEXT("Warning"), MB_OK | MB_ICONWARNING);
                GUI.DWMSync = false;
            }
        }

		return true;
	} else {
		MessageBox (GUI.hWnd, Languages[ GUI.Language].errInitDD, TEXT("Snes9x - Display Failure"), MB_OK | MB_ICONSTOP);
		return false;
	}
}

void WinDisplayApplyChanges()
{
	S9xDisplayOutput->ApplyDisplayChanges();
}

RECT CalculateDisplayRect(unsigned int sourceWidth,unsigned int sourceHeight,
						  unsigned int displayWidth,unsigned int displayHeight)
{
	double xFactor;
	double yFactor;
	double minFactor;
	double renderWidthCalc,renderHeightCalc;
	int hExtend = GUI.HeightExtend ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT;
	double snesAspect = (double)GUI.AspectWidth/hExtend;
	RECT drawRect;

	if(GUI.Stretch) {
		if(GUI.AspectRatio) {

			if (GUI.IntegerScaling && sourceHeight > 0 && sourceHeight <= displayHeight && (int)(sourceHeight * snesAspect) <= displayWidth) {
				int h;
				for (h = sourceHeight * 2; h <= displayHeight && (int)(h * snesAspect) <= displayWidth; h += sourceHeight) {}
				h -= sourceHeight;
				drawRect.right = (LONG)(h * snesAspect);
				drawRect.bottom = h;
			} else {
				//fix for hi-res images with FILTER_NONE
				//where we need to correct the aspect ratio
				renderWidthCalc = (double)sourceWidth;
				renderHeightCalc = (double)sourceHeight;
				if (renderWidthCalc / renderHeightCalc > snesAspect)
					renderWidthCalc = renderHeightCalc * snesAspect;
				else if (renderWidthCalc / renderHeightCalc < snesAspect)
					renderHeightCalc = renderWidthCalc / snesAspect;

				xFactor = (double)displayWidth / renderWidthCalc;
				yFactor = (double)displayHeight / renderHeightCalc;
				minFactor = xFactor < yFactor ? xFactor : yFactor;

				drawRect.right = (LONG)(renderWidthCalc * minFactor);
				drawRect.bottom = (LONG)(renderHeightCalc * minFactor);
			}

			drawRect.left = (displayWidth - drawRect.right) / 2;
			drawRect.top = (displayHeight - drawRect.bottom) / 2;
			drawRect.right += drawRect.left;
			drawRect.bottom += drawRect.top;

		} else {
			drawRect.top = 0;
			drawRect.left = 0;
			drawRect.right = displayWidth;
			drawRect.bottom = displayHeight;
		}
	} else {
		drawRect.left = ((int)(displayWidth) - (int)sourceWidth) / 2;
		drawRect.top = ((int)(displayHeight) - (int)sourceHeight) / 2;
		if(!GUI.AlwaysCenterImage) {
			if(drawRect.left < 0) drawRect.left = 0;
			if(drawRect.top < 0) drawRect.top = 0;
		}
		drawRect.right = drawRect.left + sourceWidth;
		drawRect.bottom = drawRect.top + sourceHeight;
	}
	return drawRect;
}

// we no longer support 8bit modes - no palette necessary
void S9xSetPalette( void)
{
	return;	
}

bool8 S9xInitUpdate (void)
{
	return (TRUE);
}

// only necessary for avi recording
// TODO: check if this can be removed
bool8 S9xContinueUpdate(int Width, int Height)
{
	// called every other frame during interlace
	
    Src.Width = Width;
	if(Height%SNES_HEIGHT)
	    Src.Height = Height;
	else
	{
		if(Height==SNES_HEIGHT)
			Src.Height=SNES_HEIGHT_EXTENDED;
		else Src.Height=SNES_HEIGHT_EXTENDED<<1;
	}
    Src.Pitch = GFX.Pitch;
    Src.Surface = (BYTE*)GFX.Screen;

	// avi writing
	DoAVIVideoFrame();

	return true;
}

// do the actual rendering of a frame
bool8 S9xDeinitUpdate (int Width, int Height)
{
    Src.Width = Width;
	Src.Height = Height;
    Src.Pitch = GFX.Pitch;

	CheckOverscanOffset();

	// avi writing
	DoAVIVideoFrame();

	// Clear some of the old SNES rendered image
	// when the resolution becomes lower in x or y,
	// otherwise the image processors (filters) might access
	// some of the old rendered data at the edges.
    {
        static int LastWidth = 0;
        static int LastHeight = 0;

        if (Width < LastWidth)
        {
            const int hh = max(LastHeight, Height);
            for (int i = 0; i < hh; i++)
                memset (GFX.Screen + i * (GFX.Pitch>>1) + Width*1, 0, 4);
        }
        if (Height < LastHeight)
		{
            const int ww = max(LastWidth, Width);
            for (int i = Height; i < LastHeight ; i++)
                memset (GFX.Screen + i * (GFX.Pitch>>1), 0, ww * 2);

			// also old clear extended height stuff from drawing surface
			if((int)Src.Height > Height)
				for (int i = Height; i < (int)Src.Height ; i++)
					memset (Src.Surface + i * Src.Pitch, 0, Src.Pitch);
		}
        LastWidth = Width;
        LastHeight = Height;
    }

    if (GUI.DWMSync && GUI.outputMethod == OPENGL)
    {
        BOOL DWMEnabled = false;
        DwmIsCompositionEnabledProc(&DWMEnabled);

        if (GUI.FullScreen || !DWMEnabled)
            ((COpenGL *)S9xDisplayOutput)->SetSwapInterval(GUI.Vsync ? 1 : 0);
        else
            ((COpenGL *)S9xDisplayOutput)->SetSwapInterval(0);

        WinRefreshDisplay();

        if (DWMEnabled && !GUI.FullScreen)
            DwmFlushProc();
    }
    else
    {
        WinRefreshDisplay();
    }

    return (true);
}

/*  S9xSetWinPixelFormat
sets default settings and calls the appropriate display object
*/
void S9xSetWinPixelFormat ()
{
    GUI.NeedDepthConvert = FALSE;
	GUI.DepthConverted = !GUI.NeedDepthConvert;

	S9xBlit2xSaIFilterDeinit();
	S9xBlitHQ2xFilterDeinit();

	S9xDisplayOutput->SetSnes9xColorFormat();
}

char *ReadShaderFileContents(const TCHAR *filename)
{
	HANDLE hFile;
	DWORD size;
	DWORD bytesRead;
	char *contents;

	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN , 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return NULL;
	}
	size = GetFileSize(hFile,NULL);
	contents = new char[size+1];
	if(!ReadFile(hFile,contents,size,&bytesRead,NULL)) {
		CloseHandle(hFile);
		delete[] contents;
		return NULL;
	}
	CloseHandle(hFile);
	contents[size] = '\0';
	return contents;

}

void ReduceToPath(TCHAR *filename)
{
    for (int i = lstrlen(filename); i >= 0; i--) {
        if (IS_SLASH(filename[i])) {
            filename[i] = TEXT('\0');
            break;
        }
    }
}

// TODO: abstract the following functions in some way - only necessary for directdraw

/* DirectDraw only begin */
void SwitchToGDI()
{
#if DIRECTDRAW_SUPPORT
	if(GUI.outputMethod!=DIRECTDRAW)
		return;

    IPPU.ColorsChanged = true;
    DirectDraw.lpDD->FlipToGDISurface();
    GUI.FlipCounter = 0;
    DirectDraw.lpDDSPrimary2->SetPalette (NULL);
#endif
}

static void ClearSurface (LPDIRECTDRAWSURFACE2 lpDDSurface)
{
#if DIRECTDRAW_SUPPORT
    DDBLTFX fx;

    memset (&fx, 0, sizeof (fx));
    fx.dwSize = sizeof (fx);

    while (lpDDSurface->Blt (NULL, DirectDraw.lpDDSPrimary2, NULL, DDBLT_WAIT, NULL) == DDERR_SURFACELOST)
        lpDDSurface->Restore ();
#endif
}

void UpdateBackBuffer()
{
#if DIRECTDRAW_SUPPORT
    if (GUI.outputMethod==DIRECTDRAW && GUI.FullScreen)
    {
        SwitchToGDI();

        DDBLTFX fx;

        memset (&fx, 0, sizeof (fx));
        fx.dwSize = sizeof (fx);

        while (DirectDraw.lpDDSPrimary2->Blt (NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx) == DDERR_SURFACELOST)
            DirectDraw.lpDDSPrimary2->Restore ();

        if (GetMenu (GUI.hWnd) != NULL)
            DrawMenuBar (GUI.hWnd);

        GUI.FlipCounter = 0;
        DDSCAPS caps;
        caps.dwCaps = DDSCAPS_BACKBUFFER;

        LPDIRECTDRAWSURFACE2 pDDSurface;

        if (DirectDraw.lpDDSPrimary2->GetAttachedSurface (&caps, &pDDSurface) == DD_OK &&
            pDDSurface != NULL)
        {
            ClearSurface (pDDSurface);
			DirectDraw.lpDDSPrimary2->Flip (NULL, GUI.Vsync?DDFLIP_WAIT:DDFLIP_NOVSYNC);
            while (DirectDraw.lpDDSPrimary2->GetFlipStatus (DDGFS_ISFLIPDONE) != DD_OK)
                Sleep (0);
			if(DirectDraw.doubleBuffered)
	            ClearSurface (pDDSurface);
        }
    }
    else
    {
        if (GetMenu( GUI.hWnd) != NULL)
            DrawMenuBar (GUI.hWnd);
    }
#endif
}

void RestoreGUIDisplay ()
{
#if DIRECTDRAW_SUPPORT
	if(GUI.outputMethod!=DIRECTDRAW)
		return;

    S9xSetPause (PAUSE_RESTORE_GUI);

    if (GUI.FullScreen &&
        (GUI.FullscreenMode.width < 640 || GUI.FullscreenMode.height < 400) &&
        !DirectDraw.SetDisplayMode (640, 480, 1, 0, 60, !GUI.FullScreen, false))
    {
        MessageBox (GUI.hWnd, Languages[ GUI.Language].errModeDD, TEXT("Snes9x - DirectDraw(1)"), MB_OK | MB_ICONSTOP);
        S9xClearPause (PAUSE_RESTORE_GUI);
        return;
    }
    SwitchToGDI();
    S9xClearPause (PAUSE_RESTORE_GUI);
#endif
}

void RestoreSNESDisplay ()
{
#if DIRECTDRAW_SUPPORT
	if(GUI.outputMethod!=DIRECTDRAW)
		return;

	if (!DirectDraw.SetFullscreen(GUI.FullScreen))
    {
        MessageBox (GUI.hWnd, Languages[ GUI.Language].errModeDD, TEXT("Snes9x - DirectDraw(4)"), MB_OK | MB_ICONSTOP);
        return;
    }

    UpdateBackBuffer();
#endif
}

/* DirectDraw only end */

void SaveMainWinPos()
{
	WINDOWPLACEMENT wndPlacement={0};
	wndPlacement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(GUI.hWnd,&wndPlacement);
	GUI.window_maximized = wndPlacement.showCmd == SW_SHOWMAXIMIZED;
	if(!GUI.FullScreen && !GUI.EmulatedFullscreen)
		GUI.window_size = wndPlacement.rcNormalPosition;
}

void RestoreMainWinPos()
{
	WINDOWPLACEMENT wndPlacement={0};
	wndPlacement.length = sizeof(WINDOWPLACEMENT);
	wndPlacement.showCmd = GUI.window_maximized?SW_SHOWMAXIMIZED:SW_SHOWNORMAL;
	wndPlacement.rcNormalPosition = GUI.window_size;
	SetWindowPlacement(GUI.hWnd,&wndPlacement);
}

/*  ToggleFullScreen
switches between fullscreen and window mode and saves the window position
if EmulateFullscreen is set we simply create a borderless window that spans the screen
*/
void ToggleFullScreen ()
{
    S9xSetPause (PAUSE_TOGGLE_FULL_SCREEN);

    SaveMainWinPos();

	if(GUI.EmulateFullscreen) {
		HMONITOR hm;
		MONITORINFO mi;
		GUI.EmulatedFullscreen = !GUI.EmulatedFullscreen;
		if(GUI.EmulatedFullscreen) {
			if(GetMenu(GUI.hWnd)!=NULL)
				SetMenu(GUI.hWnd,NULL);
			SetWindowLongPtr (GUI.hWnd, GWL_STYLE, WS_POPUP|WS_VISIBLE);
			hm = MonitorFromWindow(GUI.hWnd,MONITOR_DEFAULTTONEAREST);
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hm,&mi);
			SetWindowPos (GUI.hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_DRAWFRAME|SWP_FRAMECHANGED);
		} else {
			SetWindowLongPtr( GUI.hWnd, GWL_STYLE, WS_POPUPWINDOW|WS_CAPTION|
                   WS_THICKFRAME|WS_VISIBLE|WS_MINIMIZEBOX|WS_MAXIMIZEBOX);
			SetMenu(GUI.hWnd,GUI.hMenu);
			SetWindowPos (GUI.hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_DRAWFRAME|SWP_FRAMECHANGED);
			RestoreMainWinPos();
		}
	} else {
		GUI.FullScreen = !GUI.FullScreen;
		if(GUI.FullScreen) {
			if(GetMenu(GUI.hWnd)!=NULL)
				SetMenu(GUI.hWnd,NULL);
			SetWindowLongPtr(GUI.hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
            SetWindowLongPtr(GUI.hWnd, GWL_EXSTYLE, 0);
			if(!S9xDisplayOutput->SetFullscreen(true))
				GUI.FullScreen = false;
            else
                SetWindowPos(GUI.hWnd, HWND_TOPMOST, 0, 0, GUI.FullscreenMode.width, GUI.FullscreenMode.height, SWP_DRAWFRAME | SWP_FRAMECHANGED);
		}
		if(!GUI.FullScreen) {
			SetWindowLongPtr(GUI.hWnd, GWL_STYLE, WS_POPUPWINDOW|WS_CAPTION|
                   WS_THICKFRAME|WS_VISIBLE|WS_MINIMIZEBOX|WS_MAXIMIZEBOX);
            SetWindowLongPtr(GUI.hWnd, GWL_EXSTYLE, WS_EX_ACCEPTFILES | WS_EX_APPWINDOW);
			SetMenu(GUI.hWnd,GUI.hMenu);
			S9xDisplayOutput->SetFullscreen(false);
			SetWindowPos (GUI.hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_DRAWFRAME|SWP_FRAMECHANGED);
			RestoreMainWinPos();
		}
		if (GUI.AutomaticInputRate)
		{
			int rate = WinGetAutomaticInputRate();
			if (rate)
			{
				Settings.SoundInputRate = rate;
				S9xUpdateDynamicRate(1, 2);
			}
		}
		S9xGraphicsDeinit();
		S9xSetWinPixelFormat ();
		S9xInitUpdate();
		S9xGraphicsInit();
	}
	IPPU.RenderThisFrame = true;

	S9xClearPause (PAUSE_TOGGLE_FULL_SCREEN);
}

/*  S9xOpenSoundDevice
ennumerates the available display modes of the currently selected output
*/
void WinEnumDisplayModes(std::vector<dMode> *modeVector)
{
	S9xDisplayOutput->EnumModes(modeVector);
}

double WinGetRefreshRate(void)
{
	typedef LONG(WINAPI *PGDCBS) (UINT32, UINT32 *, UINT32 *);
	typedef LONG(WINAPI *PQDC)   (UINT32, UINT32*, DISPLAYCONFIG_PATH_INFO *, UINT32*, DISPLAYCONFIG_MODE_INFO *, DISPLAYCONFIG_TOPOLOGY_ID *);
	static PGDCBS pGDCBS = NULL;
	static PQDC   pQDC   = NULL;
	static int firstrun = 1;

	if (firstrun)
	{
		HMODULE user32 = GetModuleHandleA("user32.dll");
		pQDC = (PQDC) GetProcAddress(user32, "QueryDisplayConfig");
		pGDCBS = (PGDCBS) GetProcAddress(user32, "GetDisplayConfigBufferSizes");
		firstrun = 0;
	}

	double refreshRate = 0.0;

	if (!pGDCBS || !pQDC)
		return refreshRate;

	OSVERSIONINFO ovi;
	DISPLAYCONFIG_TOPOLOGY_ID topologyID;
	unsigned int numPathArrayElements = 0;
	unsigned int numModeInfoArrayElements = 0;
	DISPLAYCONFIG_PATH_INFO * pathInfoArray = NULL;
	DISPLAYCONFIG_MODE_INFO * modeInfoArray = NULL;
	int result = 0;

	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&ovi))
		return refreshRate;

	if (ovi.dwMajorVersion < 6 || (ovi.dwMajorVersion == 6 && ovi.dwMinorVersion < 1))
		return refreshRate;

	result = pGDCBS(QDC_DATABASE_CURRENT,
		&numPathArrayElements,
		&numModeInfoArrayElements);

	if (result != ERROR_SUCCESS)
		return refreshRate;

	pathInfoArray = (DISPLAYCONFIG_PATH_INFO *)
		malloc(sizeof(DISPLAYCONFIG_PATH_INFO) * numPathArrayElements);
	modeInfoArray = (DISPLAYCONFIG_MODE_INFO *)
		malloc(sizeof(DISPLAYCONFIG_MODE_INFO) * numModeInfoArrayElements);

	result = pQDC(QDC_DATABASE_CURRENT,
		&numPathArrayElements,
		pathInfoArray,
		&numModeInfoArrayElements,
		modeInfoArray,
		&topologyID);

	if (result == ERROR_SUCCESS && numPathArrayElements >= 1)
	{
		refreshRate = (float)pathInfoArray[0].targetInfo.refreshRate.Numerator /
			pathInfoArray[0].targetInfo.refreshRate.Denominator;
	}

	free(modeInfoArray);
	free(pathInfoArray);

	return refreshRate;
}

int WinGetAutomaticInputRate(void)
{
    double refreshRate = WinGetRefreshRate();

    if (refreshRate == 0.0)
        return 0;

    // Try for a close multiple of 60hz
    if (refreshRate > 119.0 && refreshRate < 121.0)
        refreshRate /= 2.0;
    if (refreshRate > 179.0 && refreshRate < 181.0)
        refreshRate /= 3.0;
    if (refreshRate > 239.0 && refreshRate < 241.0)
        refreshRate /= 4.0;

    double newInputRate = refreshRate * 32000.0 / 60.09881389744051 + 0.5;

    if (newInputRate > 32000.0 * 1.05 || newInputRate < 32000.0 * 0.95)
        newInputRate = 0.0;

    return (int)newInputRate;
}

GLSLShader *WinGetActiveGLSLShader()
{
	return OpenGL.GetActiveShader();
}

/* Depth conversion functions begin */

void Convert16To24 (SSurface *src, SSurface *dst, RECT *srect)
{
    const int height = srect->bottom - srect->top;
    const int width = srect->right - srect->left;
    const int offset1 = srect->top * src->Pitch + srect->left * 2;
    const int offset2 = ((dst->Height - height) >> 1) * dst->Pitch + ((dst->Width - width) >> 1) * 3;
	const int snesWidth = src->Width;
	const int snesHeight = src->Height;
	const bool doubledX = (snesWidth >= width*2) ? true : false;
	const bool doubledY = (snesHeight >= height*2) ? true : false;

    for (int y = 0; y < height; y++)
    {
		uint16 *s = (uint16 *) ((uint8 *) src->Surface + (doubledY ? y*2 : y) * src->Pitch + offset1);
        uint8 *d = ((uint8 *) dst->Surface + y * dst->Pitch + offset2);

		#define Interp(c1, c2) \
			(c1 == c2) ? c1 : \
			(((((c1 & 0x07E0)      + (c2 & 0x07E0)) >> 1) & 0x07E0) + \
			((((c1 & 0xF81F)      + (c2 & 0xF81F)) >> 1) & 0xF81F))

		if(y >= snesHeight)
		{
			// beyond SNES image - make the row black
			memset(d, 0, width*3);
		}
		else

#ifdef LSB_FIRST
		if (GUI.RedShift < GUI.BlueShift)
#else
		if (GUI.RedShift > GUI.BlueShift)
#endif
        {
			// Order is RGB
			if(!doubledX)
			{
				for (int x = 0; x < width; x++)
				{
					uint32 pixel = *s++;
					*(d + 0) = (pixel >> (11 - 3)) & 0xf8;
					*(d + 1) = (pixel >> (6 - 3)) & 0xf8;
					*(d + 2) = (pixel & 0x1f) << 3;
					d += 3;
				}
			}
			else // high-res x, blend:
			{
				for (int x = 0; x < width; x++)
				{
					uint32 pixel = Interp(s[0],s[1]);
					s += 2;
					*(d + 0) = (pixel >> (11 - 3)) & 0xf8;
					*(d + 1) = (pixel >> (6 - 3)) & 0xf8;
					*(d + 2) = (pixel & 0x1f) << 3;
					d += 3;
				}
			}
        }
        else
        {
			// Order is BGR
			if(!doubledX)
			{
				for (int x = 0; x < width; x++)
				{
					uint32 pixel = *s++;
					*(d + 0) = (pixel & 0x1f) << 3;
					*(d + 1) = (pixel >> (6 - 3)) & 0xf8;
					*(d + 2) = (pixel >> (11 - 3)) & 0xf8;
					d += 3;
				}
			}
			else // high-res x, blend:
			{
				for (int x = 0; x < width; x++)
				{
					uint32 pixel = Interp(s[0],s[1]);
					s += 2;
					*(d + 0) = (pixel & 0x1f) << 3;
					*(d + 1) = (pixel >> (6 - 3)) & 0xf8;
					*(d + 2) = (pixel >> (11 - 3)) & 0xf8;
					d += 3;
				}
			}
        }
    }
}

void Convert16To32 (SSurface *src, SSurface *dst, RECT *srect)
{
    int height = srect->bottom - srect->top;
    int width = srect->right - srect->left;
    int offset1 = srect->top * src->Pitch + srect->left * 2;
    int offset2 = 0;//((dst->Height - height) >> 1) * dst->Pitch +
        //((dst->Width - width) >> 1) * sizeof (uint32);

    for (int y = 0; y < height; y++)
    {
        uint16 *s = (uint16 *) ((uint8 *) src->Surface + y * src->Pitch + offset1);
        uint32 *d = (uint32 *) ((uint8 *) dst->Surface +
                                         y * dst->Pitch + offset2);
        for (int x = 0; x < width; x++)
        {
            uint32 pixel = *s++;
            *d++ = (((pixel >> 11) & 0x1f) << GUI.RedShift) |
                   (((pixel >> 6) & 0x1f) << GUI.GreenShift) |
                   ((pixel & 0x1f) << GUI.BlueShift);
        }
    }
}

void ConvertDepth (SSurface *src, SSurface *dst, RECT *srect)
{
    // SNES image has been rendered in 16-bit, RGB565 format
    switch (GUI.ScreenDepth)
    {
        case 15: // is this right?
        case 16:
            break;
        case 24:
            Convert16To24 (src, dst, srect);
            break;
        case 32:
            Convert16To32 (src, dst, srect);
            break;
    }
}

/* Depth conversion functions end */


/* The rest of the functions are recreations of the core string display functions with additional scaling.
   They provide the possibility to render the messages into a surface other than the core GFX.Screen.
*/

void WinDisplayStringFromBottom (const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap)
{
	if(Settings.StopEmulation)
		return;
	if(Settings.AutoDisplayMessages) {
		WinSetCustomDisplaySurface((void *)GFX.Screen, GFX.RealPPL, IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight, 1);
		WinDisplayStringInBuffer<uint16>(string, linesFromBottom, pixelsFromLeft, allowWrap);
	} else
		if(GUI.ScreenDepth == 32) {
			WinDisplayStringInBuffer<uint32>(string, linesFromBottom, pixelsFromLeft, allowWrap);
		} else {
			WinDisplayStringInBuffer<uint16>(string, linesFromBottom, pixelsFromLeft, allowWrap);
		}
}

static int	font_width = 8, font_height = 9;
static void *displayScreen;
int displayPpl, displayWidth, displayHeight, displayScale, fontwidth_scaled, fontheight_scaled;

void WinSetCustomDisplaySurface(void *screen, int ppl, int width, int height, int scale)
{
	displayScreen=screen;
	displayPpl=ppl;
	displayWidth=width;
	displayHeight=height;
	displayScale=max(1,width/IPPU.RenderedScreenWidth);
	fontwidth_scaled=font_width*displayScale;
	fontheight_scaled=font_height*displayScale;
}

template<typename screenPtrType>
void WinDisplayChar (screenPtrType *s, uint8 c)
{
	int h, w;

	int	line   = ((c - 32) >> 4) * fontheight_scaled;
	int	offset = ((c - 32) & 15) * fontwidth_scaled;

	if (GUI.filterMessagFont && (displayScale == 2 || displayScale == 3))
	{
		if (displayScale == 2) {
			for (h = 0; h < fontheight_scaled; h += 2, line += 2, s += 2 * displayPpl - fontwidth_scaled)
				for (w = 0; w < fontwidth_scaled; w += 2, s += 2)
					FontPixToScreenEPX((offset + w) / 2, line / 2, s);
		}
		else if (displayScale == 3) {
			for (h = 0; h < fontheight_scaled; h += 3, line += 3, s += 3 * displayPpl - fontwidth_scaled)
				for (w = 0; w < fontwidth_scaled; w += 3, s += 3)
					FontPixToScreenEPXSimple3((offset + w) / 3, line / 3, s);
		}
	}
	else
	{
		for(h=0; h<fontheight_scaled; h++, line++, s+=displayPpl-fontwidth_scaled)
			for(w=0; w<fontwidth_scaled; w++, s++)
				FontPixToScreen(font [(line)/displayScale] [(offset + w)/displayScale], s);
	}
}

static inline void FontPixToScreen(char p, uint16 *s)
{
	if(p == '#')
	{
		*s = Settings.DisplayColor;
	}
	else if(p == '.')
	{
		static const uint16 black = BUILD_PIXEL(0,0,0);
		*s = black;
	}
}

static inline void FontPixToScreen(char p, uint32 *s)
{
#define CONVERT_16_TO_32(pixel) \
    (((((pixel) >> 11)        ) << /*RedShift+3*/  19) | \
     ((((pixel) >> 6)   & 0x1f) << /*GreenShift+3*/11) | \
      (((pixel)         & 0x1f) << /*BlueShift+3*/ 3))

	if(p == '#')
	{
		*s = CONVERT_16_TO_32(Settings.DisplayColor);
	}
	else if(p == '.')
	{
		static const uint32 black = CONVERT_16_TO_32(BUILD_PIXEL(0,0,0));
		*s = black;
	}
}

#define CHOOSE(c1) ((c1=='#'||X=='#') ? '#' : ((c1=='.'||X=='.') ? '.' : c1))

template<typename screenPtrType>
static inline void FontPixToScreenEPX(int x, int y, screenPtrType *s)
{
	const char X = font[y][x];                // E D H
	const char A = x>0  ?font[y][x-1]:' ';    // A X C
	const char C = x<143?font[y][x+1]:' ';    // F B G
	if (A != C)
	{
		const char D = y>0  ?font[y-1][x]:' ';
		const char B = y<125?font[y+1][x]:' ';
		if (B != D)
		{
			FontPixToScreen((D == A) ? CHOOSE(D) : X, s);
			FontPixToScreen((C == D) ? CHOOSE(C) : X, s+1);
			FontPixToScreen((A == B) ? CHOOSE(A) : X, s+displayPpl);
			FontPixToScreen((B == C) ? CHOOSE(B) : X, s+displayPpl+1);
			return;
		}
	}
	FontPixToScreen(X, s);
	FontPixToScreen(X, s+1);
	FontPixToScreen(X, s+displayPpl);
	FontPixToScreen(X, s+displayPpl+1);
}
#undef CHOOSE

#define CHOOSE(c1) ((X=='#') ? '#' : c1)
template<typename screenPtrType>
inline void FontPixToScreenEPXSimple3(int x, int y, screenPtrType *s)
{
	const char X = font[y][x];                // E D H
	const char A = x>0  ?font[y][x-1]:' ';    // A X C
	const char C = x<143?font[y][x+1]:' ';    // F B G
	const char D = y>0  ?font[y-1][x]:' ';
	const char B = y<125?font[y+1][x]:' ';
	const bool XnE = y>0  &&x>0  ?(X != font[y-1][x-1]):X!=' ';
	const bool XnF = y<125&&x<143?(X != font[y+1][x-1]):X!=' ';
	const bool XnG = y<125&&x>0  ?(X != font[y+1][x+1]):X!=' ';
	const bool XnH = y>0  &&x<143?(X != font[y-1][x+1]):X!=' ';
	const bool DA = D == A && (XnE || CHOOSE(D)!=X);
	const bool AB = A == B && (XnF || CHOOSE(A)!=X);
	const bool BC = B == C && (XnG || CHOOSE(B)!=X);
	const bool CD = C == D && (XnH || CHOOSE(C)!=X);
	FontPixToScreen(DA ? A : X, s);
	FontPixToScreen(X, s+1);
	FontPixToScreen(CD ? C : X, s+2);
	FontPixToScreen(X, s+displayPpl);
	FontPixToScreen(X, s+displayPpl+1);
	FontPixToScreen(X, s+displayPpl+2);
	FontPixToScreen(AB ? A : X, s+displayPpl+displayPpl);
	FontPixToScreen(X, s+displayPpl+displayPpl+1);
	FontPixToScreen(BC ? C : X, s+displayPpl+displayPpl+2);
}
#undef CHOOSE

template<typename screenPtrType>
void WinDisplayStringInBuffer (const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap)
{
	if (linesFromBottom <= 0)
		linesFromBottom = 1;

	screenPtrType	*dst = (screenPtrType	*)displayScreen + (displayHeight - fontheight_scaled * linesFromBottom) * displayPpl + (int)(pixelsFromLeft * (float)displayWidth/IPPU.RenderedScreenWidth);

	int	len = strlen(string);
	int	max_chars = displayWidth / (fontwidth_scaled - displayScale);
	int	char_count = 0;

	for (int i = 0 ; i < len ; i++, char_count++)
	{
		if (char_count >= max_chars || (uint8) string[i] < 32)
		{
			if (!allowWrap)
				break;

			dst += fontheight_scaled * displayPpl - (fontwidth_scaled - displayScale) * max_chars;
			if (dst >= (screenPtrType	*)displayScreen + displayHeight * displayPpl)
				break;

			char_count -= max_chars;
		}

		if ((uint8) string[i] < 32)
			continue;

		WinDisplayChar(dst, string[i]);
		dst += fontwidth_scaled - displayScale;
	}
}
