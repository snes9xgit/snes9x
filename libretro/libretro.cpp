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
#include <stdio.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "../apu/bapu/dsp/SPC_DSP.h"

#define RETRO_DEVICE_JOYPAD_MULTITAP ((1 << 8) | RETRO_DEVICE_JOYPAD)
#define RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE ((1 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIER ((2 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIERS ((3 << 8) | RETRO_DEVICE_LIGHTGUN)

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
#define SNES_8_7 8.0f / 7.0f

char g_rom_dir[1024];
char g_basename[1024];
bool overclock_cycles = false;
bool sprite_flicker = false;

bool hires_blend = false;
static uint16 *gfx_blend;

char retro_system_directory[4096];
char retro_save_directory[4096];

retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_audio_sample_t audio_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_state_cb = NULL;

static int audio_interp_max = 32768;
static int audio_interp_mode = 2;

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
    ASPECT_RATIO_8_7,
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
        { "snes9x_overclock_superfx", "SuperFX Overclocking; 100%|150%|200%|250%|300%|350%|400%|450%|500%|50%" },
        { "snes9x_overclock_cycles", "CPU Overclock (Hack, Unsafe); disabled|enabled" },
        { "snes9x_sprite_flicker", "Reduce Flickering (Hack, Unsafe); disabled|enabled" },
        { "snes9x_hires_blend", "Hires Blending; disabled|enabled" },
        { "snes9x_audio_interpolation", "Audio Interpolation; gaussian|cubic|sinc|none|linear" },
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
        { "snes9x_aspect", "Preferred aspect ratio; 4:3|8:7|auto|ntsc|pal" },
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
    };

    static const struct retro_controller_description port_3[] = {
        { "None", RETRO_DEVICE_NONE },
        { "SNES Joypad", RETRO_DEVICE_JOYPAD },
    };

    static const struct retro_controller_info ports[] = {
        { port_1, 4 },
        { port_2, 6 },
        { port_3, 2 },
        { port_3, 2 },
        { port_3, 2 },
        { port_3, 2 },
        { port_3, 2 },
        { port_3, 2 },
        {},
    };

   environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

void update_geometry(void)
{
    struct retro_system_av_info av_info;
    retro_get_system_av_info(&av_info);
    environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
}

static void update_variables(void)
{
    bool geometry_update = false;
    char key[256];
    struct retro_variable var;

    var.key = "snes9x_hires_blend";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        hires_blend = !strcmp(var.value, "disabled") ? false : true;
    else
        hires_blend = false;

    var.key = "snes9x_overclock_superfx";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        int freq = atoi(var.value);
        Settings.SuperFXClockMultiplier = freq;
    }

    var.key = "snes9x_hires_blend";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        hires_blend = !strcmp(var.value, "disabled") ? false : true;
    else
        hires_blend = false;

    var.key = "snes9x_up_down_allowed";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        Settings.UpAndDown = !strcmp(var.value, "disabled") ? false : true;
    }
    else
        Settings.UpAndDown = false;

    var.key = "snes9x_audio_interpolation";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        int oldval = audio_interp_mode;

        if (strcmp(var.value, "none") == 0)
            audio_interp_mode = 0;
        else if (strcmp(var.value, "linear") == 0)
            audio_interp_mode = 1;
        else if (strcmp(var.value, "gaussian") == 0)
            audio_interp_mode = 2;
        else if (strcmp(var.value, "cubic") == 0)
            audio_interp_mode = 3;
        else if (strcmp(var.value, "sinc") == 0)
            audio_interp_mode = 4;

        if (oldval != audio_interp_mode)
            audio_interp_max = 32768;
    }

    var.key = "snes9x_overclock_cycles";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "enabled") == 0)
            overclock_cycles = true;
        else
            overclock_cycles = false;
    }

    var.key = "snes9x_sprite_flicker";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "enabled") == 0)
            sprite_flicker = true;
        else
            sprite_flicker = false;
    }

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
            geometry_update = true;
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
        else if (strcmp(var.value, "8:7") == 0)
            newval = ASPECT_RATIO_8_7;

        if (newval != aspect_ratio_mode)
        {
            aspect_ratio_mode = newval;
            geometry_update = true;
        }
    }

    if (geometry_update)
        update_geometry();
}

static void S9xAudioCallback(void*)
{
    const int BUFFER_SIZE = 256;
    // This is called every time 128 to 132 samples are generated, which happens about 8 times per frame.  A buffer size of 256 samples is enough here.
    static int16_t audio_buf[BUFFER_SIZE];

    S9xFinalizeSamples();
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
    else if (aspect_ratio_mode == ASPECT_RATIO_8_7)
    {
        return SNES_8_7;
    }

    // OV2: not sure if these really make sense - NTSC is similar to 4:3, PAL looks weird
    float sample_frequency_ntsc = 135000000.0f / 11.0f;
    float sample_frequency_pal = 14750000.0;

    float sample_freq = retro_get_region() == RETRO_REGION_NTSC ? sample_frequency_ntsc : sample_frequency_pal;
    float dot_rate = SNES::cpu.frequency / 4.0;

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

    float par = sample_freq / 2.0 / dot_rate;
    return (float)width * par / (float)height;
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
    info->geometry.max_width = MAX_SNES_WIDTH;
    info->geometry.max_height = MAX_SNES_HEIGHT;
    info->geometry.aspect_ratio = get_aspect_ratio(width, height);
    info->timing.sample_rate = 32040;
    info->timing.fps = retro_get_region() == RETRO_REGION_NTSC ? 21477272.0 / 357366.0 : 21281370.0 / 425568.0;
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
            case RETRO_DEVICE_NONE:
                S9xSetController(port, CTL_NONE, 0, 0, 0, 0);
                snes_devices[port] = RETRO_DEVICE_NONE;
                break;
            default:
                if (log_cb)
                    log_cb(RETRO_LOG_ERROR, "Invalid device (%d).\n", device);
                break;
        }
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

