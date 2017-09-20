#ifndef __GTK_CONFIG_H
#define __GTK_CONFIG_H

#include <sys/time.h>
#include <libxml/parser.h>
#include <X11/Xlib.h>
#ifdef USE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif

#include "gtk_control.h"
#include "snes_ntsc.h"

#define HWA_NONE                0
#define HWA_OPENGL              1
#define HWA_XV                  2

#define HIRES_MERGE             0
#define HIRES_NORMAL            1
#define HIRES_SCALE             2

#define ESC_TOGGLE_MENUBAR      0
#define ESC_EXIT_FULLSCREEN     1
#define ESC_EXIT_SNES9X         2

class Snes9xConfig
{
    public:
        Snes9xConfig (void);
        int load_config_file (void);
        int save_config_file (void);
        int load_defaults (void);
        void reconfigure (void);
        void rebind_keys (void);

        /* XML parsers */
        int parse_snes9x (xmlNodePtr node);
        int parse_preferences (xmlNodePtr node);
        int parse_option (xmlNodePtr node);
        int parse_joypad (xmlNodePtr node);
        int parse_binding (xmlNodePtr node, int joypad_number);
        int parse_controls (xmlNodePtr node);
        int set_option (const char *name, const char *value);

#ifdef USE_JOYSTICK
        void flush_joysticks (void);
        void set_joystick_mode (int mode);
        void joystick_register_centers (void);
        int parse_calibration (xmlNodePtr node);
        int parse_axis (xmlNodePtr node, int joystick_number);
#endif

        /* Screen options */
        unsigned char full_screen_on_open;
        unsigned char change_display_resolution;
        unsigned char fullscreen;
        unsigned char ui_visible;
        unsigned char statusbar_visible;
        unsigned char default_esc_behavior;
        unsigned char prevent_screensaver;
        unsigned int  xrr_index;
        int           xrr_width;
        int           xrr_height;
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
        unsigned char bilinear_filter;
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
        int           sound_input_rate;
        int           rom_loaded;
        int           window_width, window_height;
        int           preferences_width, preferences_height;
        int           pause_emulation_on_switch;
        int           num_threads;
        unsigned char screensaver_needs_reset;
        int           modal_dialogs;

        int            pointer_is_visible;
        struct timeval pointer_timestamp;
        
        unsigned int   rewind_granularity;
        unsigned int   rewind_buffer_size;

#ifdef USE_XRANDR
        XRRScreenConfiguration *xrr_config;
        XRRScreenSize          *xrr_sizes;
        int                    xrr_num_sizes;
        Rotation               xrr_rotation;
        SizeID                 xrr_original_size;
#endif


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

char *get_config_dir (void);
char *get_config_file_name (void);

#endif /* __GTK_CONFIG_H */
