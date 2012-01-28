#include "libsnes.hpp"

#include "snes9x.h"
#include "memmap.h"
#include "srtc.h"
#include "apu/apu.h"
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


static snes_video_refresh_t s9x_video_cb = NULL;
static snes_audio_sample_t s9x_audio_cb = NULL;
static snes_input_poll_t s9x_poller_cb = NULL;
static snes_input_state_t s9x_input_state_cb = NULL;

void snes_set_video_refresh(snes_video_refresh_t cb)
{
   s9x_video_cb = cb;
}

void snes_set_audio_sample(snes_audio_sample_t cb)
{
   s9x_audio_cb = cb;
}

void snes_set_input_poll(snes_input_poll_t cb)
{
   s9x_poller_cb = cb;
}

void snes_set_input_state(snes_input_state_t cb)
{
   s9x_input_state_cb = cb;
}

static snes_environment_t environ_cb;
static bool use_overscan;
void snes_set_environment(snes_environment_t cb)
{
   environ_cb = cb;
}

static void set_environ_timing()
{
   if (environ_cb)
   {
      snes_system_timing timing;
      timing.sample_rate = 32040.5;
      if (!Settings.PAL)
         timing.fps = 21477272.0 / 357366.0;
      else
         timing.fps = 21281370.0 / 425568.0;

      environ_cb(SNES_ENVIRONMENT_SET_TIMING, &timing);
   }
}

static void S9xAudioCallback(void*)
{
   // Just pick a big buffer. We won't use it all.
   static int16_t audio_buf[0x10000];

   S9xFinalizeSamples();
   size_t avail = S9xGetSampleCount();
   S9xMixSamples((uint8*)audio_buf, avail);
   for (size_t i = 0; i < avail; i+=2)
      s9x_audio_cb((uint16_t)audio_buf[i], (uint16_t)audio_buf[i + 1]);
}

const char *snes_library_id()
{
   return "SNES9x v" VERSION;
}

unsigned snes_library_revision_major()
{
   return 1;
}

unsigned snes_library_revision_minor()
{
   return 3;
}

void snes_power()
{
   S9xReset();
}

void snes_reset()
{
   S9xMovieUpdateOnReset();
   if (S9xMoviePlaying())
   {
      S9xMovieStop(true);
   }
   S9xSoftReset();
}

static unsigned snes_devices[2];
void snes_set_controller_port_device(bool in_port, unsigned device)
{
   int port = in_port == SNES_PORT_1 ? 0 : 1;
   switch (device)
   {
      case SNES_DEVICE_JOYPAD:
         S9xSetController(port, CTL_JOYPAD, 0, 0, 0, 0);
         snes_devices[port] = SNES_DEVICE_JOYPAD;
         break;
      case SNES_DEVICE_MULTITAP:
         S9xSetController(port, CTL_MP5, 1, 2, 3, 4);
         snes_devices[port] = SNES_DEVICE_MULTITAP;
         break;
      case SNES_DEVICE_MOUSE:
         S9xSetController(port, CTL_MOUSE, 0, 0, 0, 0);
         snes_devices[port] = SNES_DEVICE_MOUSE;
         break;
      case SNES_DEVICE_SUPER_SCOPE:
         S9xSetController(port, CTL_SUPERSCOPE, 0, 0, 0, 0);
         snes_devices[port] = SNES_DEVICE_SUPER_SCOPE;
         break;
      case SNES_DEVICE_JUSTIFIER:
         S9xSetController(port, CTL_JUSTIFIER, 0, 0, 0, 0);
         snes_devices[port] = SNES_DEVICE_JUSTIFIER;
         break;
      case SNES_DEVICE_JUSTIFIERS:
         S9xSetController(port, CTL_JUSTIFIER, 1, 0, 0, 0);
         snes_devices[port] = SNES_DEVICE_JUSTIFIERS;
         break;
      default:
         fprintf(stderr, "[libsnes]: Invalid device!\n");
   }
}

void snes_cheat_reset()
{}

void snes_cheat_set(unsigned, bool, const char*)
{}

