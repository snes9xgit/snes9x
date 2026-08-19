/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <cstdlib>
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
        return std::string{".supersnes9x"};
    }

    fs::path config;
    // If XDG_CONFIG_HOME is set, use that, otherwise guess default
    if (env_xdg_config_home)
    {
        config = env_xdg_config_home;
        config /= "supersnes9x";
    }
    else
    {
        config = env_home;
        config /= ".config/supersnes9x";
    }

    if (!fs::exists(config))
        fs::create_directories(config);

    return config;
}

std::string get_config_file_name()
{
    return get_config_dir() + "/super-snes9x-gtk.conf";
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
    hires_scale_method = 0;
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
    enable_rumble = true;
    fullscreen = false;
    ui_visible = true;
    default_esc_behavior = 1;
    prevent_screensaver = false;
    sound_driver = 0;
    sound_buffer_size = 48;
    sound_playback_rate = 7;
    sound_input_rate = 32040;
    auto_input_rate = false;
    master_volume_regular = 100;
    master_volume_fast_forward = 100;
    sgb_mix_volume_spc = 50;
    sgb_mix_volume_gb = 50;
    gain_regular = 0;
    sgb_mix_gain_spc = 0;
    sgb_mix_gain_gb = 0;
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
    bios_directory = "Bios";
    screensaver_needs_reset = false;
    ntsc_setup = snes_ntsc_composite;
    ntsc_scanline_intensity = 1;
    scanline_filter_intensity = 0;
    Settings.BilinearFilter = false;
    // Game Boy frame blending (Super Game Boy only) — single stored value in Settings,
    // same as the win32 frontend. 0=off, 1=Simple Blend, 2=LCD Blend; layer 0=all,
    // 1=bg, 2=window, 3=sprites; Auto picks both per-game from a built-in table.
    Settings.GBFrameBlend = 0;
    Settings.GBFrameBlendLayer = 0;
    Settings.GBFrameBlendAuto = true;
    Settings.ColorCorrection = false;
    Settings.AdjustmentsEnabled = false;
    Settings.Gamma = 0;
    Settings.Contrast = 0;
    Settings.Saturation = 0;
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
    ra_enabled = false;
    ra_hardcore_mode = false;
    ra_username.clear();
    ra_api_token.clear();
    ra_emulator_name = "SuperSnes9x";
    modal_dialogs = true;
    config_show_comments = true;
    config_nice_alignment = true;
    current_save_slot = 0;
    current_save_bank = 0;
    S9xCheatsEnable();

    rewind_granularity = 5;
    rewind_buffer_size = 0;
    Settings.Rewinding = false;
    Settings.RunAhead = 0;
    Settings.InRunAhead = false;

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
    Settings.SkipFrames = THROTTLE_TIMER_FRAMESKIP;
    Settings.Transparency = true;
    Settings.DisplayTime = false;
    Settings.DisplayFrameRate = false;
    Settings.DisplayIndicators = false;
    /* Embed a screenshot in each snapshot so the save/load-with-preview
     * dialog has something to show, as win32 does by default. */
    Settings.SnapshotScreenshots = true;
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
    Settings.TurboSkipFrames = 15;
    Settings.DisplayPressedKeys = false;
    Settings.InitialInfoStringTimeout   =  120;
    Settings.SGB_BIOSPreference = 2;
    
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

    auto outbool = [&](const std::string &name, bool b, const std::string &comment = "") {
        cf.SetBool((section + "::" + name).c_str(), b, "true", "false", comment.c_str());
    };

    auto outstring = [&](const std::string &name, const std::string &str, const std::string &comment = "") {
        cf.SetString((section + "::" + name).c_str(), str, comment.c_str());
    };

    auto outint = [&](const std::string &name, int i, const std::string &comment = "") {
        cf.SetInt((section + "::" + name).c_str(), i, comment.c_str());
    };

    // Section name and semantics match the win32 config (wconfig.cpp) so these
    // two knobs control the look of this file the same way on both platforms.
    section = "Config";
    outbool("NiceAlignment", config_nice_alignment, "true to line up the = and # columns in each section of this config file");
    outbool("Comments", config_show_comments, "true to keep comments such as this in this config file. To refresh all comments after an upgrade, set this to false and run Snes9x, then set it back to true and run Snes9x again.");

    section = "Display";
    outbool("FullscreenOnOpen", full_screen_on_open, "Set the screen resolution after opening a ROM");
    outbool("ChangeDisplayResolution", change_display_resolution, "Set the resolution in fullscreen mode");
    outbool("ScaleToFit", scale_to_fit, "Scale the image to fit the window size");
    outbool("ShowOverscanArea", overscan, "Show the overscan area at the top and bottom that most games hide");
    outbool("MaintainAspectRatio", maintain_aspect_ratio, "Resize the screen to the proportions set by aspect ratio option");
    outbool("Multithreading", multithreading, "Apply filters using multiple threads");
    outbool("BilinearFilter", Settings.BilinearFilter, "Smoothes scaled image");
    outbool("ForceInvertedByteOrder", force_inverted_byte_order, "Swap the pixel byte order (troubleshooting for wrong colors on unusual setups; normally off)");
    outint("VideoMode", xrr_index, "Platform-specific video mode number");
    outint("AspectRatio", aspect_ratio, "0: uncorrected, 1: uncorrected integer scale, 2: 4:3, 3: 4/3 integer scale, 4: NTSC/PAL, 5: NTSC/PAL integer scale");
    outint("SoftwareScaleFilter", scale_method, "Build-specific number of filter used for software scaling");
    outint("SoftwareScaleFilterHires", hires_scale_method, "Filter used instead for hi-res (512-wide or interlaced) frames");
    outint("ScanlineFilterIntensity", scanline_filter_intensity, "0: 0%, 1: 12.5%, 2: 25%, 3: 50%, 4: 100%");
    outint("HiresEffect", hires_effect, "0: Downscale to low-res, 1: Leave as-is, 2: Upscale low-res screens");
    outint("NumberOfThreads", num_threads, "Number of worker threads to use when Multithreading is enabled");
    outstring("HardwareAcceleration", display_driver, "none, opengl, xv, vulkan");
    outint("SplashBackground", splash_image, "0: Black, 1: Color bars, 2: Pattern, 3: Blue, 4: Default");
    outbool("AutoVRR", auto_vrr, "Automatically use the best settings for variable sync in fullscreen mode");
    outint("OSDSize", osd_size, "Size of on-screen display elements. Default: 24pt");
    outint("MessageDisplayTime", Settings.InitialInfoStringTimeout, "Display timeout length of messages, in frames. Set to 0 to disable all message text. Default: 120.");
    // Key names match the win32 config (wconfig.cpp) so a shared install reads/writes the same entries.
    outint("BlendGBFrames", Settings.GBFrameBlend, "Game Boy frame-blend mode (Super Game Boy only): 0=off, 1=Simple Blend (mix each frame 50/50 with the previous, fixes flicker-based fake transparency e.g. ZAS), 2=LCD Blend (slow-decay LCD-style ghosting)");
    outint("BlendGBFramesLayer", Settings.GBFrameBlendLayer, "Which Game Boy layer the frame-blend applies to: 0=all, 1=background (keeps moving sprites crisp), 2=window, 3=sprites");
    outbool("BlendGBFramesAuto", Settings.GBFrameBlendAuto, "Auto-pick the GB frame-blend per game from a built-in known-flicker-game table at load (off for unlisted games); when false the manual mode/layer apply to every GB game");
    outbool("ColorCorrection", Settings.ColorCorrection, "Enable accurate SNES color correction (simulates SNES CRT output)");
    outbool("AdjustmentsEnabled", Settings.AdjustmentsEnabled, "Apply the Gamma/Contrast/Saturation adjustments below");
    outint("Gamma", Settings.Gamma, "Gamma adjustment (-100..+100, 0 = no change)");
    outint("Contrast", Settings.Contrast, "Contrast adjustment (-100..+100, 0 = no change)");
    outint("Saturation", Settings.Saturation, "Saturation adjustment (-100..+100, 0 = no change)");
    
    
    // NTSC composite-video filter knobs; only used when the NTSC software filter
    // is selected. Each is a float from -1.0 to 1.0 (0.0 = neutral/default).
    section = "NTSC";
    outstring("Hue", std::to_string(ntsc_setup.hue), "Color hue shift, -1.0 to 1.0");
    outstring("Saturation", std::to_string(ntsc_setup.saturation), "Color saturation, -1.0 to 1.0");
    outstring("Contrast", std::to_string(ntsc_setup.contrast), "Contrast, -1.0 to 1.0");
    outstring("Brightness", std::to_string(ntsc_setup.brightness), "Brightness, -1.0 to 1.0");
    outstring("Sharpness", std::to_string(ntsc_setup.sharpness), "Sharpness, -1.0 to 1.0");
    outstring("Artifacts", std::to_string(ntsc_setup.artifacts), "Amount of NTSC artifact color, -1.0 to 1.0");
    outstring("Gamma", std::to_string(ntsc_setup.gamma), "Gamma, -1.0 to 1.0");
    outstring("Bleed", std::to_string(ntsc_setup.bleed), "Horizontal color bleed, -1.0 to 1.0");
    outstring("Fringing", std::to_string(ntsc_setup.fringing), "Color fringing on edges, -1.0 to 1.0");
    outstring("Resolution", std::to_string(ntsc_setup.resolution), "Image resolution/detail, -1.0 to 1.0");
    outbool("MergeFields", ntsc_setup.merge_fields, "Blend the two interlaced fields together to reduce flicker");
    outint("ScanlineIntensity", ntsc_scanline_intensity, "0: 0%, 1: 12.5%, 2: 25%, 3: 50%, 4: 100%");

    section = "OpenGL";
    outbool("VSync", sync_to_vblank, "Synchronize to the monitor's refresh to avoid tearing");
    outbool("ReduceInputLag", reduce_input_lag, "Wait for each frame to finish drawing before continuing (lower lag, may cost speed)");
    outbool("EnableCustomShaders", use_shaders, "Apply the shader named in ShaderFile below");
    outstring("ShaderFile", shader_filename, "Path to the shader preset (.glslp/.slangp) used when EnableCustomShaders is on");

    section = "Sound";
    outbool("MuteSound", mute_sound, "Silence all audio output");
    outbool("MuteSoundDuringTurbo", mute_sound_turbo, "Silence audio only while fast-forwarding");
    outint("BufferSize", sound_buffer_size, "Buffer size in milliseconds");
    outint("Driver", sound_driver, "Build-specific sound driver index (order of the drivers compiled in, e.g. PulseAudio/ALSA/OSS/SDL/PortAudio)");
    outint("InputRate", sound_input_rate, "APU sample rate in Hz that gets resampled to the output rate; default 32040. Nudges audio pitch/sync");
    outbool("DynamicRateControl", Settings.DynamicRateControl, "Slightly bend the sample rate to keep the buffer full and avoid crackle/dropouts");
    outint("DynamicRateControlLimit", Settings.DynamicRateLimit, "How far Dynamic Rate Control may bend the rate (stored as the UI ratio x 1000; larger = more correction)");
    outbool("AutomaticInputRate", auto_input_rate, "Guess input rate by asking the monitor what its refresh rate is");
    outint("PlaybackRate", gui_config->sound_playback_rate, "1: 8000Hz, 2: 11025Hz, 3: 16000Hz, 4: 22050Hz, 5: 32000Hz, 6: 44100Hz, 7: 48000Hz");
    outint("MasterVolumeRegular", master_volume_regular, "Master output volume during normal play (0..100, percent)");
    outint("MasterVolumeFastForward", master_volume_fast_forward, "Master output volume during turbo/rewind (0..100, percent)");
    outint("VolumeSGBMixSPC", sgb_mix_volume_spc, "SGB BIOS mix: SPC channel volume (0..100, percent; only active in SGB BIOS mode)");
    outint("VolumeSGBMixGB", sgb_mix_volume_gb, "SGB BIOS mix: GB channel volume (0..100, percent; only active in SGB BIOS mode)");
    outint("GainRegular", gain_regular, "Master pre-amp applied after the volume percentages (whole dB between 0 and 12, 0 = unity)");
    outint("GainSGBMixSPC", sgb_mix_gain_spc, "SGB BIOS mix: SPC channel pre-amp (whole dB between 0 and 12, 0 = unity)");
    outint("GainSGBMixGB", sgb_mix_gain_gb, "SGB BIOS mix: GB channel pre-amp (whole dB between 0 and 12, 0 = unity)");

    // Folders Snes9x reads from and writes to. Leave a folder empty to use the
    // default location (next to the ROM, or the standard config directory).
    section = "Files";
    outstring("LastDirectory", last_directory, "Last folder browsed for a ROM (remembered automatically)");
    outstring("LastShaderDirectory", last_shader_directory, "Last folder browsed for a shader (remembered automatically)");
    outstring("SRAMDirectory", sram_directory, "Where in-game battery saves (.srm) are stored");
    outstring("SaveStateDirectory", savestate_directory, "Where save states are stored");
    outstring("CheatDirectory", cheat_directory, "Where cheat files (.cht) are stored");
    outstring("PatchDirectory", patch_directory, "Where ROM patches (IPS/UPS/BPS) are looked for");
    outstring("ExportDirectory", export_directory, "Where exported files (e.g. SPC dumps) are written");
    outstring("BIOSDirectory", bios_directory, "Where BIOS images (e.g. the Super Game Boy boot ROM) are looked for");

    // Window sizes and UI state, remembered automatically between sessions.
    section = "Window State";
    outint("MainWidth", window_width);
    outint("MainHeight", window_height);
    outint("PreferencesWidth", preferences_width);
    outint("PreferencesHeight", preferences_height);
    outint("ShaderParametersWidth", shader_parameters_width);
    outint("ShaderParametersHeight", shader_parameters_height);
    outint("CurrentDisplayTab", current_display_tab, "Last-selected tab in the Preferences window");
    outbool("UIVisible", ui_visible, "Show the menu bar");
    outbool("EnableIcons", enable_icons, "Show icons next to menu items");
    if (default_esc_behavior != ESC_TOGGLE_MENUBAR)
        outbool("Fullscreen", false);
    else
        outbool("Fullscreen", fullscreen);

    section = "Netplay";
    outbool("ActAsServer", netplay_is_server, "true to host the session, false to join one");
    outbool("UseResetToSync", netplay_sync_reset, "Reset the game when players connect so everyone starts in sync");
    outbool("SendROM", netplay_send_rom, "Send the ROM to clients that don't already have it");
    outint("DefaultPort", netplay_default_port, "TCP port used for hosting/joining (default 6096)");
    outint("MaxFrameLoss", netplay_max_frame_loss, "How many frames a client may fall behind before the host waits for it");
    outint("LastUsedPort", netplay_last_port, "Port from the last session (remembered automatically)");
    outstring("LastUsedROM", netplay_last_rom, "ROM from the last session (remembered automatically)");
    outstring("LastUsedHost", netplay_last_host, "Host address from the last session (remembered automatically)");

    // Key names match the win32 (wconfig.cpp) and Qt (EmuConfig.cpp) configs so a
    // shared install reads/writes the same RetroAchievements credentials.
    section = "RetroAchievements";
    outbool("Enabled", ra_enabled, "Connect to RetroAchievements and track achievements");
    outbool("HardcoreMode", ra_hardcore_mode, "Disable save states, cheats and rewind for competitive achievement earning");
    outstring("Username", ra_username, "RetroAchievements account name");
    outstring("ApiToken", ra_api_token, "Login token issued by RetroAchievements (not your password); cleared on logout");
    outstring("EmulatorName", ra_emulator_name, "Client name reported to the RetroAchievements server");

    section = "Behavior";
    outbool("PauseEmulationWhenFocusLost", pause_emulation_on_switch, "Pause the game whenever the Snes9x window loses focus");
    outint("DefaultESCKeyBehavior", default_esc_behavior, "What the Esc key does: 0: toggle the menu bar, 1: leave fullscreen, 2: quit Snes9x");
    outbool("PreventScreensaver", prevent_screensaver, "Stop the system screensaver from starting while a game is running");
    outbool("UseModalDialogs", modal_dialogs, "Make dialogs block the main window instead of floating independently");
    outint("RewindBufferSize", rewind_buffer_size, "Amount of memory (in MB) to use for rewinding; 0 disables rewind");
    outint("RewindGranularity", rewind_granularity, "Only save rewind snapshots every N frames");
    outint("CurrentSaveSlot", current_save_slot, "Currently selected save-state slot within the bank (remembered automatically)");
    outint("CurrentSaveBank", current_save_bank, "Currently selected save-state bank (remembered automatically)");

    section = "Emulation";
    outbool("EmulateTransparency", Settings.Transparency, "Render the SNES color/transparency effects (turn off only for troubleshooting)");
    outbool("DisplayTime", Settings.DisplayTime, "Show the current wall-clock time on screen");
    outbool("DisplayFrameRate", Settings.DisplayFrameRate, "Show the frames-per-second counter on screen");
    outbool("DisplayPressedKeys", Settings.DisplayPressedKeys, "Show the controller buttons being pressed on screen");
    outbool("DisplayIndicators", Settings.DisplayIndicators, "Show on-screen indicators for turbo, pause, rewind, etc.");
    outbool("SnapshotScreenshots", Settings.SnapshotScreenshots, "Store a screenshot inside each save state, for the save/load-with-preview dialog");
    outint("SpeedControlMethod", Settings.SkipFrames, "0: Time the frames to 50 or 60Hz, 1: Same, but skip frames if too slow, 2: Synchronize to the sound buffer, 3: Unlimited, except potentially by vsync");
    outint("SaveSRAMEveryNSeconds", Settings.AutoSaveDelay, "Auto-write the battery save this many seconds after the game changes it (0: only on exit/reset)");
    outbool("BlockInvalidVRAMAccess", Settings.BlockInvalidVRAMAccessMaster, "Emulate the real hardware's VRAM access restrictions (on for accuracy; off only for a few broken ROMs/hacks)");
    outbool("AllowDPadContradictions", Settings.UpAndDown, "Allow the D-Pad to press both up + down at the same time, or left + right");
    outint("RunAhead", Settings.RunAhead, "Number of frames to run ahead for reduced input latency (0 = off, 1-4)");

    section = "Hacks";
    outint("SuperFXClockMultiplier", Settings.SuperFXClockMultiplier, "SuperFX (GSU) chip speed as a percentage of normal (50-400; 100 = accurate). Higher reduces slowdown in Star Fox and other SuperFX games");
    outint("SoundInterpolationMethod", Settings.InterpolationMethod, "0: None, 1: Linear, 2: Gaussian (what the hardware uses), 3: Cubic, 4: Sinc");
    outbool("RemoveSpriteLimit", Settings.MaxSpriteTilesPerLine != 34, "Draw more sprites per scanline than the hardware allows (reduces flicker, but can cause glitches)");
    outbool("OverclockCPU", Settings.OneClockCycle != 6, "Speed up the emulated CPU to cut in-game slowdown (inaccurate; can break some games)");
    outbool("EchoBufferHack", Settings.SeparateEchoBuffer, "Prevents echo buffer from overwriting APU RAM");

    // Key path "SGB::BIOSPreference" matches the win32 config (wconfig.cpp) and
    // the CLI (snes9x.cpp) so every port reads/writes the same entry.
    section = "SGB";
    outint("BIOSPreference", Settings.SGB_BIOSPreference, "BIOS mode for GB/GBC ROMs: 0=No BIOS (BIOS-less), 1=SGB1, 2=SGB2 (default)");

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

        outstring(name, value, "Device in this port: none, joypad, mouse, superscope, justifier, or multitap");
    }

    outint("JoystickThreshold", joystick_threshold, "How far an analog stick/trigger must move to register as pressed (percent, 1-100)");
    outbool("EnableRumble", enable_rumble, "on to pass rumble-cart motor effects (LRG SNES releases) to the port-1 gamepad");

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

    ConfigFile::SetProgramName("SuperSnes9x");
    ConfigFile::SetNiceAlignment(config_nice_alignment);
    ConfigFile::SetShowComments(config_show_comments);
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
            fmt::print(stderr, fmt::runtime(_("Couldn't create config directory: {}\n")), path.string());
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

    auto inbool = [&](const std::string &name, auto &b) {
        if (cf.Exists((section + "::" + name).c_str()))
            b = cf.GetBool((section + "::" + name).c_str());
    };

    auto inint = [&](const std::string &name, auto &i) {
        if (cf.Exists((section + "::" + name).c_str()))
            i = cf.GetInt((section + "::" + name).c_str());
    };

    auto indouble = [&](const std::string &name, double &d) {
        if (cf.Exists((section + "::" + name).c_str()))
            d = atof(cf.GetString((section + "::" + name).c_str()));
    };

    auto instr = [&](const std::string &name, std::string &str) {
        str = cf.GetString((section + "::" + name).c_str(), none);
    };

    section = "Config";
    inbool("NiceAlignment", config_nice_alignment);
    inbool("Comments", config_show_comments);

    section = "Display";
    inbool("FullscreenOnOpen", full_screen_on_open);
    inbool("ChangeDisplayResolution", change_display_resolution);
    inint("VideoMode", xrr_index);
    inbool("ScaleToFit", scale_to_fit);
    inbool("MaintainAspectRatio", maintain_aspect_ratio);
    inint("AspectRatio", aspect_ratio);
    inint("SoftwareScaleFilter", scale_method);
    inint("SoftwareScaleFilterHires", hires_scale_method);
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
    inint("MessageDisplayTime", Settings.InitialInfoStringTimeout);
    inint("BlendGBFrames", Settings.GBFrameBlend);
    inint("BlendGBFramesLayer", Settings.GBFrameBlendLayer);
    inbool("BlendGBFramesAuto", Settings.GBFrameBlendAuto);
    inbool("ColorCorrection", Settings.ColorCorrection);
    inbool("AdjustmentsEnabled", Settings.AdjustmentsEnabled);
    inint("Gamma", Settings.Gamma);
    inint("Contrast", Settings.Contrast);
    inint("Saturation", Settings.Saturation);

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
    inint("MasterVolumeRegular", master_volume_regular);
    inint("MasterVolumeFastForward", master_volume_fast_forward);
    inint("VolumeSGBMixSPC", sgb_mix_volume_spc);
    inint("VolumeSGBMixGB", sgb_mix_volume_gb);
    inint("GainRegular", gain_regular);
    inint("GainSGBMixSPC", sgb_mix_gain_spc);
    inint("GainSGBMixGB", sgb_mix_gain_gb);

    section = "Files";
    instr("LastDirectory", last_directory);
    instr("LastShaderDirectory", last_shader_directory);
    instr("SRAMDirectory", sram_directory);
    instr("SaveStateDirectory", savestate_directory);
    instr("CheatDirectory", cheat_directory);
    instr("PatchDirectory", patch_directory);
    instr("ExportDirectory", export_directory);
    instr("BIOSDirectory", bios_directory);

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

    section = "RetroAchievements";
    inbool("Enabled", ra_enabled);
    inbool("HardcoreMode", ra_hardcore_mode);
    instr("Username", ra_username);
    instr("ApiToken", ra_api_token);
    instr("EmulatorName", ra_emulator_name);

    section = "Behavior";
    inbool("PauseEmulationWhenFocusLost", pause_emulation_on_switch);
    inint("DefaultESCKeyBehavior", default_esc_behavior);
    inbool("PreventScreensaver", prevent_screensaver);
    inbool("UseModalDialogs", modal_dialogs);
    inint("RewindBufferSize", rewind_buffer_size);
    inint("RewindGranularity", rewind_granularity);
    inint("CurrentSaveSlot", current_save_slot);
    inint("CurrentSaveBank", current_save_bank);

    /* Older configs stored a flat 0-999 slot index. Split it into a bank and
     * an in-bank slot so the selection survives the upgrade. */
    if (current_save_slot >= SAVE_SLOTS_PER_BANK)
    {
        current_save_bank = current_save_slot / SAVE_SLOTS_PER_BANK;
        current_save_slot %= SAVE_SLOTS_PER_BANK;
    }
    if (current_save_slot < 0)
        current_save_slot = 0;
    if (current_save_bank < 0 || current_save_bank >= NUM_SAVE_BANKS)
        current_save_bank = 0;

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
    inbool("SnapshotScreenshots", Settings.SnapshotScreenshots);
    inint("RunAhead", Settings.RunAhead);
    if (Settings.RunAhead < 0)
        Settings.RunAhead = 0;
    if (Settings.RunAhead > 4)
        Settings.RunAhead = 4;

    section = "Hacks";
    inint("SuperFXClockMultiplier", Settings.SuperFXClockMultiplier);
    inint("SoundInterpolationMethod", Settings.InterpolationMethod);

    bool RemoveSpriteLimit = false;
    inbool("RemoveSpriteLimit", RemoveSpriteLimit);
    bool OverclockCPU = false;
    inbool("OverclockCPU", OverclockCPU);
    inbool("EchoBufferHack", Settings.SeparateEchoBuffer);

    section = "SGB";
    inint("BIOSPreference", Settings.SGB_BIOSPreference);
    if (Settings.SGB_BIOSPreference > 2)
        Settings.SGB_BIOSPreference = 2;

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
    inbool("EnableRumble", enable_rumble);

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

    // The RA-registered emulator name must be non-empty (drives the User-Agent).
    if (ra_emulator_name.empty())
        ra_emulator_name = "SuperSnes9x";

