/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <filesystem>

#include "fmt/format.h"
#include "gtk_config.h"
#include "gtk_s9x.h"
#include "gtk_display.h"
#include "conffile.h"
#include "cheats.h"
#include "apu/apu.h"
#include "netplay.h"
#include "controls.h"

namespace fs = std::filesystem;

std::string get_config_dir()
{
    // Find config directory
    char *env_home = getenv("HOME");
    char *env_xdg_config_home = getenv("XDG_CONFIG_HOME");

    if (!env_home && !env_xdg_config_home)
    {
        return std::string{".snes9x"};
    }

    fs::path config;
    // If XDG_CONFIG_HOME is set, use that, otherwise guess default
    if (env_xdg_config_home)
    {
        config = env_xdg_config_home;
        config /= "snes9x";
    }
    else
    {
        config = env_home;
        config /= ".config/snes9x";
    }

    return config;
}

std::string get_config_file_name()
{
    return get_config_dir() + "/snes9x.conf";
}

void S9xParsePortConfig(ConfigFile &conf, int pass)
{
}

Snes9xConfig::Snes9xConfig()
{
    joystick_threshold = 40;
    xrr_crtc_info = nullptr;
    xrr_screen_resources = nullptr;
}

Snes9xConfig::~Snes9xConfig()
{
    if (xrr_crtc_info)
        XRRFreeCrtcInfo(xrr_crtc_info);
    if (xrr_screen_resources)
        XRRFreeScreenResources(xrr_screen_resources);
}

int Snes9xConfig::load_defaults()
{
    full_screen_on_open = false;
    change_display_resolution = false;
    xrr_index = 0;
    scale_to_fit = true;
    maintain_aspect_ratio = true;
    aspect_ratio = 2;
    scale_method = 0;
    overscan = false;
    save_sram_after_secs = 0;
    rom_loaded = false;
    multithreading = false;
    splash_image = SPLASH_IMAGE_STARFIELD;
    display_driver = "opengl";
    allow_opengl = false;
    allow_xv = false;
    allow_xrandr = false;
    auto_vrr = false;
    osd_size = 24;
    force_inverted_byte_order = false;
    hires_effect = HIRES_NORMAL;
    pause_emulation_on_switch = false;
    num_threads = 2;
    mute_sound = false;
    mute_sound_turbo = false;
    fullscreen = false;
    ui_visible = true;
    default_esc_behavior = 1;
    prevent_screensaver = false;
    sound_driver = 0;
    sound_buffer_size = 48;
    sound_playback_rate = 7;
    sound_input_rate = 31950;
    auto_input_rate = true;
    last_directory.clear();
    last_shader_directory.clear();
    window_width = -1;
    window_height = -1;
    preferences_width = -1;
    preferences_height = -1;
    shader_parameters_width = -1;
    shader_parameters_height = -1;
    enable_icons = true;
    current_display_tab = 0;
    sram_directory.clear();
    export_directory.clear();
    savestate_directory.clear();
    cheat_directory.clear();
    patch_directory.clear();
    screensaver_needs_reset = false;
    ntsc_setup = snes_ntsc_composite;
    ntsc_scanline_intensity = 1;
    scanline_filter_intensity = 0;
    Settings.BilinearFilter = false;
    netplay_activated = false;
    netplay_server_up = false;
    netplay_is_server = false;
    netplay_sync_reset = true;
    netplay_send_rom = false;
    netplay_default_port = 6096;
    netplay_max_frame_loss = 10;
    netplay_last_rom.clear();
    netplay_last_host.clear();
    netplay_last_port = 6096;
    modal_dialogs = true;
    current_save_slot = 0;
    S9xCheatsEnable();

    rewind_granularity = 5;
    rewind_buffer_size = 0;
    Settings.Rewinding = false;

    sync_to_vblank = true;
    use_shaders = false;
    shader_filename.clear();
    reduce_input_lag = false;

    /* Snes9x Variables */
    Settings.MouseMaster = true;
    Settings.SuperScopeMaster = true;
    Settings.JustifierMaster = true;
    Settings.MultiPlayer5Master = true;
    Settings.UpAndDown = false;
    Settings.AutoSaveDelay = 0;
    Settings.SkipFrames = 0;
    Settings.Transparency = true;
    Settings.DisplayTime = false;
    Settings.DisplayFrameRate = false;
    Settings.DisplayIndicators = false;
    Settings.SixteenBitSound = true;
    Settings.Stereo = true;
    Settings.ReverseStereo = false;
    Settings.SoundPlaybackRate = 48000;
    Settings.StopEmulation = true;
    Settings.FrameTimeNTSC = 16639;
    Settings.FrameTimePAL = 20000;
    Settings.FrameTime = Settings.FrameTimeNTSC;
    Settings.BlockInvalidVRAMAccessMaster = true;
    Settings.SoundSync = false;
    Settings.DynamicRateControl = false;
    Settings.DynamicRateLimit = 5;
    Settings.InterpolationMethod = DSP_INTERPOLATION_GAUSSIAN;
    Settings.HDMATimingHack = 100;
    Settings.SuperFXClockMultiplier = 100;
    Settings.NetPlay = false;
    NetPlay.Paused = false;
    NetPlay.MaxFrameSkip = 10;
    Settings.DisplayPressedKeys = false;
#ifdef ALLOW_CPU_OVERCLOCK
    Settings.MaxSpriteTilesPerLine = 34;
    Settings.OneClockCycle = 6;
    Settings.OneSlowClockCycle = 8;
    Settings.TwoClockCycles = 12;
#endif

    for (auto &joypad : pad)
    {
        joypad.data.fill(Binding());
    }

    shortcut.fill(Binding());

    return 0;
}

