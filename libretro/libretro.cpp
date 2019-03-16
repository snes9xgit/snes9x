#include "libretro.h"

#include "snes9x.h"
#include "memmap.h"
#include "srtc.h"
#include "apu/apu.h"
#include "apu/bapu/snes/snes.hpp"
#include "gfx.h"
#include "snapshot.h"
#include "controls.h"
#include "cheats.h"
#include "movie.h"
#include "logger.h"
#include "display.h"
#include "conffile.h"
#include "crosshairs.h"
#include <stdio.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "filter/snes_ntsc.h"

#define RETRO_DEVICE_JOYPAD_MULTITAP ((1 << 8) | RETRO_DEVICE_JOYPAD)
#define RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE ((1 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIER ((2 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIER_2 ((3 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_MACS_RIFLE ((4 << 8) | RETRO_DEVICE_LIGHTGUN)

static int g_screen_gun_width = SNES_WIDTH;
static int g_screen_gun_height = SNES_HEIGHT;

#define RETRO_MEMORY_SNES_BSX_RAM ((1 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_BSX_PRAM ((2 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM ((3 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM ((4 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_GAME_BOY_RAM ((5 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_GAME_BOY_RTC ((6 << 8) | RETRO_MEMORY_RTC)

#define RETRO_GAME_TYPE_BSX             0x101 | 0x1000
#define RETRO_GAME_TYPE_BSX_SLOTTED     0x102 | 0x1000
#define RETRO_GAME_TYPE_SUFAMI_TURBO    0x103 | 0x1000
#define RETRO_GAME_TYPE_SUPER_GAME_BOY  0x104 | 0x1000
#define RETRO_GAME_TYPE_MULTI_CART      0x105 | 0x1000


#define SNES_4_3 4.0f / 3.0f

uint16 *screen_buffer = NULL;

char g_rom_dir[1024];
char g_basename[1024];

bool g_geometry_update = false;

int hires_blend = 0;
bool randomize_memory = false;

char retro_system_directory[4096];
char retro_save_directory[4096];

retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_audio_sample_t audio_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_state_cb = NULL;

static snes_ntsc_t *snes_ntsc = NULL;
static int blargg_filter = 0;
static uint16 *ntsc_screen_buffer, *snes_ntsc_buffer;

#define MAX_SNES_WIDTH_NTSC SNES_NTSC_OUT_WIDTH(256)

static void extract_basename(char *buf, const char *path, size_t size)
{
    const char *base = strrchr(path, '/');
    if (!base)
        base = strrchr(path, '\\');
    if (!base)
        base = path;

    if (*base == '\\' || *base == '/')
        base++;

    strncpy(buf, base, size - 1);
    buf[size - 1] = '\0';

    char *ext = strrchr(buf, '.');
    if (ext)
        *ext = '\0';
}

