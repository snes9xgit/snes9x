#include <cstdio>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "EmuConfig.hpp"
#include "EmuBinding.hpp"
#include <cctype>
#include <cstdlib>
#include <QGuiApplication>
#include <QDir>
#include <QFile>

// The config file is written in the shared snes9x conffile format (see
// conffile.cpp) — the same one the win32 and GTK ports use. This replaces the
// old QSettings INI backend, which could not carry inline comments, and gives
// the Qt port a documented, human-readable config with a comment toggle.
// Included last so snes9x.h's macros don't leak into the Qt headers above.
#include "conffile.h"

static const char *shortcut_names[] =
{
    "OpenROM",
    "EmuTurbo",
    "ToggleEmuTurbo",
    "PauseContinue",
    "SoftReset",
    "Reset",
    "Quit",
    "ToggleFullscreen",
    "Screenshot",
    "SaveSPC",
    "SaveState",
    "LoadState",
    "IncreaseSlot",
    "DecreaseSlot",
    "IncreaseBank",
    "DecreaseBank",
    "QuickSave000",
    "QuickSave001",
    "QuickSave002",
    "QuickSave003",
    "QuickSave004",
    "QuickSave005",
    "QuickSave006",
    "QuickSave007",
    "QuickSave008",
    "QuickSave009",
    "QuickLoad000",
    "QuickLoad001",
    "QuickLoad002",
    "QuickLoad003",
    "QuickLoad004",
    "QuickLoad005",
    "QuickLoad006",
    "QuickLoad007",
    "QuickLoad008",
    "QuickLoad009",
    "SelectSlot0",
    "SelectSlot1",
    "SelectSlot2",
    "SelectSlot3",
    "SelectSlot4",
    "SelectSlot5",
    "SelectSlot6",
    "SelectSlot7",
    "SelectSlot8",
    "SelectSlot9",
    "SaveStateDialog",
    "LoadStateDialog",
    "SaveStateFile",
    "LoadStateFile",
    "Rewind",
    "GrabMouse",
    "SwapControllers1and2",
    "ToggleBG0",
    "ToggleBG1",
    "ToggleBG2",
    "ToggleBG3",
    "ToggleSprites",
    "ToggleBackdrop",
    "SoundChannel0",
    "SoundChannel1",
    "SoundChannel2",
    "SoundChannel3",
    "SoundChannel4",
    "SoundChannel5",
    "SoundChannel6",
    "SoundChannel7",
    "SoundChannelsOn",
    "BeginRecordingMovie",
    "EndRecordingMovie",
    "SeekToFrame",
};

