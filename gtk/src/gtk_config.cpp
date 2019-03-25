/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "gtk_config.h"
#include "gtk_s9x.h"
#include "gtk_sound.h"
#include "gtk_display.h"
#include "conffile.h"

static bool directory_exists (std::string str)
{
    DIR *dir;

    dir = opendir (str.c_str ());

    if (dir)
    {
        closedir (dir);
        return true;
    }

    return false;
}

std::string get_config_dir ()
{
    // Find config directory
    char *env_home = getenv ("HOME");
    char *env_xdg_config_home = getenv ("XDG_CONFIG_HOME");

    if (!env_home && !env_xdg_config_home)
    {
        return std::string (".snes9x");
    }

    std::string config;
    std::string legacy;

    // If XDG_CONFIG_HOME is set, use that, otherwise guess default
    if (!env_xdg_config_home)
    {
        (config += env_home) += "/.config/snes9x";
        (legacy += env_home) += "/.snes9x";
    }
    else
        config = std::string (env_xdg_config_home) + "/snes9x";

    if (directory_exists (legacy) && !directory_exists(config))
        return legacy;

    return config;
}

std::string get_config_file_name ()
{
    return get_config_dir () + "/snes9x.conf";
}

void S9xParsePortConfig (ConfigFile &conf, int pass)
{
}

Snes9xConfig::Snes9xConfig ()
{
    joystick = NULL;
    joystick_threshold = 40;
}

int Snes9xConfig::load_defaults ()
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
    splash_image = SPLASH_IMAGE_COMBO;
    hw_accel = 0;
    allow_opengl = false;
    allow_xv = false;
    allow_xrandr = false;
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
    sound_buffer_size = 32;
    sound_playback_rate = 5;
    sound_input_rate = 31950;
    auto_input_rate = true;
    last_directory.clear ();
    last_shader_directory.clear ();
    window_width = -1;
    window_height = -1;
    preferences_width = -1;
    preferences_height = -1;
    shader_parameters_width = -1;
    shader_parameters_height = -1;
    current_display_tab = 0;
    sram_directory.clear ();
    export_directory.clear ();
    savestate_directory.clear ();
    cheat_directory.clear ();
    patch_directory.clear ();
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
    netplay_last_rom.clear ();
    netplay_last_host.clear ();
    netplay_last_port = 6096;
    modal_dialogs = true;
    current_save_slot = 0;
    S9xCheatsEnable ();

    rewind_granularity = 5;
    rewind_buffer_size = 0;
    Settings.Rewinding = false;

#ifdef USE_OPENGL
    sync_to_vblank = true;
    use_pbos = true;
    pbo_format = 0;
    npot_textures = false;
    use_shaders = false;
    shader_filename.clear ();
    use_glfinish = false;
    use_sync_control = false;
#endif

    /* Snes9x Variables */
    Settings.MouseMaster = true;
    Settings.SuperScopeMaster = true;
    Settings.JustifierMaster = true;
    Settings.MultiPlayer5Master = true;
    Settings.UpAndDown = false;
    Settings.AutoSaveDelay = 0;
    Settings.SkipFrames = 0;
    Settings.Transparency = true;
    Settings.DisplayFrameRate = false;
    Settings.SixteenBitSound = true;
    Settings.Stereo = true;
    Settings.ReverseStereo = false;
    Settings.SoundPlaybackRate = 48000;
    Settings.StopEmulation = true;
    Settings.FrameTimeNTSC = 16639;
    Settings.FrameTimePAL = 20000;
    Settings.SupportHiRes = true;
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

    memset (pad, 0, sizeof (JoypadBinding) * NUM_JOYPADS);
    memset (shortcut, 0, sizeof (Binding) * NUM_EMU_LINKS);

    return 0;
}

void Snes9xConfig::joystick_register_centers ()
{
    for (int i = 0; joystick[i] != NULL; i++)
        joystick[i]->register_centers ();
}

void Snes9xConfig::flush_joysticks ()
{
    int i;

    for (i = 0; joystick[i] != NULL; i++)
        joystick[i]->flush ();
}

void Snes9xConfig::set_joystick_mode (int mode)
{
    int i;
    for (i = 0; joystick[i] != NULL; i++)
        joystick[i]->mode = mode;
}

static inline void outbool (ConfigFile &cf, const char *key, bool value, const char *comment = "")
{
    cf.SetBool (key, value, "true", "false", comment);
}

