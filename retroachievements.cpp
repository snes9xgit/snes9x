#include "retroachievements.h"

#ifdef RETROACHIEVEMENTS_SUPPORT

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <mutex>
#include <chrono>
#include <cstdio>
#include <cstring>

#include "rc_client.h"
#include "rc_consoles.h"
#include "rc_hash.h"

#include "imgui.h"
#include "stb_image.h"

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "sgb/sgb.h"

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
static bool g_netplayActive = false;
static bool g_loginUserInitiated = false;
static bool g_resetPendingOnLoad = false;

// ---------------------------------------------------------------------------
// Notification queue (thread-safe)
// ---------------------------------------------------------------------------
struct RANotification
{
    std::string title;
    std::string subtitle;
    float duration;
    float elapsed;
    std::chrono::steady_clock::time_point queued;
    bool seen;
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
    n.queued = std::chrono::steady_clock::now();
    n.seen = false;
    g_notifications.push_back(std::move(n));
}

// ---------------------------------------------------------------------------
// Tracks which console rcheevos identified the loaded game as. Memory reads
// re-route accordingly: SNES games use Memory.RAM/SRAM/FillRAM, GB/GBC games
// use the SGB subsystem's GB-side address space.
// ---------------------------------------------------------------------------
static uint32_t g_loadedConsoleId = RC_CONSOLE_SUPER_NINTENDO;

// ---------------------------------------------------------------------------
// Challenge indicator badges (thread-safe)
// ---------------------------------------------------------------------------
struct RABadgeImage
{
    int width = 0;
    int height = 0;
    std::vector<uint32_t> pixels;
    bool ready = false;
    bool failed = false;
};

struct RAChallengeIndicator
{
    uint32_t achievement_id;
    std::string badge_url;
    float elapsed;
};

struct RAProgressIndicator
{
    uint32_t achievement_id;
    std::string badge_url;
    std::string progress_text;
    float elapsed;
};

struct RABadgeFetch
{
    std::string url;
};

static std::map<std::string, RABadgeImage> g_badgeCache;
static std::vector<RAChallengeIndicator> g_challengeIndicators;
static std::vector<RAProgressIndicator> g_progressIndicators;
static std::mutex g_badgeMutex;
static bool g_showChallengeImages = true;

static const int RA_BADGE_MAX_DIM = 32;

void RA_SetShowChallengeImages(bool show)
{
    g_showChallengeImages = show;
}

static void RC_CCONV ra_badge_fetch_complete(const rc_api_server_response_t *response,
                                             void *callback_data)
{
    RABadgeFetch *fetch = static_cast<RABadgeFetch *>(callback_data);

    int w = 0, h = 0, comp = 0;
    unsigned char *rgba = nullptr;
    if (response && response->http_status_code == 200 && response->body && response->body_length > 0)
        rgba = stbi_load_from_memory(reinterpret_cast<const unsigned char *>(response->body),
                                     (int)response->body_length, &w, &h, &comp, 4);

    {
        std::lock_guard<std::mutex> lock(g_badgeMutex);
        RABadgeImage &img = g_badgeCache[fetch->url];
        if (rgba && w > 0 && h > 0)
        {
            const int out_w = (w > RA_BADGE_MAX_DIM) ? RA_BADGE_MAX_DIM : w;
            const int out_h = (h > RA_BADGE_MAX_DIM) ? RA_BADGE_MAX_DIM : h;
            img.width = out_w;
            img.height = out_h;
            img.pixels.resize((size_t)out_w * out_h);
            for (int oy = 0; oy < out_h; oy++)
            {
                int sy0 = oy * h / out_h;
                int sy1 = (oy + 1) * h / out_h;
                if (sy1 <= sy0)
                    sy1 = sy0 + 1;
                for (int ox = 0; ox < out_w; ox++)
                {
                    int sx0 = ox * w / out_w;
                    int sx1 = (ox + 1) * w / out_w;
                    if (sx1 <= sx0)
                        sx1 = sx0 + 1;
                    uint32_t r = 0, g = 0, b = 0, a = 0, n = 0;
                    for (int sy = sy0; sy < sy1; sy++)
                    {
                        const unsigned char *p = rgba + ((size_t)sy * w + sx0) * 4;
                        for (int sx = sx0; sx < sx1; sx++, p += 4)
                        {
                            r += p[0];
                            g += p[1];
                            b += p[2];
                            a += p[3];
                            n++;
                        }
                    }
                    img.pixels[(size_t)oy * out_w + ox] = IM_COL32(r / n, g / n, b / n, a / n);
                }
            }
            img.ready = true;
        }
        else
        {
            img.failed = true;
        }
    }

    if (rgba)
        stbi_image_free(rgba);
    delete fetch;
}

