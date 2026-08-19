#pragma once
#include <string>
#include <vector>

#include "EmuBinding.hpp"

struct EmuConfig
{
    static std::string findConfigFile();
    static std::string findConfigDir();
    bool setDefaults(int section = -1);
    void config(const std::string &filename, bool write);
    void loadFile(const std::string &filename)
    {
        config(filename, false);
    }
    void saveFile(const std::string &filename)
    {
        config(filename, true);
    }
    void setVRRConfig(bool enable = true);
    bool vrr_enabled = false;
    int saved_input_rate = 0;
    double saved_fixed_frame_rate = 0.0;
    int saved_speed_sync_method = 0;
    bool saved_enable_vsync = false;

    // Control the appearance of the on-disk config file itself (mirrors win32/GTK).
    bool config_show_comments = true;
    bool config_nice_alignment = true;

    // Operational
    std::string last_rom_folder;
    int main_window_width = 0;
    int main_window_height = 0;
    int cheat_dialog_width = 0;
    int cheat_dialog_height = 0;
    int shader_parameters_dialog_width = 0;
    int shader_parameters_dialog_height = 0;
    int current_save_slot = 0;
    int current_save_bank = 0;
    std::vector<std::string> recently_used;

    // General
    bool fullscreen_on_open;
    bool disable_screensaver;
    bool pause_emulation_when_unfocused;

    bool show_frame_rate;
    bool show_indicators;
    bool show_pressed_keys;
    bool show_time;

    std::string language;

    // Display
    std::string display_driver;
    int display_device_index;
    bool enable_vsync;
    bool bilinear_filter;
    bool reduce_input_lag;
    bool adjust_for_vrr;
    bool use_shader;
    std::string shader;
    std::string last_shader_folder;

    bool scale_image;
    bool maintain_aspect_ratio;
    bool use_integer_scaling;
    int aspect_ratio_numerator;
    int aspect_ratio_denominator;
    bool show_overscan;
    enum HighResolutionEffect
    {
        eLeaveAlone = 0,
        eScaleDown = 1,
        eScaleUp = 2
    };
    int high_resolution_effect;

    std::string software_filter;
    std::string software_filter_hires;

    enum DisplayMessages
    {
        eOnscreen = 0,
        eInscreen = 1,
        eNone = 2
    };
    int display_messages;
    int osd_size;

    // Game Boy frame blending (Super Game Boy / SGB BIOS games only). Mirrors the
    // win32 "Game Boy Image" options: a blend mode, the layer it applies to, and an
    // "Auto Layer Transparency" toggle that picks both per-game from a built-in
    // known-flicker table. Maps onto Settings.GBFrameBlend / GBFrameBlendLayer /
    // GBFrameBlendAuto and must stay in sync with the GBBlendMode / GBBlendLayer
    // enums in snes9x.h.
    enum GBFrameBlend
    {
        eGBBlendOff = 0,
        eGBBlendSimple = 1,
        eGBBlendLCD = 2
    };
    int gb_frame_blend;
    enum GBFrameBlendLayer
    {
        eGBBlendLayerAll = 0,
        eGBBlendLayerBackground = 1,
        eGBBlendLayerWindow = 2,
        eGBBlendLayerSprites = 3
    };
    int gb_frame_blend_layer;
    bool gb_frame_blend_auto;

    bool color_correction;
    bool color_adjustments_enabled;
    int color_gamma;
    int color_contrast;
    int color_saturation;

    // Sound
    std::string sound_driver;
    std::string sound_device;
    int playback_rate;
    int audio_buffer_size_ms;

    bool adjust_input_rate_automatically;
    int input_rate;
    bool dynamic_rate_control;
    double dynamic_rate_limit;
    bool mute_audio;
    bool mute_audio_during_alternate_speed;

    // Volume: master output gain (0..100, percent) applied post-mix. Two slots —
    // Regular for normal play, FastForward for turbo/rewind. SGB-mix per-source
    // gains scale GB/SPC inside S9xMixSpcOverGB; only meaningful in SGB BIOS mode.
    int master_volume_regular;
    int master_volume_fast_forward;
    int sgb_mix_volume_spc;
    int sgb_mix_volume_gb;
    // Pre-amp gain (0..S9X_GAIN_MAX_DB, whole dB, 0 = unity) applied on top of
    // the volume percentages. Regular rides the post-mix master; SPC/GB fold
    // into the SGB-mix channel gains.
    int gain_regular;
    int sgb_mix_gain_spc;
    int sgb_mix_gain_gb;

    // Emulation

    enum SpeedSyncMethod
    {
        eTimer = 0,
        eTimerWithFrameskip = 1,
        eSoundSync = 2,
        eUnlimited = 3
    };
    int speed_sync_method;
    double fixed_frame_rate;
    int fast_forward_skip_frames;

