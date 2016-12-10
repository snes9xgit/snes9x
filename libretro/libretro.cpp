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
#ifndef __WIN32__
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define RETRO_DEVICE_JOYPAD_MULTITAP        ((1 << 8) | RETRO_DEVICE_JOYPAD)
#define RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE   ((1 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIER     ((2 << 8) | RETRO_DEVICE_LIGHTGUN)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIERS    ((3 << 8) | RETRO_DEVICE_LIGHTGUN)

#define RETRO_MEMORY_SNES_BSX_RAM             ((1 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_BSX_PRAM            ((2 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM  ((3 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM  ((4 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_GAME_BOY_RAM        ((5 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_GAME_BOY_RTC        ((6 << 8) | RETRO_MEMORY_RTC)

#define RETRO_GAME_TYPE_BSX             0x101
#define RETRO_GAME_TYPE_BSX_SLOTTED     0x102
#define RETRO_GAME_TYPE_SUFAMI_TURBO    0x103
#define RETRO_GAME_TYPE_SUPER_GAME_BOY  0x104

static retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_audio_sample_t audio_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_state_cb = NULL;

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

static retro_environment_t environ_cb;
static bool use_overscan = false;
static bool rom_loaded = false;
void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   const struct retro_variable variables[] = {
      // These variable names and possible values constitute an ABI with ZMZ (ZSNES Libretro player).
      // Changing "Show layer 1" is fine, but don't change "layer_1"/etc or the possible values ("Yes|No").
      // Adding more variables and rearranging them is safe.
      { "snes9x_layer_1", "Show layer 1; Yes|No" },
      { "snes9x_layer_2", "Show layer 2; Yes|No" },
      { "snes9x_layer_3", "Show layer 3; Yes|No" },
      { "snes9x_layer_4", "Show layer 4; Yes|No" },
      { "snes9x_layer_5", "Show sprite layer; Yes|No" },
      { "snes9x_gfx_clip", "Enable graphic clip windows; Yes|No" },
      { "snes9x_gfx_transp", "Enable transparency effects; Yes|No" },
      { "snes9x_sndchan_1", "Enable sound channel 1; Yes|No" },
      { "snes9x_sndchan_2", "Enable sound channel 2; Yes|No" },
      { "snes9x_sndchan_3", "Enable sound channel 3; Yes|No" },
      { "snes9x_sndchan_4", "Enable sound channel 4; Yes|No" },
      { "snes9x_sndchan_5", "Enable sound channel 5; Yes|No" },
      { "snes9x_sndchan_6", "Enable sound channel 6; Yes|No" },
      { "snes9x_sndchan_7", "Enable sound channel 7; Yes|No" },
      { "snes9x_sndchan_8", "Enable sound channel 8; Yes|No" },
      { NULL, NULL },
   };

   environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void *)variables);

   const struct retro_controller_description port_1[] = {
      { "SNES Joypad", RETRO_DEVICE_JOYPAD },
      { "SNES Mouse", RETRO_DEVICE_MOUSE },
      { "Multitap", RETRO_DEVICE_JOYPAD_MULTITAP },
   };

   const struct retro_controller_description port_2[] = {
      { "SNES Joypad", RETRO_DEVICE_JOYPAD },
      { "SNES Mouse", RETRO_DEVICE_MOUSE },
      { "Multitap", RETRO_DEVICE_JOYPAD_MULTITAP },
      { "SuperScope", RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE },
      { "Justifier", RETRO_DEVICE_LIGHTGUN_JUSTIFIER },
   };

   const struct retro_controller_info ports[] = {
      { port_1, 3 },
      { port_2, 5 },
      { 0 },
   };

   environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

static void update_variables(void)
{
   char key[256];
   struct retro_variable var;

   var.key=key;

   int disabled_channels=0;
   strcpy(key, "snes9x_sndchan_x");
   for (int i=0;i<8;i++)
   {
      key[strlen("snes9x_sndchan_")]='1'+i;
      var.value=NULL;
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value && var.value[0]=='N') disabled_channels|=1<<i;
   }
   S9xSetSoundControl(disabled_channels^0xFF);

   int disabled_layers=0;
   strcpy(key, "snes9x_layer_x");
   for (int i=0;i<5;i++)
   {
      key[strlen("snes9x_layer_")]='1'+i;
      var.value=NULL;
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value && var.value[0]=='N') disabled_layers|=1<<i;
   }
   Settings.BG_Forced=disabled_layers;

   //for some reason, Transparency seems to control both the fixed color and the windowing registers?
   var.key="snes9x_gfx_clip";
   var.value=NULL;
   Settings.DisableGraphicWindows=(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value && var.value[0]=='N');

   var.key="snes9x_gfx_transp";
   var.value=NULL;
   Settings.Transparency=!(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value && var.value[0]=='N');
}