static void ra_request_badge(const char *url)
{
    if (!url || !url[0] || !g_callbacks.server_call)
        return;

    {
        std::lock_guard<std::mutex> lock(g_badgeMutex);
        if (g_badgeCache.find(url) != g_badgeCache.end())
            return;
        g_badgeCache.emplace(url, RABadgeImage());
    }

    rc_api_request_t request = {};
    request.url = url;
    g_callbacks.server_call(&request, ra_badge_fetch_complete, new RABadgeFetch{url}, g_rcClient);
}

static void ra_add_challenge_indicator(const rc_client_achievement_t *achievement)
{
    char url[512];
    if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED,
                                            url, sizeof(url)) != RC_OK)
        url[0] = '\0';

    {
        std::lock_guard<std::mutex> lock(g_badgeMutex);
        for (const auto &ind : g_challengeIndicators)
        {
            if (ind.achievement_id == achievement->id)
                return;
        }
        g_challengeIndicators.push_back({achievement->id, url, 0.0f});
    }

    ra_request_badge(url);
}

static void ra_remove_challenge_indicator(uint32_t achievement_id)
{
    std::lock_guard<std::mutex> lock(g_badgeMutex);
    for (auto it = g_challengeIndicators.begin(); it != g_challengeIndicators.end(); ++it)
    {
        if (it->achievement_id == achievement_id)
        {
            g_challengeIndicators.erase(it);
            return;
        }
    }
}

static void ra_show_progress_indicator(const rc_client_achievement_t *achievement)
{
    char url[512];
    if (rc_client_achievement_get_image_url(achievement, achievement->state,
                                            url, sizeof(url)) != RC_OK)
        url[0] = '\0';

    {
        std::lock_guard<std::mutex> lock(g_badgeMutex);
        RAProgressIndicator *existing = nullptr;
        for (auto &ind : g_progressIndicators)
        {
            if (ind.achievement_id == achievement->id)
            {
                existing = &ind;
                break;
            }
        }
        if (existing)
        {
            existing->badge_url = url;
            existing->progress_text = achievement->measured_progress;
        }
        else
        {
            g_progressIndicators.push_back({achievement->id, url,
                                            achievement->measured_progress, 0.0f});
        }
    }

    ra_request_badge(url);
}

static void ra_clear_progress_indicator_for(uint32_t achievement_id)
{
    std::lock_guard<std::mutex> lock(g_badgeMutex);
    for (auto it = g_progressIndicators.begin(); it != g_progressIndicators.end(); ++it)
    {
        if (it->achievement_id == achievement_id)
        {
            g_progressIndicators.erase(it);
            return;
        }
    }
}

static void ra_clear_indicators(bool clear_cache)
{
    std::lock_guard<std::mutex> lock(g_badgeMutex);
    g_challengeIndicators.clear();
    g_progressIndicators.clear();
    if (clear_cache)
        g_badgeCache.clear();
}