static void extract_directory(char *buf, const char *path, size_t size)
{
    strncpy(buf, path, size - 1);
    buf[size - 1] = '\0';

    char *base = strrchr(buf, '/');
    if (!base)
        base = strrchr(buf, '\\');

    if (base)
        *base = '\0';
    else
        buf[0] = '\0';
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

enum overscan_mode {
    OVERSCAN_CROP_ON,
    OVERSCAN_CROP_OFF,
    OVERSCAN_CROP_AUTO
};
enum aspect_mode {
    ASPECT_RATIO_4_3,
    ASPECT_RATIO_1_1,
    ASPECT_RATIO_NTSC,
    ASPECT_RATIO_PAL,
    ASPECT_RATIO_AUTO
};
static retro_environment_t environ_cb;
static overscan_mode crop_overscan_mode = OVERSCAN_CROP_ON; // default to crop
static aspect_mode aspect_ratio_mode = ASPECT_RATIO_4_3; // default to 4:3
static bool rom_loaded = false;

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    static const struct retro_subsystem_memory_info multi_a_memory[] = {
        { "srm", RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM },
    };

    static const struct retro_subsystem_memory_info multi_b_memory[] = {
        { "srm", RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM },
    };

    static const struct retro_subsystem_rom_info multicart_roms[] = {
        { "Cart A", "smc|sfc|swc|fig|bs", false, false, false, multi_a_memory, 1 },
        { "Cart B", "smc|sfc|swc|fig|bs", false, false, false, multi_b_memory, 1 },
    };

    static const struct retro_subsystem_info subsystems[] = {
        { "Multi-Cart Link", "multicart_addon", multicart_roms, 2, RETRO_GAME_TYPE_MULTI_CART },
        {}
    };

    cb(RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO,  (void*)subsystems);


    struct retro_variable variables[] = {
        // These variable names and possible values constitute an ABI with ZMZ (ZSNES Libretro player).
        // Changing "Show layer 1" is fine, but don't change "layer_1"/etc or the possible values ("Yes|No").
        // Adding more variables and rearranging them is safe.
        { "snes9x_up_down_allowed", "Allow Opposing Directions; disabled|enabled" },
        { "snes9x_overclock_superfx", "SuperFX Overclocking; 100%|150%|200%|250%|300%|350%|400%|450%|500%|50%|60%|70%|80%|90%" },
        { "snes9x_overclock_cycles", "Reduce Slowdown (Hack, Unsafe); disabled|light|compatible|max" },
        { "snes9x_reduce_sprite_flicker", "Reduce Flickering (Hack, Unsafe); disabled|enabled" },
        { "snes9x_randomize_memory", "Randomize Memory (Unsafe); disabled|enabled" },
        { "snes9x_hires_blend", "Hires Blending; disabled|merge|blur" },
        { "snes9x_audio_interpolation", "Audio Interpolation; gaussian|cubic|sinc|none|linear" },
        { "snes9x_blargg", "Blargg NTSC filter; disabled|monochrome|rf|composite|s-video|rgb" },
        { "snes9x_layer_1", "Show layer 1; enabled|disabled" },
        { "snes9x_layer_2", "Show layer 2; enabled|disabled" },
        { "snes9x_layer_3", "Show layer 3; enabled|disabled" },
        { "snes9x_layer_4", "Show layer 4; enabled|disabled" },
        { "snes9x_layer_5", "Show sprite layer; enabled|disabled" },
        { "snes9x_gfx_clip", "Enable graphic clip windows; enabled|disabled" },
        { "snes9x_gfx_transp", "Enable transparency effects; enabled|disabled" },
        { "snes9x_gfx_hires", "Enable hires mode; enabled|disabled" },
        { "snes9x_sndchan_1", "Enable sound channel 1; enabled|disabled" },
        { "snes9x_sndchan_2", "Enable sound channel 2; enabled|disabled" },
        { "snes9x_sndchan_3", "Enable sound channel 3; enabled|disabled" },
        { "snes9x_sndchan_4", "Enable sound channel 4; enabled|disabled" },
        { "snes9x_sndchan_5", "Enable sound channel 5; enabled|disabled" },
        { "snes9x_sndchan_6", "Enable sound channel 6; enabled|disabled" },
        { "snes9x_sndchan_7", "Enable sound channel 7; enabled|disabled" },
        { "snes9x_sndchan_8", "Enable sound channel 8; enabled|disabled" },
        { "snes9x_overscan", "Crop overscan; enabled|disabled|auto" },
        { "snes9x_aspect", "Preferred aspect ratio; 4:3|uncorrected|auto|ntsc|pal" },
        { "snes9x_region", "Console region (Reload core); auto|ntsc|pal" },
        { "snes9x_superscope_crosshair", "Super Scope crosshair; 2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|0|1" },
        { "snes9x_superscope_color", "Super Scope color; White|White (blend)|Red|Red (blend)|Orange|Orange (blend)|Yellow|Yellow (blend)|Green|Green (blend)|Cyan|Cyan (blend)|Sky|Sky (blend)|Blue|Blue (blend)|Violet|Violet (blend)|Pink|Pink (blend)|Purple|Purple (blend)|Black|Black (blend)|25% Grey|25% Grey (blend)|50% Grey|50% Grey (blend)|75% Grey|75% Grey (blend)" },
        { "snes9x_justifier1_crosshair", "Justifier 1 crosshair; 4|5|6|7|8|9|10|11|12|13|14|15|16|0|1|2|3" },
        { "snes9x_justifier1_color", "Justifier 1 color; Blue|Blue (blend)|Violet|Violet (blend)|Pink|Pink (blend)|Purple|Purple (blend)|Black|Black (blend)|25% Grey|25% Grey (blend)|50% Grey|50% Grey (blend)|75% Grey|75% Grey (blend)|White|White (blend)|Red|Red (blend)|Orange|Orange (blend)|Yellow|Yellow (blend)|Green|Green (blend)|Cyan|Cyan (blend)|Sky|Sky (blend)" },
        { "snes9x_justifier2_crosshair", "Justifier 2 crosshair; 4|5|6|7|8|9|10|11|12|13|14|15|16|0|1|2|3" },
        { "snes9x_justifier2_color", "Justifier 2 color; Pink|Pink (blend)|Purple|Purple (blend)|Black|Black (blend)|25% Grey|25% Grey (blend)|50% Grey|50% Grey (blend)|75% Grey|75% Grey (blend)|White|White (blend)|Red|Red (blend)|Orange|Orange (blend)|Yellow|Yellow (blend)|Green|Green (blend)|Cyan|Cyan (blend)|Sky|Sky (blend)|Blue|Blue (blend)|Violet|Violet (blend)" },
        { "snes9x_rifle_crosshair", "M.A.C.S. rifle crosshair; 2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|0|1" },
        { "snes9x_rifle_color", "M.A.C.S. rifle color; White|White (blend)|Red|Red (blend)|Orange|Orange (blend)|Yellow|Yellow (blend)|Green|Green (blend)|Cyan|Cyan (blend)|Sky|Sky (blend)|Blue|Blue (blend)|Violet|Violet (blend)|Pink|Pink (blend)|Purple|Purple (blend)|Black|Black (blend)|25% Grey|25% Grey (blend)|50% Grey|50% Grey (blend)|75% Grey|75% Grey (blend)" },
        { "snes9x_block_invalid_vram_access", "Block Invalid VRAM Access; enabled|disabled" },
        { NULL, NULL },
    };

    environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

    static const struct retro_controller_description port_1[] = {
        { "None", RETRO_DEVICE_NONE },
        { "SNES Joypad", RETRO_DEVICE_JOYPAD },
        { "SNES Mouse", RETRO_DEVICE_MOUSE },
        { "Multitap", RETRO_DEVICE_JOYPAD_MULTITAP },
    };

    static const struct retro_controller_description port_2[] = {
        { "None", RETRO_DEVICE_NONE },
        { "SNES Joypad", RETRO_DEVICE_JOYPAD },
        { "SNES Mouse", RETRO_DEVICE_MOUSE },
        { "Multitap", RETRO_DEVICE_JOYPAD_MULTITAP },
        { "SuperScope", RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE },
        { "Justifier", RETRO_DEVICE_LIGHTGUN_JUSTIFIER },
        { "M.A.C.S. Rifle", RETRO_DEVICE_LIGHTGUN_MACS_RIFLE },
    };

    static const struct retro_controller_description port_3[] = {
        { "None", RETRO_DEVICE_NONE },
        { "SNES Joypad", RETRO_DEVICE_JOYPAD },
        { "Justifier (2P)", RETRO_DEVICE_LIGHTGUN_JUSTIFIER_2 },
    };

    static const struct retro_controller_description port_extra[] = {
        { "None", RETRO_DEVICE_NONE },
        { "SNES Joypad", RETRO_DEVICE_JOYPAD },
    };

    static const struct retro_controller_info ports[] = {
        { port_1, 4 },
        { port_2, 7 },
        { port_3, 3 },
        { port_extra, 2 },
        { port_extra, 2 },
        { port_extra, 2 },
        { port_extra, 2 },
        { port_extra, 2 },
        {},
    };

    environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

char *get_cursor_color(const char *name)
{
	static char color_names[][32] = {
		"Black", "Black",
		"Black (blend)", "tBlack",
		"25% Grey", "25Grey",
		"25% Grey (blend)", "t25Grey",
		"50% Grey", "50Grey",
		"50% Grey (blend)", "t50Grey",
		"75% Grey", "75Grey",
		"75% Grey (blend)", "t75Grey",
		"White", "White",
		"White (blend)", "tWhite",
		"Red", "Red",
		"Red (blend)", "tRed",
		"Orange", "Orange",
		"Orange (blend)", "tOrange",
		"Yellow", "Yellow",
		"Yellow (blend)", "tYellow",
		"Green", "Green",
		"Green (blend)", "tGreen",
		"Cyan", "Cyan",
		"Cyan (blend)", "tCyan",
		"Sky", "Sky",
		"Sky (blend)", "tSky",
		"Blue", "Blue",
		"Blue (blend)", "tBlue",
		"Violet", "Violet",
		"Violet (blend)", "tViolet",
		"Pink", "MagicPink",
		"Pink (blend)", "tMagicPink",
		"Purple", "Purple",
		"Purple (blend)", "tPurple",
		"\0", "\0"
	};

	int lcv = 0;
	while (color_names[lcv][0]) {
		if (strcmp(color_names[lcv], name) == 0) {
			return color_names[lcv+1];
		}

		lcv += 2;
	}

	return "None";
}

// always ensure this is only called in retro_run
void update_geometry(void)
{
    struct retro_system_av_info av_info;
    retro_get_system_av_info(&av_info);
    environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
    g_screen_gun_width = av_info.geometry.base_width;
    g_screen_gun_height = av_info.geometry.base_height;
    g_geometry_update = false;
}

static void update_variables(void)
{
    char key[256];
    struct retro_variable var;

    var.key = "snes9x_hires_blend";
    var.value = NULL;

    hires_blend = 0;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (!strcmp (var.value, "blur"))
            hires_blend = 1;
        else if (!strcmp (var.value, "merge"))
            hires_blend = 2;
    }

    var.key = "snes9x_overclock_superfx";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        int freq = atoi(var.value);
        Settings.SuperFXClockMultiplier = freq;
    }

    var.key = "snes9x_up_down_allowed";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        Settings.UpAndDown = !strcmp(var.value, "disabled") ? false : true;
    }
    else
        Settings.UpAndDown = false;

    int disabled_channels=0;
    strcpy(key, "snes9x_sndchan_x");
    var.key=key;
    for (int i=0;i<8;i++)
    {
        key[strlen("snes9x_sndchan_")]='1'+i;
        var.value=NULL;
        if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && !strcmp("disabled", var.value))
            disabled_channels|=1<<i;
    }
    S9xSetSoundControl(disabled_channels^0xFF);


    int disabled_layers=0;
    strcpy(key, "snes9x_layer_x");
    for (int i=0;i<5;i++)
    {
        key[strlen("snes9x_layer_")]='1'+i;
        var.value=NULL;
        if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && !strcmp("disabled", var.value))
            disabled_layers|=1<<i;
    }
    Settings.BG_Forced=disabled_layers;

    //for some reason, Transparency seems to control both the fixed color and the windowing registers?
    var.key="snes9x_gfx_clip";
    var.value=NULL;
    Settings.DisableGraphicWindows=(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && !strcmp("disabled", var.value));

    var.key="snes9x_gfx_transp";
    var.value=NULL;
    Settings.Transparency=!(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && !strcmp("disabled", var.value));

    var.key="snes9x_gfx_hires";
    var.value=NULL;
    Settings.SupportHiRes=!(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && !strcmp("disabled", var.value));

    var.key="snes9x_audio_interpolation";
    var.value=NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "gaussian") == 0)
            Settings.InterpolationMethod = DSP_INTERPOLATION_GAUSSIAN;
        else if (strcmp(var.value, "linear") == 0)
            Settings.InterpolationMethod = DSP_INTERPOLATION_LINEAR;
        else if (strcmp(var.value, "cubic") == 0)
            Settings.InterpolationMethod = DSP_INTERPOLATION_CUBIC;
        else if (strcmp(var.value, "sinc") == 0)
            Settings.InterpolationMethod = DSP_INTERPOLATION_SINC;
        else if (strcmp(var.value, "none") == 0)
            Settings.InterpolationMethod = DSP_INTERPOLATION_NONE;
    }
    else
        Settings.InterpolationMethod = DSP_INTERPOLATION_GAUSSIAN;


    Settings.OneClockCycle      = 6;
    Settings.OneSlowClockCycle  = 8;
    Settings.TwoClockCycles     = 12;

    var.key="snes9x_overclock_cycles";
    var.value=NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "max") == 0)
        {
            Settings.OneClockCycle      = 3;
            Settings.OneSlowClockCycle  = 3;
            Settings.TwoClockCycles     = 3;
        }
        else if (strcmp(var.value, "compatible") == 0)
        {
            Settings.OneClockCycle      = 4;
            Settings.OneSlowClockCycle  = 5;
            Settings.TwoClockCycles     = 6;
        }
        else if (strcmp(var.value, "light") == 0)
        {
            Settings.OneClockCycle      = 6;
            Settings.OneSlowClockCycle  = 6;
            Settings.TwoClockCycles     = 12;
        }
    }

    Settings.MaxSpriteTilesPerLine = 34;
    var.key="snes9x_reduce_sprite_flicker";
    var.value=NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
        if (strcmp(var.value, "enabled") == 0)
            Settings.MaxSpriteTilesPerLine = 128;

    randomize_memory = false;
    var.key = "snes9x_randomize_memory";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
        if (strcmp(var.value, "enabled") == 0)
            randomize_memory = true;

    var.key = "snes9x_overscan";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        overscan_mode newval = OVERSCAN_CROP_AUTO;
        if (strcmp(var.value, "enabled") == 0)
            newval = OVERSCAN_CROP_ON;
        else if (strcmp(var.value, "disabled") == 0)
            newval = OVERSCAN_CROP_OFF;

        if (newval != crop_overscan_mode)
        {
            crop_overscan_mode = newval;
            g_geometry_update = true;
        }
    }

    var.key = "snes9x_aspect";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        aspect_mode newval = ASPECT_RATIO_AUTO;
        if (strcmp(var.value, "ntsc") == 0)
            newval = ASPECT_RATIO_NTSC;
        else if (strcmp(var.value, "pal") == 0)
            newval = ASPECT_RATIO_PAL;
        else if (strcmp(var.value, "4:3") == 0)
            newval = ASPECT_RATIO_4_3;
        else if (strcmp(var.value, "uncorrected") == 0)
            newval = ASPECT_RATIO_1_1;

        if (newval != aspect_ratio_mode)
        {
            aspect_ratio_mode = newval;
            g_geometry_update = true;
        }
    }

    var.key = "snes9x_region";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "auto"))
        {
            Settings.ForceNTSC = false;
            Settings.ForcePAL = false;
        }
        else if (!strcmp(var.value, "ntsc"))
        {
            Settings.ForceNTSC = true;
            Settings.ForcePAL = false;
        }
        else if (!strcmp(var.value, "pal"))
        {
            Settings.ForceNTSC = false;
            Settings.ForcePAL = true;
        }
    }

    var.key="snes9x_superscope_crosshair";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        int crosshair;

        sscanf (var.value,"%d",&crosshair);
        S9xSetControllerCrosshair(X_SUPERSCOPE, crosshair, 0, 0);
    }

    var.key="snes9x_superscope_color";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        char *color = get_cursor_color(var.value);

        if (color[0] == 't')
            S9xSetControllerCrosshair(X_SUPERSCOPE, -1, get_cursor_color(var.value), "tBlack");
        else
            S9xSetControllerCrosshair(X_SUPERSCOPE, -1, get_cursor_color(var.value), "Black");
    }

    var.key="snes9x_justifier1_crosshair";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        int crosshair;

        sscanf (var.value,"%d",&crosshair);
        S9xSetControllerCrosshair(X_JUSTIFIER1, crosshair, 0, 0);
    }

    var.key="snes9x_justifier1_color";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        char *color = get_cursor_color(var.value);

        if (color[0] == 't')
            S9xSetControllerCrosshair(X_JUSTIFIER1, -1, get_cursor_color(var.value), "tBlack");
        else
            S9xSetControllerCrosshair(X_JUSTIFIER1, -1, get_cursor_color(var.value), "Black");
    }

    var.key="snes9x_justifier2_crosshair";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        int crosshair;

        sscanf (var.value,"%d",&crosshair);
        S9xSetControllerCrosshair(X_JUSTIFIER2, crosshair, 0, 0);
    }

    var.key="snes9x_justifier2_color";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        char *color = get_cursor_color(var.value);

        if (color[0] == 't')
            S9xSetControllerCrosshair(X_JUSTIFIER2, -1, get_cursor_color(var.value), "tBlack");
        else
            S9xSetControllerCrosshair(X_JUSTIFIER2, -1, get_cursor_color(var.value), "Black");
    }

    var.key="snes9x_rifle_crosshair";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        int crosshair;

        sscanf (var.value,"%d",&crosshair);
        S9xSetControllerCrosshair(X_MACSRIFLE, crosshair, 0, 0);
    }

    var.key="snes9x_rifle_color";

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        char *color = get_cursor_color(var.value);

        if (color[0] == 't')
            S9xSetControllerCrosshair(X_MACSRIFLE, -1, get_cursor_color(var.value), "tBlack");
        else
            S9xSetControllerCrosshair(X_MACSRIFLE, -1, get_cursor_color(var.value), "Black");
    }

    var.key = "snes9x_block_invalid_vram_access";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        Settings.BlockInvalidVRAMAccessMaster = !strcmp(var.value, "disabled") ? false : true;
    else
        Settings.BlockInvalidVRAMAccessMaster = true;

    var.key = "snes9x_blargg";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, "disabled") == 0)
            blargg_filter = 0;
        else
        {
            int old_filter = blargg_filter;

            if(!snes_ntsc) snes_ntsc = new snes_ntsc_t;
            snes_ntsc_setup_t setup = snes_ntsc_composite;

            if (strcmp(var.value, "monochrome") == 0)
            {
                blargg_filter = 1;

                setup = snes_ntsc_monochrome;
            }
            else if (strcmp(var.value, "rf") == 0)
            {
                blargg_filter = 2;

                setup = snes_ntsc_composite;
                setup.merge_fields = 0;
            }
            else if (strcmp(var.value, "composite") == 0)
            {
                blargg_filter = 3;

                setup = snes_ntsc_composite;
            }
            else if (strcmp(var.value, "s-video") == 0)
            {
                blargg_filter = 4;

                setup = snes_ntsc_svideo;
            }
            else if (strcmp(var.value, "rgb") == 0)
            {
                blargg_filter = 5;

                setup = snes_ntsc_rgb;
            }

            if (old_filter != blargg_filter)
                snes_ntsc_init( snes_ntsc, &setup );
        }
    }
}

