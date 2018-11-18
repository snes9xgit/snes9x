/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_CONFIG_H
#define __GTK_CONFIG_H

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <string>

#include "gtk_control.h"
#include "filter/snes_ntsc.h"

enum {
    HWA_NONE = 0,
    HWA_OPENGL = 1,
    HWA_XV = 2
};

enum {
    HIRES_MERGE = 0,
    HIRES_NORMAL = 1,
    HIRES_SCALE = 2
};

enum {
    ESC_TOGGLE_MENUBAR = 0,
    ESC_EXIT_FULLSCREEN = 1,
    ESC_EXIT_SNES9X = 2
};

enum {
    THROTTLE_TIMER = 0,
    THROTTLE_TIMER_FRAMESKIP = 1,
    THROTTLE_SOUND_SYNC = 2,
    THROTTLE_NONE = 3
};

class Snes9xConfig
{
  public:
    Snes9xConfig ();
    int load_config_file ();
    int save_config_file ();
    int load_defaults ();
    void reconfigure ();
    void rebind_keys ();

#ifdef USE_JOYSTICK
    void flush_joysticks ();
    void set_joystick_mode (int mode);
    void joystick_register_centers ();
#endif

    /* Screen options */
    unsigned char full_screen_on_open;
    unsigned char change_display_resolution;
    unsigned char fullscreen;
    unsigned char ui_visible;
    unsigned char statusbar_visible;
    unsigned char default_esc_behavior;
    unsigned char prevent_screensaver;
    int           xrr_index;
    unsigned char scale_to_fit;
    unsigned char maintain_aspect_ratio;
    int           aspect_ratio;
    unsigned int  scale_method;
    unsigned char overscan;
    unsigned char multithreading;
    int           hires_effect;
    unsigned char force_inverted_byte_order;

    snes_ntsc_setup_t ntsc_setup;
    unsigned char ntsc_format;
    float         ntsc_hue;
    float         ntsc_saturation;
    float         ntsc_contrast;
    float         ntsc_brightness;
    float         ntsc_sharpness;
    float         ntsc_artifacts;
    float         ntsc_gamma;
    float         ntsc_resolution;
    float         ntsc_fringing;
    float         ntsc_bleed;
    float         ntsc_merge_fields;
    unsigned int  ntsc_scanline_intensity;
    unsigned int  scanline_filter_intensity;
    unsigned char hw_accel;
    unsigned char allow_opengl;
    unsigned char allow_xv;
    unsigned char allow_xrandr;

    /* Data options */
    unsigned char save_sram_after_secs;
    char          sram_directory [PATH_MAX];
    char          savestate_directory [PATH_MAX];
    char          cheat_directory [PATH_MAX];
    char          patch_directory [PATH_MAX];
    char          export_directory [PATH_MAX];
    char          last_directory [PATH_MAX];
    char          last_shader_directory [PATH_MAX];

    /* Controls */
    JoypadBinding pad[NUM_JOYPADS];
    Binding       shortcut[NUM_EMU_LINKS];

    /* Netplay */
    int           netplay_is_server;
    int           netplay_sync_reset;
    int           netplay_send_rom;
    int           netplay_default_port;
    int           netplay_max_frame_loss;
    char          netplay_last_rom [PATH_MAX];
    char          netplay_last_host [PATH_MAX];
    int           netplay_last_port;
    int           netplay_activated;
    int           netplay_server_up;

    /* Operational */
    int           sound_driver;
    int           mute_sound;
    int           mute_sound_turbo;
    int           sound_buffer_size;
    int           sound_playback_rate;
    int           auto_input_rate;
    int           sound_input_rate;
    int           rom_loaded;
    int           window_width, window_height;
    int           preferences_width, preferences_height;
    int           pause_emulation_on_switch;
    int           num_threads;
    unsigned char screensaver_needs_reset;
    int           modal_dialogs;

    int           pointer_is_visible;
    gint64        pointer_timestamp;

    unsigned int  rewind_granularity;
    unsigned int  rewind_buffer_size;

    int           current_save_slot;

    XRRScreenResources *xrr_screen_resources;
    XRRCrtcInfo        *xrr_crtc_info;

#ifdef USE_OPENGL
    unsigned char sync_to_vblank;
    unsigned char opengl_activated;
    unsigned char use_pbos;
    int           pbo_format;
    unsigned char npot_textures;
    unsigned char use_shaders;
    char          fragment_shader[PATH_MAX];
    char          vertex_shader[PATH_MAX];
    unsigned char sync_every_frame;
#endif

#ifdef USE_JOYSTICK
    JoyDevice **joystick;
    int       joystick_threshold;
#endif

};

std::string get_config_dir ();
std::string get_config_file_name ();

#endif /* __GTK_CONFIG_H */