// ---------------------------------------------------------------------------
// Callback: Memory Read
// ---------------------------------------------------------------------------
static uint32_t RC_CCONV ra_read_memory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client)
{
    if (g_loadedConsoleId == RC_CONSOLE_GAMEBOY ||
        g_loadedConsoleId == RC_CONSOLE_GAMEBOY_COLOR)
    {
        for (uint32_t i = 0; i < num_bytes; ++i)
            buffer[i] = S9xSGBPeekRAByte(address + i);
        return num_bytes;
    }

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
            ra_clear_progress_indicator_for(event->achievement->id);

            char msg[256];
            snprintf(msg, sizeof(msg), "%s (%u pts)",
                     event->achievement->title, event->achievement->points);
            RA_QueueNotification("Achievement Unlocked!", msg, 5.0f);
        }
        break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_SHOW:
        if (event->achievement)
        {
            ra_add_challenge_indicator(event->achievement);
            if (!g_showChallengeImages)
                RA_QueueNotification("Challenge Active", event->achievement->title, 3.0f);
        }
        break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_HIDE:
        if (event->achievement)
            ra_remove_challenge_indicator(event->achievement->id);
        break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_SHOW:
    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_UPDATE:
        if (event->achievement && event->achievement->measured_progress[0])
        {
            if (g_showChallengeImages)
                ra_show_progress_indicator(event->achievement);
            else
                RA_QueueNotification(event->achievement->title,
                                     event->achievement->measured_progress, 2.0f);
        }
        break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_HIDE:
        // rc_client drives a single shared progress tracker; HIDE carries no
        // achievement pointer, so drop everything we're showing.
        {
            std::lock_guard<std::mutex> lock(g_badgeMutex);
            g_progressIndicators.clear();
        }
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
        // Prefer the platform's marshalled reset; events can arrive on the
        // HTTP worker thread where touching the core directly is unsafe.
        if (g_callbacks.reset_emulator)
            g_callbacks.reset_emulator();
        else
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
    const bool user_initiated = g_loginUserInitiated;
    g_loginUserInitiated = false;

    if (result == RC_OK)
    {
        const rc_client_user_t *user = rc_client_get_user_info(client);
        if (user)
        {
            if (g_callbacks.on_credentials_changed)
                g_callbacks.on_credentials_changed(user->username, user->token);

            g_raEnabled = true;

            char msg[256];
            snprintf(msg, sizeof(msg), "Logged in as %s (%u points, %u softcore)",
                     user->display_name, user->score, user->score_softcore);
            RA_QueueNotification("Login Successful", msg, 4.0f);

            if (g_callbacks.on_login_result)
                g_callbacks.on_login_result(true, msg, user_initiated);

            if (!Settings.StopEmulation)
            {
                // Achievements arm mid-session: reset once the game is
                // identified so the session starts from power-on.
                g_resetPendingOnLoad = true;
                RA_OnLoadROM();
            }
        }
    }
    else
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s",
                 (error_message && error_message[0])
                     ? error_message
                     : "Login failed. Check your credentials and try again.");
        RA_QueueNotification("Unable to Login", msg, 5.0f);

        if (g_callbacks.on_login_result)
            g_callbacks.on_login_result(false, msg, user_initiated);
    }
}