static bool8 LoadBIOS(uint8 *biosrom, char *biosname, int biossize)
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
        if (size == biossize)
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
            if (rom_loaded = LoadBIOS(biosrom,"STBIOS.bin",0x40000))
            rom_loaded = Memory.LoadMultiCartMem((const uint8_t*)game->data, game->size, 0, 0, biosrom, 0x40000);
        }

        else
        if ((is_bsx((uint8 *) game->data + 0x7fc0)==1) | (is_bsx((uint8 *) game->data + 0xffc0)==1)) {
            if (rom_loaded = LoadBIOS(biosrom,"BS-X.bin",0x100000))
            rom_loaded = Memory.LoadMultiCartMem(biosrom, 0x100000, (const uint8_t*)game->data, game->size, 0, 0);
        }

        else
            rom_loaded = Memory.LoadROMMem((const uint8_t*)game->data ,game->size);

        if(biosrom) delete[] biosrom;
    }

    int pixel_format = RGB555;
    if(environ_cb)
    {
        pixel_format = RGB565;
        enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
        if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
            pixel_format = RGB555;
    }
    S9xGraphicsDeinit();
    S9xSetRenderPixelFormat(pixel_format);
    S9xGraphicsInit();

    if (rom_loaded)
        update_geometry();

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

        if(log_cb) log_cb(RETRO_LOG_INFO,"[libretro]: ROM header removed\n");
    }

    if (multicart_sufami && (romptr + romsize) >= (romptr + 0x100000))
    {
        if (strncmp((const char*)(romptr + 0x100000), "BANDAI SFC-ADX", 14) == 0 &&
            strncmp((const char*)(romptr + 0x000000), "BANDAI SFC-ADX", 14) == 0)
        {
            romptr += 0x100000;
            romsize -= 0x100000;

            if(log_cb) log_cb(RETRO_LOG_INFO,"[libretro]: Sufami Turbo Multi-ROM bios removed\n");
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

                    if (LoadBIOS(biosptr,"STBIOS.bin",0x40000))
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
        int pixel_format = RGB555;
        if(environ_cb)
        {
            pixel_format = RGB565;
            enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
            if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
                pixel_format = RGB555;
        }
        S9xGraphicsDeinit();
        S9xSetRenderPixelFormat(pixel_format);
        S9xGraphicsInit();

        update_geometry();
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
    Settings.FrameTimePAL = 20000;
    Settings.FrameTimeNTSC = 16667;
    Settings.SixteenBitSound = TRUE;
    Settings.Stereo = TRUE;
    Settings.SoundPlaybackRate = 32040;
    Settings.SoundInputRate = 32040;
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

    //very slow devices will still pop

    //this needs to be applied to all snes9x cores

    //increasing the buffer size does not cause extra lag(tested with 1000ms buffer)
    //bool8 S9xInitSound (int buffer_ms, int lag_ms)

    S9xInitSound(32, 0); //give it a 1.9 frame long buffer, or 1026 samples.  The audio buffer is flushed every 128-132 samples anyway, so it won't get anywhere near there.

    S9xSetSoundMute(FALSE);
    S9xSetSamplesAvailableCallback(S9xAudioCallback, NULL);

    GFX.Pitch = MAX_SNES_WIDTH * sizeof(uint16);
    GFX.Screen = (uint16*) calloc(1, GFX.Pitch * MAX_SNES_HEIGHT);
    gfx_blend = (uint16*) calloc(1, GFX.Pitch * MAX_SNES_HEIGHT);
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

#define SCOPE_X RETRO_DEVICE_ID_SUPER_SCOPE_X
#define SCOPE_Y RETRO_DEVICE_ID_SUPER_SCOPE_Y
#define SCOPE_TRIGGER RETRO_DEVICE_ID_LIGHTGUN_TRIGGER
#define SCOPE_CURSOR RETRO_DEVICE_ID_LIGHTGUN_CURSOR
#define SCOPE_TURBO RETRO_DEVICE_ID_LIGHTGUN_TURBO
#define SCOPE_PAUSE RETRO_DEVICE_ID_LIGHTGUN_PAUSE
#define SCOPE_FIRST SCOPE_X
#define SCOPE_LAST SCOPE_PAUSE

#define JUSTIFIER_X RETRO_DEVICE_ID_JUSTIFIER_X
#define JUSTIFIER_Y RETRO_DEVICE_ID_JUSTIFIER_Y
#define JUSTIFIER_TRIGGER RETRO_DEVICE_ID_LIGHTGUN_TRIGGER
#define JUSTIFIER_OFFSCREEN RETRO_DEVICE_ID_LIGHTGUN_TURBO
#define JUSTIFIER_START RETRO_DEVICE_ID_LIGHTGUN_PAUSE
#define JUSTIFIER_FIRST JUSTIFIER_X
#define JUSTIFIER_LAST JUSTIFIER_START

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
    S9xMapPointer((BTN_POINTER), S9xGetCommandT("Pointer Mouse1+Superscope+Justifier1"), false);
    S9xMapPointer((BTN_POINTER2), S9xGetCommandT("Pointer Mouse2"), false);

    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_A), "Joypad2 A");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_B), "Joypad2 B");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_X), "Joypad2 X");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_Y), "Joypad2 Y");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_SELECT), "{Joypad2 Select,Mouse2 L,Superscope Fire,Justifier1 Trigger}");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_START), "{Joypad2 Start,Mouse2 R,Superscope Cursor,Justifier1 Start}");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_L), "Joypad2 L");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_R), "Joypad2 R");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_LEFT), "Joypad2 Left");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_RIGHT), "Joypad2 Right");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_UP), "{Joypad2 Up,Superscope ToggleTurbo,Justifier1 AimOffscreen}");
    MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_DOWN), "{Joypad2 Down,Superscope Pause}");

    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_A), "Joypad3 A");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_B), "Joypad3 B");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_X), "Joypad3 X");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_Y), "Joypad3 Y");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_SELECT), "Joypad3 Select");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_START), "Joypad3 Start");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_L), "Joypad3 L");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_R), "Joypad3 R");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_LEFT), "Joypad3 Left");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_RIGHT), "Joypad3 Right");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_UP), "Joypad3 Up");
    MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_DOWN), "Joypad3 Down");

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