static void S9xAudioCallback(void*)
{
    const int BUFFER_SIZE = 256;
    // This is called every time 128 to 132 samples are generated, which happens about 8 times per frame.  A buffer size of 256 samples is enough here.
    static int16_t audio_buf[BUFFER_SIZE];

    size_t avail = S9xGetSampleCount();
    while (avail >= BUFFER_SIZE)
    {
        //this loop will never be entered, but handle oversized sample counts just in case
        S9xMixSamples((uint8*)audio_buf, BUFFER_SIZE);
        audio_batch_cb(audio_buf, BUFFER_SIZE >> 1);

        avail -= BUFFER_SIZE;
    }
    if (avail > 0)
    {
        S9xMixSamples((uint8*)audio_buf, avail);
        audio_batch_cb(audio_buf, avail >> 1);
    }
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info,0,sizeof(retro_system_info));

    info->library_name = "Snes9x";
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
    info->library_version = VERSION GIT_VERSION;
    info->valid_extensions = "smc|sfc|swc|fig|bs";
    info->need_fullpath = false;
    info->block_extract = false;
}

float get_aspect_ratio(unsigned width, unsigned height)
{
    if (aspect_ratio_mode == ASPECT_RATIO_4_3)
    {
        return SNES_4_3;
    }
    else if (aspect_ratio_mode == ASPECT_RATIO_1_1)
    {
        return (float) width / (float) height;
    }

    // OV2: not sure if these really make sense - NTSC is similar to 4:3, PAL looks weird
    double sample_frequency_ntsc = 135000000.0f / 11.0f;
    double sample_frequency_pal = 14750000.0;

    double sample_freq = retro_get_region() == RETRO_REGION_NTSC ? sample_frequency_ntsc : sample_frequency_pal;
    double dot_rate = (Settings.PAL ? PAL_MASTER_CLOCK : NTSC_MASTER_CLOCK) / 4.0;

    if (aspect_ratio_mode == ASPECT_RATIO_NTSC) // ntsc
    {
        sample_freq = sample_frequency_ntsc;
        dot_rate = NTSC_MASTER_CLOCK / 4.0;
    }
    else if (aspect_ratio_mode == ASPECT_RATIO_PAL) // pal
    {
        sample_freq = sample_frequency_pal;
        dot_rate = PAL_MASTER_CLOCK / 4.0;
    }

    double par = sample_freq / 2.0 / dot_rate;
    return (float)(width * par / height);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    memset(info,0,sizeof(retro_system_av_info));
    unsigned width = SNES_WIDTH;
    unsigned height = PPU.ScreenHeight;
    if (crop_overscan_mode == OVERSCAN_CROP_ON)
        height = SNES_HEIGHT;
    else if (crop_overscan_mode == OVERSCAN_CROP_OFF)
        height = SNES_HEIGHT_EXTENDED;

    info->geometry.base_width = width;
    info->geometry.base_height = height;
    info->geometry.max_width = MAX_SNES_WIDTH_NTSC;
    info->geometry.max_height = MAX_SNES_HEIGHT;
    info->geometry.aspect_ratio = get_aspect_ratio(width, height);
    info->timing.sample_rate = 32000;
    info->timing.fps = retro_get_region() == RETRO_REGION_NTSC ? 21477272.0 / 357366.0 : 21281370.0 / 425568.0;

    g_screen_gun_width = width;
    g_screen_gun_height = height;
}