int Snes9xConfig::save_config_file()
{
    ConfigFile cf;
    std::string section;

    auto outbool = [&](std::string name, bool b, std::string comment = "") {
        cf.SetBool((section + "::" + name).c_str(), b, "true", "false", comment.c_str());
    };

    auto outstring = [&](std::string name, std::string str, std::string comment = "") {
        cf.SetString((section + "::" + name).c_str(), str, comment.c_str());
    };

    auto outint = [&](std::string name, int i, std::string comment = "") {
        cf.SetInt((section + "::" + name).c_str(), i, comment.c_str());
    };

    section = "Display";
    outbool("FullscreenOnOpen", full_screen_on_open, "Set the screen resolution after opening a ROM");
    outbool("ChangeDisplayResolution", change_display_resolution, "Set the resolution in fullscreen mode");
    outbool("ScaleToFit", scale_to_fit, "Scale the image to fit the window size");
    outbool("ShowOverscanArea", overscan);
    outbool("MaintainAspectRatio", maintain_aspect_ratio, "Resize the screen to the proportions set by aspect ratio option");
    outbool("Multithreading", multithreading, "Apply filters using multiple threads");
    outbool("BilinearFilter", Settings.BilinearFilter, "Smoothes scaled image");
    outbool("ForceInvertedByteOrder", force_inverted_byte_order);
    outint("VideoMode", xrr_index, "Platform-specific video mode number");
    outint("AspectRatio", aspect_ratio, "0: uncorrected, 1: uncorrected integer scale, 2: 4:3, 3: 4/3 integer scale, 4: NTSC/PAL, 5: NTSC/PAL integer scale");
    outint("SoftwareScaleFilter", scale_method, "Build-specific number of filter used for software scaling");
    outint("ScanlineFilterIntensity", scanline_filter_intensity, "0: 0%, 1: 12.5%, 2: 25%, 3: 50%, 4: 100%");
    outint("HiresEffect", hires_effect, "0: Downscale to low-res, 1: Leave as-is, 2: Upscale low-res screens");
    outint("NumberOfThreads", num_threads);
    outstring("HardwareAcceleration", display_driver, "none, opengl, xv, vulkan");
    outint("SplashBackground", splash_image, "0: Black, 1: Color bars, 2: Pattern, 3: Blue, 4: Default");
    outbool("AutoVRR", auto_vrr, "Automatically use the best settings for variable sync in fullscreen mode");
    outint("OSDSize", osd_size, "Size of on-screen display elements. Default: 24pt");

    section = "NTSC";
    outstring("Hue", std::to_string(ntsc_setup.hue));
    outstring("Saturation", std::to_string(ntsc_setup.saturation));
    outstring("Contrast", std::to_string(ntsc_setup.contrast));
    outstring("Brightness", std::to_string(ntsc_setup.brightness));
    outstring("Sharpness", std::to_string(ntsc_setup.sharpness));
    outstring("Artifacts", std::to_string(ntsc_setup.artifacts));
    outstring("Gamma", std::to_string(ntsc_setup.gamma));
    outstring("Bleed", std::to_string(ntsc_setup.bleed));
    outstring("Fringing", std::to_string(ntsc_setup.fringing));
    outstring("Resolution", std::to_string(ntsc_setup.resolution));
    outbool("MergeFields", ntsc_setup.merge_fields);
    outint("ScanlineIntensity", ntsc_scanline_intensity);

    section = "OpenGL";
    outbool("VSync", sync_to_vblank);
    outbool("ReduceInputLag", reduce_input_lag);
    outbool("EnableCustomShaders", use_shaders);
    outstring("ShaderFile", shader_filename);

    section = "Sound";
    outbool("MuteSound", mute_sound);
    outbool("MuteSoundDuringTurbo", mute_sound_turbo);
    outint("BufferSize", sound_buffer_size, "Buffer size in milliseconds");
    outint("Driver", sound_driver);
    outint("InputRate", sound_input_rate);
    outbool("DynamicRateControl", Settings.DynamicRateControl);
    outint("DynamicRateControlLimit", Settings.DynamicRateLimit);
    outbool("AutomaticInputRate", auto_input_rate, "Guess input rate by asking the monitor what its refresh rate is");
    outint("PlaybackRate", gui_config->sound_playback_rate, "1: 8000Hz, 2: 11025Hz, 3: 16000Hz, 4: 22050Hz, 5: 32000Hz, 6: 44100Hz, 7: 48000Hz");

    section = "Files";
    outstring("LastDirectory", last_directory);
    outstring("LastShaderDirectory", last_shader_directory);
    outstring("SRAMDirectory", sram_directory);
    outstring("SaveStateDirectory", savestate_directory);
    outstring("CheatDirectory", cheat_directory);
    outstring("PatchDirectory", patch_directory);
    outstring("ExportDirectory", export_directory);

    section = "Window State";
    outint("MainWidth", window_width);
    outint("MainHeight", window_height);
    outint("PreferencesWidth", preferences_width);
    outint("PreferencesHeight", preferences_height);
    outint("ShaderParametersWidth", shader_parameters_width);
    outint("ShaderParametersHeight", shader_parameters_height);
    outint("CurrentDisplayTab", current_display_tab);
    outbool("UIVisible", ui_visible);
    outbool("EnableIcons", enable_icons);
    if (default_esc_behavior != ESC_TOGGLE_MENUBAR)
        outbool("Fullscreen", 0);
    else
        outbool("Fullscreen", fullscreen);

    section = "Netplay";
    outbool("ActAsServer", netplay_is_server);
    outbool("UseResetToSync", netplay_sync_reset);
    outbool("SendROM", netplay_send_rom);
    outint("DefaultPort", netplay_default_port);
    outint("MaxFrameLoss", netplay_max_frame_loss);
    outint("LastUsedPort", netplay_last_port);
    outstring("LastUsedROM", netplay_last_rom);
    outstring("LastUsedHost", netplay_last_host);

    section = "Behavior";
    outbool("PauseEmulationWhenFocusLost", pause_emulation_on_switch);
    outint("DefaultESCKeyBehavior", default_esc_behavior);
    outbool("PreventScreensaver", prevent_screensaver);
    outbool("UseModalDialogs", modal_dialogs);
    outint("RewindBufferSize", rewind_buffer_size, "Amount of memory (in MB) to use for rewinding");
    outint("RewindGranularity", rewind_granularity, "Only save rewind snapshots every N frames");
    outint("CurrentSaveSlot", current_save_slot);

    section = "Emulation";
    outbool("EmulateTransparency", Settings.Transparency);
    outbool("DisplayTime", Settings.DisplayTime);
    outbool("DisplayFrameRate", Settings.DisplayFrameRate);
    outbool("DisplayPressedKeys", Settings.DisplayPressedKeys);
    outbool("DisplayIndicators", Settings.DisplayIndicators);
    outint("SpeedControlMethod", Settings.SkipFrames, "0: Time the frames to 50 or 60Hz, 1: Same, but skip frames if too slow, 2: Synchronize to the sound buffer, 3: Unlimited, except potentially by vsync");
    outint("SaveSRAMEveryNSeconds", Settings.AutoSaveDelay);
    outbool("BlockInvalidVRAMAccess", Settings.BlockInvalidVRAMAccessMaster);
    outbool("AllowDPadContradictions", Settings.UpAndDown, "Allow the D-Pad to press both up + down at the same time, or left + right");

    section = "Hacks";
    outint("SuperFXClockMultiplier", Settings.SuperFXClockMultiplier);
    outint("SoundInterpolationMethod", Settings.InterpolationMethod, "0: None, 1: Linear, 2: Gaussian (what the hardware uses), 3: Cubic, 4: Sinc");
    outbool("RemoveSpriteLimit", Settings.MaxSpriteTilesPerLine == 34 ? 0 : 1);
    outbool("OverclockCPU", Settings.OneClockCycle == 6 ? 0 : 1);
    outbool("EchoBufferHack", Settings.SeparateEchoBuffer, "Prevents echo buffer from overwriting APU RAM");

    section = "Input";
    controllers controller = CTL_NONE;
    int8 id[4];

    for (int i = 0; i < 2; i++)
    {
        std::string name;
        std::string value;

        name = "ControllerPort" + std::to_string(i);
        S9xGetController(i, &controller, &id[0], &id[1], &id[2], &id[3]);

        switch (controller)
        {
        case CTL_JOYPAD:
            value = "joypad";
            break;
        case CTL_MOUSE:
            value = "mouse";
            break;
        case CTL_SUPERSCOPE:
            value = "superscope";
            break;
        case CTL_MP5:
            value = "multitap";
            break;
        case CTL_JUSTIFIER:
            value = "justifier";
            break;
        default:
            value = "none";
        }

        outstring(name, value);
    }

    outint("JoystickThreshold", joystick_threshold);

    for (int i = 0; i < NUM_JOYPADS; i++)
    {
        auto &joypad = pad[i];

        for (int j = 0; j < NUM_JOYPAD_LINKS; j++)
        {
            section = "Joypad " + std::to_string(i);
            outstring(b_links[j].snes9x_name, joypad.data[j].as_string());
        }
    }

    section = "Shortcuts";
    for (int i = NUM_JOYPAD_LINKS; b_links[i].snes9x_name; i++)
    {
        outstring(b_links[i].snes9x_name, shortcut[i - NUM_JOYPAD_LINKS].as_string());
    }

    cf.SetNiceAlignment(true);
    cf.SetShowComments(true);
    cf.SaveTo(get_config_file_name().c_str());

    return 0;
}