int Snes9xConfig::save_config_file ()
{
    char key[PATH_MAX];
    char buffer[PATH_MAX];
    ConfigFile cf;

#undef z
#define z "Display::"
    outbool (cf, z"FullscreenOnOpen", full_screen_on_open,"Set the screen resolution after opening a ROM");
    outbool (cf, z"ChangeDisplayResolution", change_display_resolution, "Set the resolution in fullscreen mode");
    cf.SetInt   (z"VideoMode", xrr_index, "Platform-specific video mode number");
    outbool (cf, z"ScaleToFit", scale_to_fit, "Scale the image to fit the window size");
    outbool (cf, z"MaintainAspectRatio", maintain_aspect_ratio, "Resize the screen to the proportions set by aspect ratio option");
    cf.SetInt   (z"AspectRatio", aspect_ratio, "0: uncorrected, 1: uncorrected integer scale, 2: 4:3, 3: 4/3 integer scale, 4: NTSC/PAL, 5: NTSC/PAL integer scale");
    cf.SetInt   (z"SoftwareScaleFilter", scale_method, "Build-specific number of filter used for software scaling");
    cf.SetInt   (z"ScanlineFilterIntensity", scanline_filter_intensity, "0: 0%, 1: 12.5%, 2: 25%, 3: 50%, 4: 100%");
    outbool (cf, z"ShowOverscanArea", overscan);
    cf.SetInt   (z"HiresEffect", hires_effect, "0: Downscale to low-res, 1: Leave as-is, 2: Upscale low-res screens");
    cf.SetInt   (z"ForceInvertedByteOrder", force_inverted_byte_order);
    outbool (cf, z"Multithreading", multithreading, "Apply filters using multiple threads");
    cf.SetInt   (z"NumberOfThreads", num_threads);
    cf.SetInt   (z"HardwareAcceleration", hw_accel, "0: None, 1: OpenGL, 2: XVideo");
    outbool (cf, z"BilinearFilter", Settings.BilinearFilter, "Smoothes scaled image");
    cf.SetInt   (z"SplashBackground", splash_image, "0: Black, 1: Color bars, 2: Pattern, 3: Blue, 4: Default");

#undef z
#define z "NTSC::"
    cf.SetString (z"Hue", std::to_string (ntsc_setup.hue));
    cf.SetString (z"Saturation", std::to_string (ntsc_setup.saturation));
    cf.SetString (z"Contrast", std::to_string (ntsc_setup.contrast));
    cf.SetString (z"Brightness", std::to_string (ntsc_setup.brightness));
    cf.SetString (z"Sharpness", std::to_string (ntsc_setup.sharpness));
    cf.SetString (z"Artifacts", std::to_string (ntsc_setup.artifacts));
    cf.SetString (z"Gamma", std::to_string (ntsc_setup.gamma));
    cf.SetString (z"Bleed", std::to_string (ntsc_setup.bleed));
    cf.SetString (z"Fringing", std::to_string (ntsc_setup.fringing));
    cf.SetString (z"Resolution", std::to_string (ntsc_setup.resolution));
    outbool  (cf, z"MergeFields", ntsc_setup.merge_fields);
    cf.SetInt    (z"ScanlineIntensity", ntsc_scanline_intensity);

#ifdef USE_OPENGL
#undef z
#define z "OpenGL::"
    outbool   (cf, z"VSync", sync_to_vblank);
    outbool   (cf, z"glFinish", use_glfinish);
    outbool   (cf, z"SyncControl", use_sync_control);
    outbool   (cf, z"UsePixelBufferObjects", use_pbos);
    cf.SetInt     (z"PixelBufferObjectBitDepth", pbo_format);
    outbool   (cf, z"UseNonPowerOfTwoTextures", npot_textures);
    outbool   (cf, z"EnableCustomShaders", use_shaders);
    cf.SetString  (z"ShaderFile", shader_filename);
#endif

#undef z
#define z "Sound::"
    outbool (cf, z"MuteSound", mute_sound);
    outbool (cf, z"MuteSoundDuringTurbo", mute_sound_turbo);
    cf.SetInt   (z"BufferSize", sound_buffer_size, "Buffer size in milliseconds");
    cf.SetInt   (z"Driver", sound_driver);
    cf.SetInt   (z"InputRate", sound_input_rate);
    outbool (cf, z"DynamicRateControl", Settings.DynamicRateControl);
    cf.SetInt   (z"DynamicRateControlLimit", Settings.DynamicRateLimit);
    outbool (cf, z"AutomaticInputRate", auto_input_rate, "Guess input rate by asking the monitor what its refresh rate is");
    cf.SetInt   (z"PlaybackRate", gui_config->sound_playback_rate, "1: 8000Hz, 2: 11025Hz, 3: 16000Hz, 4: 22050Hz, 5: 32000Hz, 6: 44100Hz, 7: 48000Hz");

#undef z
#define z "Files::"

    cf.SetString (z"LastDirectory", last_directory);
    cf.SetString (z"LastShaderDirectory", last_shader_directory);
    cf.SetString (z"SRAMDirectory", sram_directory);
    cf.SetString (z"SaveStateDirectory", savestate_directory);
    cf.SetString (z"CheatDirectory", cheat_directory);
    cf.SetString (z"PatchDirectory", patch_directory);
    cf.SetString (z"ExportDirectory", export_directory);

#undef z
#define z "Window State::"
    cf.SetInt (z"MainWidth", window_width);
    cf.SetInt (z"MainHeight", window_height);
    cf.SetInt (z"PreferencesWidth", preferences_width);
    cf.SetInt (z"PreferencesHeight", preferences_height);
    cf.SetInt (z"ShaderParametersWidth", shader_parameters_width);
    cf.SetInt (z"ShaderParametersHeight", shader_parameters_height);
    cf.SetInt (z"CurrentDisplayTab", current_display_tab);
    outbool (cf, z"UIVisible", ui_visible);
    if (default_esc_behavior != ESC_TOGGLE_MENUBAR)
        outbool (cf, z"Fullscreen", 0);
    else
        outbool (cf, z"Fullscreen", fullscreen);

#undef z
#define z "Netplay::"
    outbool (cf, z"ActAsServer", netplay_is_server);
    outbool (cf, z"UseResetToSync", netplay_sync_reset);
    outbool (cf, z"SendROM", netplay_send_rom);
    cf.SetInt (z"DefaultPort", netplay_default_port);
    cf.SetInt (z"MaxFrameLoss", netplay_max_frame_loss);
    cf.SetInt (z"LastUsedPort", netplay_last_port);
    cf.SetString (z"LastUsedROM", netplay_last_rom);
    cf.SetString (z"LastUsedHost", netplay_last_host);

#undef z
#define z "Behavior::"
    outbool (cf, z"PauseEmulationWhenFocusLost", pause_emulation_on_switch);
    cf.SetInt (z"DefaultESCKeyBehavior", default_esc_behavior);
    outbool (cf, z"PreventScreensaver", prevent_screensaver);
    outbool (cf, z"UseModalDialogs", modal_dialogs);
    cf.SetInt (z"RewindBufferSize", rewind_buffer_size, "Amount of memory (in MB) to use for rewinding");
    cf.SetInt (z"RewindGranularity", rewind_granularity, "Only save rewind snapshots every N frames");
    cf.SetInt (z"CurrentSaveSlot", current_save_slot);

#undef z
#define z "Emulation::"
    outbool (cf, z"EmulateTransparency", Settings.Transparency);
    outbool (cf, z"DisplayFrameRate", Settings.DisplayFrameRate);
    outbool (cf, z"DisplayPressedKeys", Settings.DisplayPressedKeys);
    cf.SetInt (z"SpeedControlMethod", Settings.SkipFrames, "0: Time the frames to 50 or 60Hz, 1: Same, but skip frames if too slow, 2: Synchronize to the sound buffer, 3: Unlimited, except potentially by vsync");
    cf.SetInt (z"SaveSRAMEveryNSeconds", Settings.AutoSaveDelay);
    outbool (cf, z"BlockInvalidVRAMAccess", Settings.BlockInvalidVRAMAccessMaster);
    outbool (cf, z"AllowDPadContradictions", Settings.UpAndDown, "Allow the D-Pad to press both up + down at the same time, or left + right");

#undef z
#define z "Hacks::"
    cf.SetInt (z"SuperFXClockMultiplier", Settings.SuperFXClockMultiplier);
    cf.SetInt   (z"SoundInterpolationMethod", Settings.InterpolationMethod, "0: None, 1: Linear, 2: Gaussian (what the hardware uses), 3: Cubic, 4: Sinc");
    outbool (cf, z"RemoveSpriteLimit", Settings.MaxSpriteTilesPerLine == 34 ? 0 : 1);
    outbool (cf, z"OverclockCPU", Settings.OneClockCycle == 6 ? 0 : 1);

#undef z
#define z "Input::"
    controllers controller = CTL_NONE;
    int8 id[4];

    for (int i = 0; i < 2; i++)
    {
        const char *output_string;
        snprintf (buffer, PATH_MAX, z"ControllerPort%d", i);
        S9xGetController (i, &controller, &id[0], &id[1], &id[2], &id[3]);

        switch (controller)
        {
        case CTL_JOYPAD:
            output_string = "joypad";
            break;
        case CTL_MOUSE:
            output_string = "mouse";
            break;
        case CTL_SUPERSCOPE:
            output_string = "superscope";
            break;
        case CTL_MP5:
            output_string = "multitap";
            break;
        case CTL_JUSTIFIER:
            output_string = "justifier";
            break;
        default:
            output_string = "none";
        }

        cf.SetString (buffer, output_string);
    }

    cf.SetInt (z"JoystickThreshold", joystick_threshold);

#undef z

    for (int i = 0; i < NUM_JOYPADS; i++)
    {
        Binding *joypad = (Binding *) &pad[i];

        for (int j = 0; j < NUM_JOYPAD_LINKS; j++)
        {
            snprintf (key, PATH_MAX, "Joypad %d::%s", i, b_links[j].snes9x_name);
            joypad[j].to_string (buffer, false);
            cf.SetString (key, std::string (buffer));
        }
    }

    for (int i = NUM_JOYPAD_LINKS; b_links[i].snes9x_name; i++)
    {
        snprintf (key, PATH_MAX, "Shortcuts::%s", b_links[i].snes9x_name);
        shortcut[i - NUM_JOYPAD_LINKS].to_string (buffer, false);
        cf.SetString (key, std::string (buffer));
    }

    cf.SetNiceAlignment (true);
    cf.SetShowComments (true);
    cf.SaveTo (get_config_file_name ().c_str ());

    return 0;
}