bool snes_load_cartridge_bsx_slotted(
      const char *, const uint8_t *rom_data, unsigned rom_size,
      const char *, const uint8_t *bsx_data, unsigned bsx_size
      )
{
   int loaded = Memory.LoadMultiCartMem(rom_data, rom_size, bsx_data, bsx_size, NULL, NULL);

   if (!loaded)
   {
      fprintf(stderr, "[libsnes]: Sufami Turbo Rom loading failed...\n");
      return false;
   }

   set_environ_timing();

   return false;
}

bool snes_load_cartridge_bsx(
      const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
      const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
      )
{
   if(bsx_data==NULL)
       return snes_load_cartridge_normal(rom_xml,rom_data,rom_size);
   memcpy(Memory.BIOSROM,rom_data,rom_size);
   return snes_load_cartridge_normal(bsx_xml,bsx_data,bsx_size);
}

bool snes_load_cartridge_sufami_turbo(
      const char *, const uint8_t *rom_data, unsigned rom_size,
      const char *, const uint8_t *sta_data, unsigned sta_size,
      const char *, const uint8_t *stb_data, unsigned stb_size
      )
{
   int loaded = Memory.LoadMultiCartMem(sta_data, sta_size, stb_data, stb_size, rom_data, rom_size);

   if (!loaded)
   {
      fprintf(stderr, "[libsnes]: Sufami Turbo Rom loading failed...\n");
      return false;
   }

   set_environ_timing();
   
   return true;
}

bool snes_load_cartridge_super_game_boy(
      const char *, const uint8_t *, unsigned,
      const char *, const uint8_t *, unsigned 
      )
{
   return false;
}

static void map_buttons();


void snes_init()
{
   if (environ_cb)
   {
      if (!environ_cb(SNES_ENVIRONMENT_GET_OVERSCAN, &use_overscan))
         use_overscan = false;

      if (use_overscan)
      {
         snes_geometry geom = {256, 239, 512, 512};
         environ_cb(SNES_ENVIRONMENT_SET_GEOMETRY, &geom);
         unsigned pitch = 1024;
         environ_cb(SNES_ENVIRONMENT_SET_PITCH, &pitch);
      }
   }

   memset(&Settings, 0, sizeof(Settings));
   Settings.MouseMaster = TRUE;
   Settings.SuperScopeMaster = TRUE;
   Settings.JustifierMaster = TRUE;
   Settings.MultiPlayer5Master = TRUE;
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
   Settings.StopEmulation = TRUE;
   Settings.WrongMovieStateProtection = TRUE;
   Settings.DumpStreamsMaxFrames = -1;
   Settings.StretchScreenshots = 0;
   Settings.SnapshotScreenshots = FALSE;
   Settings.SkipFrames = AUTO_FRAMERATE;
   Settings.TurboSkipFrames = 15;
   Settings.CartAName[0] = 0;
   Settings.CartBName[0] = 0;
   Settings.AutoSaveDelay = 1;

   CPU.Flags = 0;

   if (!Memory.Init() || !S9xInitAPU())
   {
      Memory.Deinit();
      S9xDeinitAPU();
      fprintf(stderr, "[libsnes]: Failed to init Memory or APU.\n");
      exit(1);
   }

   S9xInitSound(16, 0);
   S9xSetSoundMute(FALSE);
   S9xSetSamplesAvailableCallback(S9xAudioCallback, NULL);

   S9xSetRenderPixelFormat(RGB555);
   GFX.Pitch = use_overscan ? 1024 : 2048;
   GFX.Screen = (uint16*) calloc(1, GFX.Pitch * 512 * sizeof(uint16));
   S9xGraphicsInit();

   S9xInitInputDevices();
   for (int i = 0; i < 2; i++)
   {
      S9xSetController(i, CTL_JOYPAD, i, 0, 0, 0);
      snes_devices[i] = SNES_DEVICE_JOYPAD;
   }

   S9xUnmapAllControls();
   map_buttons();

}