    int rewind_buffer_size;
    int rewind_frame_interval;

    int run_ahead_frames;

    // Emulation/Hacks

    bool allow_invalid_vram_access;
    bool snapshot_screenshots;
    bool allow_opposing_dpad_directions;
    enum Overclock
    {
        eNoOverclock = 0,
        eAutoFastROM = 1,
        eLow = 2,
        eHigh = 3
    };
    int overclock;
    bool remove_sprite_limit;
    bool enable_shadow_buffer;
    int superfx_clock_multiplier;
    enum SoundFilter
    {
        eNearest = 0,
        eLinear = 1,
        eGaussian = 2,
        eCubic = 3,
        eSinc = 4
    };
    int sound_filter;

    // SGB BIOS mode for GB/GBC ROMs: 0=No BIOS (BIOS-less), 1=SGB1, 2=SGB2 (default).
    // Persisted under [SGB] BIOSPreference to match win32/GTK/CLI, applied to
    // Settings.SGB_BIOSPreference; the BIOS menu keeps this in sync.
    int sgb_bios_preference;

    // Files
    enum FileLocation
    {
        eROMDirectory = 0,
        eConfigDirectory = 1,
        eCustomDirectory = 2
    };
    int sram_location;
    int state_location;
    int cheat_location;
    int patch_location;
    int export_location;
    int bios_location;
    std::string sram_folder;
    std::string state_folder;
    std::string cheat_folder;
    std::string patch_folder;
    std::string export_folder;
    std::string bios_folder;

    int sram_save_interval;

    // RetroAchievements
    bool ra_enabled = false;
    bool ra_hardcore_mode = false;
    std::string ra_username;
    std::string ra_api_token;
    std::string ra_emulator_name = "SuperSnes9x";

    enum PortConfiguration
    {
        eOneController = 0,
        eTwoControllers,
        eMousePlusController,
        eSuperScopePlusController,
        eControllerPlusMultitap
    };
    int port_configuration;

    static const int allowed_bindings = 4;
    static const int num_controller_bindings = 18;
    static const int num_shortcuts = 71;

    /* Save states are organized in banks of slots, as on win32. The state
     * file extension is the flat index, bank * save_slots_per_bank + slot. */
    static const int save_slots_per_bank = 10;
    static const int num_save_banks = 10;

    bool automap_gamepads;
    bool enable_rumble;

    struct
    {
        struct
        {
            EmuBinding buttons[num_controller_bindings * allowed_bindings];
        } controller[5];

        EmuBinding shortcuts[num_shortcuts * allowed_bindings];
    } binding;

    struct controller_t
    {
        EmuBinding buttons[num_controller_bindings];
    };
    std::vector<controller_t> additional_controllers;


    static const char **getDefaultShortcutKeys();
    static const char **getShortcutNames();

    enum Shortcut
    {
        eOpenROM = 0,
        eFastForward,
        eToggleFastForward,
        ePauseContinue,
        eSoftReset,
        ePowerCycle,
        eQuit,
        eToggleFullscreen,
        eSaveScreenshot,
        eSaveSPC,
        eSaveState,
        eLoadState,
        eIncreaseSlot,
        eDecreaseSlot,
        eIncreaseBank,
        eDecreaseBank,
        eSaveState0,
        eSaveState1,
        eSaveState2,
        eSaveState3,
        eSaveState4,
        eSaveState5,
        eSaveState6,
        eSaveState7,
        eSaveState8,
        eSaveState9,
        eLoadState0,
        eLoadState1,
        eLoadState2,
        eLoadState3,
        eLoadState4,
        eLoadState5,
        eLoadState6,
        eLoadState7,
        eLoadState8,
        eLoadState9,
        eSelectSlot0,
        eSelectSlot1,
        eSelectSlot2,
        eSelectSlot3,
        eSelectSlot4,
        eSelectSlot5,
        eSelectSlot6,
        eSelectSlot7,
        eSelectSlot8,
        eSelectSlot9,
        eSaveStateDialog,
        eLoadStateDialog,
        eSaveStateFile,
        eLoadStateFile,
        eRewind,
        eGrabMouse,
        eSwapControllers1and2,
        eToggleBG0,
        eToggleBG1,
        eToggleBG2,
        eToggleBG3,
        eToggleSprites,
        eChangeBackdrop,
        eToggleSoundChannel1,
        eToggleSoundChannel2,
        eToggleSoundChannel3,
        eToggleSoundChannel4,
        eToggleSoundChannel5,
        eToggleSoundChannel6,
        eToggleSoundChannel7,
        eToggleSoundChannel8,
        eToggleAllSoundChannels,
        eStartRecording,
        eStopRecording,
        eSeekToFrame,
    };
};