#pragma once

#include "../retroachievements.h"

#ifdef RETROACHIEVEMENTS_SUPPORT

// Posted to GUI.hWnd from the HTTP worker when a login attempt finishes.
// wParam: bit0 = success, bit1 = user-initiated (login dialog, not stored token)
// lParam: malloc'd message string - receiver must free()
#define WM_RA_LOGIN_RESULT (WM_USER + 52)

void RA_Win32_RegisterCallbacks();

// Routes keyboard messages to the modeless achievement list; call from the
// main message pump. Returns true if the message was consumed.
bool RA_Win32_HandleDialogMessage(struct tagMSG *msg);

#endif // RETROACHIEVEMENTS_SUPPORT