#define MAP_BUTTON(id, name) S9xMapButton((id), S9xGetCommandT((name)), false)
#define MAKE_BUTTON(pad, btn) (((pad)<<4)|(btn))

#define PAD_1 1
#define PAD_2 2
#define PAD_3 3
#define PAD_4 4
#define PAD_5 5

#define BTN_B SNES_DEVICE_ID_JOYPAD_B
#define BTN_Y SNES_DEVICE_ID_JOYPAD_Y
#define BTN_SELECT SNES_DEVICE_ID_JOYPAD_SELECT
#define BTN_START SNES_DEVICE_ID_JOYPAD_START
#define BTN_UP SNES_DEVICE_ID_JOYPAD_UP
#define BTN_DOWN SNES_DEVICE_ID_JOYPAD_DOWN
#define BTN_LEFT SNES_DEVICE_ID_JOYPAD_LEFT
#define BTN_RIGHT SNES_DEVICE_ID_JOYPAD_RIGHT
#define BTN_A SNES_DEVICE_ID_JOYPAD_A
#define BTN_X SNES_DEVICE_ID_JOYPAD_X
#define BTN_L SNES_DEVICE_ID_JOYPAD_L
#define BTN_R SNES_DEVICE_ID_JOYPAD_R
#define BTN_FIRST BTN_B
#define BTN_LAST BTN_R

#define MOUSE_X SNES_DEVICE_ID_MOUSE_X
#define MOUSE_Y SNES_DEVICE_ID_MOUSE_Y
#define MOUSE_LEFT SNES_DEVICE_ID_MOUSE_LEFT
#define MOUSE_RIGHT SNES_DEVICE_ID_MOUSE_RIGHT
#define MOUSE_FIRST MOUSE_X
#define MOUSE_LAST MOUSE_RIGHT

#define SCOPE_X SNES_DEVICE_ID_SUPER_SCOPE_X
#define SCOPE_Y SNES_DEVICE_ID_SUPER_SCOPE_Y
#define SCOPE_TRIGGER SNES_DEVICE_ID_SUPER_SCOPE_TRIGGER
#define SCOPE_CURSOR SNES_DEVICE_ID_SUPER_SCOPE_CURSOR
#define SCOPE_TURBO SNES_DEVICE_ID_SUPER_SCOPE_TURBO
#define SCOPE_PAUSE SNES_DEVICE_ID_SUPER_SCOPE_PAUSE
#define SCOPE_FIRST SCOPE_X
#define SCOPE_LAST SCOPE_PAUSE

#define JUSTIFIER_X SNES_DEVICE_ID_JUSTIFIER_X
#define JUSTIFIER_Y SNES_DEVICE_ID_JUSTIFIER_Y
#define JUSTIFIER_TRIGGER SNES_DEVICE_ID_JUSTIFIER_TRIGGER
#define JUSTIFIER_START SNES_DEVICE_ID_JUSTIFIER_START
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
   MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_UP), "{Joypad2 Up,Superscope ToggleTurbo}");
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

