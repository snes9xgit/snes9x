#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "gtk_config.h"
#include "gtk_s9x.h"
#include "gtk_sound.h"
#include "gtk_display.h"

static int
directory_exists (const char *directory)
{
    DIR *dir;

    dir = opendir (directory);

    if (dir)
    {
        closedir (dir);
        return TRUE;
    }

    return FALSE;
}

char *
get_config_dir (void)
{
    char *home_dir = NULL,
         *classic_config_dir = NULL,
         *xdg_config_dir = NULL,
         *xdg_snes9x_dir = NULL;

    /* Find config directory */
    home_dir = getenv ("HOME");
    xdg_config_dir = getenv ("XDG_CONFIG_HOME");

    if (!home_dir && !xdg_config_dir)
    {
        return strdup (".snes9x");
    }

    if (!xdg_config_dir)
    {
        xdg_snes9x_dir = (char *) malloc (strlen (home_dir) + 16);
        sprintf (xdg_snes9x_dir, "%s/.config/snes9x", home_dir);
    }
    else
    {
        xdg_snes9x_dir = (char *) malloc (strlen (xdg_config_dir) + 9);
        sprintf (xdg_snes9x_dir, "%s/snes9x", xdg_config_dir);
    }

    classic_config_dir =  (char *) malloc (strlen (home_dir) + 9);
    sprintf (classic_config_dir, "%s/.snes9x", home_dir);

    char *config_dir;

    if (directory_exists (classic_config_dir) && !directory_exists(xdg_snes9x_dir))
    {
        free (xdg_snes9x_dir);
        config_dir =  classic_config_dir;
    }
    else
    {
        free (classic_config_dir);
        config_dir = xdg_snes9x_dir;
    }

    return config_dir;
}

char *
get_config_file_name (void)
{
    char *filename;

    filename = get_config_dir ();

    filename = (char *) realloc (filename, strlen (filename) + 12);
    strcat (filename, "/snes9x.xml");

    return filename;
}

static void
xml_out_string (xmlTextWriterPtr xml, const char *name, const char *value)
{
    xmlTextWriterStartElement (xml, BAD_CAST ("option"));
    xmlTextWriterWriteAttribute (xml, BAD_CAST ("name"), BAD_CAST (name));
    xmlTextWriterWriteAttribute (xml, BAD_CAST ("value"), BAD_CAST (value));
    xmlTextWriterEndElement (xml);
}

static void
xml_out_int (xmlTextWriterPtr xml, const char *name, int value)
{
    char string[1024];
    snprintf (string, 1024, "%d", value);

    xmlTextWriterStartElement (xml, BAD_CAST "option");
    xmlTextWriterWriteAttribute (xml, BAD_CAST "name", BAD_CAST name);
    xmlTextWriterWriteAttribute (xml, BAD_CAST "value", BAD_CAST string);
    xmlTextWriterEndElement (xml);
}

static void
xml_out_float (xmlTextWriterPtr xml, const char *name, float value)
{
    char string[1024];
    snprintf (string, 1024, "%f", value);

    xmlTextWriterStartElement (xml, BAD_CAST "option");
    xmlTextWriterWriteAttribute (xml, BAD_CAST "name", BAD_CAST name);
    xmlTextWriterWriteAttribute (xml, BAD_CAST "value", BAD_CAST string);
    xmlTextWriterEndElement (xml);
}

static void
xml_out_binding (xmlTextWriterPtr xml, const char *name, unsigned int value)
{
    char string[1024];
    snprintf (string, 1024, "%u", value);

    xmlTextWriterStartElement (xml, BAD_CAST ("binding"));
    xmlTextWriterWriteAttribute (xml, BAD_CAST ("name"), BAD_CAST (name));
    xmlTextWriterWriteAttribute (xml, BAD_CAST ("binding"), BAD_CAST (string));
    xmlTextWriterEndElement (xml);
}

void
S9xParsePortConfig (ConfigFile &conf, int pass)
{
    return;
}

Snes9xConfig::Snes9xConfig (void)
{
#ifdef USE_JOYSTICK
    joystick = NULL;
    joystick_threshold = 40;
#endif

#ifdef USE_OPENGL
    opengl_activated = FALSE;
#endif

    return;
}

