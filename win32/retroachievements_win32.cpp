#include "retroachievements.h"

#ifdef RETROACHIEVEMENTS_SUPPORT

#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <process.h>
#include <string>
#include <vector>
#include <cstdio>

#include "rc_client.h"

#include "wsnes9x.h"
#include "snes9x.h"

#pragma comment(lib, "wininet.lib")

extern HINSTANCE g_hInst;

// ---------------------------------------------------------------------------
// WinINet HTTP Server Call (async on background thread)
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

    // Build user agent: "SuperSnes9x/VERSION rcheevos/VERSION"
    char ua_clause[128] = {};
    rc_client_get_user_agent_clause(RA_GetClient(), ua_clause, sizeof(ua_clause));
    char user_agent[256];
    snprintf(user_agent, sizeof(user_agent), "SuperSnes9x/%s %s", VERSION, ua_clause);

    HINTERNET hInet = InternetOpenA(user_agent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInet)
    {
        response.http_status_code = RC_API_SERVER_RESPONSE_RETRYABLE_CLIENT_ERROR;
        req->callback(&response, req->callback_data);
        delete req;
        return 0;
    }

    if (req->post_data.empty())
    {
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

static void RC_CCONV ra_win32_server_call(const rc_api_request_t *request,
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
        rc_api_server_response_t response = {};
        response.http_status_code = RC_API_SERVER_RESPONSE_CLIENT_ERROR;
        callback(&response, callback_data);
        delete req;
    }
}

// ---------------------------------------------------------------------------
// Win32 Achievement List Dialog
// ---------------------------------------------------------------------------
struct RAAchSortData
{
    char title[256];
    uint32_t points;
    char status[20];
    char description[512];
};

static int raSortColumn = 0;
static bool raSortAscending = true;
static std::vector<RAAchSortData> raAchData;

static int CALLBACK RA_AchListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM)
{
    auto *a = reinterpret_cast<RAAchSortData *>(lParam1);
    auto *b = reinterpret_cast<RAAchSortData *>(lParam2);
    int result = 0;

    switch (raSortColumn)
    {
    case 0: result = _stricmp(a->title, b->title); break;
    case 1: result = (int)a->points - (int)b->points; break;
    case 2: result = _stricmp(a->status, b->status); break;
    case 3: result = _stricmp(a->description, b->description); break;
    }
    return raSortAscending ? result : -result;
}

