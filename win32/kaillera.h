/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef KAILLERA_H
#define KAILLERA_H

#ifdef KAILLERA_SUPPORT

#include <windows.h>

// Kaillera client API info structure
typedef struct {
    char *appName;
    char *gameList;
    int  (WINAPI *gameCallback)(char *game, int player, int numplayers);
    void (WINAPI *chatReceivedCallback)(char *nick, char *text);
    void (WINAPI *clientDroppedCallback)(char *nick, int playernb);
    void (WINAPI *moreInfosCallback)(char *gamename);
} kailleraInfos;

// Function pointer typedefs for dynamic DLL loading
typedef int  (WINAPI *pfn_kailleraGetVersion)(char *version);
typedef int  (WINAPI *pfn_kailleraInit)(void);
typedef int  (WINAPI *pfn_kailleraShutdown)(void);
typedef int  (WINAPI *pfn_kailleraSetInfos)(kailleraInfos *infos);
typedef int  (WINAPI *pfn_kailleraSelectServerDialog)(HWND parent);
typedef int  (WINAPI *pfn_kailleraModifyPlayValues)(void *values, int size);
typedef int  (WINAPI *pfn_kailleraChatSend)(char *text);
typedef int  (WINAPI *pfn_kailleraEndGame)(void);

// Kaillera integration state
struct SKailleraState {
    bool     DLLLoaded;
    volatile bool GameActive;
    int      PlayerNumber;   // 1-based player number (1-8), 0 = spectator
    int      NumPlayers;
    char     GameName[256];
    HANDLE   GameEndEvent;   // signaled when game should end
    HMODULE  hDLL;

    // Thread-safe input exchange between main thread and Kaillera callback thread.
    // The DLL requires kailleraModifyPlayValues to be called from the callback thread.
    HANDLE   InputReadyEvent;   // main thread signals: local input is available
    HANDLE   OutputReadyEvent;  // callback thread signals: synced output is available
    volatile unsigned short LocalInput;
    unsigned short AllInputs[8];
    volatile int   OutputPlayerCount;  // -1 = error
};

extern SKailleraState Kaillera;

// Public API
bool KailleraLoadDLL(const char *dllPath = "kailleraclient.dll");
void KailleraUnloadDLL();
void KailleraOpenDialog(HWND hWnd);
void KailleraStopGame();
int  KailleraExchangeInput(unsigned short localInput, unsigned short *allInputs, int maxPlayers);

// Custom Windows message sent from Kaillera's game callback thread to the main window
#define WM_KAILLERA_GAME_START  (WM_USER + 10)
#define WM_KAILLERA_GAME_END    (WM_USER + 11)

#endif // KAILLERA_SUPPORT
#endif // KAILLERA_H