int Snes9xConfig::load_config_file()
{
    load_defaults();

    fs::path path = get_config_dir();

    if (!fs::exists(path))
    {
        if (!fs::create_directory(path))
        {
            fmt::print(stderr, _("Couldn't create config directory: {}\n"), path.string());
            return -1;
        }
    }
    else
    {
        if ((fs::status(path).permissions() & fs::perms::owner_write) == fs::perms::none)
            fs::permissions(path, fs::perms::owner_write, fs::perm_options::add);
    }

    path = get_config_file_name();

    // Write an on-disk config file if none exists.
    if (!fs::exists(path))
        save_config_file();

    ConfigFile cf;
    if (!cf.LoadFile(path.c_str()))
        return -1;

    std::string none;
    std::string section;

    auto inbool = [&](std::string name, auto &b) {
        if (cf.Exists((section + "::" + name).c_str()))
            b = cf.GetBool((section + "::" + name).c_str());
    };

    auto inint = [&](std::string name, auto &i) {
        if (cf.Exists((section + "::" + name).c_str()))
            i = cf.GetInt((section + "::" + name).c_str());
    };

    auto indouble = [&](std::string name, double &d) {
        if (cf.Exists((section + "::" + name).c_str()))
            d = atof(cf.GetString((section + "::" + name).c_str()));
    };

    auto instr = [&](std::string name, std::string &str) {
        str = cf.GetString((section + "::" + name).c_str(), none);
    };

    section = "Display";
    inbool("FullscreenOnOpen", full_screen_on_open);
    inbool("ChangeDisplayResolution", change_display_resolution);
    inint("VideoMode", xrr_index);
    inbool("ScaleToFit", scale_to_fit);
    inbool("MaintainAspectRatio", maintain_aspect_ratio);
    inint("AspectRatio", aspect_ratio);
    inint("SoftwareScaleFilter", scale_method);
    inint("ScanlineFilterIntensity", scanline_filter_intensity);
    inbool("ShowOverscanArea", overscan);
    inint("HiresEffect", hires_effect);
    inbool("ForceInvertedByteOrder", force_inverted_byte_order);
    inbool("Multithreading", multithreading);
    inint("NumberOfThreads", num_threads);
    instr("HardwareAcceleration", display_driver);
    inbool("BilinearFilter", Settings.BilinearFilter);
    inint("SplashBackground", splash_image);
    inbool("AutoVRR", auto_vrr);
    inint("OSDSize", osd_size);

    section = "NTSC";
    indouble("Hue", ntsc_setup.hue);
    indouble("Saturation", ntsc_setup.saturation);
    indouble("Contrast", ntsc_setup.contrast);
    indouble("Brightness", ntsc_setup.brightness);
    indouble("Sharpness", ntsc_setup.sharpness);
    indouble("Artifacts", ntsc_setup.artifacts);
    indouble("Gamma", ntsc_setup.gamma);
    indouble("Bleed", ntsc_setup.bleed);
    indouble("Fringing", ntsc_setup.fringing);
    indouble("Resolution", ntsc_setup.resolution);
    inbool("MergeFields", ntsc_setup.merge_fields);
    inint("ScanlineIntensity", ntsc_scanline_intensity);

    section = "OpenGL";
    inbool("VSync", sync_to_vblank);
    inbool("ReduceInputLag", reduce_input_lag);
    inbool("EnableCustomShaders", use_shaders);
    instr("ShaderFile", shader_filename);

    section = "Sound";
    inbool("MuteSound", mute_sound);
    inbool("MuteSoundDuringTurbo", mute_sound_turbo);
    inint("BufferSize", sound_buffer_size);
    inint("Driver", sound_driver);
    inint("InputRate", sound_input_rate);
    inbool("DynamicRateControl", Settings.DynamicRateControl);
    inint("DynamicRateControlLimit", Settings.DynamicRateLimit);
    inbool("AutomaticInputRate", auto_input_rate);
    inint("PlaybackRate", gui_config->sound_playback_rate);

    section = "Files";
    instr("LastDirectory", last_directory);
    instr("LastShaderDirectory", last_shader_directory);
    instr("SRAMDirectory", sram_directory);
    instr("SaveStateDirectory", savestate_directory);
    instr("CheatDirectory", cheat_directory);
    instr("PatchDirectory", patch_directory);
    instr("ExportDirectory", export_directory);

    section = "Window State";
    inint("MainWidth", window_width);
    inint("MainHeight", window_height);
    inint("PreferencesWidth", preferences_width);
    inint("PreferencesHeight", preferences_height);
    inint("ShaderParametersWidth", shader_parameters_width);
    inint("ShaderParametersHeight", shader_parameters_height);
    inint("CurrentDisplayTab", current_display_tab);
    inbool("UIVisible", ui_visible);
    inbool("Fullscreen", fullscreen);
    inbool("EnableIcons", enable_icons);

    section = "Netplay";
    inbool("ActAsServer", netplay_is_server);
    inbool("UseResetToSync", netplay_sync_reset);
    inbool("SendROM", netplay_send_rom);
    inint("DefaultPort", netplay_default_port);
    inint("MaxFrameLoss", netplay_max_frame_loss);
    inint("LastUsedPort", netplay_last_port);
    instr("LastUsedROM", netplay_last_rom);
    instr("LastUsedHost", netplay_last_host);

    section = "Behavior";
    inbool("PauseEmulationWhenFocusLost", pause_emulation_on_switch);
    inint("DefaultESCKeyBehavior", default_esc_behavior);
    inbool("PreventScreensaver", prevent_screensaver);
    inbool("UseModalDialogs", modal_dialogs);
    inint("RewindBufferSize", rewind_buffer_size);
    inint("RewindGranularity", rewind_granularity);
    inint("CurrentSaveSlot", current_save_slot);

    section = "Emulation";
    inbool("EmulateTransparency", Settings.Transparency);
    inbool("DisplayTime", Settings.DisplayTime);
    inbool("DisplayFrameRate", Settings.DisplayFrameRate);
    inbool("DisplayPressedKeys", Settings.DisplayPressedKeys);
    inint("SpeedControlMethod", Settings.SkipFrames);
    inint("SaveSRAMEveryNSeconds", Settings.AutoSaveDelay);
    inbool("BlockInvalidVRAMAccess", Settings.BlockInvalidVRAMAccessMaster);
    inbool("AllowDPadContradictions", Settings.UpAndDown);
    inbool("DisplayIndicators", Settings.DisplayIndicators);

    section = "Hacks";
    inint("SuperFXClockMultiplier", Settings.SuperFXClockMultiplier);
    inint("SoundInterpolationMethod", Settings.InterpolationMethod);

    bool RemoveSpriteLimit = false;
    inbool("RemoveSpriteLimit", RemoveSpriteLimit);
    bool OverclockCPU = false;
    inbool("OverclockCPU", OverclockCPU);
    inbool("EchoBufferHack", Settings.SeparateEchoBuffer);

    section = "Input";

    for (int i = 0; i < 2; i++)
    {
        std::string name = "ControllerPort" + std::to_string(i);
        std::string value;
        instr(name, value);

        if (value.find("joypad") != std::string::npos)
            S9xSetController(i, CTL_JOYPAD, i, 0, 0, 0);
        else if (value.find("multitap") != std::string::npos)
            S9xSetController(i, CTL_MP5, i, i + 1, i + 2, i + 3);
        else if (value.find("superscope") != std::string::npos)
            S9xSetController(i, CTL_SUPERSCOPE, 0, 0, 0, 0);
        else if (value.find("mouse") != std::string::npos)
            S9xSetController(i, CTL_MOUSE, i, 0, 0, 0);
        else if (value.find("none") != std::string::npos)
            S9xSetController(i, CTL_NONE, 0, 0, 0, 0);
    }

    inint("JoystickThreshold", joystick_threshold);

    std::string buffer;
    for (int i = 0; i < NUM_JOYPADS; i++)
    {
        auto &joypad = pad[i];

        section = "Joypad " + std::to_string(i);
        for (int j = 0; j < NUM_JOYPAD_LINKS; j++)
        {
            instr(b_links[j].snes9x_name, buffer);
            joypad.data[j] = Binding(buffer.c_str());
        }
    }

    section = "Shortcuts";
    for (int i = NUM_JOYPAD_LINKS; b_links[i].snes9x_name; i++)
    {
        instr(b_links[i].snes9x_name, buffer);
        shortcut[i - NUM_JOYPAD_LINKS] = Binding(buffer.c_str());
    }

    /* Validation */

    if (RemoveSpriteLimit)
        Settings.MaxSpriteTilesPerLine = 128;
    else
        Settings.MaxSpriteTilesPerLine = 34;

    if (OverclockCPU)
    {
        Settings.OneClockCycle = 4;
        Settings.OneSlowClockCycle = 5;
        Settings.TwoClockCycles = 6;
    }
    else
    {
        Settings.OneClockCycle = 6;
        Settings.OneSlowClockCycle = 8;
        Settings.TwoClockCycles = 12;
    }

#ifndef ALLOW_CPU_OVERCLOCK
    Settings.OneClockCycle = 6;
    Settings.OneSlowClockCycle = 8;
    Settings.TwoClockCycles = 12;
    Settings.MaxSpriteTilesPerLine = 34;
    Settings.SeparateEchoBuffer = false;
    Settings.InterpolationMethod = 2;
    Settings.BlockInvalidVRAMAccessMaster = true;
#endif

    if (default_esc_behavior != ESC_TOGGLE_MENUBAR)
        fullscreen = false;

#ifdef USE_HQ2X
    if (scale_method >= NUM_FILTERS)
        scale_method = 0;
#else
    if (scale_method >= NUM_FILTERS - 3)
        scale_method = 0;
#endif /* USE_HQ2X */

#ifdef USE_XBRZ
    if (scale_method >= NUM_FILTERS)
        scale_method = 0;
#else
    if (scale_method >= NUM_FILTERS - 3)
        scale_method = 0;
#endif /* USE_XBRZ */

    if (Settings.SkipFrames == THROTTLE_SOUND_SYNC)
        Settings.SoundSync = true;
    else
        Settings.SoundSync = false;

    hires_effect = CLAMP(hires_effect, 0, 2);
    Settings.DynamicRateLimit = CLAMP(Settings.DynamicRateLimit, 1, 1000);
    Settings.SuperFXClockMultiplier = CLAMP(Settings.SuperFXClockMultiplier, 50, 400);
    ntsc_scanline_intensity = MIN(ntsc_scanline_intensity, 4);
    scanline_filter_intensity = MIN(scanline_filter_intensity, 3);

    return 0;
}