unsigned retro_api_version()
{
    return RETRO_API_VERSION;
}


void retro_reset()
{
    S9xSoftReset();
}

static unsigned snes_devices[8];
void retro_set_controller_port_device(unsigned port, unsigned device)
{
    if (port < 8)
    {
        int offset = snes_devices[0] == RETRO_DEVICE_JOYPAD_MULTITAP ? 4 : 1;
        switch (device)
        {
            case RETRO_DEVICE_JOYPAD:
                S9xSetController(port, CTL_JOYPAD, port * offset, 0, 0, 0);
                snes_devices[port] = RETRO_DEVICE_JOYPAD;
                break;
            case RETRO_DEVICE_JOYPAD_MULTITAP:
                S9xSetController(port, CTL_MP5, port * offset, port * offset + 1, port * offset + 2, port * offset + 3);
                snes_devices[port] = RETRO_DEVICE_JOYPAD_MULTITAP;
                break;
            case RETRO_DEVICE_MOUSE:
                S9xSetController(port, CTL_MOUSE, port, 0, 0, 0);
                snes_devices[port] = RETRO_DEVICE_MOUSE;
                break;
            case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE:
                S9xSetController(port, CTL_SUPERSCOPE, 0, 0, 0, 0);
                snes_devices[port] = RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE;
                break;
            case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:
                S9xSetController(port, CTL_JUSTIFIER, 0, 0, 0, 0);
                snes_devices[port] = RETRO_DEVICE_LIGHTGUN_JUSTIFIER;
                break;
            case RETRO_DEVICE_LIGHTGUN_JUSTIFIER_2:
            	if ( port == 2 )
            	{
					S9xSetController(1, CTL_JUSTIFIER, 1, 0, 0, 0);
                	snes_devices[port] = RETRO_DEVICE_LIGHTGUN_JUSTIFIER_2;
				}
				else
				{
					if (log_cb)
						log_cb(RETRO_LOG_ERROR, "Invalid Justifier (2P) assignment to port %d, must be port 2.\n", port);
					S9xSetController(port, CTL_NONE, 0, 0, 0, 0);
					snes_devices[port] = RETRO_DEVICE_NONE;
				}
                break;
            case RETRO_DEVICE_LIGHTGUN_MACS_RIFLE:
                S9xSetController(port, CTL_MACSRIFLE, 0, 0, 0, 0);
                snes_devices[port] = RETRO_DEVICE_LIGHTGUN_MACS_RIFLE;
                break;
            case RETRO_DEVICE_NONE:
                S9xSetController(port, CTL_NONE, 0, 0, 0, 0);
                snes_devices[port] = RETRO_DEVICE_NONE;
                break;
            default:
                if (log_cb)
                    log_cb(RETRO_LOG_ERROR, "Invalid device (%d).\n", device);
                break;
        }

        S9xControlsSoftReset();
    }
    else if(device != RETRO_DEVICE_NONE)
        log_cb(RETRO_LOG_INFO, "Nonexistent Port (%d).\n", port);
}

void retro_cheat_reset()
{
    S9xDeleteCheats();
}

void retro_cheat_set(unsigned index, bool enabled, const char *codeline)
{
    char codeCopy[256];
    char* code;

    if (codeline == (char *)'\0') return;

    strcpy(codeCopy,codeline);
    code=strtok(codeCopy,"+,.; ");
    while (code != NULL) {
        //Convert GH RAW to PAR
        if (strlen(code)==9 && code[6]==':')
        {
            code[6]=code[7];
            code[7]=code[8];
            code[8]='\0';
        }

        /* Goldfinger was broken and nobody noticed. Removed */
        if (S9xAddCheatGroup ("retro", code) >= 0)
        {
            if (enabled)
                S9xEnableCheatGroup (Cheat.g.size () - 1);
        }
        else
        {
            printf("CHEAT: Failed to recognize %s\n",code);
        }

        code=strtok(NULL,"+,.; "); // bad code, ignore
    }

    S9xCheatsEnable();
}

static void init_descriptors(void)
{
    struct retro_input_descriptor desc[] = {
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,		"D-Pad Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,		"B" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,		"A" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,		"X" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,		"Y" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,		"L" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,		"R" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,	"Select" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,		"Start" },

        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,		"D-Pad Up" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,		"B" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,		"A" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,		"X" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,		"Y" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,		"L" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,		"R" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,	"Select" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,		"Start" },

        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,		"D-Pad Up" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,		"B" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,		"A" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,		"X" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,		"Y" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,		"L" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,		"R" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,	"Select" },
        { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,		"Start" },

        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,		"D-Pad Up" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,		"B" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,		"A" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,		"X" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,		"Y" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,		"L" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,		"R" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,	"Select" },
        { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,		"Start" },

        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,		"D-Pad Up" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,		"B" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,		"A" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,		"X" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,		"Y" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,		"L" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,		"R" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,	"Select" },
        { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,		"Start" },

        { 0, 0, 0, 0, NULL },
    };

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

static bool valid_normal_bank (uint8 bankbyte)
{
    switch (bankbyte)
    {
        case 32: case 33: case 48: case 49:
            return (true);
    }

    return (false);
}

static int is_bsx (uint8 *p)
{
    if ((p[26] == 0x33 || p[26] == 0xFF) && (!p[21] || (p[21] & 131) == 128) && valid_normal_bank(p[24]))
    {
        unsigned char	m = p[22];

        if (!m && !p[23])
            return (2);

        if ((m == 0xFF && p[23] == 0xFF) || (!(m & 0xF) && ((m >> 4) - 1 < 12)))
            return (1);
    }

    return (0);
}

static bool8 LoadBIOS(uint8 *biosrom, const char *biosname, int biossize)
{
    FILE	*fp;
    char	name[PATH_MAX + 1];
    bool8 r = FALSE;

    strcpy(name, S9xGetDirectory(ROMFILENAME_DIR));
    strcat(name, SLASH_STR);
    strcat(name, biosname);

    fp = fopen(name, "rb");
    if (!fp)
    {
        strcpy(name, S9xGetDirectory(BIOS_DIR));
        strcat(name, SLASH_STR);
        strcat(name, biosname);

        fp = fopen(name, "rb");
    }

    if (fp)
    {
        size_t size;

        size = fread((void *) biosrom, 1, biossize, fp);
        fclose(fp);
        if (size == (unsigned int) biossize)
            r = TRUE;
    }

    return (r);
}