#ifdef USE_HQ2X
    if (scale_method >= NUM_FILTERS)
        scale_method = 0;
    if (hires_scale_method >= NUM_FILTERS)
        hires_scale_method = 0;
#else
    if (scale_method >= NUM_FILTERS - 3)
        scale_method = 0;
    if (hires_scale_method >= NUM_FILTERS - 3)
        hires_scale_method = 0;
#endif /* USE_HQ2X */

#ifdef USE_XBRZ
    if (scale_method >= NUM_FILTERS)
        scale_method = 0;
    if (hires_scale_method >= NUM_FILTERS)
        hires_scale_method = 0;
#else
    if (scale_method >= NUM_FILTERS - 3)
        scale_method = 0;
    if (hires_scale_method >= NUM_FILTERS - 3)
        hires_scale_method = 0;
#endif /* USE_XBRZ */

    if (Settings.SkipFrames == THROTTLE_SOUND_SYNC)
        Settings.SoundSync = true;
    else
        Settings.SoundSync = false;

    hires_effect = CLAMP(hires_effect, 0, 2);
    Settings.GBFrameBlend = CLAMP(Settings.GBFrameBlend, 0, 2);
    Settings.GBFrameBlendLayer = CLAMP(Settings.GBFrameBlendLayer, 0, 3);
    Settings.Gamma = CLAMP(Settings.Gamma, -100, 100);
    Settings.Contrast = CLAMP(Settings.Contrast, -100, 100);
    Settings.Saturation = CLAMP(Settings.Saturation, -100, 100);
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

    cmd = S9xGetPortCommandT("Superscope ToggleTurbo");
    S9xMapButton(BINDING_MOUSE_BUTTON1, cmd, false);

    cmd = S9xGetPortCommandT("{Mouse1 R,Superscope Pause,Justifier1 Start}");
    S9xMapButton(BINDING_MOUSE_BUTTON2, cmd, false);

    cmd = S9xGetPortCommandT("Superscope Cursor");
    S9xMapButton(BINDING_MOUSE_BUTTON0 + 7, cmd, false);

    cmd = S9xGetPortCommandT("{Superscope AimOffscreen,Justifier1 AimOffscreen}");
    S9xMapButton(BINDING_MOUSE_BUTTON0 + 8, cmd, false);
}
