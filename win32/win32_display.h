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
#include "../shaders/glsl.h"
#include <vector>

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
void WinDisplayStringFromBottom (const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap);
void WinSetCustomDisplaySurface(void *screen, int ppl, int width, int height, int scale);
template<typename screenPtrType>
void WinDisplayStringInBuffer (const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap);
char *ReadShaderFileContents(const TCHAR *filename);
void ReduceToPath(TCHAR *filename);
double WinGetRefreshRate();
int WinGetAutomaticInputRate();
GLSLShader *WinGetActiveGLSLShader();

#endif