static bool8 is_SufamiTurbo_Cart (const uint8 *data, uint32 size)
{
    if (size >= 0x80000 && size <= 0x100000 &&
        strncmp((char *) data, "BANDAI SFC-ADX", 14) == 0 && strncmp((char * ) (data + 0x10), "SFC-ADX BACKUP", 14) != 0)
        return (TRUE);
    else
        return (FALSE);
}

bool retro_load_game(const struct retro_game_info *game)
{
    init_descriptors();

    update_variables();

    if(game->data == NULL && game->size == 0 && game->path != NULL)
        rom_loaded = Memory.LoadROM(game->path);
    else
    {
        uint8 *biosrom = new uint8[0x100000];

        if (game->path != NULL)
        {
            extract_basename(g_basename, game->path, sizeof(g_basename));
            extract_directory(g_rom_dir, game->path, sizeof(g_rom_dir));
        }

        if (is_SufamiTurbo_Cart((uint8 *) game->data, game->size)) {
            if ((rom_loaded = LoadBIOS(biosrom,"STBIOS.bin",0x40000)))
            rom_loaded = Memory.LoadMultiCartMem((const uint8_t*)game->data, game->size, 0, 0, biosrom, 0x40000);
        }

        else
        if ((is_bsx((uint8 *) game->data + 0x7fc0)==1) | (is_bsx((uint8 *) game->data + 0xffc0)==1)) {
            if ((rom_loaded = LoadBIOS(biosrom,"BS-X.bin",0x100000)))
            rom_loaded = Memory.LoadMultiCartMem(biosrom, 0x100000, (const uint8_t*)game->data, game->size, 0, 0);
        }

        else
            rom_loaded = Memory.LoadROMMem((const uint8_t*)game->data ,game->size);

        if(biosrom) delete[] biosrom;
    }

    if (rom_loaded)
    {
        /* If we're in RGB565 format, switch frontend to that */
        if (RED_SHIFT_BITS == 11) 
        {
            enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
            if (!environ_cb || !environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
            {
                return false;
            }
        }

        g_geometry_update = true;

        if (randomize_memory)
        {
            srand(time(NULL));
            for(int lcv = 0; lcv < 0x20000; lcv++)
                Memory.RAM[lcv] = rand() % 256;
        }
    }

    if (!rom_loaded && log_cb)
        log_cb(RETRO_LOG_ERROR, "ROM loading failed...\n");

    return rom_loaded;
}

static void remove_header(uint8_t *&romptr, size_t &romsize, bool multicart_sufami)
{
    if (romptr==0 || romsize==0) return;

    uint32 calc_size = (romsize / 0x2000) * 0x2000;
    if ((romsize - calc_size == 512 && !Settings.ForceNoHeader) || Settings.ForceHeader)
    {
        romptr += 512;
        romsize -= 512;

        if(log_cb) log_cb(RETRO_LOG_INFO,"ROM header removed\n");
    }

    if (multicart_sufami && (romptr + romsize) >= (romptr + 0x100000))
    {
        if (strncmp((const char*)(romptr + 0x100000), "BANDAI SFC-ADX", 14) == 0 &&
            strncmp((const char*)(romptr + 0x000000), "BANDAI SFC-ADX", 14) == 0)
        {
            romptr += 0x100000;
            romsize -= 0x100000;

            if(log_cb) log_cb(RETRO_LOG_INFO,"Sufami Turbo Multi-ROM bios removed\n");
        }
    }
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
    uint8_t *romptr[3];
    size_t romsize[3];

    for(size_t i=0; i < num_info; i++)
    {
        romptr[i] = (uint8_t *) info[i].data;
        romsize[i] = info[i].size;
        remove_header(romptr[i], romsize[i], true);
    }

    init_descriptors();
    rom_loaded = false;

    update_variables();
    switch (game_type)
    {
        case RETRO_GAME_TYPE_BSX:
            if(num_info == 1)
            {
              rom_loaded = Memory.LoadROMMem((const uint8_t*)romptr[0],romsize[0]);
            }
            else if(num_info == 2)
            {
                memcpy(Memory.BIOSROM,(const uint8_t*)romptr[0],info[0].size);
                rom_loaded = Memory.LoadROMMem((const uint8_t*)romptr[1],info[1].size);
            }

            if (!rom_loaded && log_cb)
                log_cb(RETRO_LOG_ERROR, "BSX ROM loading failed...\n");
            break;
        case RETRO_GAME_TYPE_BSX_SLOTTED:
        case RETRO_GAME_TYPE_MULTI_CART:
            if(num_info == 2)
            {
                if (is_SufamiTurbo_Cart((const uint8_t*)romptr[0], romsize[0]))
                {
                    log_cb(RETRO_LOG_ERROR, "Cart is Sufami Turbo...\n");
                    uint8 *biosrom = new uint8[0x40000];
                    uint8 *biosptr = biosrom;

                    if (LoadBIOS(biosptr, "STBIOS.bin", 0x40000))
                    {
                        if (log_cb)
                            log_cb(RETRO_LOG_INFO, "Loading Sufami Turbo link game\n");

                        rom_loaded = Memory.LoadMultiCartMem((const uint8_t*)romptr[0], romsize[0],
                               (const uint8_t*)romptr[1], romsize[1], biosptr, 0x40000);
                    }

                if (biosrom)
                    delete[] biosrom;
                }
                else
                {
                    if (log_cb)
                        log_cb(RETRO_LOG_INFO, "Loading Multi-Cart link game\n");

                    rom_loaded = Memory.LoadMultiCartMem((const uint8_t*)romptr[0], romsize[0],
                        (const uint8_t*)romptr[1], romsize[1], NULL, 0);
                }
            }

            if (!rom_loaded && log_cb)
                log_cb(RETRO_LOG_ERROR, "Multirom loading failed...\n");
            break;

        case RETRO_GAME_TYPE_SUFAMI_TURBO:
            if(num_info == 2)
            {
                uint8 *biosrom = new uint8[0x100000];

                if ((rom_loaded = LoadBIOS(biosrom,"STBIOS.bin",0x100000)))
                    rom_loaded = Memory.LoadMultiCartMem((const uint8_t*)romptr[0], romsize[0],
                        (const uint8_t*)romptr[1], romsize[1], biosrom, 0x40000);

                if (biosrom)
                    delete[] biosrom;
            }

            if (!rom_loaded && log_cb)
                log_cb(RETRO_LOG_ERROR, "Sufami Turbo ROM loading failed...\n");
            break;

        default:
            rom_loaded = false;
            log_cb(RETRO_LOG_ERROR, "Multi-cart ROM loading failed...\n");
            break;
    }

    if (rom_loaded)
    {
        if(RED_SHIFT_BITS == 11)
        {
            enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
            if (!environ_cb || !environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
                return false;
        }

        g_geometry_update = true;
    }

    return rom_loaded;
}

void retro_unload_game(void)
{}

static void map_buttons();

static void check_system_specs(void)
{
    /* TODO - might have to variably set performance level based on SuperFX/SA-1/etc */
    unsigned level = 12;
    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_init(void)
{
    struct retro_log_callback log;

    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
        log_cb = log.log;
    else
        log_cb = NULL;

    const char *dir = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
        snprintf(retro_system_directory, sizeof(retro_system_directory), "%s", dir);
    else
        snprintf(retro_system_directory, sizeof(retro_system_directory), "%s", ".");

    if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir)
        snprintf(retro_save_directory, sizeof(retro_save_directory), "%s", dir);
    else
        snprintf(retro_save_directory, sizeof(retro_save_directory), "%s", ".");

    // State that SNES9X supports achievements.
    bool achievements = true;
    environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS, &achievements);

    memset(&Settings, 0, sizeof(Settings));
    Settings.MouseMaster = TRUE;
    Settings.SuperScopeMaster = TRUE;
    Settings.JustifierMaster = TRUE;
    Settings.MultiPlayer5Master = TRUE;
    Settings.MacsRifleMaster = TRUE;
    Settings.FrameTimePAL = 20000;
    Settings.FrameTimeNTSC = 16667;
    Settings.SixteenBitSound = TRUE;
    Settings.Stereo = TRUE;
    Settings.SoundPlaybackRate = 32000;
    Settings.SoundInputRate = 32000;
    Settings.SupportHiRes = TRUE;
    Settings.Transparency = TRUE;
    Settings.AutoDisplayMessages = TRUE;
    Settings.InitialInfoStringTimeout = 120;
    Settings.HDMATimingHack = 100;
    Settings.BlockInvalidVRAMAccessMaster = TRUE;
    Settings.CartAName[0] = 0;
    Settings.CartBName[0] = 0;
    Settings.AutoSaveDelay = 1;
    Settings.DontSaveOopsSnapshot = TRUE;

    CPU.Flags = 0;

    if (!Memory.Init() || !S9xInitAPU())
    {
        Memory.Deinit();
        S9xDeinitAPU();

        if (log_cb)
            log_cb(RETRO_LOG_ERROR, "Failed to init Memory or APU.\n");
        exit(1);
    }

    S9xInitSound(0);

    S9xSetSoundMute(FALSE);
    S9xSetSamplesAvailableCallback(S9xAudioCallback, NULL);

    GFX.Pitch = MAX_SNES_WIDTH_NTSC * sizeof(uint16);
    screen_buffer = (uint16*) calloc(1, GFX.Pitch * (MAX_SNES_HEIGHT + 16));
    GFX.Screen = screen_buffer + (GFX.Pitch >> 1) * 16;
    ntsc_screen_buffer = (uint16*) calloc(1, GFX.Pitch * (MAX_SNES_HEIGHT + 16));
    snes_ntsc_buffer = ntsc_screen_buffer + (GFX.Pitch >> 1) * 16;
    S9xGraphicsInit();

    S9xInitInputDevices();
    for (int i = 0; i < 2; i++)
    {
        S9xSetController(i, CTL_JOYPAD, i, 0, 0, 0);
        snes_devices[i] = RETRO_DEVICE_JOYPAD;
    }

    S9xUnmapAllControls();
    map_buttons();
    check_system_specs();
}

#define MAP_BUTTON(id, name) S9xMapButton((id), S9xGetCommandT((name)), false)
#define MAKE_BUTTON(pad, btn) (((pad)<<4)|(btn))

#define PAD_1 1
#define PAD_2 2
#define PAD_3 3
#define PAD_4 4
#define PAD_5 5

#define BTN_B RETRO_DEVICE_ID_JOYPAD_B
#define BTN_Y RETRO_DEVICE_ID_JOYPAD_Y
#define BTN_SELECT RETRO_DEVICE_ID_JOYPAD_SELECT
#define BTN_START RETRO_DEVICE_ID_JOYPAD_START
#define BTN_UP RETRO_DEVICE_ID_JOYPAD_UP
#define BTN_DOWN RETRO_DEVICE_ID_JOYPAD_DOWN
#define BTN_LEFT RETRO_DEVICE_ID_JOYPAD_LEFT
#define BTN_RIGHT RETRO_DEVICE_ID_JOYPAD_RIGHT
#define BTN_A RETRO_DEVICE_ID_JOYPAD_A
#define BTN_X RETRO_DEVICE_ID_JOYPAD_X
#define BTN_L RETRO_DEVICE_ID_JOYPAD_L
#define BTN_R RETRO_DEVICE_ID_JOYPAD_R
#define BTN_FIRST BTN_B
#define BTN_LAST BTN_R

#define MOUSE_X RETRO_DEVICE_ID_MOUSE_X
#define MOUSE_Y RETRO_DEVICE_ID_MOUSE_Y
#define MOUSE_LEFT RETRO_DEVICE_ID_MOUSE_LEFT
#define MOUSE_RIGHT RETRO_DEVICE_ID_MOUSE_RIGHT
#define MOUSE_FIRST MOUSE_X
#define MOUSE_LAST MOUSE_RIGHT

static int scope_buttons[] =
{
	RETRO_DEVICE_ID_LIGHTGUN_TRIGGER, // 2
	RETRO_DEVICE_ID_LIGHTGUN_CURSOR, // 3
	RETRO_DEVICE_ID_LIGHTGUN_TURBO, // 4
	RETRO_DEVICE_ID_LIGHTGUN_START, // 5
	RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN, // 6
};
static int scope_button_count = sizeof( scope_buttons ) / sizeof( int );

#define JUSTIFIER_TRIGGER 2
#define JUSTIFIER_START 3
#define JUSTIFIER_OFFSCREEN 4

#define MACS_RIFLE_TRIGGER 2

#define BTN_POINTER (BTN_LAST + 1)
#define BTN_POINTER2 (BTN_POINTER + 1)


static void map_buttons()
{
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_A), "Joypad1 A");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_B), "Joypad1 B");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_X), "Joypad1 X");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_Y), "Joypad1 Y");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_SELECT), "{Joypad1 Select,Mouse1 L}");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_START), "{Joypad1 Start,Mouse1 R}");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_L), "Joypad1 L");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_R), "Joypad1 R");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_LEFT), "Joypad1 Left");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_RIGHT), "Joypad1 Right");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_UP), "Joypad1 Up");
    MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_DOWN), "Joypad1 Down");
    S9xMapPointer((BTN_POINTER), S9xGetCommandT("Pointer Mouse1+Superscope+Justifier1+MacsRifle"), false);
    S9xMapPointer((BTN_POINTER2), S9xGetCommandT("Pointer Mouse2+Justifier2"), false);

    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_B), "Joypad2 B");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_Y), "Joypad2 Y");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_SELECT), "{Joypad2 Select,Mouse2 L,Superscope Fire,Justifier1 Trigger,MacsRifle Trigger}");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_START), "{Joypad2 Start,Mouse2 R,Superscope Cursor,Justifier1 Start}");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_UP), "{Joypad2 Up,Superscope ToggleTurbo,Justifier1 AimOffscreen}");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_DOWN), "{Joypad2 Down,Superscope Pause}");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_LEFT), "{Joypad2 Left,Superscope AimOffscreen}");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_RIGHT), "Joypad2 Right");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_A), "Joypad2 A");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_X), "Joypad2 X");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_L), "Joypad2 L");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_R), "Joypad2 R");

    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_B), "Joypad3 B");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_Y), "Joypad3 Y");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_SELECT), "{Joypad3 Select,Justifier2 Trigger}");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_START), "{Joypad3 Start,Justifier2 Start}");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_UP), "{Joypad3 Up,Justifier2 AimOffscreen}");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_DOWN), "Joypad3 Down");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_LEFT), "Joypad3 Left");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_RIGHT), "Joypad3 Right");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_A), "Joypad3 A");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_X), "Joypad3 X");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_L), "Joypad3 L");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_R), "Joypad3 R");

    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_A), "Joypad4 A");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_B), "Joypad4 B");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_X), "Joypad4 X");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_Y), "Joypad4 Y");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_SELECT), "Joypad4 Select");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_START), "Joypad4 Start");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_L), "Joypad4 L");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_R), "Joypad4 R");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_LEFT), "Joypad4 Left");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_RIGHT), "Joypad4 Right");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_UP), "Joypad4 Up");
    MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_DOWN), "Joypad4 Down");

    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_A), "Joypad5 A");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_B), "Joypad5 B");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_X), "Joypad5 X");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_Y), "Joypad5 Y");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_SELECT), "Joypad5 Select");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_START), "Joypad5 Start");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_L), "Joypad5 L");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_R), "Joypad5 R");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_LEFT), "Joypad5 Left");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_RIGHT), "Joypad5 Right");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_UP), "Joypad5 Up");
    MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_DOWN), "Joypad5 Down");

}