static const char *default_controller_keys[] =
{
    "Keyboard Ctrl+o", //    eOpenROM
    "Keyboard Tab", //    eFastForward
    "Keyboard `", //    eToggleFastForward
    "Keyboard p", //    ePauseContinue
    "Keyboard Ctrl+r", //    eSoftReset
    "", //    ePowerCycle
    "Keyboard Ctrl+q", //    eQuit
    "Keyboard F11", //    eToggleFullscreen
    "", //    eSaveScreenshot
    "", //    eSaveSPC
    "Keyboard F2", //    eSaveState
    "Keyboard F4", //    eLoadState
    "Keyboard F6", //    eIncreaseSlot
    "Keyboard F5", //    eDecreaseSlot
    "Keyboard Shift+F6", //    eIncreaseBank
    "Keyboard Shift+F5", //    eDecreaseBank
    "Keyboard 0", //    eSaveState0
    "Keyboard 1", //    eSaveState1
    "Keyboard 2", //    eSaveState2
    "Keyboard 3", //    eSaveState3
    "Keyboard 4", //    eSaveState4
    "Keyboard 5", //    eSaveState5
    "Keyboard 6", //    eSaveState6
    "Keyboard 7", //    eSaveState7
    "Keyboard 8", //    eSaveState8
    "Keyboard 9", //    eSaveState9
    "Keyboard Ctrl+0", //    eLoadState0
    "Keyboard Ctrl+1", //    eLoadState1
    "Keyboard Ctrl+2", //    eLoadState2
    "Keyboard Ctrl+3", //    eLoadState3
    "Keyboard Ctrl+4", //    eLoadState4
    "Keyboard Ctrl+5", //    eLoadState5
    "Keyboard Ctrl+6", //    eLoadState6
    "Keyboard Ctrl+7", //    eLoadState7
    "Keyboard Ctrl+8", //    eLoadState8
    "Keyboard Ctrl+9", //    eLoadState9
    "", //    eSelectSlot0
    "", //    eSelectSlot1
    "", //    eSelectSlot2
    "", //    eSelectSlot3
    "", //    eSelectSlot4
    "", //    eSelectSlot5
    "", //    eSelectSlot6
    "", //    eSelectSlot7
    "", //    eSelectSlot8
    "", //    eSelectSlot9
    // win32 defaults these to Shift+F11 / F11, but F11 is already Qt's
    // fullscreen toggle and the first binding registered wins, so leave them
    // unassigned rather than shadowing one silently.
    "", //    eSaveStateDialog
    "", //    eLoadStateDialog
    "", //    eSaveStateFile
    "", //    eLoadStateFile
    "", //    eRewind
    "Keyboard Ctrl+g", //    eGrabMouse
    "", //    eSwapControllers1and2
    "", //    eToggleBG0
    "", //    eToggleBG1
    "", //    eToggleBG2
    "", //    eToggleBG3
    "", //    eToggleSprites
    "", //    eChangeBackdrop
    "", //    eToggleSoundChannel1
    "", //    eToggleSoundChannel2
    "", //    eToggleSoundChannel3
    "", //    eToggleSoundChannel4
    "", //    eToggleSoundChannel5
    "", //    eToggleSoundChannel6
    "", //    eToggleSoundChannel7
    "", //    eToggleSoundChannel8
    "", //    eToggleAllSoundChannels
    "", //    eStartRecording
    "", //    eStopRecording
    ""
};

const char **EmuConfig::getDefaultShortcutKeys()
{
    return default_controller_keys;
}

const char **EmuConfig::getShortcutNames()
{
    return shortcut_names;
}

std::string EmuConfig::findConfigDir()
{
    char *dir;
    fs::path path;

    auto app_dir_path = QGuiApplication::applicationDirPath();
    auto config_file = QDir(app_dir_path).absoluteFilePath("super-snes9x-qt.conf");
    if (QFile::exists(config_file))
        return app_dir_path.toStdString();

#ifndef _WIN32
    if ((dir = getenv("XDG_CONFIG_HOME")))
    {
        path = dir;
        path /= "supersnes9x";
    }
    else if ((dir = getenv("HOME")))
    {
        path = dir;
        path /= ".config/supersnes9x";
    }
    else
    {
        path = "./.supersnes9x";
    }
#else
    if ((dir = getenv("APPDATA")))
    {
        path = dir;
        path /= "SuperSnes9x";
    }
    else if ((dir = getenv("LOCALAPPDATA")))
    {
        path = dir;
        path /= "SuperSnes9x";
    }
    else
        path = "supersnes9x";
#endif

    if (!fs::exists(path))
        fs::create_directories(path);

    return path.string();
}

std::string EmuConfig::findConfigFile()
{
    fs::path path(findConfigDir());
    path /= "super-snes9x-qt.conf";
    return path.string();
}