// libretro uses relative values for analogue devices.
// S9x seems to use absolute values, but do convert these into relative values in the core. (Why?!)
// Hack around it. :)

static int16_t snes_mouse_state[2][2] = {{0}, {0}};
static int16_t snes_scope_state[2] = {0};
static int16_t snes_justifier_state[2][2] = {{0}, {0}};
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
                snes_scope_state[0] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE, 0, RETRO_DEVICE_ID_LIGHTGUN_X);
                snes_scope_state[1] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE, 0, RETRO_DEVICE_ID_LIGHTGUN_Y);
                if (snes_scope_state[0] < 0)
                    snes_scope_state[0] = 0;
                else if (snes_scope_state[0] > (SNES_WIDTH-1))
                    snes_scope_state[0] = SNES_WIDTH-1;
                if (snes_scope_state[1] < 0)
                    snes_scope_state[1] = 0;
                else if (snes_scope_state[1] > (SNES_HEIGHT-1))
                    snes_scope_state[1] = SNES_HEIGHT-1;

                S9xReportPointer(BTN_POINTER, snes_scope_state[0], snes_scope_state[1]);
                for (int i = SCOPE_TRIGGER; i <= SCOPE_LAST; i++)
                    S9xReportButton(MAKE_BUTTON(2, i), input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, i));
                break;

            case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:
            case RETRO_DEVICE_LIGHTGUN_JUSTIFIERS:
                snes_justifier_state[port][0] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_JUSTIFIER, 0, RETRO_DEVICE_ID_LIGHTGUN_X);
                snes_justifier_state[port][1] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_JUSTIFIER, 0, RETRO_DEVICE_ID_LIGHTGUN_Y);

                if (snes_justifier_state[port][0] < 0)
                    snes_justifier_state[port][0] = 0;
                else if (snes_justifier_state[port][0] > (SNES_WIDTH-1))
                    snes_justifier_state[port][0] = SNES_WIDTH-1;
                if (snes_justifier_state[port][1] < 0)
                    snes_justifier_state[port][1] = 0;
                else if (snes_justifier_state[port][1] > (SNES_HEIGHT-1))
                    snes_justifier_state[port][1] = SNES_HEIGHT-1;

                S9xReportPointer(BTN_POINTER, snes_justifier_state[port][0], snes_justifier_state[port][1]);
                for (int i = JUSTIFIER_TRIGGER; i <= JUSTIFIER_LAST; i++)
                    S9xReportButton(MAKE_BUTTON(2, i), input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, i));
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
    if (height != PPU.ScreenHeight)
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
        bool hardDisableAudio = 0 != (result & 8);
        IPPU.RenderThisFrame = videoEnabled;
        S9xSetSoundMute(!audioEnabled || hardDisableAudio);
        Settings.HardDisableAudio = hardDisableAudio;
    }
    else
    {
        IPPU.RenderThisFrame = true;
        S9xSetSoundMute(false);
        Settings.HardDisableAudio = false;
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

    free(GFX.Screen);
    free(gfx_blend);
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
    if (crop_overscan_mode == OVERSCAN_CROP_ON)
    {
        if (height >= SNES_HEIGHT << 1)
        {
            height = SNES_HEIGHT << 1;
        }
        else
        {
            height = SNES_HEIGHT;
        }
    }
    else if (crop_overscan_mode == OVERSCAN_CROP_OFF)
    {
        if (height > SNES_HEIGHT_EXTENDED)
        {
            if (height < SNES_HEIGHT_EXTENDED << 1)
            memset(GFX.Screen + (GFX.Pitch >> 1) * height,0,GFX.Pitch * ((SNES_HEIGHT_EXTENDED << 1) - height));
            height = SNES_HEIGHT_EXTENDED << 1;
        }
        else
        {
            if (height < SNES_HEIGHT_EXTENDED)
            memset(GFX.Screen + (GFX.Pitch >> 1) * height,0,GFX.Pitch * (SNES_HEIGHT_EXTENDED - height));
            height = SNES_HEIGHT_EXTENDED;
        }
    }


        if(width==MAX_SNES_WIDTH && hires_blend)
        {
#define AVERAGE_565(el0, el1) (((el0) & (el1)) + ((((el0) ^ (el1)) & 0xF7DE) >> 1))
        for (register int y = 0; y < height; y++)
        {
            register uint16 *input = (uint16 *) ((uint8 *) GFX.Screen + y * GFX.Pitch);
            register uint16 *output = (uint16 *) ((uint8 *) gfx_blend + y * GFX.Pitch);
            register uint16 l, r;

            l = 0;
            for (register int x = 0; x < (width >> 1); x++)
            {
            r = *input++;
            *output++ = AVERAGE_565 (l, r);
            l = r;

            r = *input++;
            *output++ = AVERAGE_565 (l, r);
            l = r;
            }
        }

        video_cb(gfx_blend, width, height, GFX.Pitch);
    }
    else
    { 
        video_cb(GFX.Screen, width, height, GFX.Pitch);
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
        case ROMFILENAME_DIR:
            return g_rom_dir;
        case BIOS_DIR:
            return retro_system_directory;
        default:
            break;
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

// interpolation by Mudlord
static short const cubic [514] =
{
   0,  -4,  -8, -12, -16, -20, -23, -27, -30, -34, -37, -41, -44, -47, -50, -53,
 -56, -59, -62, -65, -68, -71, -73, -76, -78, -81, -84, -87, -89, -91, -93, -95,
 -98,-100,-102,-104,-106,-109,-110,-112,-113,-116,-117,-119,-121,-122,-123,-125,
-126,-128,-129,-131,-132,-134,-134,-136,-136,-138,-138,-140,-141,-141,-142,-143,
-144,-144,-145,-146,-147,-148,-147,-148,-148,-149,-149,-150,-150,-150,-150,-151,
-151,-151,-151,-151,-152,-152,-151,-152,-151,-152,-151,-151,-151,-151,-150,-150,
-150,-149,-149,-149,-149,-148,-147,-147,-146,-146,-145,-145,-144,-144,-143,-142,
-141,-141,-140,-139,-139,-138,-137,-136,-135,-135,-133,-133,-132,-131,-130,-129,
-128,-127,-126,-125,-124,-123,-121,-121,-119,-118,-117,-116,-115,-114,-112,-111,
-110,-109,-107,-106,-105,-104,-102,-102,-100, -99, -97, -97, -95, -94, -92, -91,
 -90, -88, -87, -86, -85, -84, -82, -81, -79, -78, -76, -76, -74, -73, -71, -70,
 -68, -67, -66, -65, -63, -62, -60, -60, -58, -57, -55, -55, -53, -52, -50, -49,
 -48, -46, -45, -44, -43, -42, -40, -39, -38, -37, -36, -35, -34, -32, -31, -30,
 -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -19, -17, -16, -15, -14,
 -14, -13, -12, -11, -11, -10,  -9,  -9,  -8,  -8,  -7,  -7,  -6,  -5,  -4,  -4,
  -3,  -3,  -3,  -2,  -2,  -2,  -1,  -1,   0,  -1,   0,  -1,   0,   0,   0,   0,
   0,
2048,2048,2048,2048,2047,2047,2046,2045,2043,2042,2041,2039,2037,2035,2033,2031,
2028,2026,2024,2021,2018,2015,2012,2009,2005,2002,1999,1995,1991,1987,1982,1978,
1974,1969,1965,1960,1955,1951,1946,1940,1934,1929,1924,1918,1912,1906,1900,1895,
1888,1882,1875,1869,1862,1856,1849,1842,1835,1828,1821,1814,1806,1799,1791,1783,
1776,1768,1760,1753,1744,1737,1728,1720,1711,1703,1695,1686,1677,1668,1659,1651,
1641,1633,1623,1614,1605,1596,1587,1577,1567,1559,1549,1539,1529,1520,1510,1499,
1490,1480,1470,1460,1450,1440,1430,1420,1408,1398,1389,1378,1367,1357,1346,1336,
1325,1315,1304,1293,1282,1272,1261,1250,1239,1229,1218,1207,1196,1185,1174,1163,
1152,1141,1130,1119,1108,1097,1086,1075,1063,1052,1042,1030,1019,1008, 997, 986,
 974, 964, 952, 941, 930, 919, 908, 897, 886, 875, 864, 853, 842, 831, 820, 809,
 798, 787, 776, 765, 754, 744, 733, 722, 711, 700, 690, 679, 668, 658, 647, 637,
 626, 616, 605, 595, 584, 574, 564, 554, 543, 534, 524, 514, 503, 494, 483, 473,
 464, 454, 444, 435, 425, 416, 407, 397, 387, 378, 370, 360, 351, 342, 333, 325,
 315, 307, 298, 290, 281, 273, 265, 256, 248, 241, 233, 225, 216, 209, 201, 193,
 186, 178, 171, 164, 157, 150, 143, 137, 129, 123, 117, 110, 103,  97,  91,  85,
  79,  74,  68,  62,  56,  51,  46,  41,  35,  31,  27,  22,  17,  13,   8,   4,
   0
};

static short const sinc [2048] =
{
    39,  -315,   666, 15642,   666,  -315,    39,   -38,
    38,  -302,   613, 15642,   718,  -328,    41,   -38,
    36,  -288,   561, 15641,   772,  -342,    42,   -38,
    35,  -275,   510, 15639,   826,  -355,    44,   -38,
    33,  -263,   459, 15636,   880,  -369,    46,   -38,
    32,  -250,   408, 15632,   935,  -383,    47,   -38,
    31,  -237,   358, 15628,   990,  -396,    49,   -38,
    29,  -224,   309, 15622,  1046,  -410,    51,   -38,
    28,  -212,   259, 15616,  1103,  -425,    53,   -38,
    27,  -200,   211, 15609,  1159,  -439,    54,   -38,
    25,  -188,   163, 15601,  1216,  -453,    56,   -38,
    24,  -175,   115, 15593,  1274,  -467,    58,   -38,
    23,  -164,    68, 15583,  1332,  -482,    60,   -38,
    22,  -152,    22, 15573,  1391,  -496,    62,   -37,
    21,  -140,   -24, 15562,  1450,  -511,    64,   -37,
    19,  -128,   -70, 15550,  1509,  -526,    66,   -37,
    18,  -117,  -115, 15538,  1569,  -540,    68,   -37,
    17,  -106,  -159, 15524,  1629,  -555,    70,   -37,
    16,   -94,  -203, 15510,  1690,  -570,    72,   -36,
    15,   -83,  -247, 15495,  1751,  -585,    74,   -36,
    14,   -72,  -289, 15479,  1813,  -600,    76,   -36,
    13,   -62,  -332, 15462,  1875,  -616,    79,   -36,
    12,   -51,  -374, 15445,  1937,  -631,    81,   -35,
    11,   -40,  -415, 15426,  2000,  -646,    83,   -35,
    11,   -30,  -456, 15407,  2063,  -662,    85,   -35,
    10,   -20,  -496, 15387,  2127,  -677,    88,   -34,
     9,    -9,  -536, 15366,  2191,  -693,    90,   -34,
     8,     1,  -576, 15345,  2256,  -708,    92,   -34,
     7,    10,  -614, 15323,  2321,  -724,    95,   -33,
     7,    20,  -653, 15300,  2386,  -740,    97,   -33,
     6,    30,  -690, 15276,  2451,  -755,    99,   -33,
     5,    39,  -728, 15251,  2517,  -771,   102,   -32,
     5,    49,  -764, 15226,  2584,  -787,   104,   -32,
     4,    58,  -801, 15200,  2651,  -803,   107,   -32,
     3,    67,  -836, 15173,  2718,  -819,   109,   -31,
     3,    76,  -871, 15145,  2785,  -835,   112,   -31,
     2,    85,  -906, 15117,  2853,  -851,   115,   -30,
     2,    93,  -940, 15087,  2921,  -867,   117,   -30,
     1,   102,  -974, 15057,  2990,  -883,   120,   -29,
     1,   110, -1007, 15027,  3059,  -899,   122,   -29,
     0,   118, -1039, 14995,  3128,  -915,   125,   -29,
     0,   127, -1071, 14963,  3198,  -931,   128,   -28,
    -1,   135, -1103, 14930,  3268,  -948,   131,   -28,
    -1,   142, -1134, 14896,  3338,  -964,   133,   -27,
    -1,   150, -1164, 14862,  3409,  -980,   136,   -27,
    -2,   158, -1194, 14827,  3480,  -996,   139,   -26,
    -2,   165, -1224, 14791,  3551, -1013,   142,   -26,
    -3,   172, -1253, 14754,  3622, -1029,   144,   -25,
    -3,   179, -1281, 14717,  3694, -1045,   147,   -25,
    -3,   187, -1309, 14679,  3766, -1062,   150,   -24,
    -3,   193, -1337, 14640,  3839, -1078,   153,   -24,
    -4,   200, -1363, 14601,  3912, -1094,   156,   -23,
    -4,   207, -1390, 14561,  3985, -1110,   159,   -23,
    -4,   213, -1416, 14520,  4058, -1127,   162,   -22,
    -4,   220, -1441, 14479,  4131, -1143,   165,   -22,
    -4,   226, -1466, 14437,  4205, -1159,   168,   -22,
    -5,   232, -1490, 14394,  4279, -1175,   171,   -21,
    -5,   238, -1514, 14350,  4354, -1192,   174,   -21,
    -5,   244, -1537, 14306,  4428, -1208,   177,   -20,
    -5,   249, -1560, 14261,  4503, -1224,   180,   -20,
    -5,   255, -1583, 14216,  4578, -1240,   183,   -19,
    -5,   260, -1604, 14169,  4653, -1256,   186,   -19,
    -5,   265, -1626, 14123,  4729, -1272,   189,   -18,
    -5,   271, -1647, 14075,  4805, -1288,   192,   -18,
    -5,   276, -1667, 14027,  4881, -1304,   195,   -17,
    -6,   280, -1687, 13978,  4957, -1320,   198,   -17,
    -6,   285, -1706, 13929,  5033, -1336,   201,   -16,
    -6,   290, -1725, 13879,  5110, -1352,   204,   -16,
    -6,   294, -1744, 13829,  5186, -1368,   207,   -15,
    -6,   299, -1762, 13777,  5263, -1383,   210,   -15,
    -6,   303, -1779, 13726,  5340, -1399,   213,   -14,
    -6,   307, -1796, 13673,  5418, -1414,   216,   -14,
    -6,   311, -1813, 13620,  5495, -1430,   219,   -13,
    -5,   315, -1829, 13567,  5573, -1445,   222,   -13,
    -5,   319, -1844, 13512,  5651, -1461,   225,   -13,
    -5,   322, -1859, 13458,  5728, -1476,   229,   -12,
    -5,   326, -1874, 13402,  5806, -1491,   232,   -12,
    -5,   329, -1888, 13347,  5885, -1506,   235,   -11,
    -5,   332, -1902, 13290,  5963, -1521,   238,   -11,
    -5,   335, -1915, 13233,  6041, -1536,   241,   -10,
    -5,   338, -1928, 13176,  6120, -1551,   244,   -10,
    -5,   341, -1940, 13118,  6199, -1566,   247,   -10,
    -5,   344, -1952, 13059,  6277, -1580,   250,    -9,
    -5,   347, -1964, 13000,  6356, -1595,   253,    -9,
    -5,   349, -1975, 12940,  6435, -1609,   256,    -8,
    -4,   352, -1986, 12880,  6514, -1623,   259,    -8,
    -4,   354, -1996, 12819,  6594, -1637,   262,    -8,
    -4,   356, -2005, 12758,  6673, -1651,   265,    -7,
    -4,   358, -2015, 12696,  6752, -1665,   268,    -7,
    -4,   360, -2024, 12634,  6831, -1679,   271,    -7,
    -4,   362, -2032, 12572,  6911, -1693,   274,    -6,
    -4,   364, -2040, 12509,  6990, -1706,   277,    -6,
    -4,   366, -2048, 12445,  7070, -1719,   280,    -6,
    -3,   367, -2055, 12381,  7149, -1732,   283,    -5,
    -3,   369, -2062, 12316,  7229, -1745,   286,    -5,
    -3,   370, -2068, 12251,  7308, -1758,   289,    -5,
    -3,   371, -2074, 12186,  7388, -1771,   291,    -4,
    -3,   372, -2079, 12120,  7467, -1784,   294,    -4,
    -3,   373, -2084, 12054,  7547, -1796,   297,    -4,
    -3,   374, -2089, 11987,  7626, -1808,   300,    -4,
    -2,   375, -2094, 11920,  7706, -1820,   303,    -3,
    -2,   376, -2098, 11852,  7785, -1832,   305,    -3,
    -2,   376, -2101, 11785,  7865, -1844,   308,    -3,
    -2,   377, -2104, 11716,  7944, -1855,   311,    -3,
    -2,   377, -2107, 11647,  8024, -1866,   313,    -2,
    -2,   378, -2110, 11578,  8103, -1877,   316,    -2,
    -2,   378, -2112, 11509,  8182, -1888,   318,    -2,
    -1,   378, -2113, 11439,  8262, -1899,   321,    -2,
    -1,   378, -2115, 11369,  8341, -1909,   323,    -2,
    -1,   378, -2116, 11298,  8420, -1920,   326,    -2,
    -1,   378, -2116, 11227,  8499, -1930,   328,    -1,
    -1,   378, -2116, 11156,  8578, -1940,   331,    -1,
    -1,   378, -2116, 11084,  8656, -1949,   333,    -1,
    -1,   377, -2116, 11012,  8735, -1959,   335,    -1,
    -1,   377, -2115, 10940,  8814, -1968,   337,    -1,
    -1,   377, -2114, 10867,  8892, -1977,   340,    -1,
    -1,   376, -2112, 10795,  8971, -1985,   342,    -1,
     0,   375, -2111, 10721,  9049, -1994,   344,    -1,
     0,   375, -2108, 10648,  9127, -2002,   346,     0,
     0,   374, -2106, 10574,  9205, -2010,   348,     0,
     0,   373, -2103, 10500,  9283, -2018,   350,     0,
     0,   372, -2100, 10426,  9360, -2025,   352,     0,
     0,   371, -2097, 10351,  9438, -2032,   354,     0,
     0,   370, -2093, 10276,  9515, -2039,   355,     0,
     0,   369, -2089, 10201,  9592, -2046,   357,     0,
     0,   367, -2084, 10126,  9669, -2052,   359,     0,
     0,   366, -2080, 10050,  9745, -2058,   360,     0,
     0,   365, -2075,  9974,  9822, -2064,   362,     0,
     0,   363, -2070,  9898,  9898, -2070,   363,     0,
     0,   362, -2064,  9822,  9974, -2075,   365,     0,
     0,   360, -2058,  9745, 10050, -2080,   366,     0,
     0,   359, -2052,  9669, 10126, -2084,   367,     0,
     0,   357, -2046,  9592, 10201, -2089,   369,     0,
     0,   355, -2039,  9515, 10276, -2093,   370,     0,
     0,   354, -2032,  9438, 10351, -2097,   371,     0,
     0,   352, -2025,  9360, 10426, -2100,   372,     0,
     0,   350, -2018,  9283, 10500, -2103,   373,     0,
     0,   348, -2010,  9205, 10574, -2106,   374,     0,
     0,   346, -2002,  9127, 10648, -2108,   375,     0,
    -1,   344, -1994,  9049, 10721, -2111,   375,     0,
    -1,   342, -1985,  8971, 10795, -2112,   376,    -1,
    -1,   340, -1977,  8892, 10867, -2114,   377,    -1,
    -1,   337, -1968,  8814, 10940, -2115,   377,    -1,
    -1,   335, -1959,  8735, 11012, -2116,   377,    -1,
    -1,   333, -1949,  8656, 11084, -2116,   378,    -1,
    -1,   331, -1940,  8578, 11156, -2116,   378,    -1,
    -1,   328, -1930,  8499, 11227, -2116,   378,    -1,
    -2,   326, -1920,  8420, 11298, -2116,   378,    -1,
    -2,   323, -1909,  8341, 11369, -2115,   378,    -1,
    -2,   321, -1899,  8262, 11439, -2113,   378,    -1,
    -2,   318, -1888,  8182, 11509, -2112,   378,    -2,
    -2,   316, -1877,  8103, 11578, -2110,   378,    -2,
    -2,   313, -1866,  8024, 11647, -2107,   377,    -2,
    -3,   311, -1855,  7944, 11716, -2104,   377,    -2,
    -3,   308, -1844,  7865, 11785, -2101,   376,    -2,
    -3,   305, -1832,  7785, 11852, -2098,   376,    -2,
    -3,   303, -1820,  7706, 11920, -2094,   375,    -2,
    -4,   300, -1808,  7626, 11987, -2089,   374,    -3,
    -4,   297, -1796,  7547, 12054, -2084,   373,    -3,
    -4,   294, -1784,  7467, 12120, -2079,   372,    -3,
    -4,   291, -1771,  7388, 12186, -2074,   371,    -3,
    -5,   289, -1758,  7308, 12251, -2068,   370,    -3,
    -5,   286, -1745,  7229, 12316, -2062,   369,    -3,
    -5,   283, -1732,  7149, 12381, -2055,   367,    -3,
    -6,   280, -1719,  7070, 12445, -2048,   366,    -4,
    -6,   277, -1706,  6990, 12509, -2040,   364,    -4,
    -6,   274, -1693,  6911, 12572, -2032,   362,    -4,
    -7,   271, -1679,  6831, 12634, -2024,   360,    -4,
    -7,   268, -1665,  6752, 12696, -2015,   358,    -4,
    -7,   265, -1651,  6673, 12758, -2005,   356,    -4,
    -8,   262, -1637,  6594, 12819, -1996,   354,    -4,
    -8,   259, -1623,  6514, 12880, -1986,   352,    -4,
    -8,   256, -1609,  6435, 12940, -1975,   349,    -5,
    -9,   253, -1595,  6356, 13000, -1964,   347,    -5,
    -9,   250, -1580,  6277, 13059, -1952,   344,    -5,
   -10,   247, -1566,  6199, 13118, -1940,   341,    -5,
   -10,   244, -1551,  6120, 13176, -1928,   338,    -5,
   -10,   241, -1536,  6041, 13233, -1915,   335,    -5,
   -11,   238, -1521,  5963, 13290, -1902,   332,    -5,
   -11,   235, -1506,  5885, 13347, -1888,   329,    -5,
   -12,   232, -1491,  5806, 13402, -1874,   326,    -5,
   -12,   229, -1476,  5728, 13458, -1859,   322,    -5,
   -13,   225, -1461,  5651, 13512, -1844,   319,    -5,
   -13,   222, -1445,  5573, 13567, -1829,   315,    -5,
   -13,   219, -1430,  5495, 13620, -1813,   311,    -6,
   -14,   216, -1414,  5418, 13673, -1796,   307,    -6,
   -14,   213, -1399,  5340, 13726, -1779,   303,    -6,
   -15,   210, -1383,  5263, 13777, -1762,   299,    -6,
   -15,   207, -1368,  5186, 13829, -1744,   294,    -6,
   -16,   204, -1352,  5110, 13879, -1725,   290,    -6,
   -16,   201, -1336,  5033, 13929, -1706,   285,    -6,
   -17,   198, -1320,  4957, 13978, -1687,   280,    -6,
   -17,   195, -1304,  4881, 14027, -1667,   276,    -5,
   -18,   192, -1288,  4805, 14075, -1647,   271,    -5,
   -18,   189, -1272,  4729, 14123, -1626,   265,    -5,
   -19,   186, -1256,  4653, 14169, -1604,   260,    -5,
   -19,   183, -1240,  4578, 14216, -1583,   255,    -5,
   -20,   180, -1224,  4503, 14261, -1560,   249,    -5,
   -20,   177, -1208,  4428, 14306, -1537,   244,    -5,
   -21,   174, -1192,  4354, 14350, -1514,   238,    -5,
   -21,   171, -1175,  4279, 14394, -1490,   232,    -5,
   -22,   168, -1159,  4205, 14437, -1466,   226,    -4,
   -22,   165, -1143,  4131, 14479, -1441,   220,    -4,
   -22,   162, -1127,  4058, 14520, -1416,   213,    -4,
   -23,   159, -1110,  3985, 14561, -1390,   207,    -4,
   -23,   156, -1094,  3912, 14601, -1363,   200,    -4,
   -24,   153, -1078,  3839, 14640, -1337,   193,    -3,
   -24,   150, -1062,  3766, 14679, -1309,   187,    -3,
   -25,   147, -1045,  3694, 14717, -1281,   179,    -3,
   -25,   144, -1029,  3622, 14754, -1253,   172,    -3,
   -26,   142, -1013,  3551, 14791, -1224,   165,    -2,
   -26,   139,  -996,  3480, 14827, -1194,   158,    -2,
   -27,   136,  -980,  3409, 14862, -1164,   150,    -1,
   -27,   133,  -964,  3338, 14896, -1134,   142,    -1,
   -28,   131,  -948,  3268, 14930, -1103,   135,    -1,
   -28,   128,  -931,  3198, 14963, -1071,   127,     0,
   -29,   125,  -915,  3128, 14995, -1039,   118,     0,
   -29,   122,  -899,  3059, 15027, -1007,   110,     1,
   -29,   120,  -883,  2990, 15057,  -974,   102,     1,
   -30,   117,  -867,  2921, 15087,  -940,    93,     2,
   -30,   115,  -851,  2853, 15117,  -906,    85,     2,
   -31,   112,  -835,  2785, 15145,  -871,    76,     3,
   -31,   109,  -819,  2718, 15173,  -836,    67,     3,
   -32,   107,  -803,  2651, 15200,  -801,    58,     4,
   -32,   104,  -787,  2584, 15226,  -764,    49,     5,
   -32,   102,  -771,  2517, 15251,  -728,    39,     5,
   -33,    99,  -755,  2451, 15276,  -690,    30,     6,
   -33,    97,  -740,  2386, 15300,  -653,    20,     7,
   -33,    95,  -724,  2321, 15323,  -614,    10,     7,
   -34,    92,  -708,  2256, 15345,  -576,     1,     8,
   -34,    90,  -693,  2191, 15366,  -536,    -9,     9,
   -34,    88,  -677,  2127, 15387,  -496,   -20,    10,
   -35,    85,  -662,  2063, 15407,  -456,   -30,    11,
   -35,    83,  -646,  2000, 15426,  -415,   -40,    11,
   -35,    81,  -631,  1937, 15445,  -374,   -51,    12,
   -36,    79,  -616,  1875, 15462,  -332,   -62,    13,
   -36,    76,  -600,  1813, 15479,  -289,   -72,    14,
   -36,    74,  -585,  1751, 15495,  -247,   -83,    15,
   -36,    72,  -570,  1690, 15510,  -203,   -94,    16,
   -37,    70,  -555,  1629, 15524,  -159,  -106,    17,
   -37,    68,  -540,  1569, 15538,  -115,  -117,    18,
   -37,    66,  -526,  1509, 15550,   -70,  -128,    19,
   -37,    64,  -511,  1450, 15562,   -24,  -140,    21,
   -37,    62,  -496,  1391, 15573,    22,  -152,    22,
   -38,    60,  -482,  1332, 15583,    68,  -164,    23,
   -38,    58,  -467,  1274, 15593,   115,  -175,    24,
   -38,    56,  -453,  1216, 15601,   163,  -188,    25,
   -38,    54,  -439,  1159, 15609,   211,  -200,    27,
   -38,    53,  -425,  1103, 15616,   259,  -212,    28,
   -38,    51,  -410,  1046, 15622,   309,  -224,    29,
   -38,    49,  -396,   990, 15628,   358,  -237,    31,
   -38,    47,  -383,   935, 15632,   408,  -250,    32,
   -38,    46,  -369,   880, 15636,   459,  -263,    33,
   -38,    44,  -355,   826, 15639,   510,  -275,    35,
   -38,    42,  -342,   772, 15641,   561,  -288,    36,
   -38,    41,  -328,   718, 15642,   613,  -302,    38,
};

bool8 libretro_get_snes_interp()
{
    if(audio_interp_mode != 2) return true;
    return false;
}

#define CLAMP16( io )\
{\
    if ( (int16_t) io != io )\
        io = (io >> 31) ^ 0x7FFF;\
}

int libretro_snes_interp(void *ptr)
{
    SNES::SPC_DSP::voice_t const* v = (SNES::SPC_DSP::voice_t const*) ptr;
    int out = 0;

    switch(audio_interp_mode) {
        // none
        case 0:
            out = v->buf [(v->interp_pos >> 12) + v->buf_pos] & ~1;
            break;

        // linear
        case 1: {
            int fract = v->interp_pos & 0xFFF;

            int const* in = &v->buf [(v->interp_pos >> 12) + v->buf_pos];
            out  = (0x1000 - fract) * in [0];
            out +=           fract  * in [1];
            out >>= 12;
            break;
        }

        // gaussian
        case 2:
            break;

        // cubic
        case 3: {
            // Make pointers into cubic based on fractional position between samples
            int offset = v->interp_pos >> 4 & 0xFF;
            short const* fwd = cubic       + offset;
            short const* rev = cubic + 256 - offset; // mirror left half of cubic

            int const* in = &v->buf [(v->interp_pos >> 12) + v->buf_pos];
            out  = fwd [  0] * in [0];
            out += fwd [257] * in [1];
            out += rev [257] * in [2];
            out += rev [  0] * in [3];
            out >>= 11;
            break;
        }

        // sinc
        case 4: {
            // Make pointers into cubic based on fractional position between samples
            int offset = v->interp_pos & 0xFF0;
            short const* filt = (short const*) (((char const*)sinc) + offset);

            int const* in = &v->buf [(v->interp_pos >> 12) + v->buf_pos];
            out  = filt [0] * in [0];
            out += filt [1] * in [1];
            out += filt [2] * in [2];
            out += filt [3] * in [3];
            out += filt [4] * in [4];
            out += filt [5] * in [5];
            out += filt [6] * in [6];
            out += filt [7] * in [7];
            out >>= 14;
            break;
	    }
    }

    int out_max = out;
    out *= 32768;
    out /= audio_interp_max;

    if( out > 32767 )
    {
        //if(log_cb) log_cb(RETRO_LOG_INFO,"audio max = %X\n",out_max);
        audio_interp_max = out_max;
        out = 32767;
    }

    else if( out < -32768 )
    {
        //if(log_cb) log_cb(RETRO_LOG_INFO,"audio max = %X\n",out_max);
        audio_interp_max = -out_max;
        out = -32768;
    }

    return out;
}