static int16_t snes_mouse_state[2][2] = {{0}, {0}};
static bool snes_superscope_turbo_latch = false;

static void input_report_gun_position( unsigned port, int s9xinput )
{
	int x, y;

	x = input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X);
	y = input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y);

	/*scale & clamp*/
	x = ( ( x + 0x7FFF ) * g_screen_gun_width ) / 0xFFFF;
	if ( x < 0 )
		x = 0;
	else if ( x >= g_screen_gun_width )
		x = g_screen_gun_width - 1;

	/*scale & clamp*/
	y = ( ( y + 0x7FFF ) * g_screen_gun_height ) / 0xFFFF;
	if ( y < 0 )
		y = 0;
	else if ( y >= g_screen_gun_height )
		y = g_screen_gun_height - 1;

	S9xReportPointer(s9xinput, (int16_t)x, (int16_t)y);
}

static void report_buttons()
{
    int offset = snes_devices[0] == RETRO_DEVICE_JOYPAD_MULTITAP ? 4 : 1;
    int _x, _y;

    for (int port = 0; port <= 1; port++)
    {
        switch (snes_devices[port])
        {
            case RETRO_DEVICE_JOYPAD:
                for (int i = BTN_FIRST; i <= BTN_LAST; i++)
                    S9xReportButton(MAKE_BUTTON(port * offset + 1, i), input_state_cb(port * offset, RETRO_DEVICE_JOYPAD, 0, i));
                break;

            case RETRO_DEVICE_JOYPAD_MULTITAP:
                for (int j = 0; j < 4; j++)
                    for (int i = BTN_FIRST; i <= BTN_LAST; i++)
                        S9xReportButton(MAKE_BUTTON(port * offset + j + 1, i), input_state_cb(port * offset + j, RETRO_DEVICE_JOYPAD, 0, i));
                break;

            case RETRO_DEVICE_MOUSE:
                _x = input_state_cb(port, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
                _y = input_state_cb(port, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
                snes_mouse_state[port][0] += _x;
                snes_mouse_state[port][1] += _y;
                S9xReportPointer(BTN_POINTER + port, snes_mouse_state[port][0], snes_mouse_state[port][1]);
                for (int i = MOUSE_LEFT; i <= MOUSE_LAST; i++)
                    S9xReportButton(MAKE_BUTTON(port + 1, i), input_state_cb(port, RETRO_DEVICE_MOUSE, 0, i));
                break;

            case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE:

				input_report_gun_position( port, BTN_POINTER );

				for (int i = 0; i < scope_button_count; i++)
				{
					int id = scope_buttons[i];
					bool btn = input_state_cb( port, RETRO_DEVICE_LIGHTGUN, 0, id )?true:false;

					/* RETRO_DEVICE_ID_LIGHTGUN_TURBO special case - core needs a rising-edge trigger */
					if ( id == RETRO_DEVICE_ID_LIGHTGUN_TURBO )
					{
						bool old = btn;
						btn = btn && !snes_superscope_turbo_latch;
						snes_superscope_turbo_latch = old;
					}

					S9xReportButton(MAKE_BUTTON(PAD_2, i+2), btn);
				}
                break;

            case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:

				input_report_gun_position( port, BTN_POINTER );

				{
					/* Special Reload Button */
					int btn_offscreen_shot = input_state_cb( port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD );

					/* Trigger ? */
					int btn_trigger = input_state_cb( port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER );
					S9xReportButton(MAKE_BUTTON(PAD_2, JUSTIFIER_TRIGGER), btn_trigger || btn_offscreen_shot);

					/* Start Button ? */
					int btn_start = input_state_cb( port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START );
					S9xReportButton(MAKE_BUTTON(PAD_2, JUSTIFIER_START), btn_start ? 1 : 0 );

					/* Aiming off-screen ? */
					int btn_offscreen = input_state_cb( port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN );
					S9xReportButton(MAKE_BUTTON(PAD_2, JUSTIFIER_OFFSCREEN), btn_offscreen || btn_offscreen_shot);
				}

				/* Second Gun? */
				if ( snes_devices[port+1] == RETRO_DEVICE_LIGHTGUN_JUSTIFIER_2 )
				{
					int second = port+1;

					input_report_gun_position( second, BTN_POINTER2 );

					/* Special Reload Button */
					int btn_offscreen_shot = input_state_cb( second, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD );

					/* Trigger ? */
					int btn_trigger = input_state_cb( second, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER );
					S9xReportButton(MAKE_BUTTON(PAD_3, JUSTIFIER_TRIGGER), btn_trigger || btn_offscreen_shot);

					/* Start Button ? */
					int btn_start = input_state_cb( second, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START );
					S9xReportButton(MAKE_BUTTON(PAD_3, JUSTIFIER_START), btn_start ? 1 : 0 );

					/* Aiming off-screen ? */
					int btn_offscreen = input_state_cb( second, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN );
					S9xReportButton(MAKE_BUTTON(PAD_3, JUSTIFIER_OFFSCREEN), btn_offscreen || btn_offscreen_shot);
				}

                break;

            case RETRO_DEVICE_LIGHTGUN_MACS_RIFLE:

 				input_report_gun_position( port, BTN_POINTER );

				{
					/* Trigger ? */
					int btn_trigger = input_state_cb( port, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER );
					S9xReportButton(MAKE_BUTTON(PAD_2, MACS_RIFLE_TRIGGER), btn_trigger);
				}

                break;

            case RETRO_DEVICE_NONE:
                break;

            default:
                if (log_cb)
                    log_cb(RETRO_LOG_ERROR, "Unknown device...\n");
        }
    }
}

void retro_run()
{
    static uint16 height = PPU.ScreenHeight;
    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        update_variables();

    if (g_geometry_update || height != PPU.ScreenHeight)
    {
        update_geometry();
        height = PPU.ScreenHeight;
    }

    int result = -1;
    bool okay = environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &result);
    if (okay)
    {
        bool audioEnabled = 0 != (result & 2);
        bool videoEnabled = 0 != (result & 1);
        IPPU.RenderThisFrame = videoEnabled;
        S9xSetSoundMute(!audioEnabled);
    }
    else
    {
        IPPU.RenderThisFrame = true;
        S9xSetSoundMute(false);
    }

    poll_cb();
    report_buttons();
    S9xMainLoop();
    S9xAudioCallback(NULL);
}

void retro_deinit()
{
    S9xDeinitAPU();
    Memory.Deinit();
    S9xGraphicsDeinit();
    S9xUnmapAllControls();

    free(screen_buffer);
    free(ntsc_screen_buffer);
}


unsigned retro_get_region()
{
    return Settings.PAL ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
}

void* retro_get_memory_data(unsigned type)
{
    void* data;

    switch(type)
    {
        case RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM:
        case RETRO_MEMORY_SAVE_RAM:
            data = Memory.SRAM;
            break;
        case RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM:
            data = Multi.sramB;
            break;
        case RETRO_MEMORY_RTC:
            data = RTCData.reg;
            break;
        case RETRO_MEMORY_SYSTEM_RAM:
        data = Memory.RAM;
        break;
        case RETRO_MEMORY_VIDEO_RAM:
        data = Memory.VRAM;
        break;
        //case RETRO_MEMORY_ROM:
        //	data = Memory.ROM;
        //	break;
        default:
            data = NULL;
            break;
    }

    return data;
}

size_t retro_get_memory_size(unsigned type)
{
    size_t size;

    switch(type) {
        case RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM:
        case RETRO_MEMORY_SAVE_RAM:
            size = (unsigned) (Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0);
            if (size > 0x20000)
            size = 0x20000;
            break;
        case RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM:
            size = (unsigned) (Multi.cartType==4 && Multi.sramSizeB ? (1 << (Multi.sramSizeB + 3)) * 128 : 0);
            break;
        case RETRO_MEMORY_RTC:
            size = (Settings.SRTC || Settings.SPC7110RTC)?20:0;
            break;
        case RETRO_MEMORY_SYSTEM_RAM:
            size = 128 * 1024;
            break;
        case RETRO_MEMORY_VIDEO_RAM:
            size = 64 * 1024;
            break;
        //case RETRO_MEMORY_ROM:
        //	size = Memory.CalculatedSize;
        //	break;
        default:
            size = 0;
            break;
    }

    return size;
}

size_t retro_serialize_size()
{
    return rom_loaded ? S9xFreezeSize() : 0;
}

bool retro_serialize(void *data, size_t size)
{
    int result = -1;
    bool okay = false;
    okay = environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &result);
    if (okay)
    {
        Settings.FastSavestates = 0 != (result & 4);
    }
    if (S9xFreezeGameMem((uint8_t*)data,size) == FALSE)
        return false;

    return true;
}

