/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef WIN32_DISPLAY_H
#define WIN32_DISPLAY_H

#include "wsnes9x.h"
#include "port.h"
#include "render.h"
#include <vector>
#include <functional>

#define IsHiRes(x) ((x.Height > SNES_HEIGHT_EXTENDED || x.Width == 512))
#define CurrentScale (IsHiRes(Src) ? GUI.ScaleHiRes : GUI.Scale)

#define IS_SLASH(x) ((x) == TEXT('\\') || (x) == TEXT('/'))

void WinRefreshDisplay(void);
void S9xSetWinPixelFormat ();
void SwitchToGDI();
void SaveMainWinPos();
void RestoreMainWinPos();
void ToggleFullScreen ();
void RestoreGUIDisplay ();
void RestoreSNESDisplay ();
void WinChangeWindowSize(unsigned int newWidth, unsigned int newHeight);
bool WinDisplayReset(void);
void WinDisplayApplyChanges();
RECT CalculateDisplayRect(unsigned int sourceWidth,unsigned int sourceHeight,
						  unsigned int displayWidth,unsigned int displayHeight);
void WinEnumDisplayModes(std::vector<dMode> *modeVector);
void ConvertDepth (SSurface *src, SSurface *dst, RECT *srect);
char *ReadShaderFileContents(const TCHAR *filename);
void ReduceToPath(TCHAR *filename);
double WinGetRefreshRate();
void S9xWinDisplayString(const char*, int, int, bool, int);
int WinGetAutomaticInputRate();
void WinThrottleFramerate();
std::vector<ShaderParam> *WinGetShaderParameters();
std::function<void(const char*)> WinGetShaderSaveFunction();

#endif
