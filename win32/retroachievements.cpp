#include "retroachievements.h"

#ifdef RETROACHIEVEMENTS_SUPPORT

#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <process.h>
#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <cstring>

#include "rc_client.h"
#include "rc_consoles.h"
#include "rc_hash.h"

#include "imgui.h"

#include "../snes9x.h"
#include "../memmap.h"
#include "../cpuexec.h"
#include "wsnes9x.h"

#pragma comment(lib, "wininet.lib")

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
extern HINSTANCE g_hInst;

static rc_client_t *g_rcClient = nullptr;
static HWND g_hWnd = nullptr;
static CRITICAL_SECTION g_notifCS;
static bool g_initialized = false;

// ---------------------------------------------------------------------------
// Notification queue (thread-safe)
// ---------------------------------------------------------------------------
struct RANotification
{
    std::string title;
    std::string subtitle;
    float duration;   // seconds total
    float elapsed;    // seconds elapsed
};

static std::deque<RANotification> g_notifications;
static LARGE_INTEGER g_perfFreq;
static LARGE_INTEGER g_lastPerfCount;

static void RA_QueueNotification(const char *title, const char *subtitle, float duration)
{
    EnterCriticalSection(&g_notifCS);
    RANotification n;
    n.title = title ? title : "";
    n.subtitle = subtitle ? subtitle : "";
    n.duration = duration;
    n.elapsed = 0.0f;
    g_notifications.push_back(std::move(n));
    LeaveCriticalSection(&g_notifCS);
}

// ---------------------------------------------------------------------------
// Callback: Memory Read
// ---------------------------------------------------------------------------
static uint32_t RC_CCONV ra_read_memory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client)
{
    const uint8_t *src = nullptr;
    uint32_t available = 0;

    if (address < 0x20000)
    {
        // System WRAM: 0x000000-0x01FFFF -> Memory.RAM[0x20000]
        src = Memory.RAM + address;
        available = 0x20000 - address;
    }
    else if (address < 0x0A0000)
    {
        // SRAM: 0x020000-0x09FFFF -> Memory.SRAM[0x80000]
        uint32_t offset = address - 0x020000;
        src = Memory.SRAM + offset;
        available = 0x80000 - offset;
    }
    else if (address < 0x0A0800)
    {
        // SA-1 IRAM: 0x0A0000-0x0A07FF -> Memory.FillRAM + 0x3000
        uint32_t offset = address - 0x0A0000;
        src = Memory.FillRAM + 0x3000 + offset;
        available = 0x800 - offset;
    }
    else
    {
        return 0;
    }

    uint32_t to_copy = (num_bytes < available) ? num_bytes : available;
    memcpy(buffer, src, to_copy);
    return to_copy;
}

// ---------------------------------------------------------------------------
// Callback: Logging
// ---------------------------------------------------------------------------
static void RC_CCONV ra_log_message(const char *message, const rc_client_t *client)
{
    char buf[2048];
    snprintf(buf, sizeof(buf), "[RetroAchievements] %s\n", message);
    OutputDebugStringA(buf);
}

// ---------------------------------------------------------------------------
// Callback: HTTP Server Call (async via WinINet on background thread)
// ---------------------------------------------------------------------------
struct RAHttpRequest
{
    std::string url;
    std::string post_data;
    std::string content_type;
    rc_client_server_callback_t callback;
    void *callback_data;
};

