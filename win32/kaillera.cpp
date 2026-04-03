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
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <process.h>

// Global Kaillera state
SKailleraState Kaillera = {};

// DLL function pointers
static pfn_kailleraGetVersion         pKailleraGetVersion = NULL;
static pfn_kailleraInit               pKailleraInit = NULL;
static pfn_kailleraShutdown           pKailleraShutdown = NULL;
static pfn_kailleraSetInfos           pKailleraSetInfos = NULL;
static pfn_kailleraSelectServerDialog pKailleraSelectServerDialog = NULL;
static pfn_kailleraModifyPlayValues   pKailleraModifyPlayValues = NULL;
static pfn_kailleraChatSend           pKailleraChatSend = NULL;
static pfn_kailleraEndGame            pKailleraEndGame = NULL;

// Helper to resolve a function from the DLL, trying multiple name decorations
template <typename T>
static bool ResolveDLLFunc(HMODULE hDLL, T *pFunc, const char *name,
                           const char *decorated, const char *underscoreDecorated)
{
    *pFunc = (T)GetProcAddress(hDLL, name);
    if (!*pFunc)
        *pFunc = (T)GetProcAddress(hDLL, decorated);
    if (!*pFunc)
        *pFunc = (T)GetProcAddress(hDLL, underscoreDecorated);
    return *pFunc != NULL;
}

bool KailleraLoadDLL(const char *dllPath)
{
    if (Kaillera.DLLLoaded)
        return true;

    HMODULE hDLL = LoadLibraryA(dllPath);
    if (!hDLL)
    {
        S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Failed to load kailleraclient.dll");
        return false;
    }

    bool ok = true;
    ok &= ResolveDLLFunc(hDLL, &pKailleraGetVersion,         "kailleraGetVersion",         "kailleraGetVersion@4",         "_kailleraGetVersion@4");
    ok &= ResolveDLLFunc(hDLL, &pKailleraInit,               "kailleraInit",               "kailleraInit@0",               "_kailleraInit@0");
    ok &= ResolveDLLFunc(hDLL, &pKailleraShutdown,           "kailleraShutdown",           "kailleraShutdown@0",           "_kailleraShutdown@0");
    ok &= ResolveDLLFunc(hDLL, &pKailleraSetInfos,           "kailleraSetInfos",           "kailleraSetInfos@4",           "_kailleraSetInfos@4");
    ok &= ResolveDLLFunc(hDLL, &pKailleraSelectServerDialog, "kailleraSelectServerDialog", "kailleraSelectServerDialog@4", "_kailleraSelectServerDialog@4");
    ok &= ResolveDLLFunc(hDLL, &pKailleraModifyPlayValues,   "kailleraModifyPlayValues",   "kailleraModifyPlayValues@8",   "_kailleraModifyPlayValues@8");
    ok &= ResolveDLLFunc(hDLL, &pKailleraChatSend,           "kailleraChatSend",           "kailleraChatSend@4",           "_kailleraChatSend@4");
    ok &= ResolveDLLFunc(hDLL, &pKailleraEndGame,            "kailleraEndGame",            "kailleraEndGame@0",            "_kailleraEndGame@0");

    if (!ok)
    {
        S9xMessage(S9X_ERROR, S9X_ROM_INFO, "kailleraclient.dll is missing required exports");
        FreeLibrary(hDLL);
        return false;
    }

    Kaillera.hDLL = hDLL;
    Kaillera.DLLLoaded = true;
    Kaillera.GameActive = false;
    Kaillera.GameEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    Kaillera.InputReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);  // auto-reset
    Kaillera.OutputReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset

    pKailleraInit();

    char version[16] = {};
    pKailleraGetVersion(version);
    char msg[64];
    sprintf(msg, "Kaillera client loaded (v%s)", version);
    S9xMessage(S9X_INFO, S9X_ROM_INFO, msg);

    return true;
}

void KailleraUnloadDLL()
{
    if (!Kaillera.DLLLoaded)
        return;

    if (Kaillera.GameActive)
        KailleraStopGame();

    pKailleraShutdown();
    FreeLibrary(Kaillera.hDLL);

    if (Kaillera.GameEndEvent)  { CloseHandle(Kaillera.GameEndEvent);  Kaillera.GameEndEvent = NULL; }
    if (Kaillera.InputReadyEvent)  { CloseHandle(Kaillera.InputReadyEvent);  Kaillera.InputReadyEvent = NULL; }
    if (Kaillera.OutputReadyEvent) { CloseHandle(Kaillera.OutputReadyEvent); Kaillera.OutputReadyEvent = NULL; }

    Kaillera.hDLL = NULL;
    Kaillera.DLLLoaded = false;

    pKailleraGetVersion = NULL;
    pKailleraInit = NULL;
    pKailleraShutdown = NULL;
    pKailleraSetInfos = NULL;
    pKailleraSelectServerDialog = NULL;
    pKailleraModifyPlayValues = NULL;
    pKailleraChatSend = NULL;
    pKailleraEndGame = NULL;
}

