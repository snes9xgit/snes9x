#include "retroachievements.h"

#ifdef RETROACHIEVEMENTS_SUPPORT

#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include <cstdio>
#include <cstring>

#include "rc_client.h"
#include "rc_consoles.h"
#include "rc_hash.h"

#include "imgui.h"

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"

// ---------------------------------------------------------------------------
// Platform callbacks (set by platform before RA_Init)
// ---------------------------------------------------------------------------
static RAPlatformCallbacks g_callbacks = {};

void RA_RegisterPlatformCallbacks(const RAPlatformCallbacks &callbacks)
{
    g_callbacks = callbacks;
}

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
static rc_client_t *g_rcClient = nullptr;
static bool g_initialized = false;
static bool g_raEnabled = false;

// ---------------------------------------------------------------------------
// Notification queue (thread-safe)
// ---------------------------------------------------------------------------
struct RANotification
{
    std::string title;
    std::string subtitle;
    float duration;
    float elapsed;
};

static std::deque<RANotification> g_notifications;
static std::mutex g_notifMutex;
static std::chrono::steady_clock::time_point g_lastTime;

static void RA_QueueNotification(const char *title, const char *subtitle, float duration)
{
    std::lock_guard<std::mutex> lock(g_notifMutex);
    RANotification n;
    n.title = title ? title : "";
    n.subtitle = subtitle ? subtitle : "";
    n.duration = duration;
    n.elapsed = 0.0f;
    g_notifications.push_back(std::move(n));
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
        src = Memory.RAM + address;
        available = 0x20000 - address;
    }
    else if (address < 0x0A0000)
    {
        uint32_t offset = address - 0x020000;
        src = Memory.SRAM + offset;
        available = 0x80000 - offset;
    }
    else if (address < 0x0A0800)
    {
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
    if (g_callbacks.log_message)
    {
        char buf[2048];
        snprintf(buf, sizeof(buf), "[RetroAchievements] %s", message);
        g_callbacks.log_message(buf);
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
            if (g_callbacks.on_credentials_changed)
                g_callbacks.on_credentials_changed(user->username, user->token);

            g_raEnabled = true;

            char msg[256];
            snprintf(msg, sizeof(msg), "Welcome, %s!", user->display_name);
            RA_QueueNotification("Login Successful", msg, 3.0f);

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
        RA_QueueNotification("RetroAchievements", "Game not recognized", 3.0f);
    }
    else
    {
        RA_QueueNotification("RA Error", error_message ? error_message : "Failed to load game", 5.0f);
    }
}

// ---------------------------------------------------------------------------
// Callback: HTTP server call (delegates to platform)
// ---------------------------------------------------------------------------
static void RC_CCONV ra_server_call_dispatch(const rc_api_request_t *request,
                                              rc_client_server_callback_t callback,
                                              void *callback_data, rc_client_t *client)
{
    if (g_callbacks.server_call)
        g_callbacks.server_call(request, callback, callback_data, client);
}

// ---------------------------------------------------------------------------
// Public API: Lifecycle
// ---------------------------------------------------------------------------
void RA_Init()
{
    if (g_initialized)
        return;

    g_lastTime = std::chrono::steady_clock::now();

    g_rcClient = rc_client_create(ra_read_memory, ra_server_call_dispatch);
    if (!g_rcClient)
        return;

    rc_client_enable_logging(g_rcClient, RC_CLIENT_LOG_LEVEL_VERBOSE, ra_log_message);
    rc_client_set_event_handler(g_rcClient, ra_event_handler);
    rc_client_set_allow_background_memory_reads(g_rcClient, 1);

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

    {
        std::lock_guard<std::mutex> lock(g_notifMutex);
        g_notifications.clear();
    }

    g_initialized = false;
    g_raEnabled = false;
}

void RA_DoFrame()
{
    if (!g_rcClient || !g_raEnabled)
        return;

    rc_client_do_frame(g_rcClient);
}