void Snes9xConfig::rebind_keys()
{
    s9xcommand_t cmd;
    std::string string;

    S9xUnmapAllControls();

    for (int joypad_i = 0; joypad_i < NUM_JOYPADS; joypad_i++)
    {
        auto &bin = pad[joypad_i].data;

        for (int button_i = 0; button_i < NUM_JOYPAD_LINKS; button_i++)
        {
            int dupe;
            for (dupe = button_i + 1; dupe < NUM_JOYPAD_LINKS; dupe++)
            {
                if (bin[button_i] == bin[dupe] && bin[button_i].hex() != 0)
                    break;
            }
            if (dupe < NUM_JOYPAD_LINKS || bin[button_i].hex() == 0)
                continue;

            string = "Joypad" + std::to_string((joypad_i % 5) + 1) + " ";
            string += b_links[button_i].snes9x_name;

            bool ismulti = false;
            for (dupe = button_i - 1; dupe > 0; dupe--)
            {
                if (bin[button_i] == bin[dupe])
                {
                    ismulti = true;
                    string += ",Joypad" + std::to_string((joypad_i % 5) + 1) + " ";
                    string += b_links[dupe].snes9x_name;
                }
            }

            if (ismulti)
                string = std::string("{") + string + "}";

            cmd = S9xGetPortCommandT(string.c_str());

            S9xMapButton(bin[button_i].base_hex(), cmd, false);
        }
    }

    for (int i = NUM_JOYPAD_LINKS; b_links[i].snes9x_name; i++)
    {
        cmd = S9xGetPortCommandT(b_links[i].snes9x_name);
        S9xMapButton(shortcut[i - NUM_JOYPAD_LINKS].base_hex(),
                     cmd,
                     false);
    }

    cmd = S9xGetPortCommandT("Pointer Mouse1+Superscope+Justifier1");
    S9xMapPointer(BINDING_MOUSE_POINTER, cmd, true);

    cmd = S9xGetPortCommandT("{Mouse1 L,Superscope Fire,Justifier1 Trigger}");
    S9xMapButton(BINDING_MOUSE_BUTTON0, cmd, false);

    cmd = S9xGetPortCommandT("{Justifier1 AimOffscreen Trigger,Superscope AimOffscreen}");
    S9xMapButton(BINDING_MOUSE_BUTTON1, cmd, false);

    cmd = S9xGetPortCommandT("{Mouse1 R,Superscope Cursor,Justifier1 Start}");
    S9xMapButton(BINDING_MOUSE_BUTTON2, cmd, false);
}
