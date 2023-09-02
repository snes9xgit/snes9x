#ifndef __EMU_CONFIG_HPP
#define __EMU_CONFIG_HPP

#include <string>
#include <vector>

#include "EmuBinding.hpp"

struct EmuConfig
{
    static std::string findConfigFile();
    static std::string findConfigDir();
    bool setDefaults(int section = -1);
    void config(std::string filename, bool write);
    void loadFile(std::string filename)
    {
        config(filename, false);
    }
    void saveFile(std::string filename)
    {
        config(filename, true);
    }
    void setVRRConfig(bool enable = true);
    bool vrr_enabled = false;
    int saved_input_rate = 0;
    double saved_fixed_frame_rate = 0.0;
    int saved_speed_sync_method = 0;
    bool saved_enable_vsync = false;

    // Operational
    std::string last_rom_folder;
    int main_window_width = 0;
    int main_window_height = 0;
    int cheat_dialog_width = 0;
    int cheat_dialog_height = 0;
    int shader_parameters_dialog_width = 0;
    int shader_parameters_dialog_height = 0;
    std::vector<std::string> recently_used;

    // General
    bool fullscreen_on_open;
    bool disable_screensaver;
    bool pause_emulation_when_unfocused;

    bool show_frame_rate;
    bool show_indicators;
    bool show_pressed_keys;
    bool show_time;

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

    enum DisplayMessages
    {
        eOnscreen = 0,
        eInscreen = 1,
        eNone = 2
    };
    int display_messages;
    int osd_size;

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

    // Emulation/Hacks

    bool allow_invalid_vram_access;
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
    std::string sram_folder;
    std::string state_folder;
    std::string cheat_folder;
    std::string patch_folder;
    std::string export_folder;

    int sram_save_interval;

    static const int allowed_bindings = 4;
    static const int num_controller_bindings = 18;
    static const int num_shortcuts = 55;

    struct
    {
        struct
        {
            EmuBinding buttons[num_controller_bindings * allowed_bindings];
        } controller[5];

        EmuBinding shortcuts[num_shortcuts * allowed_bindings];
    } binding;

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

#endif