// ---------------------------------------------------------------------------
// Public API: Game lifecycle
// ---------------------------------------------------------------------------
void RA_OnLoadROM()
{
    if (!g_rcClient || !g_raEnabled)
        return;

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
    if (!g_rcClient || !g_raEnabled)
        return;

    rc_client_reset(g_rcClient);
}

void RA_OnCloseROM()
{
    if (!g_rcClient || !g_raEnabled)
        return;

    rc_client_unload_game(g_rcClient);
}

// ---------------------------------------------------------------------------
// Public API: Save state integration
// ---------------------------------------------------------------------------
void RA_OnSaveState(const char *filename)
{
    if (!g_rcClient || !g_raEnabled)
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
    if (!g_rcClient || !g_raEnabled)
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
        rc_client_deserialize_progress(g_rcClient, nullptr);
    }
}

// ---------------------------------------------------------------------------
// Public API: Hardcore mode
// ---------------------------------------------------------------------------
bool RA_IsHardcoreModeActive()
{
    if (!g_rcClient || !g_raEnabled)
        return false;

    return rc_client_get_hardcore_enabled(g_rcClient) != 0;
}

bool RA_WarnDisableHardcore(const char *activity)
{
    if (!RA_IsHardcoreModeActive())
        return true;

    if (g_callbacks.confirm_disable_hardcore)
    {
        if (g_callbacks.confirm_disable_hardcore(activity))
        {
            rc_client_set_hardcore_enabled(g_rcClient, 0);
            return true;
        }
        return false;
    }

    return true;
}

void RA_SetEnabled(bool enabled)
{
    g_raEnabled = enabled;
}

void RA_SetHardcoreEnabled(bool enabled)
{
    if (g_rcClient)
        rc_client_set_hardcore_enabled(g_rcClient, enabled ? 1 : 0);
}

// ---------------------------------------------------------------------------
// Public API: Login
// ---------------------------------------------------------------------------
void RA_AttemptLogin(const char *username, const char *token)
{
    if (!g_rcClient)
        return;

    if (username && username[0] && token && token[0])
    {
        rc_client_begin_login_with_token(g_rcClient,
                                         username, token,
                                         ra_login_callback, nullptr);
    }
}

void RA_LoginWithPassword(const char *username, const char *password)
{
    if (!g_rcClient)
        return;

    if (username && username[0] && password && password[0])
    {
        rc_client_begin_login_with_password(g_rcClient, username, password,
                                             ra_login_callback, nullptr);
    }
}

void RA_ShowLoginDialog()
{
    if (g_callbacks.show_login_dialog)
        g_callbacks.show_login_dialog();
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

    if (g_callbacks.on_credentials_changed)
        g_callbacks.on_credentials_changed("", "");

    RA_QueueNotification("RetroAchievements", "Logged out", 3.0f);
}

// ---------------------------------------------------------------------------
// Public API: UI
// ---------------------------------------------------------------------------
void RA_ShowAchievementList()
{
    if (g_callbacks.show_achievement_list)
        g_callbacks.show_achievement_list();
}

rc_client_t *RA_GetClient()
{
    return g_rcClient;
}

// ---------------------------------------------------------------------------
// Public API: ImGui Overlay
// ---------------------------------------------------------------------------
void RA_RenderOverlay(int width, int height)
{
    if (!g_initialized)
        return;

    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - g_lastTime).count();
    g_lastTime = now;

    if (dt > 0.5f)
        dt = 0.5f;

    std::lock_guard<std::mutex> lock(g_notifMutex);

    auto *drawList = ImGui::GetForegroundDrawList();
    float y = 10.0f;
    const float padding = 8.0f;
    const float maxWidth = 400.0f;
    const ImU32 bgColor = IM_COL32(0x10, 0x10, 0x40, 0xE0);
    const ImU32 titleColor = IM_COL32(0xFF, 0xD7, 0x00, 0xFF);
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
            boxH += 4.0f;

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
}

#endif // RETROACHIEVEMENTS_SUPPORT