// ---------------------------------------------------------------------------
// Game loaded callback
// ---------------------------------------------------------------------------
static void RC_CCONV ra_game_loaded_callback(int result, const char *error_message,
                                              rc_client_t *client, void *userdata)
{
    const bool reset_pending = g_resetPendingOnLoad;
    g_resetPendingOnLoad = false;

    if (result == RC_OK)
    {
        const rc_client_game_t *game = rc_client_get_game_info(client);
        if (game)
        {
            rc_client_user_game_summary_t summary = {};
            rc_client_get_user_game_summary(client, &summary);

            char msg[256];
            snprintf(msg, sizeof(msg), "%s (%u of %u achievements earned)",
                     game->title, summary.num_unlocked_achievements,
                     summary.num_core_achievements);
            RA_QueueNotification("Game Loaded", msg, 4.0f);

            // Login happened while this game was already running — restart it
            // so achievement/hardcore state is valid from power-on.
            if (reset_pending && !Settings.StopEmulation && g_callbacks.reset_emulator)
                g_callbacks.reset_emulator();
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

    ra_clear_indicators(true);

    g_initialized = false;
    g_raEnabled = false;
    g_netplayActive = false;
    g_loginUserInitiated = false;
    g_resetPendingOnLoad = false;
}

void RA_DoFrame()
{
    if (!g_rcClient || !g_raEnabled)
        return;

    // Netplay: peers emulate everyone's inputs, so other players would earn
    // unlocks here. Idle keeps the server session alive without triggers.
    if (g_netplayActive)
    {
        rc_client_idle(g_rcClient);
        return;
    }

    rc_client_do_frame(g_rcClient);
}

void RA_SetNetplayActive(bool active)
{
    if (active == g_netplayActive)
        return;

    g_netplayActive = active;

    if (!g_rcClient || !g_raEnabled || !rc_client_get_game_info(g_rcClient))
        return;

    if (active)
    {
        RA_QueueNotification("RetroAchievements", "Achievements suspended during netplay", 5.0f);
    }
    else
    {
        // Frames advanced while suspended; re-arm triggers from a clean slate.
        rc_client_reset(g_rcClient);
        RA_QueueNotification("RetroAchievements", "Achievements re-enabled", 3.0f);
    }
}

// ---------------------------------------------------------------------------
// Public API: Game lifecycle
// ---------------------------------------------------------------------------
void RA_OnLoadROM()
{
    if (!g_rcClient || !g_raEnabled)
        return;

    ra_clear_indicators(true);

    // Game Boy / Game Boy Color routed through SGB. Memory.ROM here either
    // holds the SGB BIOS (authentic-BIOS mode) or the leftover SNES ROM
    // bytes — neither hashes to anything in the RA database. Hash the GB
    // ROM bytes from the SGB subsystem against the GB / GBC console IDs
    // instead. CGB flag at cart header byte 0x143: 0x80 = GB+GBC, 0xC0 =
    // GBC only; anything else is plain DMG/SGB.
    if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive)
    {
        const unsigned char *gb_data = nullptr;
        size_t gb_size = 0;
        if (S9xSGBGetROMBytes(&gb_data, &gb_size) && gb_data && gb_size >= 0x150)
        {
            const uint8_t cgb_flag = gb_data[0x143];
            const uint32_t console = (cgb_flag == 0x80 || cgb_flag == 0xC0)
                                         ? RC_CONSOLE_GAMEBOY_COLOR
                                         : RC_CONSOLE_GAMEBOY;
            g_loadedConsoleId = console;
            const char *gb_path = (Settings.GBRomPath[0] != '\0') ? Settings.GBRomPath : NULL;
            rc_client_begin_identify_and_load_game(g_rcClient, console, gb_path,
                                                   gb_data, gb_size,
                                                   ra_game_loaded_callback, nullptr);
            return;
        }
    }

    g_loadedConsoleId = RC_CONSOLE_SUPER_NINTENDO;
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

    ra_clear_indicators(false);
    rc_client_reset(g_rcClient);
}

void RA_OnCloseROM()
{
    if (!g_rcClient || !g_raEnabled)
        return;

    ra_clear_indicators(true);
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
        g_loginUserInitiated = false;
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
        g_loginUserInitiated = true;
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

bool RA_GetLoggedInUserString(char *buf, size_t size)
{
    if (!g_rcClient)
        return false;

    const rc_client_user_t *user = rc_client_get_user_info(g_rcClient);
    if (!user)
        return false;

    const bool hardcore = rc_client_get_hardcore_enabled(g_rcClient) != 0;
    snprintf(buf, size, "%s (%u points)", user->display_name,
             hardcore ? user->score : user->score_softcore);
    return true;
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
static float ra_overlay_scale(int height)
{
    float scale = (float)height / 720.0f;
    if (scale < 1.0f)
        scale = 1.0f;
    if (scale > 3.0f)
        scale = 3.0f;
    return scale;
}

static void ra_draw_badge(ImDrawList *drawList, const std::string &badge_url,
                          float x0, float y0, float badge_size, float scale, float fade)
{
    const RABadgeImage *img = nullptr;
    auto found = g_badgeCache.find(badge_url);
    if (found != g_badgeCache.end() && found->second.ready)
        img = &found->second;

    if (img)
    {
        const float px_w = badge_size / img->width;
        const float px_h = badge_size / img->height;
        for (int y = 0; y < img->height; y++)
        {
            const uint32_t *row = img->pixels.data() + (size_t)y * img->width;
            int x = 0;
            while (x < img->width)
            {
                const uint32_t color = row[x];
                int run = x + 1;
                while (run < img->width && row[run] == color)
                    run++;

                uint32_t a = (color >> IM_COL32_A_SHIFT) & 0xFF;
                if (a != 0)
                {
                    a = (uint32_t)(a * fade);
                    const ImU32 c = (color & ~((ImU32)0xFF << IM_COL32_A_SHIFT)) | (a << IM_COL32_A_SHIFT);
                    drawList->AddRectFilled(ImVec2(x0 + x * px_w, y0 + y * px_h),
                                            ImVec2(x0 + run * px_w, y0 + (y + 1) * px_h),
                                            c);
                }
                x = run;
            }
        }
    }
    else
    {
        const ImU32 bg = IM_COL32(0x10, 0x10, 0x40, (int)(0xC0 * fade));
        const ImU32 frame = IM_COL32(0xFF, 0xD7, 0x00, (int)(0xFF * fade));
        drawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + badge_size, y0 + badge_size),
                                bg, 2.0f * scale);
        drawList->AddRect(ImVec2(x0, y0), ImVec2(x0 + badge_size, y0 + badge_size),
                          frame, 2.0f * scale);
    }
}

static void ra_render_challenge_indicators(ImDrawList *drawList, int width, int height, float dt)
{
    std::lock_guard<std::mutex> lock(g_badgeMutex);

    if (g_challengeIndicators.empty())
        return;

    const float scale = ra_overlay_scale(height);
    const float badge_size = 32.0f * scale;
    const float margin = 8.0f * scale;
    const float gap = 4.0f * scale;
    const float y0 = (float)height - margin - badge_size;

    float x_right = (float)width - margin;

    int drawn = 0;
    for (auto &ind : g_challengeIndicators)
    {
        if (drawn >= 8)
            break;

        ind.elapsed += dt;
        float fade = ind.elapsed / 0.25f;
        if (fade > 1.0f)
            fade = 1.0f;

        const float x0 = x_right - badge_size;
        ra_draw_badge(drawList, ind.badge_url, x0, y0, badge_size, scale, fade);

        x_right = x0 - gap;
        drawn++;
    }
}

static void ra_render_progress_indicators(ImDrawList *drawList, int width, int height, float dt)
{
    std::lock_guard<std::mutex> lock(g_badgeMutex);

    if (g_progressIndicators.empty())
        return;

    const float scale = ra_overlay_scale(height);
    const float badge_size = 32.0f * scale;
    const float margin = 8.0f * scale;
    const float gap = 4.0f * scale;
    const float pad = 6.0f;

    float y1 = (float)height - margin;
    if (!g_challengeIndicators.empty())
        y1 -= badge_size + gap;
    const float y0 = y1 - badge_size;

    float x_right = (float)width - margin;

    int drawn = 0;
    for (auto &ind : g_progressIndicators)
    {
        if (drawn >= 4)
            break;

        ind.elapsed += dt;
        float fade = ind.elapsed / 0.25f;
        if (fade > 1.0f)
            fade = 1.0f;

        const float bx1 = x_right;
        const float bx0 = bx1 - badge_size;
        float unit_x0 = bx0;

        if (!ind.progress_text.empty())
        {
            const ImVec2 textSize = ImGui::CalcTextSize(ind.progress_text.c_str());
            unit_x0 = bx0 - textSize.x - pad * 2;
            const ImU32 bg = IM_COL32(0x10, 0x10, 0x40, (int)(0xE0 * fade));
            const ImU32 txt = IM_COL32(0xFF, 0xFF, 0xFF, (int)(0xFF * fade));

            drawList->AddRectFilled(ImVec2(unit_x0, y0), ImVec2(bx1, y1), bg, 4.0f);
            drawList->AddText(ImVec2(unit_x0 + pad, y0 + (badge_size - textSize.y) * 0.5f), txt,
                              ind.progress_text.c_str());
        }

        ra_draw_badge(drawList, ind.badge_url, bx0, y0, badge_size, scale, fade);

        x_right = unit_x0 - gap;
        drawn++;
    }
}

void RA_RenderOverlay(int width, int height)
{
    if (!g_initialized)
        return;

    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - g_lastTime).count();
    g_lastTime = now;

    if (dt > 0.5f)
        dt = 0.5f;

    auto *drawList = ImGui::GetForegroundDrawList();

    {
        std::lock_guard<std::mutex> lock(g_notifMutex);

        float y = 10.0f;
        const float padding = 8.0f;
        const float maxWidth = 400.0f;
        const ImU32 bgColor = IM_COL32(0x10, 0x10, 0x40, 0xE0);
        const ImU32 titleColor = IM_COL32(0xFF, 0xD7, 0x00, 0xFF);
        const ImU32 textColor = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF);

        auto it = g_notifications.begin();
        while (it != g_notifications.end())
        {
            // The overlay only renders while a game runs; drop toasts that
            // aged out unseen so a ROM load doesn't replay the backlog.
            if (!it->seen)
            {
                if (std::chrono::duration<float>(now - it->queued).count() > it->duration + 2.0f)
                {
                    it = g_notifications.erase(it);
                    continue;
                }
                it->seen = true;
            }

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

    if (g_showChallengeImages)
    {
        ra_render_challenge_indicators(drawList, width, height, dt);
        ra_render_progress_indicators(drawList, width, height, dt);
    }
}

#endif // RETROACHIEVEMENTS_SUPPORT