int
Snes9xConfig::load_defaults (void)
{
    full_screen_on_open = 0;
    change_display_resolution = 0;
    xrr_index = 0;
    xrr_width = 0;
    xrr_height = 0;
    scale_to_fit = 1;
    maintain_aspect_ratio = 0;
    aspect_ratio = 0;
    scale_method = 0;
    overscan = 0;
    save_sram_after_secs = 0;
    rom_loaded = 0;
    multithreading = 0;
    hw_accel = 0;
    allow_opengl = 0;
    allow_xv = 0;
    allow_xrandr = 0;
    force_inverted_byte_order = FALSE;
    hires_effect = HIRES_NORMAL;
    pause_emulation_on_switch = 0;
    num_threads = 2;
    mute_sound = FALSE;
    mute_sound_turbo = FALSE;
    fullscreen = FALSE;
    ui_visible = TRUE;
    statusbar_visible = FALSE;
    default_esc_behavior = 1;
    prevent_screensaver = FALSE;
    sound_driver = 0;
    sound_buffer_size = 32;
    sound_playback_rate = 5;
    sound_input_rate = 31950;
    last_directory[0] = '\0';
    window_width = -1;
    window_height = -1;
    preferences_width = -1;
    preferences_height = -1;
    sram_directory[0] = '\0';
    export_directory[0] = '\0';
    savestate_directory[0] = '\0';
    cheat_directory[0] = '\0';
    patch_directory[0] = '\0';
    screensaver_needs_reset = FALSE;
    ntsc_setup = snes_ntsc_composite;
    ntsc_scanline_intensity = 1;
    scanline_filter_intensity = 0;
    bilinear_filter = 0;
    netplay_activated = FALSE;
    netplay_server_up = FALSE;
    netplay_is_server = FALSE;
    netplay_sync_reset = TRUE;
    netplay_send_rom = FALSE;
    netplay_default_port = 6096;
    netplay_max_frame_loss = 10;
    netplay_last_rom [0] = '\0';
    netplay_last_host [0] = '\0';
    netplay_last_port = 6096;
    modal_dialogs = 1;

    rewind_granularity = 5;
    rewind_buffer_size = 0;

#ifdef USE_OPENGL
    sync_to_vblank = 1;
    use_pbos = 1;
    pbo_format = 0;
    npot_textures = FALSE;
    use_shaders = 0;
    fragment_shader[0] = '\0';
    vertex_shader[0] = '\0';
    sync_every_frame = FALSE;
#endif

    /* Snes9X Variables */
    Settings.MouseMaster = TRUE;
    Settings.SuperScopeMaster = TRUE;
    Settings.JustifierMaster = TRUE;
    Settings.MultiPlayer5Master = TRUE;
    Settings.UpAndDown = FALSE;
    Settings.AutoSaveDelay = 0;
    Settings.SkipFrames = AUTO_FRAMERATE;
    Settings.Transparency = TRUE;
    Settings.DisplayFrameRate = FALSE;
    Settings.SixteenBitSound = TRUE;
    Settings.Stereo = TRUE;
    Settings.ReverseStereo = FALSE;
    Settings.SoundPlaybackRate = 32000;
    Settings.StopEmulation = TRUE;
    Settings.FrameTimeNTSC = 16667;
    Settings.FrameTimePAL = 20000;
    Settings.SupportHiRes = true;
    Settings.FrameTime = Settings.FrameTimeNTSC;
    Settings.BlockInvalidVRAMAccessMaster = TRUE;
    Settings.SoundSync = 1;
    Settings.HDMATimingHack = 100;
    Settings.ApplyCheats = 1;

#ifdef NETPLAY_SUPPORT
    Settings.NetPlay = FALSE;
    NetPlay.Paused = FALSE;
    NetPlay.MaxFrameSkip = 10;
#endif

    memset (pad, 0, sizeof (JoypadBinding) * NUM_JOYPADS);
    memset (shortcut, 0, sizeof (Binding) * NUM_EMU_LINKS);

    return 0;
}

#ifdef USE_JOYSTICK

void
Snes9xConfig::joystick_register_centers (void)
{
    for (int i = 0; joystick[i] != NULL; i++)
        joystick[i]->register_centers ();

    return;
}

void
Snes9xConfig::flush_joysticks (void)
{
    int i;

    for (i = 0; joystick[i] != NULL; i++)
        joystick[i]->flush ();

    return;
}

void
Snes9xConfig::set_joystick_mode (int mode)
{
    int i;
    for (i = 0; joystick[i] != NULL; i++)
        joystick[i]->mode = mode;

    return;
}

#endif