bool retro_unserialize(const void* data, size_t size)
{
    int result = -1;
    bool okay = false;
    okay = environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &result);
    if (okay)
    {
        Settings.FastSavestates = 0 != (result & 4);
    }
    if (S9xUnfreezeGameMem((const uint8_t*)data,size) != SUCCESS)
        return false;

    return true;
}

bool8 S9xDeinitUpdate(int width, int height)
{
    int overscan_offset = 0;

    if (crop_overscan_mode == OVERSCAN_CROP_ON)
    {
        if (height > SNES_HEIGHT * 2)
        {
            overscan_offset = 14;
            height = SNES_HEIGHT * 2;
        }
        else if ((height > SNES_HEIGHT) && (height != SNES_HEIGHT * 2))
        {
            overscan_offset = 7;
            height = SNES_HEIGHT;
        }
    }
    else if (crop_overscan_mode == OVERSCAN_CROP_OFF)
    {
        if (height > SNES_HEIGHT_EXTENDED)
        {
            if (height < SNES_HEIGHT_EXTENDED * 2)
            {
                overscan_offset = -16;
                memset(GFX.Screen + (GFX.Pitch >> 1) * height,0,GFX.Pitch * ((SNES_HEIGHT_EXTENDED << 1) - height));
            }
            height = SNES_HEIGHT_EXTENDED * 2;
        }
        else
        {
            if (height < SNES_HEIGHT_EXTENDED)
            {
                overscan_offset = -8;
                memset(GFX.Screen + (GFX.Pitch >> 1) * height,0,GFX.Pitch * (SNES_HEIGHT_EXTENDED - height));
            }
            height = SNES_HEIGHT_EXTENDED;
        }
    }


    if(blargg_filter)
    {
        if(width == 512)
            snes_ntsc_blit_hires(snes_ntsc, GFX.Screen, GFX.Pitch/2, 0, width, height, snes_ntsc_buffer, GFX.Pitch);
        else
            snes_ntsc_blit(snes_ntsc, GFX.Screen, GFX.Pitch/2, 0, width, height, snes_ntsc_buffer, GFX.Pitch);

        video_cb(snes_ntsc_buffer + ((int)(GFX.Pitch >> 1) * overscan_offset), MAX_SNES_WIDTH_NTSC, height, GFX.Pitch);
    }
    else if (width == MAX_SNES_WIDTH && hires_blend)
    {
        #define AVERAGE_565(el0, el1) (((el0) & (el1)) + ((((el0) ^ (el1)) & 0xF7DE) >> 1))

        if (hires_blend == 1) /* Blur method */
        {
            for (int y = 0; y < height; y++)
            {
                uint16 *input = (uint16 *) ((uint8 *) GFX.Screen + y * GFX.Pitch);
                uint16 *output = (uint16 *) ((uint8 *) GFX.Screen + y * GFX.Pitch);
                uint16 l, r;

                l = 0;
                for (int x = 0; x < (width >> 1); x++)
                {
                    r = *input++;
                    *output++ = AVERAGE_565 (l, r);
                    l = r;

                    r = *input++;
                    *output++ = AVERAGE_565 (l, r);
                    l = r;
                }
            }
        }
        else if (hires_blend == 2) /* Merge method */
        {
            for (int y = 0; y < height; y++)
            {
                uint16 *input = (uint16 *) ((uint8 *) GFX.Screen + y * GFX.Pitch);
                uint16 *output = (uint16 *) ((uint8 *) GFX.Screen + y * GFX.Pitch);
                uint16 l, r;

                for (int x = 0; x < (width >> 1); x++)
                {
                    l = *input++;
                    r = *input++;
                    *output++ = AVERAGE_565 (l, r);
                }
            }

            width >>= 1;
        }

        video_cb(GFX.Screen + ((int)(GFX.Pitch >> 1) * overscan_offset), width, height, GFX.Pitch);
    }
    else
    {
        video_cb(GFX.Screen + ((int)(GFX.Pitch >> 1) * overscan_offset), width, height, GFX.Pitch);
    }

    return TRUE;
}

