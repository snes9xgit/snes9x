#pragma once

#ifdef RETROACHIEVEMENTS_SUPPORT

#include <cstdint>

#include "rc_client.h"

// ---------------------------------------------------------------------------
// Platform callbacks — must be registered before calling RA_Init()
// ---------------------------------------------------------------------------
struct RAPlatformCallbacks
{
    // HTTP: must perform async request and invoke callback with response (even on error)
    rc_client_server_call_t server_call;

    // UI: show login dialog (platform-specific)
    void (*show_login_dialog)();

    // UI: show achievement list dialog (platform-specific)
    void (*show_achievement_list)();

    // UI: blocking yes/no confirmation for disabling hardcore mode
    // Returns true if user confirmed (disable hardcore), false to cancel
    bool (*confirm_disable_hardcore)(const char *activity);

    // Logging
    void (*log_message)(const char *message);

    // Config persistence: called when credentials change (login success / logout)
    void (*on_credentials_changed)(const char *username, const char *token);
};

void RA_RegisterPlatformCallbacks(const RAPlatformCallbacks &callbacks);

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------
void RA_Init();
void RA_Shutdown();
void RA_DoFrame();

// ---------------------------------------------------------------------------
// Game lifecycle
// ---------------------------------------------------------------------------
void RA_OnLoadROM();
void RA_OnReset();
void RA_OnCloseROM();

// ---------------------------------------------------------------------------
// Save state integration
// ---------------------------------------------------------------------------
void RA_OnSaveState(const char *filename);
void RA_OnLoadState(const char *filename);

// ---------------------------------------------------------------------------
// Hardcore mode
// ---------------------------------------------------------------------------
bool RA_IsHardcoreModeActive();
bool RA_WarnDisableHardcore(const char *activity);
void RA_SetEnabled(bool enabled);
void RA_SetHardcoreEnabled(bool enabled);

// ---------------------------------------------------------------------------
// Login
// ---------------------------------------------------------------------------
void RA_AttemptLogin(const char *username, const char *token);
void RA_LoginWithPassword(const char *username, const char *password);
void RA_ShowLoginDialog();
void RA_Logout();
bool RA_IsLoggedIn();

// ---------------------------------------------------------------------------
// UI
// ---------------------------------------------------------------------------
void RA_ShowAchievementList();

// ---------------------------------------------------------------------------
// ImGui overlay
// ---------------------------------------------------------------------------
void RA_RenderOverlay(int width, int height);

// ---------------------------------------------------------------------------
// Access to rc_client for platform dialogs
// ---------------------------------------------------------------------------
rc_client_t *RA_GetClient();

#endif // RETROACHIEVEMENTS_SUPPORT
