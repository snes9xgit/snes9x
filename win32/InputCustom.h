/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef CUSTCTRL_INCLUDED
#define CUSTCTRL_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void InitInputCustomControl(void);
HWND CreateInputCustom(HWND hwndParent);
void InitKeyCustomControl(void);
HWND CreateKeyCustom(HWND hwndParent);

typedef struct
{
    COLORREF crForeGnd;    // Foreground text colour
    COLORREF crBackGnd;    // Background text colour
    HFONT    hFont;        // The font
    HWND     hwnd;         // The control's window handle
} InputCust;
COLORREF CheckButtonKey( WORD Key);
COLORREF CheckHotKey( WORD Key, int modifiers);
InputCust * GetInputCustom(HWND hwnd);


#ifdef __cplusplus
}
#endif

#endif