static void S9xAudioCallback(void*)
{
   // Just pick a big buffer. We won't use it all.
   static int16_t audio_buf[0x10000];

   S9xFinalizeSamples();
   size_t avail = S9xGetSampleCount();
   S9xMixSamples((uint8*)audio_buf, avail);
   audio_batch_cb(audio_buf,avail >> 1);
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info,0,sizeof(retro_system_info));

    info->library_name = "Snes9x";
    info->library_version = VERSION;
    info->valid_extensions = "smc|sfc|swc|fig|bs";
    info->need_fullpath = false;
    info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    memset(info,0,sizeof(retro_system_av_info));

    info->geometry.base_width = SNES_WIDTH;
    info->geometry.base_height = use_overscan ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT;
    info->geometry.max_width = MAX_SNES_WIDTH;
    info->geometry.max_height = MAX_SNES_HEIGHT;
    info->geometry.aspect_ratio = 4.0f / 3.0f;
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

static unsigned snes_devices[2];
void retro_set_controller_port_device(unsigned port, unsigned device)
{
   if(port < 2)
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
             S9xSetController(port, CTL_MOUSE, 0, 0, 0, 0);
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
          default:
             if (log_cb)
                log_cb(RETRO_LOG_ERROR, "[libretro]: Invalid device (%d).\n", device);
       }
       if (!port)
          retro_set_controller_port_device(1, snes_devices[1]);
   }
   else if(device != RETRO_DEVICE_NONE)
       log_cb(RETRO_LOG_INFO, "[libretro]: Nonexistent Port (%d).\n", port);
}

void retro_cheat_reset()
{
   S9xDeleteCheats();
   S9xApplyCheats();
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   uint32 address;
   uint8 val;

   bool8 sram;
   uint8 bytes[3];//used only by GoldFinger, ignored for now

   if (S9xGameGenieToRaw(code, address, val)!=NULL &&
       S9xProActionReplayToRaw(code, address, val)!=NULL &&
       S9xGoldFingerToRaw(code, address, sram, val, bytes)!=NULL)
   { // bad code, ignore
      return;
   }
   if (index>Cheat.num_cheats) return; // cheat added in weird order, ignore
   if (index==Cheat.num_cheats) Cheat.num_cheats++;

   Cheat.c[index].address = address;
   Cheat.c[index].byte = val;
   Cheat.c[index].enabled = enabled;

   Cheat.c[index].saved = FALSE; // it'll be saved next time cheats run anyways

   Settings.ApplyCheats=true;
   S9xApplyCheats();
}

static void init_descriptors(void)
{
   struct retro_input_descriptor desc[] = {
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 0 },
   };

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

bool retro_load_game(const struct retro_game_info *game)
{
   init_descriptors();
   if(game->data == NULL && game->size == 0 && game->path != NULL)
      rom_loaded = Memory.LoadROM(game->path);
   else
      rom_loaded = Memory.LoadROMMem((const uint8_t*)game->data ,game->size);

   int pixel_format = RGB555;
   if(environ_cb) {
       pixel_format = RGB565;
       enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
       if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
           pixel_format = RGB555;
   }
   S9xGraphicsDeinit();
   S9xSetRenderPixelFormat(pixel_format);
   S9xGraphicsInit();

   if (!rom_loaded && log_cb)
      log_cb(RETRO_LOG_ERROR, "[libretro]: Rom loading failed...\n");

   return rom_loaded;
}

void retro_unload_game(void)
{}

bool retro_load_game_special(unsigned game_type,
      const struct retro_game_info *info, size_t num_info) {

   init_descriptors();
  switch (game_type) {
     case RETRO_GAME_TYPE_BSX:

       if(num_info == 1) {
          rom_loaded = Memory.LoadROMMem((const uint8_t*)info[0].data,info[0].size);
       } else if(num_info == 2) {
          memcpy(Memory.BIOSROM,(const uint8_t*)info[0].data,info[0].size);
          rom_loaded = Memory.LoadROMMem((const uint8_t*)info[1].data,info[1].size);
       }

       if (!rom_loaded && log_cb)
          log_cb(RETRO_LOG_ERROR, "[libretro]: BSX ROM loading failed...\n");

       break;

     case RETRO_GAME_TYPE_BSX_SLOTTED:

       if(num_info == 2)
           rom_loaded = Memory.LoadMultiCartMem((const uint8_t*)info[0].data, info[0].size,
                        (const uint8_t*)info[1].data, info[1].size, NULL, 0);

       if (!rom_loaded && log_cb)
          log_cb(RETRO_LOG_ERROR, "[libretro]: Multirom loading failed...\n");

       break;

     case RETRO_GAME_TYPE_SUFAMI_TURBO:

       if(num_info == 3)
           rom_loaded = Memory.LoadMultiCartMem((const uint8_t*)info[1].data, info[1].size,
                        (const uint8_t*)info[2].data, info[2].size, (const uint8_t*)info[0].data, info[0].size);

       if (!rom_loaded && log_cb)
          log_cb(RETRO_LOG_ERROR, "[libretro]: Sufami Turbo ROM loading failed...\n");

       break;

     default:
       rom_loaded = false;
       break;
  }

  return rom_loaded;
}

static void map_buttons();