static unsigned __stdcall ra_http_thread(void *param)
{
    RAHttpRequest *req = static_cast<RAHttpRequest *>(param);

    rc_api_server_response_t response = {};
    std::string response_body;

    HINTERNET hInet = InternetOpenA("snes9x", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInet)
    {
        response.http_status_code = RC_API_SERVER_RESPONSE_RETRYABLE_CLIENT_ERROR;
        req->callback(&response, req->callback_data);
        delete req;
        return 0;
    }

    if (req->post_data.empty())
    {
        // GET request
        HINTERNET hUrl = InternetOpenUrlA(hInet, req->url.c_str(), NULL, 0,
                                          INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if (hUrl)
        {
            char buf[8192];
            DWORD bytesRead;
            while (InternetReadFile(hUrl, buf, sizeof(buf), &bytesRead) && bytesRead > 0)
                response_body.append(buf, bytesRead);

            DWORD statusCode = 0;
            DWORD statusSize = sizeof(statusCode);
            HttpQueryInfoA(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                           &statusCode, &statusSize, NULL);
            response.http_status_code = (int)statusCode;
            InternetCloseHandle(hUrl);
        }
        else
        {
            response.http_status_code = RC_API_SERVER_RESPONSE_RETRYABLE_CLIENT_ERROR;
        }
    }
    else
    {
        // POST request - parse host and path from URL
        URL_COMPONENTSA urlParts = {};
        char hostBuf[256], pathBuf[1024];
        urlParts.dwStructSize = sizeof(urlParts);
        urlParts.lpszHostName = hostBuf;
        urlParts.dwHostNameLength = sizeof(hostBuf);
        urlParts.lpszUrlPath = pathBuf;
        urlParts.dwUrlPathLength = sizeof(pathBuf);

        if (InternetCrackUrlA(req->url.c_str(), 0, 0, &urlParts))
        {
            HINTERNET hConnect = InternetConnectA(hInet, hostBuf, urlParts.nPort,
                                                   NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
            if (hConnect)
            {
                DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
                if (urlParts.nScheme == INTERNET_SCHEME_HTTPS)
                    flags |= INTERNET_FLAG_SECURE;

                HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", pathBuf,
                                                       NULL, NULL, NULL, flags, 0);
                if (hRequest)
                {
                    const char *ct = req->content_type.empty()
                                         ? "application/x-www-form-urlencoded"
                                         : req->content_type.c_str();
                    std::string headers = "Content-Type: ";
                    headers += ct;

                    BOOL sent = HttpSendRequestA(hRequest, headers.c_str(), (DWORD)headers.size(),
                                                  (LPVOID)req->post_data.c_str(),
                                                  (DWORD)req->post_data.size());
                    if (sent)
                    {
                        char buf[8192];
                        DWORD bytesRead;
                        while (InternetReadFile(hRequest, buf, sizeof(buf), &bytesRead) && bytesRead > 0)
                            response_body.append(buf, bytesRead);

                        DWORD statusCode = 0;
                        DWORD statusSize = sizeof(statusCode);
                        HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                                       &statusCode, &statusSize, NULL);
                        response.http_status_code = (int)statusCode;
                    }
                    else
                    {
                        response.http_status_code = RC_API_SERVER_RESPONSE_RETRYABLE_CLIENT_ERROR;
                    }
                    InternetCloseHandle(hRequest);
                }
                InternetCloseHandle(hConnect);
            }
        }
        else
        {
            response.http_status_code = RC_API_SERVER_RESPONSE_CLIENT_ERROR;
        }
    }

    InternetCloseHandle(hInet);

    response.body = response_body.c_str();
    response.body_length = response_body.size();

    req->callback(&response, req->callback_data);
    delete req;
    return 0;
}

static void RC_CCONV ra_server_call(const rc_api_request_t *request,
                                     rc_client_server_callback_t callback,
                                     void *callback_data, rc_client_t *client)
{
    RAHttpRequest *req = new RAHttpRequest();
    req->url = request->url ? request->url : "";
    req->post_data = request->post_data ? request->post_data : "";
    req->content_type = request->content_type ? request->content_type : "";
    req->callback = callback;
    req->callback_data = callback_data;

    unsigned tid;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ra_http_thread, req, 0, &tid);
    if (hThread)
        CloseHandle(hThread);
    else
    {
        // Thread creation failed - still must call callback
        rc_api_server_response_t response = {};
        response.http_status_code = RC_API_SERVER_RESPONSE_CLIENT_ERROR;
        callback(&response, callback_data);
        delete req;
    }
}

