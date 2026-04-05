/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifdef KAILLERA_SUPPORT

#include "kaillera.h"
#include "wsnes9x.h"
#include "../snes9x.h"
#include "../memmap.h"
#include "../display.h"

#include <windows.h>
#include <wininet.h>
#include <cstdio>
#include <cstring>

#pragma comment(lib, "wininet.lib")

// ---------------------------------------------------------------------------
// Win32 HTTP callbacks (WinINet)
// ---------------------------------------------------------------------------
static int kaillera_win32_http_get(const char *url, char *buffer, int bufferSize)
{
    HINTERNET hInet = InternetOpenA("snes9x", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInet)
        return 0;

    HINTERNET hUrl = InternetOpenUrlA(hInet, url, NULL, 0,
                                       INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl)
    {
        InternetCloseHandle(hInet);
        return 0;
    }

    int totalRead = 0;
    DWORD bytesRead;
    while (InternetReadFile(hUrl, buffer + totalRead, bufferSize - totalRead - 1, &bytesRead) && bytesRead > 0)
    {
        totalRead += bytesRead;
        if (totalRead >= bufferSize - 1)
            break;
    }
    buffer[totalRead] = '\0';

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInet);
    return totalRead;
}

static void kaillera_win32_http_post(const char *url)
{
    HINTERNET hInet = InternetOpenA("snes9x", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInet)
        return;

    HINTERNET hUrl = InternetOpenUrlA(hInet, url, NULL, 0,
                                       INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (hUrl)
    {
        char buf[256];
        DWORD bytesRead;
        while (InternetReadFile(hUrl, buf, sizeof(buf), &bytesRead) && bytesRead > 0)
            ;
        InternetCloseHandle(hUrl);
    }

    InternetCloseHandle(hInet);
}

// ---------------------------------------------------------------------------
// Win32 UI callbacks
// ---------------------------------------------------------------------------
static void kaillera_win32_game_started(const char *gameName, int playerNumber, int numPlayers)
{
    // Post message to main window so it can load the ROM on the UI thread
    char *nameCopy = _strdup(gameName);
    PostMessage(GUI.hWnd, WM_KAILLERA_GAME_START, (WPARAM)playerNumber,
                (LPARAM)nameCopy);
}

static void kaillera_win32_game_ended()
{
    PostMessage(GUI.hWnd, WM_KAILLERA_GAME_END, 0, 0);
}

static void kaillera_win32_status_changed(const char *status)
{
    // Could update a status bar or dialog here
}

static void kaillera_win32_chat_updated()
{
    // Could refresh a chat window here
}

static void kaillera_win32_log(const char *message)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
}

// ---------------------------------------------------------------------------
// Public: Register Win32 callbacks
// ---------------------------------------------------------------------------
void Kaillera_Win32_RegisterCallbacks()
{
    KailleraPlatformCallbacks cb = {};
    cb.http_get = kaillera_win32_http_get;
    cb.http_post = kaillera_win32_http_post;
    cb.on_game_started = kaillera_win32_game_started;
    cb.on_game_ended = kaillera_win32_game_ended;
    cb.on_status_changed = kaillera_win32_status_changed;
    cb.on_chat_updated = kaillera_win32_chat_updated;
    cb.log_message = kaillera_win32_log;
    KailleraClientSetCallbacks(cb);
}

#endif // KAILLERA_SUPPORT