// libsnes uses relative values for analogue devices. 
// S9x seems to use absolute values, but do convert these into relative values in the core. (Why?!)
// Hack around it. :)
static int16_t snes_mouse_state[2][2] = {{0}, {0}};
static int16_t snes_scope_state[2] = {0};
static int16_t snes_justifier_state[2][2] = {{0}, {0}};
static void report_buttons()
{
   int _x, _y;
   for (int port = SNES_PORT_1; port <= SNES_PORT_2; port++)
   {
      switch (snes_devices[port])
      {
         case SNES_DEVICE_JOYPAD:
            for (int i = BTN_FIRST; i <= BTN_LAST; i++)
               S9xReportButton(MAKE_BUTTON(port + 1, i), s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_JOYPAD, 0, i));
            break;

         case SNES_DEVICE_MULTITAP:
            for (int j = 0; j < 4; j++)
               for (int i = BTN_FIRST; i <= BTN_LAST; i++)
                  S9xReportButton(MAKE_BUTTON(j + 2, i), s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_MULTITAP, j, i));
            break;

         case SNES_DEVICE_MOUSE:
            _x = s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_MOUSE, 0, SNES_DEVICE_ID_MOUSE_X);
            _y = s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_MOUSE, 0, SNES_DEVICE_ID_MOUSE_Y);
            snes_mouse_state[port][0] += _x;
            snes_mouse_state[port][1] += _y;
            S9xReportPointer(BTN_POINTER + port, snes_mouse_state[port][0], snes_mouse_state[port][1]);
            for (int i = MOUSE_LEFT; i <= MOUSE_LAST; i++)
               S9xReportButton(MAKE_BUTTON(port + 1, i), s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_MOUSE, 0, i));
            break;

         case SNES_DEVICE_SUPER_SCOPE:
            snes_scope_state[0] += s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_SUPER_SCOPE, 0, SNES_DEVICE_ID_SUPER_SCOPE_X);
            snes_scope_state[1] += s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_SUPER_SCOPE, 0, SNES_DEVICE_ID_SUPER_SCOPE_Y);
            S9xReportPointer(BTN_POINTER, snes_scope_state[0], snes_scope_state[1]);
            for (int i = SCOPE_TRIGGER; i <= SCOPE_LAST; i++)
               S9xReportButton(MAKE_BUTTON(port + 1, i), s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_SUPER_SCOPE, 0, i));
            break;

         case SNES_DEVICE_JUSTIFIER:
         case SNES_DEVICE_JUSTIFIERS:
            snes_justifier_state[0][0] += s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_JUSTIFIER, 0, SNES_DEVICE_ID_JUSTIFIER_X);
            snes_justifier_state[0][1] += s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_JUSTIFIER, 0, SNES_DEVICE_ID_JUSTIFIER_Y);
            S9xReportPointer(BTN_POINTER, snes_justifier_state[0][0], snes_justifier_state[0][1]);
            for (int i = JUSTIFIER_TRIGGER; i <= JUSTIFIER_LAST; i++)
               S9xReportButton(MAKE_BUTTON(port + 1, i), s9x_input_state_cb(port == SNES_PORT_2, SNES_DEVICE_JUSTIFIER, 0, i));
            break;
            
         default:
            fprintf(stderr, "[libsnes]: Unknown device...\n");

      }
   }
}

bool snes_load_cartridge_normal(const char *, const uint8_t *rom_data, unsigned rom_size)
{
   int loaded = Memory.LoadROMMem(rom_data,rom_size);
   if (!loaded)
   {
      fprintf(stderr, "[libsnes]: Rom loading failed...\n");
      return false;
   }

   set_environ_timing();
   
   return true;
}

void snes_run()
{
   s9x_poller_cb();
   report_buttons();
   S9xMainLoop();
}

void snes_term()
{
   S9xDeinitAPU();
   Memory.Deinit();
   S9xGraphicsDeinit();
   S9xUnmapAllControls();
}


bool snes_get_region()
{ 
   return Settings.PAL ? SNES_REGION_PAL : SNES_REGION_NTSC; 
}

uint8_t* snes_get_memory_data(unsigned type)
{
   uint8_t* data;

   switch(type) {
      case SNES_MEMORY_SUFAMI_TURBO_A_RAM:
      case SNES_MEMORY_CARTRIDGE_RAM:
         data = Memory.SRAM;
		 break;
      case SNES_MEMORY_SUFAMI_TURBO_B_RAM:
         data = Multi.sramB;
         break;
	  case SNES_MEMORY_CARTRIDGE_RTC:
	     data = RTCData.reg;
         break;
     case SNES_MEMORY_WRAM:
        data = Memory.RAM;
       break;
     case SNES_MEMORY_APURAM:
        data = spc_core->apuram();
       break;
     case SNES_MEMORY_VRAM:
        data = Memory.VRAM;
       break;
     case SNES_MEMORY_CGRAM:
        data = (uint8_t*)PPU.CGDATA;
       break;
     case SNES_MEMORY_OAM:
        data = PPU.OAMData;
       break;
	  default:
	     data = NULL;
		 break;
   }

   return data;
}

void snes_unload_cartridge()
{

}