static void check_system_specs(void)
{
   /* TODO - might have to variably set performance level based on SuperFX/SA-1/etc */
   unsigned level = 12;
   environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_init()
{
   struct retro_log_callback log;
   if (environ_cb)
   {
      if (!environ_cb(RETRO_ENVIRONMENT_GET_OVERSCAN, &use_overscan))
         use_overscan = false;
   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;

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
   Settings.WrongMovieStateProtection = TRUE;
   Settings.DumpStreamsMaxFrames = -1;
   Settings.StretchScreenshots = 0;
   Settings.SnapshotScreenshots = FALSE;
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
         log_cb(RETRO_LOG_ERROR, "[libretro]: Failed to init Memory or APU.\n");
      exit(1);
   }

   S9xInitSound(16, 0);
   S9xSetSoundMute(FALSE);
   S9xSetSamplesAvailableCallback(S9xAudioCallback, NULL);

   GFX.Pitch = MAX_SNES_WIDTH * sizeof(uint16);
   GFX.Screen = (uint16*) calloc(1, GFX.Pitch * MAX_SNES_HEIGHT);
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
   int _x, _y;
   int offset = snes_devices[0] == RETRO_DEVICE_JOYPAD_MULTITAP ? 4 : 1;
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
            if (snes_scope_state[0] < 0) snes_scope_state[0] = 0;
            else if (snes_scope_state[0] > (SNES_WIDTH-1)) snes_scope_state[0] = SNES_WIDTH-1;
            if (snes_scope_state[1] < 0) snes_scope_state[1] = 0;
            else if (snes_scope_state[1] > (SNES_HEIGHT-1)) snes_scope_state[1] = SNES_HEIGHT-1;
            S9xReportPointer(BTN_POINTER, snes_scope_state[0], snes_scope_state[1]);
            for (int i = SCOPE_TRIGGER; i <= SCOPE_LAST; i++)
                S9xReportButton(MAKE_BUTTON(2, i), input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, i));
            break;

         case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:
         case RETRO_DEVICE_LIGHTGUN_JUSTIFIERS:
            snes_justifier_state[port][0] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_JUSTIFIER, 0, RETRO_DEVICE_ID_LIGHTGUN_X);
            snes_justifier_state[port][1] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_JUSTIFIER, 0, RETRO_DEVICE_ID_LIGHTGUN_Y);
            if (snes_justifier_state[port][0] < 0) snes_justifier_state[port][0] = 0;
            else if (snes_justifier_state[port][0] > (SNES_WIDTH-1)) snes_justifier_state[port][0] = SNES_WIDTH-1;
            if (snes_justifier_state[port][1] < 0) snes_justifier_state[port][1] = 0;
            else if (snes_justifier_state[port][1] > (SNES_HEIGHT-1)) snes_justifier_state[port][1] = SNES_HEIGHT-1;
            S9xReportPointer(BTN_POINTER, snes_justifier_state[port][0], snes_justifier_state[port][1]);
            for (int i = JUSTIFIER_TRIGGER; i <= JUSTIFIER_LAST; i++)
               S9xReportButton(MAKE_BUTTON(2, i), input_state_cb(port, RETRO_DEVICE_LIGHTGUN, 0, i));
            break;

         default:
            if (log_cb)
               log_cb(RETRO_LOG_ERROR, "[libretro]: Unknown device...\n");

      }
   }
}

void retro_run()
{
   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      update_variables();

   poll_cb();
   report_buttons();
   S9xMainLoop();
}

void retro_deinit()
{
   S9xDeinitAPU();
   Memory.Deinit();
   S9xGraphicsDeinit();
   S9xUnmapAllControls();

   free(GFX.Screen);
}


unsigned retro_get_region()
{
   return Settings.PAL ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
}

void* retro_get_memory_data(unsigned type)
{
   void* data;

   switch(type) {
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
         size = (unsigned) (Multi.cartType && Multi.sramSizeB ? (1 << (Multi.sramSizeB + 3)) * 128 : 0);
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
   if (S9xFreezeGameMem((uint8_t*)data,size) == FALSE)
      return false;

   return true;
}

bool retro_unserialize(const void* data, size_t size)
{
   if (S9xUnfreezeGameMem((const uint8_t*)data,size) != SUCCESS)
      return false;
   return true;
}

bool8 S9xDeinitUpdate(int width, int height)
{
   if (!use_overscan)
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
   else
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

   video_cb(GFX.Screen, width, height, GFX.Pitch);
   return TRUE;
}

bool8 S9xContinueUpdate(int width, int height)
{
   return true;
}

// Dummy functions that should probably be implemented correctly later.
void S9xParsePortConfig(ConfigFile&, int) {}
void S9xSyncSpeed() {}
const char* S9xStringInput(const char* in) { return in; }
const char* S9xGetFilename(const char* in, s9x_getdirtype) { return in; }
const char* S9xGetDirectory(s9x_getdirtype) { return ""; }
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
void S9xMessage(int, int, const char*) {}
bool S9xPollAxis(unsigned int, short*) { return FALSE; }
void S9xSetPalette() {}
void S9xParseArg(char**, int&, int) {}
void S9xExit() {}
bool S9xPollPointer(unsigned int, short*, short*) { return false; }
const char *S9xChooseMovieFilename(unsigned char) { return NULL; }

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
         *dot   = strrchr(path, '.');

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