// Kaillera callbacks - called from Kaillera's internal thread

static int WINAPI KailleraGameCallback(char *game, int player, int numplayers)
{
    strncpy(Kaillera.GameName, game, sizeof(Kaillera.GameName) - 1);
    Kaillera.GameName[sizeof(Kaillera.GameName) - 1] = '\0';
    Kaillera.PlayerNumber = player;
    Kaillera.NumPlayers = numplayers;
    Kaillera.OutputPlayerCount = 0;

    ResetEvent(Kaillera.GameEndEvent);
    Kaillera.GameActive = true;

    // Tell the main window to load the ROM and start emulation.
    PostMessage(GUI.hWnd, WM_KAILLERA_GAME_START, 0, (LPARAM)Kaillera.GameName);

    // Run the input exchange loop on THIS thread (the Kaillera callback thread).
    // kailleraModifyPlayValues MUST be called from this thread context.
    // The main thread communicates with us via InputReadyEvent/OutputReadyEvent.
    while (Kaillera.GameActive)
    {
        // Wait for the main thread to provide local input, or for game end
        HANDLE waitHandles[2] = { Kaillera.InputReadyEvent, Kaillera.GameEndEvent };
        DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0 + 1 || !Kaillera.GameActive)
            break; // game ended

        if (waitResult == WAIT_OBJECT_0)
        {
            // Main thread has written Kaillera.LocalInput - call the DLL
            unsigned short buffer[8] = {};
            buffer[0] = Kaillera.LocalInput;

            int result = pKailleraModifyPlayValues(buffer, sizeof(unsigned short));

            if (result < 0)
            {
                Kaillera.OutputPlayerCount = -1;
            }
            else
            {
                int numReceived = result / (int)sizeof(unsigned short);
                if (numReceived > 8) numReceived = 8;
                for (int i = 0; i < numReceived; i++)
                    Kaillera.AllInputs[i] = buffer[i];
                Kaillera.OutputPlayerCount = numReceived;
            }

            // Signal the main thread that output is ready
            SetEvent(Kaillera.OutputReadyEvent);
        }
    }

    Kaillera.GameActive = false;
    return 0;
}

static void WINAPI KailleraChatReceivedCallback(char *nick, char *text)
{
    char msg[512];
    sprintf(msg, "[Kaillera] %s: %s", nick ? nick : "???", text ? text : "");
    S9xMessage(S9X_INFO, S9X_ROM_INFO, msg);
}

static void WINAPI KailleraClientDroppedCallback(char *nick, int playernb)
{
    char msg[256];
    sprintf(msg, "[Kaillera] Player %d (%s) disconnected", playernb, nick ? nick : "???");
    S9xMessage(S9X_INFO, S9X_ROM_INFO, msg);
}

// Worker thread data for the Kaillera server dialog
#define KAILLERA_GAMELIST_SIZE (64 * 1024)

struct KailleraDialogThreadData {
    HWND hWnd;
    char *gameList;
};

static unsigned __stdcall KailleraDialogThread(void *param)
{
    KailleraDialogThreadData *data = (KailleraDialogThreadData *)param;

    kailleraInfos infos;
    infos.appName = (char *)"Snes9x";
    infos.gameList = data->gameList;
    infos.gameCallback = KailleraGameCallback;
    infos.chatReceivedCallback = KailleraChatReceivedCallback;
    infos.clientDroppedCallback = KailleraClientDroppedCallback;
    infos.moreInfosCallback = NULL;

    pKailleraSetInfos(&infos);
    pKailleraSelectServerDialog(data->hWnd);

    delete[] data->gameList;
    delete data;
    return 0;
}

// Strip file extension from a filename
static void StripExtension(char *name)
{
    char *dot = strrchr(name, '.');
    if (dot) *dot = '\0';
}

// Check if a filename has a SNES ROM extension
static bool IsSNESRomFile(const TCHAR *filename)
{
    const TCHAR *ext = _tcsrchr(filename, TEXT('.'));
    if (!ext) return false;
    ext++;
    return (_tcsicmp(ext, TEXT("smc")) == 0 ||
            _tcsicmp(ext, TEXT("sfc")) == 0 ||
            _tcsicmp(ext, TEXT("fig")) == 0 ||
            _tcsicmp(ext, TEXT("swc")) == 0 ||
            _tcsicmp(ext, TEXT("zip")) == 0 ||
            _tcsicmp(ext, TEXT("7z")) == 0 ||
            _tcsicmp(ext, TEXT("gz")) == 0);
}