bool8 S9xContinueUpdate(int width, int height)
{
    S9xDeinitUpdate(width, height);
    return true;
}

// Dummy functions that should probably be implemented correctly later.
void S9xParsePortConfig(ConfigFile&, int) {}
void S9xSyncSpeed() {}
const char* S9xStringInput(const char* in) { return in; }

#ifdef _WIN32
#define SLASH '\\'
#else
#define SLASH '/'
#endif

const char* S9xGetFilename(const char* in, s9x_getdirtype type)
{
    static char newpath[2048];

    newpath[0] = '\0';

    switch (type)
    {
        case ROMFILENAME_DIR:
            sprintf(newpath, "%s%c%s%s", g_rom_dir, SLASH, g_basename, in);
            return newpath;
        default:
            break;
    }

    return in;
}

const char* S9xGetDirectory(s9x_getdirtype type)
{
    switch (type)
    {
        case BIOS_DIR:
            return retro_system_directory;
        default:
            return g_rom_dir;
    }

    return "";
}
void S9xInitInputDevices() {}
const char* S9xChooseFilename(unsigned char) { return ""; }
void S9xHandlePortCommand(s9xcommand_t, short, short) {}
bool S9xPollButton(unsigned int, bool*) { return false; }
void S9xToggleSoundChannel(int) {}
const char* S9xGetFilenameInc(const char* in, s9x_getdirtype) { return ""; }
const char* S9xBasename(const char* in) { return in; }
bool8 S9xInitUpdate() { return TRUE; }
void S9xExtraUsage() {}
bool8 S9xOpenSoundDevice() { return TRUE; }
bool S9xPollAxis(unsigned int, short*) { return FALSE; }
void S9xSetPalette() {}
void S9xParseArg(char**, int&, int) {}
void S9xExit() {}
bool S9xPollPointer(unsigned int, short*, short*) { return false; }

const char *S9xChooseMovieFilename(unsigned char) { return NULL; }
void S9xMessage(int type, int, const char* s)
{
    if (!log_cb) return;

    switch (type)
    {
        case S9X_DEBUG:
            log_cb(RETRO_LOG_DEBUG, "%s\n", s);
            break;
        case S9X_WARNING:
            log_cb(RETRO_LOG_WARN, "%s\n", s);
            break;
        case S9X_INFO:
            log_cb(RETRO_LOG_INFO, "%s\n", s);
            break;
        case S9X_ERROR:
            log_cb(RETRO_LOG_ERROR, "%s\n", s);
            break;
        default:
            log_cb(RETRO_LOG_DEBUG, "%s\n", s);
            break;
    }
}

bool8 S9xOpenSnapshotFile(const char* filepath, bool8 read_only, STREAM *file)
{
    if(read_only)
    {
        if((*file = OPEN_STREAM(filepath, "rb")) != 0)
        {
            return (TRUE);
        }
    }
    else
    {
        if((*file = OPEN_STREAM(filepath, "wb")) != 0)
        {
            return (TRUE);
        }
    }
    return (FALSE);
}

void S9xCloseSnapshotFile(STREAM file)
{
    CLOSE_STREAM(file);
}

void S9xAutoSaveSRAM()
{
    return;
}

#ifndef __WIN32__
// S9x weirdness.
void _splitpath (const char *path, char *drive, char *dir, char *fname, char *ext)
{
    *drive = 0;

    const char	*slash = strrchr(path, SLASH_CHAR),
            *dot	= strrchr(path, '.');

    if (dot && slash && dot < slash)
        dot = NULL;

    if (!slash)
    {
        *dir = 0;

        strcpy(fname, path);

        if (dot)
        {
            fname[dot - path] = 0;
            strcpy(ext, dot + 1);
        }
        else
            *ext = 0;
    }
    else
    {
        strcpy(dir, path);
        dir[slash - path] = 0;

        strcpy(fname, slash + 1);

        if (dot)
        {
            fname[dot - slash - 1] = 0;
            strcpy(ext, dot + 1);
        }
        else
            *ext = 0;
    }
}

void _makepath (char *path, const char *, const char *dir, const char *fname, const char *ext)
{
    if (dir && *dir)
    {
        strcpy(path, dir);
        strcat(path, SLASH_STR);
    }
    else
        *path = 0;

    strcat(path, fname);

    if (ext && *ext)
    {
        strcat(path, ".");
        strcat(path, ext);
    }
}
#endif // __WIN32__