bool EmuConfig::setDefaults(int section)
{
    main_window_width = 640;
    main_window_height = 480;

    bool restart = false;
    auto restart_set = [&](auto& dst, auto str) {
        if (dst != str)
        {
            restart = true;
            dst = str;
        }
    };

    if (section == -1 || section == 0)
    {
        // General
        fullscreen_on_open = false;
        disable_screensaver = true;
        pause_emulation_when_unfocused = true;

        show_frame_rate = false;
        show_indicators = true;
        show_pressed_keys = false;
        show_time = false;
        language = "";
    }

    if (section == -1 || section == 1)
    {
        // Display
        restart_set(display_driver, "vulkan");
        restart_set(display_device_index, 0);
        enable_vsync = true;
        bilinear_filter = true;
        reduce_input_lag = true;
        adjust_for_vrr = false;
        restart_set(use_shader, false);
        if (use_shader)
            restart_set(shader, "");
        else
            shader = {};
        last_shader_folder = {};

        scale_image = true;
        maintain_aspect_ratio = true;
        use_integer_scaling = false;
        aspect_ratio_numerator = 4;
        aspect_ratio_denominator = 3;
        show_overscan = false;
        high_resolution_effect = eLeaveAlone;

        software_filter = {};
        software_filter_hires = {};

        display_messages = eOnscreen;
        osd_size = 24;

        gb_frame_blend = eGBBlendOff;
        gb_frame_blend_layer = eGBBlendLayerAll;
        gb_frame_blend_auto = true;

        color_correction = false;
        color_adjustments_enabled = false;
        color_gamma = 0;
        color_contrast = 0;
        color_saturation = 0;
    }

    if (section == -1 || section == 2)
    {
        // Sound
        restart_set(sound_driver, "cubeb");
        sound_device = {};
        restart_set(playback_rate, 48000);
        restart_set(audio_buffer_size_ms, 64);

        adjust_input_rate_automatically = false;
        input_rate = 32040;
        dynamic_rate_control = false;
        dynamic_rate_limit = 0.005;
        mute_audio = false;
        mute_audio_during_alternate_speed = false;

        master_volume_regular = 100;
        master_volume_fast_forward = 100;
        sgb_mix_volume_spc = 50;
        sgb_mix_volume_gb = 50;
        gain_regular = 0;
        sgb_mix_gain_spc = 0;
        sgb_mix_gain_gb = 0;
    }

    if (section == -1 || section == 3)
    {
        speed_sync_method = eTimer;
        fixed_frame_rate = 0.0;
        fast_forward_skip_frames = 9;

        rewind_buffer_size = 0;
        rewind_frame_interval = 5;

        run_ahead_frames = 0;

        allow_invalid_vram_access = false;
        snapshot_screenshots = true;
        allow_opposing_dpad_directions = false;
        overclock = eNoOverclock;
        remove_sprite_limit = false;
        enable_shadow_buffer = false;
        superfx_clock_multiplier = 100;
        sound_filter = eGaussian;
        sgb_bios_preference = 2;
    }

    if (section == -1 || section == 4)
    {
        automap_gamepads = true;
        enable_rumble = true;
        // Controllers
        port_configuration = 0;
        memset(binding.controller, 0, sizeof(binding.controller));

        const char *button_list[] = { "Up", "Down", "Left", "Right", "d", "c", "s", "x", "z", "a", "Return", "Space" };
        for (int i = 0; i < std::size(button_list); i++)
        {
            binding.controller[0].buttons[i * 4] = EmuBinding::from_config_string("Keyboard " + std::string(button_list[i]));
        }
    }

    if (section == -1 || section == 5)
    {
        // Shortcuts
        memset(binding.shortcuts, 0, sizeof(binding.shortcuts));
        for (auto i = 0; i < num_shortcuts; i++)
        {
            binding.shortcuts[i * 4] = EmuBinding::from_config_string(getDefaultShortcutKeys()[i]);
        }
    }

    if (section == -1 || section == 6)
    {
        // Files
        sram_folder = {};
        state_folder = {};
        cheat_folder = {};
        patch_folder = {};
        export_folder = {};
        bios_folder = "Bios";

        sram_location = eROMDirectory;
        state_location = eROMDirectory;
        cheat_location = eROMDirectory;
        patch_location = eROMDirectory;
        export_location = eROMDirectory;
        bios_location = eCustomDirectory;

        sram_save_interval = 0;
    }

    return restart;
}