static void RA_PopulateAchievementList(HWND hList)
{
    ListView_DeleteAllItems(hList);
    raAchData.clear();

    rc_client_t *client = RA_GetClient();
    if (!client)
        return;

    rc_client_achievement_list_t *list = rc_client_create_achievement_list(client,
        RC_CLIENT_ACHIEVEMENT_CATEGORY_CORE_AND_UNOFFICIAL,
        RC_CLIENT_ACHIEVEMENT_LIST_GROUPING_LOCK_STATE);

    if (!list)
        return;

    // Count total achievements and pre-allocate
    int total = 0;
    for (uint32_t b = 0; b < list->num_buckets; b++)
        total += list->buckets[b].num_achievements;
    raAchData.resize(total);

    int index = 0;
    for (uint32_t b = 0; b < list->num_buckets; b++)
    {
        const rc_client_achievement_bucket_t *bucket = &list->buckets[b];
        for (uint32_t a = 0; a < bucket->num_achievements; a++)
        {
            const rc_client_achievement_t *ach = bucket->achievements[a];
            RAAchSortData &sd = raAchData[index];

            strncpy(sd.title, ach->title ? ach->title : "", sizeof(sd.title) - 1);
            sd.points = ach->points;
            if (ach->unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_HARDCORE)
                strncpy(sd.status, "Unlocked (HC)", sizeof(sd.status));
            else if (ach->unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_SOFTCORE)
                strncpy(sd.status, "Unlocked", sizeof(sd.status));
            else if (ach->state == RC_CLIENT_ACHIEVEMENT_STATE_ACTIVE)
                strncpy(sd.status, "Locked", sizeof(sd.status));
            else
                strncpy(sd.status, "Inactive", sizeof(sd.status));
            strncpy(sd.description, ach->description ? ach->description : "", sizeof(sd.description) - 1);

            LVITEMA lvi = {};
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = index;
            lvi.lParam = (LPARAM)&sd;

            lvi.iSubItem = 0;
            lvi.pszText = sd.title;
            SendMessageA(hList, LVM_INSERTITEMA, 0, (LPARAM)&lvi);

            char pts[16];
            snprintf(pts, sizeof(pts), "%u", ach->points);
            lvi.iSubItem = 1;
            lvi.pszText = pts;
            SendMessageA(hList, LVM_SETITEMA, 0, (LPARAM)&lvi);

            lvi.iSubItem = 2;
            lvi.pszText = sd.status;
            SendMessageA(hList, LVM_SETITEMA, 0, (LPARAM)&lvi);

            lvi.iSubItem = 3;
            lvi.pszText = sd.description;
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
        ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

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

        rc_client_t *client = RA_GetClient();
        const rc_client_game_t *game = client ? rc_client_get_game_info(client) : nullptr;
        if (game)
            SetDlgItemTextA(hDlg, IDC_RA_GAME_TITLE, game->title);
        else
            SetDlgItemTextA(hDlg, IDC_RA_GAME_TITLE, "No game loaded");

        RA_PopulateAchievementList(hList);
        return TRUE;
    }

    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hDlg, &rc);
        HWND hList = GetDlgItem(hDlg, IDC_RA_ACHLIST);
        MoveWindow(hList, 6, 20, rc.right - 12, rc.bottom - 46, TRUE);
        HWND hBtn = GetDlgItem(hDlg, IDCANCEL);
        MoveWindow(hBtn, (rc.right - 50) / 2, rc.bottom - 20, 50, 14, TRUE);
        return TRUE;
    }

    case WM_NOTIFY:
    {
        NMHDR *pnm = (NMHDR *)lParam;
        if (pnm->idFrom == IDC_RA_ACHLIST && pnm->code == LVN_COLUMNCLICK)
        {
            NMLISTVIEW *pnmlv = (NMLISTVIEW *)lParam;
            if (raSortColumn == pnmlv->iSubItem)
                raSortAscending = !raSortAscending;
            else
            {
                raSortColumn = pnmlv->iSubItem;
                raSortAscending = true;
            }
            ListView_SortItems(GetDlgItem(hDlg, IDC_RA_ACHLIST), RA_AchListCompare, 0);
            return TRUE;
        }
        break;
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

// ---------------------------------------------------------------------------
// Win32 Login Dialog
// ---------------------------------------------------------------------------
static INT_PTR CALLBACK DlgRALoginProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetDlgItemTextA(hDlg, IDC_RA_USERNAME, GUI.RAUsername);
        SetFocus(GetDlgItem(hDlg, GUI.RAUsername[0] ? IDC_RA_PASSWORD : IDC_RA_USERNAME));
        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            char username[256] = {}, password[256] = {};
            GetDlgItemTextA(hDlg, IDC_RA_USERNAME, username, sizeof(username));
            GetDlgItemTextA(hDlg, IDC_RA_PASSWORD, password, sizeof(password));

            if (username[0] && password[0])
                RA_LoginWithPassword(username, password);

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

// ---------------------------------------------------------------------------
// Win32 Callback Implementations
// ---------------------------------------------------------------------------
static void ra_win32_show_login()
{
    DialogBoxA(g_hInst, MAKEINTRESOURCEA(IDD_RA_LOGIN), GUI.hWnd, DlgRALoginProc);
}

static void ra_win32_show_achievements()
{
    DialogBoxA(g_hInst, MAKEINTRESOURCEA(IDD_RA_ACHIEVEMENTS), GUI.hWnd, DlgRAAchievementsProc);
}

static bool ra_win32_confirm_disable_hardcore(const char *activity)
{
    char msg[512];
    snprintf(msg, sizeof(msg),
             "%s is not allowed in Hardcore mode.\n\n"
             "Disable Hardcore mode to proceed?\n"
             "(Achievement progress for the current session will be lost)",
             activity);

    int res = MessageBoxA(GUI.hWnd, msg, "RetroAchievements - Hardcore Mode",
                          MB_YESNO | MB_ICONWARNING);
    return (res == IDYES);
}

static void ra_win32_log(const char *message)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
}

static void ra_win32_credentials_changed(const char *username, const char *token)
{
    strncpy(GUI.RAUsername, username ? username : "", sizeof(GUI.RAUsername) - 1);
    GUI.RAUsername[sizeof(GUI.RAUsername) - 1] = '\0';
    strncpy(GUI.RAApiToken, token ? token : "", sizeof(GUI.RAApiToken) - 1);
    GUI.RAApiToken[sizeof(GUI.RAApiToken) - 1] = '\0';

    GUI.RAEnabled = (username && username[0] && token && token[0]);
}

// ---------------------------------------------------------------------------
// Public: Register Win32 callbacks
// ---------------------------------------------------------------------------
void RA_Win32_RegisterCallbacks()
{
    RAPlatformCallbacks cb = {};
    cb.server_call = ra_win32_server_call;
    cb.show_login_dialog = ra_win32_show_login;
    cb.show_achievement_list = ra_win32_show_achievements;
    cb.confirm_disable_hardcore = ra_win32_confirm_disable_hardcore;
    cb.log_message = ra_win32_log;
    cb.on_credentials_changed = ra_win32_credentials_changed;
    RA_RegisterPlatformCallbacks(cb);
}

#endif // RETROACHIEVEMENTS_SUPPORT
