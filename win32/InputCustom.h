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

#ifndef MAX_BIND_KEYS
#define MAX_BIND_KEYS 4
#endif

typedef struct
{
    COLORREF crForeGnd;    // Foreground text colour
    COLORREF crBackGnd;    // Background text colour
    HFONT    hFont;        // The font
    HWND     hwnd;         // The control's window handle
    WORD     keys[MAX_BIND_KEYS];  // accumulated multi-bind keys
    int      numKeys;               // count of bound keys
} InputCust;
COLORREF CheckButtonKey( WORD Key);
COLORREF CheckHotKey( WORD Key, int modifiers);
InputCust * GetInputCustom(HWND hwnd);


#ifdef __cplusplus
}
#endif

// C++ linkage declarations (must be outside extern "C")
void TranslateKey(WORD keyz, char *out);

#endif