int
Snes9xConfig::save_config_file (void)
{
    xmlTextWriterPtr xml;
    char             *filename;
    char             buffer[256];

    filename = get_config_file_name ();
    xml = xmlNewTextWriterFilename (filename, 0);
    free (filename);

    xmlTextWriterSetIndent (xml, 1);

    xmlTextWriterStartDocument (xml, NULL, NULL, NULL);
    xmlTextWriterStartElement (xml, BAD_CAST "snes9x");
    xmlTextWriterStartElement (xml, BAD_CAST "preferences");

    xml_out_int (xml, "full_screen_on_open", full_screen_on_open);
    xml_out_int (xml, "change_display_resolution", change_display_resolution);
    xml_out_int (xml, "video_mode", xrr_index);
    xml_out_int (xml, "video_mode_width", xrr_width);
    xml_out_int (xml, "video_mode_height", xrr_height);
    xml_out_int (xml, "scale_to_fit", scale_to_fit);
    xml_out_int (xml, "maintain_aspect_ratio", maintain_aspect_ratio);
    xml_out_int (xml, "aspect_ratio", aspect_ratio);
    xml_out_int (xml, "scale_method", scale_method);
    xml_out_int (xml, "overscan", overscan);
    xml_out_int (xml, "hires_effect", hires_effect);
    xml_out_int (xml, "force_inverted_byte_order", force_inverted_byte_order);
    xml_out_int (xml, "multithreading", multithreading);
    xml_out_string (xml, "last_directory", last_directory);
    xml_out_string (xml, "sram_directory", sram_directory);
    xml_out_string (xml, "savestate_directory", savestate_directory);
    xml_out_string (xml, "cheat_directory", cheat_directory);
    xml_out_string (xml, "patch_directory", patch_directory);
    xml_out_string (xml, "export_directory", export_directory);
    xml_out_int (xml, "window_width", window_width);
    xml_out_int (xml, "window_height", window_height);
    xml_out_int (xml, "preferences_width", preferences_width);
    xml_out_int (xml, "preferences_height", preferences_height);
    xml_out_int (xml, "pause_emulation_on_switch", pause_emulation_on_switch);
    xml_out_int (xml, "num_threads", num_threads);
    xml_out_int (xml, "default_esc_behavior", default_esc_behavior);
    xml_out_int (xml, "prevent_screensaver", prevent_screensaver);
    if (default_esc_behavior != ESC_TOGGLE_MENUBAR)
        xml_out_int (xml, "fullscreen", 0);
    else
        xml_out_int (xml, "fullscreen", fullscreen);
    xml_out_int (xml, "ui_visible", ui_visible);
    xml_out_int (xml, "statusbar_visible", statusbar_visible);
    xml_out_int (xml, "modal_dialogs", modal_dialogs);

    xml_out_float (xml, "ntsc_hue", ntsc_setup.hue);
    xml_out_float (xml, "ntsc_saturation", ntsc_setup.saturation);
    xml_out_float (xml, "ntsc_contrast", ntsc_setup.contrast);
    xml_out_float (xml, "ntsc_brightness", ntsc_setup.brightness);
    xml_out_float (xml, "ntsc_sharpness", ntsc_setup.sharpness);
    xml_out_float (xml, "ntsc_artifacts", ntsc_setup.artifacts);
    xml_out_float (xml, "ntsc_gamma", ntsc_setup.gamma);
    xml_out_float (xml, "ntsc_bleed", ntsc_setup.bleed);
    xml_out_float (xml, "ntsc_fringing", ntsc_setup.fringing);
    xml_out_float (xml, "ntsc_resolution", ntsc_setup.resolution);
    xml_out_int (xml, "ntsc_merge_fields", ntsc_setup.merge_fields);
    xml_out_int (xml, "ntsc_scanline_intensity", ntsc_scanline_intensity);
    xml_out_int (xml, "scanline_filter_intensity", scanline_filter_intensity);
    xml_out_int (xml, "hw_accel", hw_accel);
    xml_out_int (xml, "bilinear_filter", bilinear_filter);

    xml_out_int (xml, "rewind_buffer_size", rewind_buffer_size);
    xml_out_int (xml, "rewind_granularity", rewind_granularity);

#ifdef USE_OPENGL
    xml_out_int (xml, "sync_to_vblank", sync_to_vblank);
    xml_out_int (xml, "sync_every_frame", sync_every_frame);
    xml_out_int (xml, "use_pbos", use_pbos);
    xml_out_int (xml, "pbo_format", pbo_format);
    xml_out_int (xml, "npot_textures", npot_textures);
    xml_out_int (xml, "use_shaders", use_shaders);
    xml_out_string (xml, "fragment_shader", fragment_shader);
#endif

#ifdef USE_JOYSTICK
    xml_out_int (xml, "joystick_threshold", joystick_threshold);
#endif

    xml_out_int (xml, "netplay_is_server", netplay_is_server);
    xml_out_int (xml, "netplay_sync_reset", netplay_sync_reset);
    xml_out_int (xml, "netplay_send_rom", netplay_send_rom);
    xml_out_int (xml, "netplay_default_port", netplay_default_port);
    xml_out_int (xml, "netplay_max_frame_loss", netplay_max_frame_loss);
    xml_out_int (xml, "netplay_last_port", netplay_last_port);
    xml_out_string (xml, "netplay_last_rom", netplay_last_rom);
    xml_out_string (xml, "netplay_last_host", netplay_last_host);

    xml_out_int (xml, "mute_sound", mute_sound);
    xml_out_int (xml, "mute_sound_turbo", mute_sound_turbo);
    xml_out_int (xml, "sound_buffer_size", sound_buffer_size);
    xml_out_int (xml, "sound_driver", sound_driver);
    xml_out_int (xml, "sound_input_rate", sound_input_rate);
    xml_out_int (xml, "sound_sync", Settings.SoundSync);

    /* Snes9X core-stored variables */
    xml_out_int (xml, "transparency", Settings.Transparency);
    xml_out_int (xml, "show_frame_rate", Settings.DisplayFrameRate);
    xml_out_int (xml, "frameskip", Settings.SkipFrames);
    xml_out_int (xml, "save_sram_after_secs", Settings.AutoSaveDelay);
    xml_out_int (xml, "16bit_sound", Settings.SixteenBitSound);
    xml_out_int (xml, "stereo", Settings.Stereo);
    xml_out_int (xml, "reverse_stereo", Settings.ReverseStereo);
    xml_out_int (xml, "playback_rate", gui_config->sound_playback_rate);
    xml_out_int (xml, "block_invalid_vram_access", Settings.BlockInvalidVRAMAccessMaster);
    xml_out_int (xml, "upanddown", Settings.UpAndDown);

    xmlTextWriterEndElement (xml); /* preferences */

    xmlTextWriterStartElement (xml, BAD_CAST "controls");

#ifdef USE_JOYSTICK
    for (int i = 0; joystick[i]; i++)
    {
        xmlTextWriterStartElement (xml, BAD_CAST "calibration");
        snprintf (buffer, 256, "%d", i);
        xmlTextWriterWriteAttribute (xml, BAD_CAST "joystick", BAD_CAST buffer);

        for (int j = 0; j < joystick[i]->num_axes; j++)
        {
            xmlTextWriterStartElement (xml, BAD_CAST "axis");

            snprintf (buffer, 256, "%d", j);
            xmlTextWriterWriteAttribute (xml, BAD_CAST "number", BAD_CAST buffer);

            snprintf (buffer, 256, "%d", joystick[i]->calibration[j].min);
            xmlTextWriterWriteAttribute (xml, BAD_CAST "min", BAD_CAST buffer);

            snprintf (buffer, 256, "%d", joystick[i]->calibration[j].max);
            xmlTextWriterWriteAttribute (xml, BAD_CAST "max", BAD_CAST buffer);

            snprintf (buffer, 256, "%d", joystick[i]->calibration[j].center);
            xmlTextWriterWriteAttribute (xml, BAD_CAST "center", BAD_CAST buffer);

            xmlTextWriterEndElement (xml); /* axis */
        }

        xmlTextWriterEndElement (xml); /* Calibration */
    }
#endif

    for (int i = 0; i < NUM_JOYPADS; i++)
    {
        Binding *joypad = (Binding *) &pad[i];

        xmlTextWriterStartElement (xml, BAD_CAST "joypad");
        snprintf (buffer, 256, "%d", i);
        xmlTextWriterWriteAttribute (xml, BAD_CAST "number", BAD_CAST buffer);

        for (int j = 0; j < NUM_JOYPAD_LINKS; j++)
        {
            xml_out_binding (xml, b_links[j].snes9x_name, joypad[j].hex ());
        }

        xmlTextWriterEndElement (xml); /* joypad */
    }

    for (int i = NUM_JOYPAD_LINKS; b_links[i].snes9x_name; i++)
    {
        xml_out_binding (xml,
                         b_links[i].snes9x_name,
                         shortcut[i - NUM_JOYPAD_LINKS].hex ());
    }

    xmlTextWriterEndElement (xml); /* controls */
    xmlTextWriterEndElement (xml); /* snes9x */
    xmlTextWriterEndDocument (xml);

    xmlFreeTextWriter (xml);

    return 0;
}

