#pragma once

#ifdef RETROACHIEVEMENTS_SUPPORT

#include <cstdint>

// Lifecycle
void RA_Init(void *hWnd);
void RA_Shutdown();
void RA_DoFrame();

// Game lifecycle
void RA_OnLoadROM();
void RA_OnReset();
void RA_OnCloseROM();

// Save state integration
void RA_OnSaveState(const char *filename);
void RA_OnLoadState(const char *filename);

// Hardcore mode
bool RA_IsHardcoreModeActive();
bool RA_WarnDisableHardcore(const char *activity);

// Login
void RA_AttemptLogin();
void RA_ShowLoginDialog(void *hWnd);
void RA_Logout();
bool RA_IsLoggedIn();

// UI
void RA_ShowAchievementList(void *hWnd);

// ImGui overlay
void RA_RenderOverlay(int width, int height);

#endif // RETROACHIEVEMENTS_SUPPORT