// ---------------------------------------------------------------------------
// Callback: Event Handler
// ---------------------------------------------------------------------------
static void RC_CCONV ra_event_handler(const rc_client_event_t *event, rc_client_t *client)
{
    switch (event->type)
    {
    case RC_CLIENT_EVENT_ACHIEVEMENT_TRIGGERED:
        if (event->achievement)
        {
            char msg[256];
            snprintf(msg, sizeof(msg), "%s (%u pts)",
                     event->achievement->title, event->achievement->points);
            RA_QueueNotification("Achievement Unlocked!", msg, 5.0f);
        }
        break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_SHOW:
        if (event->achievement)
            RA_QueueNotification("Challenge Active", event->achievement->title, 3.0f);
        break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_SHOW:
    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_UPDATE:
        if (event->achievement && event->achievement->measured_progress[0])
            RA_QueueNotification(event->achievement->title,
                                 event->achievement->measured_progress, 2.0f);
        break;

    case RC_CLIENT_EVENT_LEADERBOARD_STARTED:
        if (event->leaderboard)
            RA_QueueNotification("Leaderboard Started", event->leaderboard->title, 3.0f);
        break;

    case RC_CLIENT_EVENT_LEADERBOARD_FAILED:
        if (event->leaderboard)
            RA_QueueNotification("Leaderboard Failed", event->leaderboard->title, 3.0f);
        break;

    case RC_CLIENT_EVENT_LEADERBOARD_SUBMITTED:
        if (event->leaderboard)
            RA_QueueNotification("Leaderboard Submitted", event->leaderboard->title, 3.0f);
        break;

    case RC_CLIENT_EVENT_GAME_COMPLETED:
        RA_QueueNotification("Congratulations!", "All achievements earned!", 8.0f);
        break;

    case RC_CLIENT_EVENT_RESET:
        S9xReset();
        break;

    case RC_CLIENT_EVENT_SERVER_ERROR:
        if (event->server_error)
            RA_QueueNotification("RA Server Error", event->server_error->error_message, 5.0f);
        break;

    case RC_CLIENT_EVENT_DISCONNECTED:
        RA_QueueNotification("RetroAchievements", "Disconnected - unlocks pending", 5.0f);
        break;

    case RC_CLIENT_EVENT_RECONNECTED:
        RA_QueueNotification("RetroAchievements", "Reconnected", 3.0f);
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// Login callback
// ---------------------------------------------------------------------------
static void RC_CCONV ra_login_callback(int result, const char *error_message,
                                        rc_client_t *client, void *userdata)
{
    if (result == RC_OK)
    {
        const rc_client_user_t *user = rc_client_get_user_info(client);
        if (user)
        {
            // Persist token for future auto-login
            strncpy(GUI.RAUsername, user->username, sizeof(GUI.RAUsername) - 1);
            GUI.RAUsername[sizeof(GUI.RAUsername) - 1] = '\0';
            strncpy(GUI.RAApiToken, user->token, sizeof(GUI.RAApiToken) - 1);
            GUI.RAApiToken[sizeof(GUI.RAApiToken) - 1] = '\0';

            // Auto-enable RA on successful login
            GUI.RAEnabled = true;

            char msg[256];
            snprintf(msg, sizeof(msg), "Welcome, %s!", user->display_name);
            RA_QueueNotification("Login Successful", msg, 3.0f);

            // If a game is already loaded, identify it
            if (!Settings.StopEmulation)
                RA_OnLoadROM();
        }
    }
    else
    {
        RA_QueueNotification("Unable to Login", "Check your credentials and try again.", 5.0f);
    }
}

// ---------------------------------------------------------------------------
// Game loaded callback
// ---------------------------------------------------------------------------
static void RC_CCONV ra_game_loaded_callback(int result, const char *error_message,
                                              rc_client_t *client, void *userdata)
{
    if (result == RC_OK)
    {
        const rc_client_game_t *game = rc_client_get_game_info(client);
        if (game)
        {
            char msg[256];
            snprintf(msg, sizeof(msg), "%s", game->title);
            RA_QueueNotification("Game Loaded", msg, 3.0f);
        }
    }
    else if (result == RC_NO_GAME_LOADED)
    {
        // Game not in RA database - not an error
        RA_QueueNotification("RetroAchievements", "Game not recognized", 3.0f);
    }
    else
    {
        RA_QueueNotification("RA Error", error_message ? error_message : "Failed to load game", 5.0f);
    }
}

// ---------------------------------------------------------------------------
// Public API: Lifecycle
// ---------------------------------------------------------------------------
void RA_Init(void *hWnd)
{
    if (g_initialized)
        return;

    g_hWnd = (HWND)hWnd;
    InitializeCriticalSection(&g_notifCS);
    QueryPerformanceFrequency(&g_perfFreq);
    QueryPerformanceCounter(&g_lastPerfCount);

    g_rcClient = rc_client_create(ra_read_memory, ra_server_call);
    if (!g_rcClient)
        return;

    rc_client_enable_logging(g_rcClient, RC_CLIENT_LOG_LEVEL_VERBOSE, ra_log_message);
    rc_client_set_event_handler(g_rcClient, ra_event_handler);
    rc_client_set_hardcore_enabled(g_rcClient, GUI.RAHardcoreMode ? 1 : 0);

    g_initialized = true;
}

void RA_Shutdown()
{
    if (!g_initialized)
        return;

    if (g_rcClient)
    {
        rc_client_destroy(g_rcClient);
        g_rcClient = nullptr;
    }

    EnterCriticalSection(&g_notifCS);
    g_notifications.clear();
    LeaveCriticalSection(&g_notifCS);

    DeleteCriticalSection(&g_notifCS);
    g_initialized = false;
}

void RA_DoFrame()
{
    if (!g_rcClient || !GUI.RAEnabled)
        return;

    rc_client_do_frame(g_rcClient);
}

// ---------------------------------------------------------------------------
// Public API: Game lifecycle
// ---------------------------------------------------------------------------
void RA_OnLoadROM()
{
    if (!g_rcClient || !GUI.RAEnabled)
        return;

    // Identify and load game using ROM data
    rc_client_begin_identify_and_load_game(g_rcClient,
                                           RC_CONSOLE_SUPER_NINTENDO,
                                           NULL,
                                           Memory.ROM,
                                           Memory.CalculatedSize,
                                           ra_game_loaded_callback,
                                           nullptr);
}

void RA_OnReset()
{
    if (!g_rcClient || !GUI.RAEnabled)
        return;

    rc_client_reset(g_rcClient);
}

void RA_OnCloseROM()
{
    if (!g_rcClient || !GUI.RAEnabled)
        return;

    rc_client_unload_game(g_rcClient);
}

// ---------------------------------------------------------------------------
// Public API: Save state integration
// ---------------------------------------------------------------------------
void RA_OnSaveState(const char *filename)
{
    if (!g_rcClient || !GUI.RAEnabled)
        return;

    size_t size = rc_client_progress_size(g_rcClient);
    if (size == 0)
        return;

    std::vector<uint8_t> buffer(size);
    if (rc_client_serialize_progress_sized(g_rcClient, buffer.data(), size) == RC_OK)
    {
        std::string ra_filename = std::string(filename) + ".rcheevos";
        FILE *f = fopen(ra_filename.c_str(), "wb");
        if (f)
        {
            fwrite(buffer.data(), 1, size, f);
            fclose(f);
        }
    }
}

void RA_OnLoadState(const char *filename)
{
    if (!g_rcClient || !GUI.RAEnabled)
        return;

    std::string ra_filename = std::string(filename) + ".rcheevos";
    FILE *f = fopen(ra_filename.c_str(), "rb");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        size_t size = (size_t)ftell(f);
        fseek(f, 0, SEEK_SET);
        std::vector<uint8_t> buffer(size);
        fread(buffer.data(), 1, size, f);
        fclose(f);
        rc_client_deserialize_progress_sized(g_rcClient, buffer.data(), size);
    }
    else
    {
        // No companion file - reset achievement progress
        rc_client_deserialize_progress(g_rcClient, nullptr);
    }
}

// ---------------------------------------------------------------------------
// Public API: Hardcore mode
// ---------------------------------------------------------------------------
bool RA_IsHardcoreModeActive()
{
    if (!g_rcClient || !GUI.RAEnabled)
        return false;

    return rc_client_get_hardcore_enabled(g_rcClient) != 0;
}

bool RA_WarnDisableHardcore(const char *activity)
{
    if (!RA_IsHardcoreModeActive())
        return true;

    char msg[512];
    snprintf(msg, sizeof(msg),
             "%s is not allowed in Hardcore mode.\n\n"
             "Disable Hardcore mode to proceed?\n"
             "(Achievement progress for the current session will be lost)",
             activity);

    int res = MessageBoxA(g_hWnd, msg, "RetroAchievements - Hardcore Mode",
                          MB_YESNO | MB_ICONWARNING);
    if (res == IDYES)
    {
        rc_client_set_hardcore_enabled(g_rcClient, 0);
        GUI.RAHardcoreMode = false;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Public API: Login
// ---------------------------------------------------------------------------
void RA_AttemptLogin()
{
    if (!g_rcClient)
        return;

    if (GUI.RAApiToken[0] && GUI.RAUsername[0])
    {
        rc_client_begin_login_with_token(g_rcClient,
                                         GUI.RAUsername, GUI.RAApiToken,
                                         ra_login_callback, nullptr);
    }
}

// ---------------------------------------------------------------------------
// Achievement List dialog
// ---------------------------------------------------------------------------
static void RA_PopulateAchievementList(HWND hList)
{
    ListView_DeleteAllItems(hList);

    if (!g_rcClient)
        return;

    rc_client_achievement_list_t *list = rc_client_create_achievement_list(g_rcClient,
        RC_CLIENT_ACHIEVEMENT_CATEGORY_CORE_AND_UNOFFICIAL,
        RC_CLIENT_ACHIEVEMENT_LIST_GROUPING_LOCK_STATE);

    if (!list)
        return;

    int index = 0;
    for (uint32_t b = 0; b < list->num_buckets; b++)
    {
        const rc_client_achievement_bucket_t *bucket = &list->buckets[b];
        for (uint32_t a = 0; a < bucket->num_achievements; a++)
        {
            const rc_client_achievement_t *ach = bucket->achievements[a];

            LVITEMA lvi = {};
            lvi.mask = LVIF_TEXT;
            lvi.iItem = index;

            // Column 0: Title
            lvi.iSubItem = 0;
            lvi.pszText = const_cast<char *>(ach->title);
            SendMessageA(hList, LVM_INSERTITEMA, 0, (LPARAM)&lvi);

            // Column 1: Points
            char pts[16];
            snprintf(pts, sizeof(pts), "%u", ach->points);
            lvi.iSubItem = 1;
            lvi.pszText = pts;
            SendMessageA(hList, LVM_SETITEMA, 0, (LPARAM)&lvi);

            // Column 2: Status
            const char *status;
            if (ach->unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_HARDCORE)
                status = "Unlocked (HC)";
            else if (ach->unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_SOFTCORE)
                status = "Unlocked";
            else if (ach->state == RC_CLIENT_ACHIEVEMENT_STATE_ACTIVE)
                status = "Locked";
            else
                status = "Inactive";
            lvi.iSubItem = 2;
            lvi.pszText = const_cast<char *>(status);
            SendMessageA(hList, LVM_SETITEMA, 0, (LPARAM)&lvi);

            // Column 3: Description
            lvi.iSubItem = 3;
            lvi.pszText = const_cast<char *>(ach->description);
            SendMessageA(hList, LVM_SETITEMA, 0, (LPARAM)&lvi);

            index++;
        }
    }

    rc_client_destroy_achievement_list(list);
}

static INT_PTR CALLBACK DlgRAAchievementsProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        HWND hList = GetDlgItem(hDlg, IDC_RA_ACHLIST);

        // Set full-row select and grid lines
        ListView_SetExtendedListViewStyle(hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Add columns
        LVCOLUMNA col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH;

        col.pszText = (char *)"Title";
        col.cx = 150;
        SendMessageA(hList, LVM_INSERTCOLUMNA, 0, (LPARAM)&col);

        col.pszText = (char *)"Points";
        col.cx = 45;
        SendMessageA(hList, LVM_INSERTCOLUMNA, 1, (LPARAM)&col);

        col.pszText = (char *)"Status";
        col.cx = 85;
        SendMessageA(hList, LVM_INSERTCOLUMNA, 2, (LPARAM)&col);

        col.pszText = (char *)"Description";
        col.cx = 280;
        SendMessageA(hList, LVM_INSERTCOLUMNA, 3, (LPARAM)&col);

        // Set game title
        const rc_client_game_t *game = g_rcClient ? rc_client_get_game_info(g_rcClient) : nullptr;
        if (game)
            SetDlgItemTextA(hDlg, IDC_RA_GAME_TITLE, game->title);
        else
            SetDlgItemTextA(hDlg, IDC_RA_GAME_TITLE, "No game loaded");

        RA_PopulateAchievementList(hList);
        return TRUE;
    }

    case WM_SIZE:
    {
        // Resize list to fill dialog
        RECT rc;
        GetClientRect(hDlg, &rc);
        HWND hList = GetDlgItem(hDlg, IDC_RA_ACHLIST);
        MoveWindow(hList, 6, 20, rc.right - 12, rc.bottom - 46, TRUE);
        // Reposition close button
        HWND hBtn = GetDlgItem(hDlg, IDCANCEL);
        MoveWindow(hBtn, (rc.right - 50) / 2, rc.bottom - 20, 50, 14, TRUE);
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

void RA_ShowAchievementList(void *hWnd)
{
    if (!g_rcClient)
        return;

    DialogBoxA(g_hInst, MAKEINTRESOURCEA(IDD_RA_ACHIEVEMENTS), (HWND)hWnd, DlgRAAchievementsProc);
}

// Login dialog procedure
static INT_PTR CALLBACK DlgRALoginProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetDlgItemTextA(hDlg, IDC_RA_USERNAME, GUI.RAUsername);
        SetFocus(GetDlgItem(hDlg, GUI.RAUsername[0] ? IDC_RA_PASSWORD : IDC_RA_USERNAME));
        return FALSE; // we set focus manually

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            char username[256] = {}, password[256] = {};
            GetDlgItemTextA(hDlg, IDC_RA_USERNAME, username, sizeof(username));
            GetDlgItemTextA(hDlg, IDC_RA_PASSWORD, password, sizeof(password));

            if (g_rcClient && username[0] && password[0])
            {
                rc_client_begin_login_with_password(g_rcClient, username, password,
                                                     ra_login_callback, nullptr);
            }
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

void RA_ShowLoginDialog(void *hWnd)
{
    if (!g_rcClient)
        return;

    DialogBoxA(g_hInst, MAKEINTRESOURCEA(IDD_RA_LOGIN), (HWND)hWnd, DlgRALoginProc);
}

bool RA_IsLoggedIn()
{
    if (!g_rcClient)
        return false;

    return rc_client_get_user_info(g_rcClient) != nullptr;
}

void RA_Logout()
{
    if (!g_rcClient)
        return;

    rc_client_logout(g_rcClient);

    // Clear stored credentials
    GUI.RAUsername[0] = '\0';
    GUI.RAApiToken[0] = '\0';

    RA_QueueNotification("RetroAchievements", "Logged out", 3.0f);
}

// ---------------------------------------------------------------------------
// Public API: ImGui Overlay
// ---------------------------------------------------------------------------
void RA_RenderOverlay(int width, int height)
{
    if (!g_initialized)
        return;

    // Calculate delta time
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float dt = (float)(now.QuadPart - g_lastPerfCount.QuadPart) / (float)g_perfFreq.QuadPart;
    g_lastPerfCount = now;

    // Clamp dt to avoid jumps
    if (dt > 0.5f)
        dt = 0.5f;

    EnterCriticalSection(&g_notifCS);

    // Update and render notifications
    auto *drawList = ImGui::GetForegroundDrawList();
    float y = 10.0f;
    const float padding = 8.0f;
    const float maxWidth = 400.0f;
    const ImU32 bgColor = IM_COL32(0x10, 0x10, 0x40, 0xE0);
    const ImU32 titleColor = IM_COL32(0xFF, 0xD7, 0x00, 0xFF); // gold
    const ImU32 textColor = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF);

    auto it = g_notifications.begin();
    while (it != g_notifications.end())
    {
        it->elapsed += dt;
        if (it->elapsed >= it->duration)
        {
            it = g_notifications.erase(it);
            continue;
        }

        // Fade in/out
        float alpha = 1.0f;
        if (it->elapsed < 0.3f)
            alpha = it->elapsed / 0.3f;
        else if (it->elapsed > it->duration - 0.5f)
            alpha = (it->duration - it->elapsed) / 0.5f;

        ImU32 bg = (bgColor & 0x00FFFFFF) | ((uint32_t)(((bgColor >> 24) & 0xFF) * alpha) << 24);
        ImU32 tc = (titleColor & 0x00FFFFFF) | ((uint32_t)(((titleColor >> 24) & 0xFF) * alpha) << 24);
        ImU32 txt = (textColor & 0x00FFFFFF) | ((uint32_t)(((textColor >> 24) & 0xFF) * alpha) << 24);

        ImVec2 titleSize = ImGui::CalcTextSize(it->title.c_str(), nullptr, false, maxWidth);
        ImVec2 subtitleSize = {0, 0};
        if (!it->subtitle.empty())
            subtitleSize = ImGui::CalcTextSize(it->subtitle.c_str(), nullptr, false, maxWidth);

        float boxW = (titleSize.x > subtitleSize.x ? titleSize.x : subtitleSize.x) + padding * 2;
        float boxH = titleSize.y + subtitleSize.y + padding * 2;
        if (!it->subtitle.empty())
            boxH += 4.0f; // spacing between title and subtitle

        float x = ((float)width - boxW) * 0.5f;

        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + boxW, y + boxH), bg, 6.0f);

        drawList->AddText(nullptr, 0.0f, ImVec2(x + padding, y + padding), tc,
                          it->title.c_str(), nullptr, maxWidth);

        if (!it->subtitle.empty())
        {
            drawList->AddText(nullptr, 0.0f,
                              ImVec2(x + padding, y + padding + titleSize.y + 4.0f), txt,
                              it->subtitle.c_str(), nullptr, maxWidth);
        }

        y += boxH + 6.0f;
        ++it;
    }

    LeaveCriticalSection(&g_notifCS);
}

#endif // RETROACHIEVEMENTS_SUPPORT