int Snes9xConfig::load_config_file ()
{
    struct stat file_info;
    std::string path;
    ConfigFile  cf;
    char        key[PATH_MAX];

    load_defaults ();

    path = get_config_dir ();

    if (stat (path.c_str (), &file_info))
    {
        if (mkdir (path.c_str (), 0755))
        {
            fprintf (stderr,
                     _("Couldn't create config directory: %s\n"),
                     path.c_str ());
            return -1;
        }
    }
    else
    {
        chmod (path.c_str (), 0755);
    }

    path = get_config_file_name ();

    if (stat (path.c_str (), &file_info))
    {
        save_config_file ();
    }

    if (!cf.LoadFile (path.c_str ()))
        return -1;

    std::string none;
#define inbool(key, var) { if (cf.Exists (key)) var = cf.GetBool (key); }
#define inint(key, var) { if (cf.Exists(key)) var = cf.GetInt (key); }
#define infloat(key, var) { if (cf.Exists(key)) var = atof (cf.GetString (key, none).c_str()); }
#define instr(key, var) var = cf.GetString (key, none);

#undef z
#define z "Display::"
    inbool (z"FullscreenOnOpen", full_screen_on_open);
    inbool (z"ChangeDisplayResolution", change_display_resolution);
    inint  (z"VideoMode", xrr_index);
    inbool (z"ScaleToFit", scale_to_fit);
    inbool (z"MaintainAspectRatio", maintain_aspect_ratio);
    inint  (z"AspectRatio", aspect_ratio);
    inint  (z"SoftwareScaleFilter", scale_method);
    inint  (z"ScanlineFilterIntensity", scanline_filter_intensity);
    inbool (z"ShowOverscanArea", overscan);
    inint  (z"HiresEffect", hires_effect);
    inint  (z"ForceInvertedByteOrder", force_inverted_byte_order);
    inbool (z"Multithreading", multithreading);
    inint  (z"NumberOfThreads", num_threads);
    inint  (z"HardwareAcceleration", hw_accel);
    inbool (z"BilinearFilter", Settings.BilinearFilter);
    inint  (z"SplashBackground", splash_image);

#undef z
#define z "NTSC::"
    infloat (z"Hue", ntsc_setup.hue);
    infloat (z"Saturation", ntsc_setup.saturation);
    infloat (z"Contrast", ntsc_setup.contrast);
    infloat (z"Brightness", ntsc_setup.brightness);
    infloat (z"Sharpness", ntsc_setup.sharpness);
    infloat (z"Artifacts", ntsc_setup.artifacts);
    infloat (z"Gamma", ntsc_setup.gamma);
    infloat (z"Bleed", ntsc_setup.bleed);
    infloat (z"Fringing", ntsc_setup.fringing);
    infloat (z"Resolution", ntsc_setup.resolution);
    inbool  (z"MergeFields", ntsc_setup.merge_fields);
    inint   (z"ScanlineIntensity", ntsc_scanline_intensity);

#ifdef USE_OPENGL
#undef z
#define z "OpenGL::"
    inbool (z"VSync", sync_to_vblank);
    inbool (z"glFinish", use_glfinish);
    inbool (z"SyncControl", use_sync_control);
    inbool (z"UsePixelBufferObjects", use_pbos);
    inint  (z"PixelBufferObjectBitDepth", pbo_format);
    inbool (z"UseNonPowerOfTwoTextures", npot_textures);
    inbool (z"EnableCustomShaders", use_shaders);
    instr  (z"ShaderFile", shader_filename);
#endif

#undef z
#define z "Sound::"
    inbool (z"MuteSound", mute_sound);
    inbool (z"MuteSoundDuringTurbo", mute_sound_turbo);
    inint  (z"BufferSize", sound_buffer_size);
    inint  (z"Driver", sound_driver);
    inint  (z"InputRate", sound_input_rate);
    inbool (z"DynamicRateControl", Settings.DynamicRateControl);
    inint  (z"DynamicRateControlLimit", Settings.DynamicRateLimit);
    inbool (z"AutomaticInputRate", auto_input_rate);
    inint  (z"PlaybackRate", gui_config->sound_playback_rate);

#undef z
#define z "Files::"
    instr (z"LastDirectory", last_directory);
    instr (z"LastShaderDirectory", last_shader_directory);
    instr (z"SRAMDirectory", sram_directory);
    instr (z"SaveStateDirectory", savestate_directory);
    instr (z"CheatDirectory", cheat_directory);
    instr (z"PatchDirectory", patch_directory);
    instr (z"ExportDirectory", export_directory);

#undef z
#define z "Window State::"

    inint (z"MainWidth", window_width);
    inint (z"MainHeight", window_height);
    inint (z"PreferencesWidth", preferences_width);
    inint (z"PreferencesHeight", preferences_height);
    inint (z"ShaderParametersWidth", shader_parameters_width);
    inint (z"ShaderParametersHeight", shader_parameters_height);
    inint (z"CurrentDisplayTab", current_display_tab);
    inbool (z"UIVisible", ui_visible);
    inbool (z"Fullscreen", fullscreen);

#undef z
#define z "Netplay::"
    inbool (z"ActAsServer", netplay_is_server);
    inbool (z"UseResetToSync", netplay_sync_reset);
    inbool (z"SendROM", netplay_send_rom);
    inint (z"DefaultPort", netplay_default_port);
    inint (z"MaxFrameLoss", netplay_max_frame_loss);
    inint (z"LastUsedPort", netplay_last_port);
    instr (z"LastUsedROM", netplay_last_rom);
    instr (z"LastUsedHost", netplay_last_host);

#undef z
#define z "Behavior::"
    inbool (z"PauseEmulationWhenFocusLost", pause_emulation_on_switch);
    inint (z"DefaultESCKeyBehavior", default_esc_behavior);
    inbool (z"PreventScreensaver", prevent_screensaver);
    inbool (z"UseModalDialogs", modal_dialogs);
    inint (z"RewindBufferSize", rewind_buffer_size);
    inint (z"RewindGranularity", rewind_granularity);
    inint (z"CurrentSaveSlot", current_save_slot);

#undef z
#define z "Emulation::"
    inbool (z"EmulateTransparency", Settings.Transparency);
    inbool (z"DisplayFrameRate", Settings.DisplayFrameRate);
    inbool (z"DisplayPressedKeys", Settings.DisplayPressedKeys);
    inint (z"SpeedControlMethod", Settings.SkipFrames);
    inint (z"SaveSRAMEveryNSeconds", Settings.AutoSaveDelay);
    inbool (z"BlockInvalidVRAMAccess", Settings.BlockInvalidVRAMAccessMaster);
    inbool (z"AllowDPadContradictions", Settings.UpAndDown);

#undef z
#define z "Hacks::"
    inint  (z"SuperFXClockMultiplier", Settings.SuperFXClockMultiplier);
    inint  (z"SoundInterpolationMethod", Settings.InterpolationMethod);

    bool RemoveSpriteLimit = false;
    inbool (z"RemoveSpriteLimit", RemoveSpriteLimit);
    bool OverclockCPU = false;
    inbool (z"OverclockCPU", OverclockCPU);

#undef z
#define z "Input::"

    for (int i = 0; i < 2; i++)
    {
        snprintf (key, PATH_MAX, z"ControllerPort%d", i);
        std::string tmp = cf.GetString (key, "");

        if (tmp.find ("joypad") != std::string::npos)
            S9xSetController (i, CTL_JOYPAD, i, 0, 0, 0);
        else if (tmp.find ("multitap") != std::string::npos)
            S9xSetController (i, CTL_MP5, i, i + 1, i + 2, i + 3);
        else if (tmp.find ("superscope") != std::string::npos)
            S9xSetController (i, CTL_SUPERSCOPE, 0, 0, 0, 0);
        else if (tmp.find ("mouse") != std::string::npos)
            S9xSetController (i, CTL_MOUSE, i, 0, 0, 0);
        else if (tmp.find ("none") != std::string::npos)
            S9xSetController (i, CTL_NONE, 0, 0, 0, 0);
    }

    inint (z"JoystickThreshold", joystick_threshold);

#undef z

    std::string buffer;

    for (int i = 0; i < NUM_JOYPADS; i++)
    {
        Binding *joypad = (Binding *) &pad[i];

        for (int j = 0; j < NUM_JOYPAD_LINKS; j++)
        {
            snprintf (key, PATH_MAX, "Joypad %d::%s", i, b_links[j].snes9x_name);
            instr (key, buffer);
            joypad[j] = Binding (buffer.c_str ());
        }
    }

    for (int i = NUM_JOYPAD_LINKS; b_links[i].snes9x_name; i++)
    {
        snprintf (key, PATH_MAX, "Shortcuts::%s", b_links[i].snes9x_name);
        instr (key, buffer);
        shortcut[i - NUM_JOYPAD_LINKS] = Binding (buffer.c_str ());
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

#ifdef USE_OPENGL
    if (pbo_format != 32)
        pbo_format = 16;
#endif

    if (Settings.SkipFrames == THROTTLE_SOUND_SYNC)
        Settings.SoundSync = true;
    else
        Settings.SoundSync = false;

    hires_effect = CLAMP (hires_effect, 0, 2);
    Settings.DynamicRateLimit = CLAMP (Settings.DynamicRateLimit, 1, 1000);
    Settings.SuperFXClockMultiplier = CLAMP (Settings.SuperFXClockMultiplier, 50, 400);
    ntsc_scanline_intensity = MIN (ntsc_scanline_intensity, 4);
    scanline_filter_intensity = MIN (scanline_filter_intensity, 3);

    return 0;
}

void Snes9xConfig::rebind_keys()
{
    s9xcommand_t cmd;
    std::string string;

    S9xUnmapAllControls();

    for (int joypad_i = 0; joypad_i < NUM_JOYPADS; joypad_i++)
    {
        Binding *bin = (Binding *)&pad[joypad_i];

        for (int button_i = 0; button_i < NUM_JOYPAD_LINKS; button_i++)
        {
            int dupe;
            for (dupe = button_i + 1; dupe < NUM_JOYPAD_LINKS; dupe++)
            {
                if (bin[button_i].matches(bin[dupe]) && bin[button_i].hex() != 0)
                    break;
            }
            if (dupe < NUM_JOYPAD_LINKS || bin[button_i].hex() == 0)
                continue;

            string = "Joypad" + std::to_string((joypad_i % 5) + 1) + " ";
            string += b_links[button_i].snes9x_name;

            bool ismulti = false;
            for (dupe = button_i - 1; dupe > 0; dupe--)
            {
                if (bin[button_i].matches(bin[dupe]))
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