int
Snes9xConfig::set_option (const char *name, const char *value)
{
    if (!strcasecmp (name, "full_screen_on_open"))
    {
        full_screen_on_open = atoi (value);
    }
    else if (!strcasecmp (name, "change_display_resolution"))
    {
        change_display_resolution = atoi (value);
    }
    else if (!strcasecmp (name, "video_mode"))
    {
    }
    else if (!strcasecmp (name, "video_mode_width"))
    {
        xrr_width = atoi (value);
    }
    else if (!strcasecmp (name, "video_mode_height"))
    {
        xrr_height = atoi (value);
    }
    else if (!strcasecmp (name, "scale_to_fit"))
    {
        scale_to_fit = atoi (value);
    }
    else if (!strcasecmp (name, "maintain_aspect_ratio"))
    {
        maintain_aspect_ratio = atoi (value);
    }
    else if (!strcasecmp (name, "aspect_ratio"))
    {
        aspect_ratio = atoi (value);
    }
    else if (!strcasecmp (name, "force_hires"))
    {
        /* Deprecated */
    }
    else if (!strcasecmp (name, "hires_effect"))
    {
        hires_effect = atoi (value);
        hires_effect = CLAMP (hires_effect, 0, 2);
    }
    else if (!strcasecmp (name, "scale_method"))
    {
        scale_method = atoi (value);
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
    }
    else if (!strcasecmp (name, "multithreading"))
    {
        multithreading = atoi (value);
    }
    else if (!strcasecmp (name, "hw_accel"))
    {
        hw_accel = atoi (value);
    }
    else if (!strcasecmp (name, "use_opengl"))
    {
#ifdef USE_OPENGL
        if (atoi (value) == 1)
            hw_accel = HWA_OPENGL;
#endif
    }
    else if (!strcasecmp (name, "bilinear_filter"))
    {
        bilinear_filter = atoi (value);
    }
    else if (!strcasecmp (name, "sync_to_vblank"))
    {
#ifdef USE_OPENGL
        sync_to_vblank = atoi (value);
#endif
    }
    else if (!strcasecmp (name, "sync_every_frame"))
    {
#ifdef USE_OPENGL
        sync_every_frame = atoi (value);
#endif
    }
    else if (!strcasecmp (name, "use_pbos"))
    {
#ifdef USE_OPENGL
        use_pbos = atoi (value);
#endif
    }
    else if (!strcasecmp (name, "pbo_format"))
    {
#ifdef USE_OPENGL
        pbo_format = atoi (value);
        pbo_format = CLAMP (pbo_format, 0, 2);
#endif
    }
    else if (!strcasecmp (name, "npot_textures"))
    {
#ifdef USE_OPENGL
        npot_textures = atoi (value);
#endif
    }
    else if (!strcasecmp (name, "use_shaders"))
    {
#ifdef USE_OPENGL
        use_shaders = atoi (value);
#endif
    }
    else if (!strcasecmp (name, "fragment_shader"))
    {
#ifdef USE_OPENGL
        strncpy (fragment_shader, value, PATH_MAX);
#endif
    }
    else if (!strcasecmp (name, "vertex_shader"))
    {
#ifdef USE_OPENGL
        /* Deprecated */
#endif
    }
    else if (!strcasecmp (name, "joystick_threshold"))
    {
#ifdef USE_JOYSTICK
        joystick_threshold = atoi (value);
#endif
    }
    else if (!strcasecmp (name, "data_location"))
    {
        /* Deprecated */
    }
    else if (!strcasecmp (name, "save_sram_after_secs"))
    {
        Settings.AutoSaveDelay = atoi (value);
    }
    else if (!strcasecmp (name, "pause_emulation_on_switch"))
    {
        pause_emulation_on_switch = atoi (value);
    }
    else if (!strcasecmp (name, "transparency"))
    {
        Settings.Transparency = atoi (value);
    }
    else if (!strcasecmp (name, "show_frame_rate"))
    {
        Settings.DisplayFrameRate = atoi (value);
    }
    else if (!strcasecmp (name, "frameskip"))
    {
        Settings.SkipFrames = atoi (value);
    }
    else if (!strcasecmp (name, "sound_emulation"))
    {
        mute_sound = !(atoi (value));
    }
    else if (!strcasecmp (name, "mute_sound"))
    {
        mute_sound = atoi (value);
    }
    else if (!strcasecmp (name, "mute_sound_turbo"))
    {
        mute_sound_turbo = atoi (value);
    }
    else if (!strcasecmp (name, "16bit_sound"))
    {
        Settings.SixteenBitSound = atoi (value);
    }
    else if (!strcasecmp (name, "stereo"))
    {
        Settings.Stereo = atoi (value);
    }
    else if (!strcasecmp (name, "reverse_stereo"))
    {
        Settings.ReverseStereo = atoi (value);
    }
    else if (!strcasecmp (name, "gaussian_interpolation"))
    {
    }
    else if (!strcasecmp (name, "envelope_reading"))
    {
    }
    else if (!strcasecmp (name, "sound_echo"))
    {
    }
    else if (!strcasecmp (name, "master_volume"))
    {
    }
    else if (!strcasecmp (name, "playback_rate"))
    {
        sound_playback_rate = atoi (value);
    }
    else if (!strcasecmp (name, "sound_decoder"))
    {
    }
    else if (!strcasecmp (name, "hdma"))
    {
        /* Deprecated */
    }
    else if (!strcasecmp (name, "speedhacks"))
    {
    }
    else if (!strcasecmp (name, "overscan"))
    {
        overscan = atoi (value);
    }
    else if (!strcasecmp (name, "last_directory"))
    {
        strncpy (last_directory, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "custom_sram_directory"))
    {
        strncpy (sram_directory, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "sram_directory"))
    {
        strncpy (sram_directory, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "savestate_directory"))
    {
        strncpy (savestate_directory, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "cheat_directory"))
    {
        strncpy (cheat_directory, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "patch_directory"))
    {
        strncpy (patch_directory, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "export_directory"))
    {
        strncpy (export_directory, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "modal_dialogs"))
    {
        modal_dialogs = atoi (value) ? 1 : 0;
    }
    else if (!strcasecmp (name, "window_width"))
    {
        window_width = atoi (value);
    }
    else if (!strcasecmp (name, "window_height"))
    {
        window_height = atoi (value);
    }
    else if (!strcasecmp (name, "preferences_width"))
    {
        preferences_width = atoi (value);
    }
    else if (!strcasecmp (name, "preferences_height"))
    {
        preferences_height = atoi (value);
    }
    else if (!strcasecmp (name, "ntsc_format"))
    {
        /* Deprecated */
    }
    else if (!strcasecmp (name, "ntsc_hue"))
    {
        ntsc_setup.hue = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_saturation"))
    {
        ntsc_setup.saturation = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_contrast"))
    {
        ntsc_setup.contrast = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_brightness"))
    {
        ntsc_setup.brightness = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_sharpness"))
    {
        ntsc_setup.sharpness = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_warping"))
    {
        /* Deprecated */
    }
    else if (!strcasecmp (name, "ntsc_artifacts"))
    {
        ntsc_setup.artifacts = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_gamma"))
    {
        ntsc_setup.gamma = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_fringing"))
    {
        ntsc_setup.fringing = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_bleed"))
    {
        ntsc_setup.bleed = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_resolution"))
    {
        ntsc_setup.resolution = (float) atof (value);
    }
    else if (!strcasecmp (name, "ntsc_merge_fields"))
    {
        ntsc_setup.merge_fields = atoi (value);
    }
    else if (!strcasecmp (name, "ntsc_scanline_intensity"))
    {
        ntsc_scanline_intensity = CLAMP (atoi (value), 0, 4);
    }
    else if (!strcasecmp (name, "scanline_filter_intensity"))
    {
        scanline_filter_intensity = CLAMP (atoi (value), 0, 3);
    }
    else if (!strcasecmp (name, "num_threads"))
    {
        num_threads = atoi (value);
    }
    else if (!strcasecmp (name, "netplay_is_server"))
    {
        netplay_is_server = atoi (value);
    }
    else if (!strcasecmp (name, "netplay_sync_reset"))
    {
        netplay_sync_reset = atoi (value);
    }
    else if (!strcasecmp (name, "netplay_send_rom"))
    {
        netplay_send_rom = atoi (value);
    }
    else if (!strcasecmp (name, "netplay_default_port"))
    {
        netplay_default_port = atoi (value);
    }
    else if (!strcasecmp (name, "netplay_max_frame_loss"))
    {
        netplay_max_frame_loss = atoi (value);
    }
    else if (!strcasecmp (name, "netplay_last_port"))
    {
        netplay_last_port = atoi (value);
    }
    else if (!strcasecmp (name, "netplay_last_rom"))
    {
        strncpy (netplay_last_rom, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "netplay_last_host"))
    {
        strncpy (netplay_last_host, value, PATH_MAX);
    }
    else if (!strcasecmp (name, "fullscreen"))
    {
        fullscreen = atoi (value);
    }
    else if (!strcasecmp (name, "ui_visible"))
    {
        ui_visible = atoi (value);
    }
    else if (!strcasecmp (name, "statusbar_visible"))
    {
        statusbar_visible = atoi (value);
    }
    else if (!strcasecmp (name, "default_esc_behavior"))
    {
        default_esc_behavior = atoi (value);
    }
    else if (!strcasecmp (name, "sound_buffer_size"))
    {
        sound_buffer_size = atoi (value);
    }
    else if (!strcasecmp (name, "sound_input_rate"))
    {
        sound_input_rate = atoi (value);
    }
    else if (!strcasecmp (name, "sound_driver"))
    {
        sound_driver = atoi (value);
    }
    else if (!strcasecmp (name, "prevent_screensaver"))
    {
        prevent_screensaver = atoi (value);
    }
    else if (!strcasecmp (name, "force_inverted_byte_order"))
    {
        force_inverted_byte_order = atoi (value);
    }
    else if (!strcasecmp (name, "block_invalid_vram_access"))
    {
        Settings.BlockInvalidVRAMAccessMaster = CLAMP (atoi (value), 0, 1);
    }
    else if (!strcasecmp (name, "upanddown"))
    {
        Settings.UpAndDown = CLAMP (atoi (value), 0, 1);
    }
    else if (!strcasecmp (name, "sound_sync"))
    {
        Settings.SoundSync = atoi (value) ? 1 : 0;
    }
    else if (!strcasecmp (name, "rewind_buffer_size"))
    {
        rewind_buffer_size = CLAMP (atoi (value), 0, 2000);
    }
    else if (!strcasecmp (name, "rewind_granularity"))
    {
        rewind_granularity = CLAMP (atoi (value), 0, 600);
    }
    else
    {
        fprintf (stderr, _("bad option name: %s\n"), name);
        return 1;
    }

    return 0;
}

int
Snes9xConfig::parse_option (xmlNodePtr node)
{
    xmlAttrPtr attr = NULL;
    char       *name, *value;

    /* Find name string */
    for (attr = node->properties; attr; attr = attr->next)
    {
        if (!xmlStrcasecmp (attr->name, BAD_CAST "name"))
        {
            name = (char *) attr->children->content;
            break;
        }
    }

    if (!attr)
        return 1;

    /* Find value string */
    for (attr = node->properties; attr; attr = attr->next)
    {
        if (!xmlStrcasecmp (attr->name, BAD_CAST "value"))
        {
            value = (char *) attr->children->content;
            break;
        }
    }

    if (!attr)
        return 1;

    return set_option (name, value);
}

int
Snes9xConfig::parse_binding (xmlNodePtr node, int joypad_number)
{
    char    *name = NULL;
    char    *type = NULL;
    Binding b;

    for (xmlAttrPtr attr = node->properties; attr; attr = attr->next)
    {
        if (!xmlStrcasecmp (attr->name, BAD_CAST "name"))
            name = (char *) attr->children->content;

        else if (!xmlStrcasecmp (attr->name, BAD_CAST "binding"))
            type = (char *) attr->children->content;
    }

    b = Binding ((unsigned int) strtoul (type, NULL, 10));

    if (joypad_number > -1 && joypad_number < NUM_JOYPAD_LINKS)
    {
        for (int i = 0; i < NUM_JOYPAD_LINKS; i++)
        {
            if (!strcasecmp (b_links[i].snes9x_name, name))
            {
                Binding *buttons = (Binding *) &pad[joypad_number];

                if (b.is_key () || b.is_joy ())
                    buttons[i] = b;
                else
                    buttons[i].clear ();
            }
        }
    }
    else
    {
        for (int i = NUM_JOYPAD_LINKS; b_links[i].snes9x_name; i++)
        {
            if (!strcasecmp (b_links[i].snes9x_name, name))
            {
                if (b.is_key () || b.is_joy ())
                    shortcut[i - NUM_JOYPAD_LINKS] = b;
                else
                    shortcut[i - NUM_JOYPAD_LINKS].clear ();

            }
        }
    }

    return 0;
}

int
Snes9xConfig::parse_joypad (xmlNodePtr node)
{
    xmlAttrPtr attr;
    int        joypad_number = -1;
    int        retval = 0;

    /* Try to read joypad number */
    for (attr = node->properties; attr; attr = attr->next)
    {
        if (!xmlStrcasecmp (attr->name, BAD_CAST "number"))
        {
            joypad_number = atoi ((char *) attr->children->content);

            if (joypad_number < 0 || joypad_number > (NUM_JOYPADS - 1))
                return 1;
        }
    }

    for (xmlNodePtr i = node->children; i; i = i->next)
    {
        if (!xmlStrcasecmp (i->name, BAD_CAST "binding"))
        {
            retval = parse_binding (i, joypad_number) || retval;
        }
    }

    return retval;
}

#ifdef USE_JOYSTICK

int
Snes9xConfig::parse_axis (xmlNodePtr node, int joynum)
{
    xmlAttrPtr attr;
    int        retval = 0;

    int number = -1, min = -32767, max = 32767, center = 0;

    for (attr = node->properties; attr; attr = attr->next)
    {
        if (!xmlStrcasecmp (attr->name, BAD_CAST "number"))
        {
            number = atoi ((char *) attr->children->content);
        }

        else if (!xmlStrcasecmp (attr->name, BAD_CAST "min"))
        {
            min = atoi ((char *) attr->children->content);
        }

        else if (!xmlStrcasecmp (attr->name, BAD_CAST "max"))
        {
            max = atoi ((char *) attr->children->content);
        }

        else if (!xmlStrcasecmp (attr->name, BAD_CAST "center"))
        {
            center = atoi ((char *) attr->children->content);
        }
    }

    if (number >= 0 && number < joystick[joynum]->num_axes)
    {
        joystick[joynum]->calibration[number].min = min;
        joystick[joynum]->calibration[number].max = max;
        joystick[joynum]->calibration[number].center = center;
    }

    return retval;
}

int
Snes9xConfig::parse_calibration (xmlNodePtr node)
{
    xmlAttrPtr attr;
    int        joynum = -1;
    int        num_joysticks = 0;
    int        retval = 0;

    for (num_joysticks = 0; joystick[num_joysticks]; num_joysticks++)
    {
    }

    for (attr = node->properties; attr; attr = attr->next)
    {
        if (!xmlStrcasecmp (attr->name, BAD_CAST "joystick"))
        {
            joynum = atoi ((char *) attr->children->content);

            if (joynum < 0 || joynum >= num_joysticks)
                return 0;
        }
    }

    for (xmlNodePtr i = node->children; i; i = i->next)
    {
        if (!xmlStrcasecmp (i->name, BAD_CAST "axis"))
        {
            retval = parse_axis (i, joynum) | retval;
        }
    }

    return retval;
}

#endif

int
Snes9xConfig::parse_preferences (xmlNodePtr node)
{
    xmlNodePtr i = NULL;
    int        retval = 0;

    for (i = node->children; i; i = i->next)
    {
        if (!xmlStrcasecmp (i->name, BAD_CAST "option"))
            retval = parse_option (i) || retval;
    }

    return retval;
}

int
Snes9xConfig::parse_controls (xmlNodePtr node)
{
    int retval = 0;

    for (xmlNodePtr i = node->children; i; i = i->next)
    {
        if (!xmlStrcasecmp (i->name, BAD_CAST "joypad"))
            retval = parse_joypad (i) || retval;

        if (!xmlStrcasecmp (i->name, BAD_CAST "binding"))
            retval = parse_binding (i, -1) || retval;

#ifdef USE_JOYSTICK
        if (!xmlStrcasecmp (i->name, BAD_CAST "calibration"))
            retval = parse_calibration (i) || retval;
#endif
    }

    return retval;
}

int
Snes9xConfig::parse_snes9x (xmlNodePtr node)
{
    xmlNodePtr i = NULL;
    int        retval = 0;

    if (xmlStrcasecmp (node->name, BAD_CAST "snes9x"))
    {
        fprintf (stderr, _("failure to read snes9x node"));
        return 1;
    }

    for (i = node->children; i; i = i->next)
    {
        if (!xmlStrcasecmp (i->name, BAD_CAST "preferences"))
        {
            retval = parse_preferences (i) || retval;
        }

        else if (!xmlStrcasecmp (i->name, BAD_CAST "controls"))
        {
            retval = parse_controls (i) || retval;
        }
    }

    return 0;
}

int
Snes9xConfig::load_config_file (void)
{
    struct stat file_info;
    char        *pathname;
    xmlDoc      *xml_doc = NULL;
    xmlNodePtr  xml_root = NULL;

    load_defaults ();

    pathname = get_config_dir ();

    if (stat (pathname, &file_info))
    {
        if (mkdir (pathname, 0755))
        {
            fprintf (stderr,
                     _("Couldn't create config directory: %s\n"),
                     pathname);
            return -1;
        }
    }
    else
    {
        chmod (pathname, 0755);
    }

    free (pathname);

    pathname = get_config_file_name ();

    if (stat (pathname, &file_info))
    {
        save_config_file ();
    }

    xml_doc = xmlReadFile (pathname, NULL, 0);

    if (!xml_doc)
    {
        fprintf (stderr, _("Couldn't open config file: %s\n"), pathname);
        return -2;
    }

    free (pathname);

    xml_root = xmlDocGetRootElement (xml_doc);

    parse_snes9x (xml_root);

    xmlFreeDoc (xml_doc);

    return 0;
}

void
Snes9xConfig::rebind_keys (void)
{
    s9xcommand_t cmd;
    char         buf[256];

    S9xUnmapAllControls ();

    for (int joypad_i = 0; joypad_i < NUM_JOYPADS; joypad_i++ )
    {
        Binding *bin = (Binding *) &pad[joypad_i];

        for (int button_i = 0; button_i < NUM_JOYPAD_LINKS; button_i++)
        {
            snprintf (buf,
                      256,
                      "Joypad%d %s",
                      (joypad_i % 5) + 1,
                      b_links[button_i].snes9x_name);

            cmd = S9xGetPortCommandT (buf);

            S9xMapButton (bin[button_i].base_hex (), cmd, FALSE);
        }
    }

    for (int i = NUM_JOYPAD_LINKS; b_links[i].snes9x_name; i++)
    {
        snprintf (buf, 256, "%s", b_links[i].snes9x_name);
        cmd = S9xGetPortCommandT (buf);
        S9xMapButton (shortcut[i - NUM_JOYPAD_LINKS].base_hex (),
                      cmd,
                      FALSE);
    }

    cmd = S9xGetPortCommandT ("Pointer Mouse1+Superscope+Justifier1");
    S9xMapPointer (BINDING_MOUSE_POINTER, cmd, TRUE);

    cmd = S9xGetPortCommandT ("{Mouse1 L,Superscope Fire,Justifier1 Trigger}");
    S9xMapButton (BINDING_MOUSE_BUTTON0, cmd, FALSE);

    cmd = S9xGetPortCommandT ("{Justifier1 AimOffscreen Trigger,Superscope AimOffscreen}");
    S9xMapButton (BINDING_MOUSE_BUTTON1, cmd, FALSE);

    cmd = S9xGetPortCommandT ("{Mouse1 R,Superscope Cursor,Justifier1 Start}");
    S9xMapButton (BINDING_MOUSE_BUTTON2, cmd, FALSE);

    return;
}

void
Snes9xConfig::reconfigure (void)
{
    rebind_keys ();

    return;
}