void EmuConfig::config(const std::string &filename, bool write)
{
    ConfigFile cf;
    if (!write)
        cf.LoadFile(filename.c_str());

    // Current section prefix. ConfigFile keys are "Section::Key"; BeginSection
    // sets the prefix and EndSection clears it (no nesting is used).
    std::string section;
    auto fullkey = [&](const std::string &key) {
        return section.empty() ? key : section + "::" + key;
    };

    // Case-insensitive compare, portable (the old QString path lower-cased both
    // sides before matching enum tokens).
    auto ieq = [](const std::string &a, const std::string &b) {
        if (a.size() != b.size())
            return false;
        for (size_t i = 0; i < a.size(); i++)
            if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
                return false;
        return true;
    };

    // Each accessor reads or writes depending on `write`; the trailing comment
    // is only emitted on write and ignored on read. Reads leave the value at its
    // default when the key is absent, matching the old QSettings behavior.
    auto Bool = [&](const std::string &key, bool &value, const std::string &comment = "") {
        std::string k = fullkey(key);
        if (write)
            cf.SetBool(k.c_str(), value, "true", "false", comment.c_str());
        else if (cf.Exists(k.c_str()))
            value = cf.GetBool(k.c_str());
    };

    auto Int = [&](const std::string &key, int &value, const std::string &comment = "") {
        std::string k = fullkey(key);
        if (write)
            cf.SetInt(k.c_str(), value, comment.c_str());
        else if (cf.Exists(k.c_str()))
            value = cf.GetInt(k.c_str());
    };

    auto String = [&](const std::string &key, std::string &value, const std::string &comment = "") {
        std::string k = fullkey(key);
        if (write)
            cf.SetString(k.c_str(), value, comment.c_str());
        else
            value = cf.GetString(k.c_str(), value);
    };

    auto Double = [&](const std::string &key, double &value, const std::string &comment = "") {
        std::string k = fullkey(key);
        if (write)
            cf.SetString(k.c_str(), std::to_string(value), comment.c_str());
        else if (cf.Exists(k.c_str()))
            value = atof(cf.GetString(k.c_str(), std::string()).c_str());
    };

    auto Enum = [&](const std::string &key, int &value,
                    const std::vector<const char *> &map, const std::string &comment = "") {
        std::string k = fullkey(key);
        if (write)
        {
            cf.SetString(k.c_str(), map[value], comment.c_str());
        }
        else if (cf.Exists(k.c_str()))
        {
            std::string entry = cf.GetString(k.c_str(), std::string());
            for (size_t i = 0; i < map.size(); i++)
                if (ieq(entry, map[i]))
                {
                    value = (int)i;
                    break;
                }
        }
    };

    auto Binding = [&](const std::string &key, EmuBinding &binding) {
        std::string k = fullkey(key);
        if (write)
            cf.SetString(k.c_str(), binding.to_config_string());
        else if (cf.Exists(k.c_str()))
            binding = EmuBinding::from_config_string(cf.GetString(k.c_str(), std::string()));
    };

    auto BeginSection = [&](const std::string &str) { section = str; };
    auto EndSection = [&]() { section.clear(); };

    // Section name and semantics match the win32/GTK configs so these two knobs
    // control the look of this file the same way across all ports.
    BeginSection("Config");
    Bool("NiceAlignment", config_nice_alignment, "true to line up the = and # columns in each section of this config file");
    Bool("Comments", config_show_comments, "true to keep comments such as this in this config file. To refresh all comments after an upgrade, set this to false and run Snes9x, then set it back to true and run Snes9x again.");
    EndSection();

    // Window sizes and recently-used lists, remembered automatically.
    BeginSection("Operational");
    String("LastROMFolder", last_rom_folder, "Folder last browsed for a ROM");
    Int("MainWindowWidth", main_window_width);
    Int("MainWindowHeight", main_window_height);
    Int("ShaderParametersDialogWidth", shader_parameters_dialog_width);
    Int("ShaderParametersDialogHeight", shader_parameters_dialog_height);
    Int("CheatDialogWidth", cheat_dialog_width);
    Int("CheatDialogHeight", cheat_dialog_height);
    Int("CurrentSaveSlot", current_save_slot, "Currently selected save-state slot within the bank (remembered automatically)");
    Int("CurrentSaveBank", current_save_bank, "Currently selected save-state bank (remembered automatically)");

    if (!write)
    {
        if (current_save_slot < 0 || current_save_slot >= save_slots_per_bank)
            current_save_slot = 0;
        if (current_save_bank < 0 || current_save_bank >= num_save_banks)
            current_save_bank = 0;
    }

    int recent_count = recently_used.size();
    Int("RecentlyUsedEntries", recent_count, "Number of RecentlyUsed entries below");
    if (!write)
        recently_used.resize(recent_count);
    for (int i = 0; i < recent_count; i++)
    {
        String("RecentlyUsed" + std::to_string(i), recently_used[i]);
    }
    EndSection();

    BeginSection("General");
    Bool("FullscreenOnOpen", fullscreen_on_open, "Enter fullscreen automatically when a ROM is opened");
    Bool("DisableScreensaver", disable_screensaver, "Stop the system screensaver from starting while a game is running");
    Bool("PauseEmulationWhenUnfocused", pause_emulation_when_unfocused, "Pause the game whenever the Snes9x window loses focus");

    Bool("ShowFrameRate", show_frame_rate, "Show the frames-per-second counter on screen");
    Bool("ShowIndicators", show_indicators, "Show on-screen indicators for turbo, pause, rewind, etc.");
    Bool("ShowPressedKeys", show_pressed_keys, "Show the controller buttons being pressed on screen");
    Bool("ShowTime", show_time, "Show the current wall-clock time on screen");
    String("Language", language, "UI language code (e.g. en, es); empty follows the system locale");
    EndSection();

    BeginSection("Display");
    String("DisplayDriver", display_driver, "Rendering backend (e.g. vulkan, opengl)");
    Int("DisplayDevice", display_device_index, "Index of the GPU/output to render on (0 = default)");
    Bool("VSync", enable_vsync, "Synchronize to the monitor's refresh to avoid tearing");
    Bool("ReduceInputLag", reduce_input_lag, "Wait for each frame to finish drawing before continuing (lower lag, may cost speed)");
    Bool("BilinearFilter", bilinear_filter, "Smooth the scaled image");
    Bool("AdjustForVRR", adjust_for_vrr, "Adjust timing for variable-refresh-rate (FreeSync/G-Sync) displays");
    Bool("UseShader", use_shader, "Apply the shader named in Shader below");
    String("Shader", shader, "Path to the shader preset used when UseShader is on");
    String("LastShaderFolder", last_shader_folder, "Folder last browsed for a shader");

    Bool("ScaleImage", scale_image, "Scale the image to fit the window");
    Bool("MaintainAspectRatio", maintain_aspect_ratio, "Keep the correct proportions when scaling instead of stretching");
    Bool("UseIntegerScaling", use_integer_scaling, "Restrict scaling to whole-number multiples to keep pixels even");
    Int("AspectRatioNumerator", aspect_ratio_numerator, "Aspect-ratio width term (e.g. 4 in 4:3)");
    Int("AspectRatioDenominator", aspect_ratio_denominator, "Aspect-ratio height term (e.g. 3 in 4:3)");
    Bool("ShowOverscan", show_overscan, "Show the overscan area at the top and bottom that most games hide");
    Enum("HighResolutionEffect", high_resolution_effect, { "LeaveAlone", "ScaleDown", "ScaleUp" }, "How to handle hi-res (512-wide) frames: LeaveAlone, ScaleDown, or ScaleUp");

    String("SoftwareFilter", software_filter, "Software scaling filter name, e.g. \"HQ2x\" or \"Blargg's NTSC (Composite)\"; empty means none");
    String("SoftwareFilterHiRes", software_filter_hires, "Software scaling filter used for hi-res (512-wide or interlaced) frames; empty means none");

    Enum("DisplayMessages", display_messages, { "Onscreen", "Inscreen", "None" }, "Where to draw on-screen messages: Onscreen, Inscreen, or None");
    Int("OSDSize", osd_size, "Size of on-screen display text in points (default 24)");

    // Key names match the win32 config (wconfig.cpp) so a shared install reads/writes
    // the same entries.
    Enum("BlendGBFrames", gb_frame_blend, { "Off", "SimpleBlend", "LCDBlend" }, "Game Boy frame-blend (Super Game Boy only): Off, SimpleBlend (fixes flicker fake-transparency), or LCDBlend (LCD-style ghosting)");
    Enum("BlendGBFramesLayer", gb_frame_blend_layer, { "All", "Background", "Window", "Sprites" }, "Which Game Boy layer the frame-blend applies to: All, Background, Window, or Sprites");
    Bool("BlendGBFramesAuto", gb_frame_blend_auto, "Auto-pick the GB frame-blend per game from a built-in known-flicker table");

    Bool("ColorCorrection", color_correction, "Enable accurate SNES color correction (simulates SNES CRT output)");
    Bool("AdjustmentsEnabled", color_adjustments_enabled, "Apply the gamma/contrast/saturation adjustments below");
    Int("Gamma", color_gamma, "Gamma adjustment (-100..+100, 0 = no change)");
    Int("Contrast", color_contrast, "Contrast adjustment (-100..+100, 0 = no change)");
    Int("Saturation", color_saturation, "Saturation adjustment (-100..+100, 0 = no change)");

    if (!write)
    {
        auto clamp_adjustment = [](int &v) {
            if (v < -100) v = -100;
            if (v > 100) v = 100;
        };
        clamp_adjustment(color_gamma);
        clamp_adjustment(color_contrast);
        clamp_adjustment(color_saturation);
    }
    EndSection();

    BeginSection("Sound");
    String("SoundDriver", sound_driver, "Audio backend name (e.g. cubeb)");
    String("SoundDevice", sound_device, "Specific output device name; empty uses the system default");
    Int("PlaybackRate", playback_rate, "Output sample rate in Hz (e.g. 48000)");
    Int("BufferSize", audio_buffer_size_ms, "Audio buffer size in milliseconds (larger is safer against crackle, adds latency)");
    Bool("AdjustInputRateAutomatically", adjust_input_rate_automatically, "Guess the input rate from the monitor's refresh rate");
    Int("InputRate", input_rate, "APU sample rate in Hz resampled to the output rate; default 32040. Nudges pitch/sync");
    Bool("DynamicRateControl", dynamic_rate_control, "Slightly bend the sample rate to keep the buffer full and avoid dropouts");
    Double("DynamicRateLimit", dynamic_rate_limit, "How far Dynamic Rate Control may bend the rate, as a fraction (e.g. 0.005 = 0.5%)");
    Bool("Mute", mute_audio, "Silence all audio output");
    Bool("MuteAudioDuringAlternateSpeed", mute_audio_during_alternate_speed, "Silence audio while fast-forwarding or rewinding");
    Int("MasterVolumeRegular", master_volume_regular, "Master output volume during normal play (0..100, percent)");
    Int("MasterVolumeFastForward", master_volume_fast_forward, "Master output volume during turbo/rewind (0..100, percent)");
    Int("VolumeSGBMixSPC", sgb_mix_volume_spc, "SGB BIOS mix: SPC channel volume (0..100, percent; only active in SGB BIOS mode)");
    Int("VolumeSGBMixGB", sgb_mix_volume_gb, "SGB BIOS mix: GB channel volume (0..100, percent; only active in SGB BIOS mode)");
    Int("GainRegular", gain_regular, "Master pre-amp applied after the volume percentages (whole dB, 0 = unity)");
    Int("GainSGBMixSPC", sgb_mix_gain_spc, "SGB BIOS mix: SPC channel pre-amp (whole dB, 0 = unity)");
    Int("GainSGBMixGB", sgb_mix_gain_gb, "SGB BIOS mix: GB channel pre-amp (whole dB, 0 = unity)");
    EndSection();

    BeginSection("Emulation");
    Enum("SpeedSyncMethod", speed_sync_method, { "Timer", "TimerFrameskip", "SoundSync", "Unlimited" }, "How gameplay speed is regulated: Timer, TimerFrameskip, SoundSync, or Unlimited");
    Double("FixedFrameRate", fixed_frame_rate, "Force this frame rate in frames per second; 0 uses the console's native rate");
    Int("FastForwardSkipFrames", fast_forward_skip_frames, "How many frames to skip drawing while fast-forwarding (higher is faster)");
    Int("RewindBufferSize", rewind_buffer_size, "Memory (in MB) reserved for rewind; 0 disables rewind");
    Int("RewindFrameInterval", rewind_frame_interval, "Save a rewind snapshot every N frames");
    Int("RunAhead", run_ahead_frames, "Number of frames to run ahead for reduced input latency (0 = off, 1-4)");
    Bool("AllowInvalidVRAMAccess", allow_invalid_vram_access, "Let games make the VRAM accesses real hardware blocks (off for accuracy; on only for a few broken hacks)");
    Bool("SnapshotScreenshots", snapshot_screenshots, "Store a screenshot inside each save state, for the save/load-with-preview dialog");
    Bool("AllowOpposingDpadDirections", allow_opposing_dpad_directions, "Allow the D-Pad to press both left+right or up+down at once");
    Int("Overclock", overclock, "CPU overclock: 0 none, 1 auto-FastROM, 2 low, 3 high (reduces slowdown; inaccurate, can break games)");
    Bool("RemoveSpriteLimit", remove_sprite_limit, "Draw more sprites per line than the hardware allows (reduces flicker, may glitch)");
    Bool("EnableShadowBuffer", enable_shadow_buffer, "Use a separate echo buffer so the SPC echo can't overwrite APU RAM");
    Int("SuperFXClockMultiplier", superfx_clock_multiplier, "SuperFX (GSU) chip speed as a percentage of normal (50-400; 100 = accurate). Higher reduces slowdown in Star Fox and other SuperFX games");
    // Audio interpolation. The legacy "SoundFilter" key used a token map whose
    // first three entries were scrambled (indices 0-2 wrote the wrong word, so
    // the default Gaussian was saved as "Linear"). Write a corrected
    // "SoundInterpolation" key going forward, and migrate old configs by decoding
    // the legacy key with its original buggy map so the real value is preserved
    // rather than silently changed. sound_filter is used directly as the combo
    // index and as Settings.InterpolationMethod, so only the on-disk token was
    // ever wrong.
    const std::vector<const char *> sound_filter_map = { "Nearest", "Linear", "Gaussian", "Cubic", "Sinc" };
    if (write)
    {
        Enum("SoundInterpolation", sound_filter, sound_filter_map, "Audio interpolation filter: Nearest, Linear, Gaussian (hardware-accurate default), Cubic, or Sinc");
    }
    else if (cf.Exists(fullkey("SoundInterpolation").c_str()))
    {
        Enum("SoundInterpolation", sound_filter, sound_filter_map);
    }
    else
    {
        const std::vector<const char *> legacy_sound_filter_map = { "Gaussian", "Nearest", "Linear", "Cubic", "Sinc" };
        Enum("SoundFilter", sound_filter, legacy_sound_filter_map);
    }
    EndSection();

    // Key path "SGB::BIOSPreference" matches the win32 config (wconfig.cpp) and
    // the CLI (snes9x.cpp) so every port reads/writes the same entry.
    BeginSection("SGB");
    Int("BIOSPreference", sgb_bios_preference, "BIOS mode for GB/GBC ROMs: 0=No BIOS (BIOS-less), 1=SGB1, 2=SGB2 (default)");
    EndSection();

    BeginSection("Ports");
    Bool("AutomapGamepads", automap_gamepads, "Automatically map newly connected gamepads to a sensible default layout");
    Bool("EnableRumble", enable_rumble, "on to pass rumble-cart motor effects (LRG SNES releases) to the port-1 gamepad");
    Enum("PortConfiguration", port_configuration, { "OneController", "TwoControllers", "Mouse", "SuperScope", "Multitap" }, "What is plugged into the console's controller ports: OneController, TwoControllers, Mouse, SuperScope, or Multitap");
    EndSection();

    for (int c = 0; c < 5; c++)
    {
        BeginSection("Controller_" + std::to_string(c));

        for (int y = 0; y < num_controller_bindings; y++)
            for (int x = 0; x < allowed_bindings; x++)
            {
                const char *names[] = {"Up",      "Down",    "Left",    "Right",
                                       "A",       "B",       "X",       "Y",
                                       "L",       "R",       "Start",   "Select",
                                       "Turbo_A", "Turbo_B", "Turbo_X", "Turbo_Y",
                                       "Turbo_L", "Turbo_R"};
                std::string keyname = names[y] + std::to_string(x);
                Binding(keyname, binding.controller[c].buttons[y * allowed_bindings + x]);
            }

        EndSection();
    }

    BeginSection("Shortcuts");
    for (int i = 0; i < num_shortcuts; i++)
    {
        Binding(getShortcutNames()[i] + std::to_string(0), binding.shortcuts[i * 4]);
        Binding(getShortcutNames()[i] + std::to_string(1), binding.shortcuts[i * 4 + 1]);
        Binding(getShortcutNames()[i] + std::to_string(2), binding.shortcuts[i * 4 + 2]);
        Binding(getShortcutNames()[i] + std::to_string(3), binding.shortcuts[i * 4 + 3]);
    }
    EndSection();

    // Each *Location picks where that file type lives; the matching *Folder is
    // only used when the location is set to Custom.
    BeginSection("Files");
    Enum("SRAMLocation", sram_location, { "ROMDirectory", "ConfigDirectory", "Custom" }, "Where battery saves (.srm) go: ROMDirectory, ConfigDirectory, or Custom");
    Enum("StateLocation", state_location, { "ROMDirectory", "ConfigDirectory", "Custom" }, "Where save states go: ROMDirectory, ConfigDirectory, or Custom");
    Enum("CheatLocation", cheat_location, { "ROMDirectory", "ConfigDirectory", "Custom" }, "Where cheat files go: ROMDirectory, ConfigDirectory, or Custom");
    Enum("PatchLocation", patch_location, { "ROMDirectory", "ConfigDirectory", "Custom" }, "Where ROM patches are looked for: ROMDirectory, ConfigDirectory, or Custom");
    Enum("ExportLocation", export_location, { "ROMDirectory", "ConfigDirectory", "Custom" }, "Where exported files go: ROMDirectory, ConfigDirectory, or Custom");
    Enum("BIOSLocation", bios_location, { "ROMDirectory", "ConfigDirectory", "Custom" }, "Where BIOS images are looked for: ROMDirectory, ConfigDirectory, or Custom");

    String("SRAMFolder", sram_folder, "Custom folder for battery saves (used when SRAMLocation is Custom)");
    String("StateFolder", state_folder, "Custom folder for save states (used when StateLocation is Custom)");
    String("CheatFolder", cheat_folder, "Custom folder for cheat files (used when CheatLocation is Custom)");
    String("PatchFolder", patch_folder, "Custom folder for ROM patches (used when PatchLocation is Custom)");
    String("ExportFolder", export_folder, "Custom folder for exported files (used when ExportLocation is Custom)");
    String("BIOSFolder", bios_folder, "Custom folder for BIOS images (used when BIOSLocation is Custom)");

    Int("SRAMSaveInterval", sram_save_interval, "Auto-write the battery save this many seconds after the game changes it (0 = only on exit/reset)");
    EndSection();

    BeginSection("RetroAchievements");
    Bool("Enabled", ra_enabled, "Connect to RetroAchievements and track achievements");
    Bool("HardcoreMode", ra_hardcore_mode, "Disable save states, cheats and rewind for competitive achievement earning");
    String("Username", ra_username, "RetroAchievements account name");
    String("ApiToken", ra_api_token, "Login token issued by RetroAchievements (not your password); cleared on logout");
    String("EmulatorName", ra_emulator_name, "Client name reported to the RetroAchievements server");
    EndSection();

    if (write)
    {
        ConfigFile::SetProgramName("SuperSnes9x");
        ConfigFile::SetNiceAlignment(config_nice_alignment);
        ConfigFile::SetShowComments(config_show_comments);
        cf.SaveTo(filename.c_str());
    }
}

void EmuConfig::setVRRConfig(bool enable)
{
    if (enable == vrr_enabled)
        return;

    if (!adjust_for_vrr && enable)
        return;

    vrr_enabled = enable;

    if (enable)
    {
        saved_fixed_frame_rate = fixed_frame_rate;
        saved_input_rate = input_rate;
        saved_speed_sync_method = speed_sync_method;
        saved_enable_vsync = enable_vsync;

        fixed_frame_rate = 0.0;
        input_rate = 32040;
        enable_vsync = true;
        speed_sync_method = eTimer;
    }
    else
    {
        fixed_frame_rate = saved_fixed_frame_rate;
        input_rate = saved_input_rate;
        speed_sync_method = saved_speed_sync_method;
        enable_vsync = saved_enable_vsync;
    }
}