unsigned snes_get_memory_size(unsigned type)
{
   unsigned size;

   switch(type) {
      case SNES_MEMORY_SUFAMI_TURBO_A_RAM:
      case SNES_MEMORY_CARTRIDGE_RAM:
         size = (unsigned) (Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0);
         if (size > 0x20000)
		    size = 0x20000;
		 break;
      case SNES_MEMORY_SUFAMI_TURBO_B_RAM:
         size = (unsigned) (Multi.cartType && Multi.sramSizeB ? (1 << (Multi.sramSizeB + 3)) * 128 : 0);
         break;
	  case SNES_MEMORY_CARTRIDGE_RTC:
		 size = (Settings.SRTC || Settings.SPC7110RTC)?20:0;
		 break;
     case SNES_MEMORY_WRAM:
       size = 128 * 1024;
      break;
     case SNES_MEMORY_VRAM:
     case SNES_MEMORY_APURAM:
       size = 64 * 1024;
      break;
     case SNES_MEMORY_CGRAM:
       size = 512;
      break;
     case SNES_MEMORY_OAM:
       size = 512 + 32;
      break;
	  default:
	     size = 0;
		 break;
   }

   return size;
}

void snes_set_cartridge_basename(const char*)
{}

unsigned snes_serialize_size()
{
   return S9xFreezeSize();
}

bool snes_serialize(uint8_t *data, unsigned size)
{ 
   if (S9xFreezeGameMem(data,size) == FALSE)
      return false;

   return true;
}

bool snes_unserialize(const uint8_t* data, unsigned size)
{ 
   if (S9xUnfreezeGameMem(data,size) != SUCCESS)
      return false;
   return true;
}

// Pitch 2048 -> 1024, only done once per res-change.
static void pack_frame(uint16_t *frame, int width, int height)
{
   for (int y = 1; y < height; y++)
   {
      uint16_t *src = frame + y * 1024;
      uint16_t *dst = frame + y * 512;

      memcpy(dst, src, width * sizeof(uint16_t));
   }
}

// Pitch 1024 -> 2048, only done once per res-change.
static void stretch_frame(uint16_t *frame, int width, int height)
{
   for (int y = height - 1; y >= 0; y--)
   {
      uint16_t *src = frame + y * 512;
      uint16_t *dst = frame + y * 1024;

      memcpy(dst, src, width * sizeof(uint16_t));
   }
}

bool8 S9xDeinitUpdate(int width, int height)
{
   if (use_overscan)
   {
      if (height == 224)
      {
         memmove(GFX.Screen + (GFX.Pitch / 2) * 7, GFX.Screen, GFX.Pitch * height);
         memset(GFX.Screen, 0x00, GFX.Pitch * 7);
         memset(GFX.Screen + (GFX.Pitch / 2) * (7 + 224), 0, GFX.Pitch * 8);
         height = 239;
      }
      else if (height == 448)
      {
         memmove(GFX.Screen + (GFX.Pitch / 2) * 15, GFX.Screen, GFX.Pitch * height);
         memset(GFX.Screen, 0x00, GFX.Pitch * 15);
         memset(GFX.Screen + (GFX.Pitch / 2) * (15 + 224), 0x00, GFX.Pitch * 17);
         height = 478;
      }
   }
   else // libsnes classic behavior
   {
      if (height == 448 || height == 478)
      {
         if (GFX.Pitch == 2048)
            pack_frame(GFX.Screen, width, height);
         GFX.Pitch = 1024;
      }
      else
      {
         if (GFX.Pitch == 1024)
            stretch_frame(GFX.Screen, width, height);
         GFX.Pitch = 2048;
      }
   }

   s9x_video_cb(GFX.Screen, width, height);
   return TRUE;
}

bool8 S9xContinueUpdate(int width, int height)
{
   return S9xDeinitUpdate(width, height);
}

// Dummy functions that should probably be implemented correctly later.
void S9xParsePortConfig(ConfigFile&, int) {}
void S9xSyncSpeed() {}
//void S9xPollPointer(int, short*, short*) {}
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
