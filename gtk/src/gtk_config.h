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

enum {
    SPLASH_IMAGE_BLACK = 0,
    SPLASH_IMAGE_SMTPE = 1,
    SPLASH_IMAGE_PATTERN = 2,
    SPLASH_IMAGE_BLUE = 3,
    SPLASH_IMAGE_COMBO = 4
};

class Snes9xConfig
{
  public:
    Snes9xConfig ();
    int load_config_file ();
    int save_config_file ();
    int load_defaults ();
    void rebind_keys ();
    void flush_joysticks ();
    void set_joystick_mode (int mode);
    void joystick_register_centers ();

    /* Screen options */
    bool full_screen_on_open;
    bool change_display_resolution;
    bool fullscreen;
    bool ui_visible;
    int default_esc_behavior;
    bool prevent_screensaver;
    int xrr_index;
    bool scale_to_fit;
    bool maintain_aspect_ratio;
    int aspect_ratio;
    int scale_method;
    bool overscan;
    bool multithreading;
    int hires_effect;
    bool force_inverted_byte_order;
    int splash_image;

    snes_ntsc_setup_t ntsc_setup;
    int ntsc_format;
    float ntsc_hue;
    float ntsc_saturation;
    float ntsc_contrast;
    float ntsc_brightness;
    float ntsc_sharpness;
    float ntsc_artifacts;
    float ntsc_gamma;
    float ntsc_resolution;
    float ntsc_fringing;
    float ntsc_bleed;
    float ntsc_merge_fields;
    int ntsc_scanline_intensity;
    int scanline_filter_intensity;
    int hw_accel;
    bool allow_opengl;
    bool allow_xv;
    bool allow_xrandr;

    /* Data options */
    int save_sram_after_secs;
    std::string sram_directory;
    std::string savestate_directory;
    std::string cheat_directory;
    std::string patch_directory;
    std::string export_directory;
    std::string last_directory;
    std::string last_shader_directory;

    /* Controls */
    JoypadBinding pad[NUM_JOYPADS];
    Binding       shortcut[NUM_EMU_LINKS];

    /* Netplay */
    bool netplay_is_server;
    bool netplay_sync_reset;
    bool netplay_send_rom;
    int netplay_default_port;
    int netplay_max_frame_loss;
    std::string netplay_last_rom;
    std::string netplay_last_host;
    int netplay_last_port;
    bool netplay_activated;
    bool netplay_server_up;

    /* Operational */
    int sound_driver;
    bool mute_sound;
    bool mute_sound_turbo;
    int sound_buffer_size;
    int sound_playback_rate;
    bool auto_input_rate;
    int sound_input_rate;
    bool rom_loaded;
    int window_width, window_height;
    int preferences_width, preferences_height;
    int shader_parameters_width, shader_parameters_height;
    int current_display_tab;
    bool pause_emulation_on_switch;
    int num_threads;
    bool screensaver_needs_reset;
    bool modal_dialogs;

    bool pointer_is_visible;
    gint64 pointer_timestamp;

    unsigned int rewind_granularity;
    unsigned int rewind_buffer_size;

    int current_save_slot;

    XRRScreenResources *xrr_screen_resources;
    XRRCrtcInfo *xrr_crtc_info;

#ifdef USE_OPENGL
    bool sync_to_vblank;
    bool use_pbos;
    int pbo_format;
    bool npot_textures;
    bool use_shaders;
    std::string shader_filename;
    bool use_glfinish;
    bool use_sync_control;
#endif

    JoyDevice **joystick;
    int joystick_threshold;
};

std::string get_config_dir ();
std::string get_config_file_name ();

#endif /* __GTK_CONFIG_H */