// Scan the ROM directory and build a null-separated, double-null-terminated game list
static int BuildGameList(char *gameList, int maxSize)
{
    int pos = 0;

    // First add the currently loaded ROM if any
    if (!Settings.StopEmulation && Memory.ROMName[0])
    {
        int len = (int)strlen(Memory.ROMName);
        if (pos + len + 2 < maxSize)
        {
            memcpy(gameList + pos, Memory.ROMName, len);
            pos += len + 1; // null separator included
        }
    }

    // Scan ROM directory for ROM files
    TCHAR searchPath[MAX_PATH];
    _stprintf(searchPath, TEXT("%s\\*"), GUI.RomDir);

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            if (!IsSNESRomFile(findData.cFileName))
                continue;

            // Convert filename to char, strip extension to get game name
            const char *fname = _tToChar(findData.cFileName);
            char gameName[256];
            strncpy(gameName, fname, sizeof(gameName) - 1);
            gameName[sizeof(gameName) - 1] = '\0';
            StripExtension(gameName);

            // Skip if already in list (e.g. matches currently loaded ROM)
            if (!Settings.StopEmulation && Memory.ROMName[0] &&
                strcmp(gameName, Memory.ROMName) == 0)
                continue;

            int len = (int)strlen(gameName);
            if (pos + len + 2 >= maxSize)
                break; // list full

            memcpy(gameList + pos, gameName, len);
            pos += len + 1;

        } while (FindNextFile(hFind, &findData));

        FindClose(hFind);
    }

    gameList[pos] = '\0'; // double-null terminator
    return pos;
}

// Send a dummy PING to warm up the Kaillera DLL's winsock state.
// The v0.84 DLL has a quirk where the first connection attempt
// doesn't actually send any packets. This pre-flight nudges winsock.
static void KailleraWarmup()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s != INVALID_SOCKET)
    {
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(KAILLERA_SERVER_PORT);
        const char ping[] = "PING\0";
        sendto(s, ping, 5, 0, (sockaddr *)&addr, sizeof(addr));
        // Wait briefly for PONG
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(s, &fds);
        timeval tv = { 0, 100000 }; // 100ms
        if (select(0, &fds, NULL, NULL, &tv) > 0)
        {
            char buf[32];
            recvfrom(s, buf, sizeof(buf), 0, NULL, NULL);
        }
        closesocket(s);
    }
    WSACleanup();
}

void KailleraOpenDialog(HWND hWnd)
{
    if (!Kaillera.DLLLoaded)
    {
        if (!KailleraLoadDLL())
            return;
    }

    // Warm up winsock/loopback before the DLL tries to connect
    KailleraWarmup();

    KailleraDialogThreadData *data = new KailleraDialogThreadData();
    data->hWnd = hWnd;
    data->gameList = new char[KAILLERA_GAMELIST_SIZE];
    memset(data->gameList, 0, KAILLERA_GAMELIST_SIZE);

    int count = BuildGameList(data->gameList, KAILLERA_GAMELIST_SIZE);

    if (count == 0)
    {
        // No ROMs found - add a placeholder message
        // (user must have ROMs in the ROM directory)
        MessageBox(hWnd,
            TEXT("No SNES ROM files found in the ROM directory.\n\n")
            TEXT("Please load a ROM first or set the ROM directory\n")
            TEXT("in Options -> Settings to a folder containing ROMs."),
            TEXT("Kaillera Netplay"), MB_OK | MB_ICONWARNING);
        delete[] data->gameList;
        delete data;
        return;
    }

    // Run the server dialog on a worker thread so it doesn't block the
    // main message loop (which drives the emulation)
    unsigned threadId;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, KailleraDialogThread, data, 0, &threadId);
    if (hThread)
        CloseHandle(hThread);
    else
    {
        delete[] data->gameList;
        delete data;
    }
}

void KailleraStopGame()
{
    if (!Kaillera.DLLLoaded || !Kaillera.GameActive)
        return;

    Kaillera.GameActive = false;
    SetEvent(Kaillera.GameEndEvent);
    // Wake the callback thread if it's waiting for input
    SetEvent(Kaillera.InputReadyEvent);
    // Wake the main thread if it's waiting for output
    SetEvent(Kaillera.OutputReadyEvent);
    pKailleraEndGame();
}

int KailleraExchangeInput(unsigned short localInput, unsigned short *allInputs, int maxPlayers)
{
    if (!Kaillera.DLLLoaded || !Kaillera.GameActive)
        return -1;

    // Pass our local input to the Kaillera callback thread
    Kaillera.LocalInput = localInput;
    SetEvent(Kaillera.InputReadyEvent);

    // Wait for the callback thread to return synchronized input
    DWORD waitResult = WaitForSingleObject(Kaillera.OutputReadyEvent, 5000);
    if (waitResult != WAIT_OBJECT_0)
        return -1; // timeout or error

    int count = Kaillera.OutputPlayerCount;
    if (count < 0)
        return -1;

    if (count > maxPlayers)
        count = maxPlayers;

    for (int i = 0; i < count; i++)
        allInputs[i] = Kaillera.AllInputs[i];

    return count;
}

#endif // KAILLERA_SUPPORT
