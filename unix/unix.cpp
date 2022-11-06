/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef USE_THREADS
#include <sched.h>
#include <pthread.h>
#include <vector>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifndef NOSOUND 
#ifndef ALSA
#include <sys/soundcard.h>
#include <sys/mman.h>
#else
#include <alsa/asoundlib.h>
#endif
#endif

#ifdef JOYSTICK_SUPPORT
#include <linux/joystick.h>
#endif

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "gfx.h"
#include "snapshot.h"
#include "controls.h"
#include "cheats.h"
#include "movie.h"
#include "display.h"
#include "conffile.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif
#ifdef DEBUGGER
#include "debug.h"
#endif
#include "statemanager.h"

#ifdef NETPLAY_SUPPORT
#ifdef _DEBUG
#define NP_DEBUG 2
#endif
#endif

typedef std::pair<std::string, std::string>	strpair_t;

ConfigFile::secvec_t	keymaps;

StateManager stateMan;

#define FIXED_POINT				0x10000
#define FIXED_POINT_SHIFT		16
#define FIXED_POINT_REMAINDER	0xffff
#define SOUND_BUFFER_SIZE		(1024 * 16)
#define SOUND_BUFFER_SIZE_MASK	(SOUND_BUFFER_SIZE - 1)

static volatile bool8	block_signal         = FALSE;
static volatile bool8	block_generate_sound = FALSE;

static const char	*sound_device = NULL;

static const char	*s9x_base_dir        = NULL,
					*rom_filename        = NULL,
					*snapshot_filename   = NULL,
					*play_smv_filename   = NULL,
					*record_smv_filename = NULL;

static char		default_dir[PATH_MAX + 1];

static const char	dirNames[13][32] =
{
	"",				// DEFAULT_DIR
	"",				// HOME_DIR
	"",				// ROMFILENAME_DIR
	"rom",			// ROM_DIR
	"sram",			// SRAM_DIR
	"savestate",	// SNAPSHOT_DIR
	"screenshot",	// SCREENSHOT_DIR
	"spc",			// SPC_DIR
	"cheat",		// CHEAT_DIR
	"patch",		// PATCH_DIR
	"bios",			// BIOS_DIR
	"log",			// LOG_DIR
	""
};

struct SUnixSettings
{
	bool8	JoystickEnabled;
	bool8	ThreadSound;
	uint32	SoundBufferSize;
	uint32	SoundFragmentSize;
	uint32	rewindBufferSize;
	uint32	rewindGranularity;
};

struct SoundStatus
{
#ifndef ALSA
	int		sound_fd;
	uint32	fragment_size;
#else
    snd_pcm_t *pcm_handle;
    int output_buffer_size;
#endif
};


static int frame_advance = 0;
static SUnixSettings	unixSettings;
static SoundStatus		so;

static bool8	rewinding;

#ifdef JOYSTICK_SUPPORT
static uint8		js_mod[8]     = { 0, 0, 0, 0, 0, 0, 0, 0 };
static int			js_fd[8]      = { -1, -1, -1, -1, -1, -1, -1, -1 };
static const char	*js_device[8] = { "/dev/js0", "/dev/js1", "/dev/js2", "/dev/js3", "/dev/js4", "/dev/js5", "/dev/js6", "/dev/js7" };
static bool8		js_unplugged[8] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };
#endif

#ifdef NETPLAY_SUPPORT
static uint32	joypads[8];
static uint32	old_joypads[8];
#endif

bool8 S9xMapDisplayInput (const char *, s9xcommand_t *);
s9xcommand_t S9xGetDisplayCommandT (const char *);
char * S9xGetDisplayCommandName (s9xcommand_t);
void S9xHandleDisplayCommand (s9xcommand_t, int16, int16);
bool S9xDisplayPollButton (uint32, bool *);
bool S9xDisplayPollAxis (uint32, int16 *);
bool S9xDisplayPollPointer (uint32, int16 *, int16 *);

static void NSRTControllerSetup (void);
static int make_snes9x_dirs (void);
#ifdef JOYSTICK_SUPPORT
static void InitJoysticks (void);
static bool8 ReadJoysticks (void);
void S9xLatchJSEvent();
#endif

static long log2 (long num)
{
	long	n = 0;

	while (num >>= 1)
		n++;

	return (n);
}

namespace {

#if ! defined(NOSOUND) && ! defined(ALSA)
	class S9xAudioOutput
	{
	public:
		S9xAudioOutput(int fd, uint32 sampleRateHz, bool isThreaded)
		{
			m_FD = fd;
			uint32 bufferSizeMS = unixSettings.SoundBufferSize; // milliseconds
			// 4 = sizeof(uint16) * STEREO
			m_BufferSize = int(uint64(sampleRateHz) * bufferSizeMS / 1000 * 4);

#if defined(USE_THREADS)
			m_WrittenSize = 0;
			m_Thread = pthread_t();
			m_isExit = false;
			if (isThreaded)
			{
				m_BufferMutex = PTHREAD_MUTEX_INITIALIZER;
				m_hasBuffer = PTHREAD_COND_INITIALIZER;
				if (pthread_create(&m_Thread, NULL, AudioOutputThreadEntry, this))
				{
					return;
				}
			}
#endif
		}

		~S9xAudioOutput()
		{
#if defined(USE_THREADS)
			if (m_Thread)
			{
				pthread_mutex_lock(&m_BufferMutex);
				{
					m_isExit = true;
					pthread_cond_signal(&m_hasBuffer);
				}
				pthread_mutex_unlock(&m_BufferMutex);

				pthread_join(m_Thread, NULL);
				pthread_mutex_destroy(&m_BufferMutex);
				pthread_cond_destroy(&m_hasBuffer);
			}
#endif
		}

		void Write(void* data, int size)
		{
#if defined(USE_THREADS)
			if (m_Thread)
			{
				pthread_mutex_lock(&m_BufferMutex);
				{
					if (int(m_Buffering.size()) < m_BufferSize)
					{
						size_t oldSize = m_Buffering.size();
						size_t newSize = oldSize + size;
						m_WrittenSize = newSize;
						m_Buffering.resize(newSize);
						memcpy(&m_Buffering[oldSize], data, size);
						pthread_cond_signal(&m_hasBuffer);
					}
				}
				pthread_mutex_unlock(&m_BufferMutex);
			}
			else
#endif
			{
				WriteImpl(data, size);
			}
		}

		int GetFreeBufferSize()
		{
#if defined(USE_THREADS)
			if (m_Thread)
			{
				int writtenSize;
				pthread_mutex_lock(&m_BufferMutex);
				{
					writtenSize = m_WrittenSize;
				}
				pthread_mutex_unlock(&m_BufferMutex);
				return m_BufferSize - writtenSize;
			}
			else
#endif
			{
				audio_buf_info info;
				ioctl(m_FD, SNDCTL_DSP_GETOSPACE, &info);
				int writtenSize = info.fragsize * info.fragstotal - info.bytes;
				return std::max(0, m_BufferSize - writtenSize);
			}
		}

	private:
		void WriteImpl(const void* data, int size)
		{
			const char* p = reinterpret_cast<const char*>(data);
			while (size > 0)
			{
				int result = write(m_FD, p, size);
				if (result < 0)
				{
					return;
				}
				p += result;
				size -= result;
			}
		}

		int m_FD;
		int m_BufferSize;

#if defined(USE_THREADS)
		pthread_t m_Thread;
		volatile bool m_isExit;
		pthread_mutex_t m_BufferMutex;
		pthread_cond_t m_hasBuffer;
		std::vector<uint8> m_PlayingBuffer;
		std::vector<uint8> m_Buffering;
		int m_WrittenSize; // for dynamic rate control

		static void* AudioOutputThreadEntry(void* arg)
		{
			S9xAudioOutput* obj = reinterpret_cast<S9xAudioOutput*>(arg);
			obj->AudioOutputThread();
			return NULL;
		}

		void AudioOutputThread()
		{
			while (true)
			{
				pthread_mutex_lock(&m_BufferMutex);
				{
					pthread_cond_wait(&m_hasBuffer, &m_BufferMutex);
					if (m_isExit)
					{
						return;
					}
					m_PlayingBuffer.swap(m_Buffering);
					m_WrittenSize = 0;
				}
				pthread_mutex_unlock(&m_BufferMutex);

				if (! m_PlayingBuffer.empty())
				{
					WriteImpl(&m_PlayingBuffer[0], m_PlayingBuffer.size());
					m_PlayingBuffer.resize(0);
				}
			}
		}
#endif // USE_THREADS
	};

	S9xAudioOutput* s_AudioOutput = NULL;
#endif // NOSOUND

}

void S9xExtraUsage (void)
{
	/*                               12345678901234567890123456789012345678901234567890123456789012345678901234567890 */

	S9xMessage(S9X_INFO, S9X_USAGE, "-multi                          Enable multi cartridge system");
	S9xMessage(S9X_INFO, S9X_USAGE, "-carta <filename>               ROM in slot A (use with -multi)");
	S9xMessage(S9X_INFO, S9X_USAGE, "-cartb <filename>               ROM in slot B (use with -multi)");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

#ifdef JOYSTICK_SUPPORT
	S9xMessage(S9X_INFO, S9X_USAGE, "-nogamepad                      Disable gamepad reading");
	S9xMessage(S9X_INFO, S9X_USAGE, "-paddev1 <string>               Specify gamepad device 1");
	S9xMessage(S9X_INFO, S9X_USAGE, "-paddev2 <string>               Specify gamepad device 2");
	S9xMessage(S9X_INFO, S9X_USAGE, "-paddev3 <string>               Specify gamepad device 3");
	S9xMessage(S9X_INFO, S9X_USAGE, "-paddev4 <string>               Specify gamepad device 4");
	S9xMessage(S9X_INFO, S9X_USAGE, "-paddev5 <string>               Specify gamepad device 5");
	S9xMessage(S9X_INFO, S9X_USAGE, "-paddev6 <string>               Specify gamepad device 6");
	S9xMessage(S9X_INFO, S9X_USAGE, "-paddev7 <string>               Specify gamepad device 7");
	S9xMessage(S9X_INFO, S9X_USAGE, "-paddev8 <string>               Specify gamepad device 8");
	S9xMessage(S9X_INFO, S9X_USAGE, "");
#endif

#ifndef NOSOUND
#ifdef USE_THREADS && ! defined(ALSA)
	S9xMessage(S9X_INFO, S9X_USAGE, "-threadsound                    Use a separate thread to output sound");
#endif
	S9xMessage(S9X_INFO, S9X_USAGE, "-buffersize                     Sound generating buffer size in millisecond");
#ifndef ALSA
	S9xMessage(S9X_INFO, S9X_USAGE, "-fragmentsize                   Sound playback buffer fragment size in bytes");
#endif
	S9xMessage(S9X_INFO, S9X_USAGE, "-sounddev <string>              Specify sound device");
	S9xMessage(S9X_INFO, S9X_USAGE, "");
#endif
	S9xMessage(S9X_INFO, S9X_USAGE, "-loadsnapshot                   Load snapshot file at start");
	S9xMessage(S9X_INFO, S9X_USAGE, "-playmovie <filename>           Start emulator playing the .smv file");
	S9xMessage(S9X_INFO, S9X_USAGE, "-recordmovie <filename>         Start emulator recording the .smv file");
	S9xMessage(S9X_INFO, S9X_USAGE, "-dumpstreams                    Save audio/video data to disk");
	S9xMessage(S9X_INFO, S9X_USAGE, "-dumpmaxframes <num>            Stop emulator after saving specified number of");
	S9xMessage(S9X_INFO, S9X_USAGE, "                                frames (use with -dumpstreams)");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	S9xMessage(S9X_INFO, S9X_USAGE, "-rwbuffersize                   Rewind buffer size in MB");
	S9xMessage(S9X_INFO, S9X_USAGE, "-rwgranularity                  Rewind granularity in frames");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	S9xExtraDisplayUsage();
}

void S9xParseArg (char **argv, int &i, int argc)
{
	if (!strcasecmp(argv[i], "-multi"))
		Settings.Multi = TRUE;
	else
	if (!strcasecmp(argv[i], "-carta"))
	{
		if (i + 1 < argc)
			strncpy(Settings.CartAName, argv[++i], _MAX_PATH);
		else
			S9xUsage();
	}
	else
	if (!strcasecmp(argv[i], "-cartb"))
	{
		if (i + 1 < argc)
			strncpy(Settings.CartBName, argv[++i], _MAX_PATH);
		else
			S9xUsage();
	}
	else
#ifdef JOYSTICK_SUPPORT
	if (!strcasecmp(argv[i], "-nogamepad"))
		unixSettings.JoystickEnabled = FALSE;
	else
	if (!strncasecmp(argv[i], "-paddev", 7) &&
		argv[i][7] >= '1' && argv[i][7] <= '8' && argv[i][8] == '\0')
	{
		int	j = argv[i][7] - '1';

		if (i + 1 < argc)
			js_device[j] = argv[++i];
		else
			S9xUsage();
	}
	else
#endif
#ifdef USE_THREADS
	if (!strcasecmp(argv[i], "-threadsound"))
		unixSettings.ThreadSound = TRUE;
	else
#endif
	if (!strcasecmp(argv[i], "-buffersize"))
	{
		if (i + 1 < argc)
			unixSettings.SoundBufferSize = atoi(argv[++i]);
		else
			S9xUsage();
	}
	else
	if (!strcasecmp(argv[i], "-fragmentsize"))
	{
		if (i + 1 < argc)
			unixSettings.SoundFragmentSize = atoi(argv[++i]);
		else
			S9xUsage();
	}
	else
	if (!strcasecmp(argv[i], "-sounddev"))
	{
		if (i + 1 < argc)
			sound_device = argv[++i];
		else
			S9xUsage();
	}
	else
	if (!strcasecmp(argv[i], "-loadsnapshot"))
	{
		if (i + 1 < argc)
			snapshot_filename = argv[++i];
		else
			S9xUsage();
	}
	else
	if (!strcasecmp(argv[i], "-playmovie"))
	{
		if (i + 1 < argc)
			play_smv_filename = argv[++i];
		else
			S9xUsage();
	}
	else
	if (!strcasecmp(argv[i], "-recordmovie"))
	{
		if (i + 1 < argc)
			record_smv_filename = argv[++i];
		else
			S9xUsage();
	}
	else
	if (!strcasecmp(argv[i], "-dumpstreams"))
		Settings.DumpStreams = TRUE;
	else
	if (!strcasecmp(argv[i], "-dumpmaxframes"))
		Settings.DumpStreamsMaxFrames = atoi(argv[++i]);
	else
	if (!strcasecmp(argv[i], "-rwbuffersize"))
	{
		if (i + 1 < argc)
			unixSettings.rewindBufferSize = atoi(argv[++i]);
		else
			S9xUsage();
	}
	else
	if (!strcasecmp(argv[i], "-rwgranularity"))
	{
		if (i + 1 < argc)
			unixSettings.rewindGranularity = atoi(argv[++i]);
		else
			S9xUsage();
	}
	else
		S9xParseDisplayArg(argv, i, argc);
}

static void NSRTControllerSetup (void)
{
	if (!strncmp((const char *) Memory.NSRTHeader + 24, "NSRT", 4))
	{
		// First plug in both, they'll change later as needed
		S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
		S9xSetController(1, CTL_JOYPAD, 1, 0, 0, 0);

		switch (Memory.NSRTHeader[29])
		{
			case 0x00:	// Everything goes
				break;

			case 0x10:	// Mouse in Port 0
				S9xSetController(0, CTL_MOUSE,      0, 0, 0, 0);
				break;

			case 0x01:	// Mouse in Port 1
				S9xSetController(1, CTL_MOUSE,      1, 0, 0, 0);
				break;

			case 0x03:	// Super Scope in Port 1
				S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
				break;

			case 0x06:	// Multitap in Port 1
				S9xSetController(1, CTL_MP5,        1, 2, 3, 4);
				break;

			case 0x66:	// Multitap in Ports 0 and 1
				S9xSetController(0, CTL_MP5,        0, 1, 2, 3);
				S9xSetController(1, CTL_MP5,        4, 5, 6, 7);
				break;

			case 0x08:	// Multitap in Port 1, Mouse in new Port 1
				S9xSetController(1, CTL_MOUSE,      1, 0, 0, 0);
				// There should be a toggle here for putting in Multitap instead
				break;

			case 0x04:	// Pad or Super Scope in Port 1
				S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
				// There should be a toggle here for putting in a pad instead
				break;

			case 0x05:	// Justifier - Must ask user...
				S9xSetController(1, CTL_JUSTIFIER,  1, 0, 0, 0);
				// There should be a toggle here for how many justifiers
				break;

			case 0x20:	// Pad or Mouse in Port 0
				S9xSetController(0, CTL_MOUSE,      0, 0, 0, 0);
				// There should be a toggle here for putting in a pad instead
				break;

			case 0x22:	// Pad or Mouse in Port 0 & 1
				S9xSetController(0, CTL_MOUSE,      0, 0, 0, 0);
				S9xSetController(1, CTL_MOUSE,      1, 0, 0, 0);
				// There should be a toggles here for putting in pads instead
				break;

			case 0x24:	// Pad or Mouse in Port 0, Pad or Super Scope in Port 1
				// There should be a toggles here for what to put in, I'm leaving it at gamepad for now
				break;

			case 0x27:	// Pad or Mouse in Port 0, Pad or Mouse or Super Scope in Port 1
				// There should be a toggles here for what to put in, I'm leaving it at gamepad for now
				break;

			// Not Supported yet
			case 0x99:	// Lasabirdie
				break;

			case 0x0A:	// Barcode Battler
				break;
		}
	}
}

void S9xParsePortConfig (ConfigFile &conf, int pass)
{
	s9x_base_dir                   = conf.GetStringDup("Unix::BaseDir",             default_dir);
	snapshot_filename              = conf.GetStringDup("Unix::SnapshotFilename",    NULL);
	play_smv_filename              = conf.GetStringDup("Unix::PlayMovieFilename",   NULL);
	record_smv_filename            = conf.GetStringDup("Unix::RecordMovieFilename", NULL);

#ifdef JOYSTICK_SUPPORT
	unixSettings.JoystickEnabled   = conf.GetBool     ("Unix::EnableGamePad",       true);
	js_device[0]                   = conf.GetStringDup("Unix::PadDevice1",          NULL);
	js_device[1]                   = conf.GetStringDup("Unix::PadDevice2",          NULL);
	js_device[2]                   = conf.GetStringDup("Unix::PadDevice3",          NULL);
	js_device[3]                   = conf.GetStringDup("Unix::PadDevice4",          NULL);
	js_device[4]                   = conf.GetStringDup("Unix::PadDevice5",          NULL);
	js_device[5]                   = conf.GetStringDup("Unix::PadDevice6",          NULL);
	js_device[6]                   = conf.GetStringDup("Unix::PadDevice7",          NULL);
	js_device[7]                   = conf.GetStringDup("Unix::PadDevice8",          NULL);
#endif

#ifdef USE_THREADS
	unixSettings.ThreadSound       = conf.GetBool     ("Unix::ThreadSound",         false);
#endif
	unixSettings.SoundBufferSize   = conf.GetUInt     ("Unix::SoundBufferSize",     100);
	unixSettings.SoundFragmentSize = conf.GetUInt     ("Unix::SoundFragmentSize",   2048);
#ifndef ALSA
	sound_device                   = conf.GetStringDup("Unix::SoundDevice",         "/dev/dsp");
#else
	sound_device                   = conf.GetStringDup("Unix::SoundDevice",         "default");
#endif    
	keymaps.clear();
	if (!conf.GetBool("Unix::ClearAllControls", false))
	{
	#if 0
		// Using an axis to control Pseudo-pointer #1
		keymaps.push_back(strpair_t("J00:Axis0",      "AxisToPointer 1h Var"));
		keymaps.push_back(strpair_t("J00:Axis1",      "AxisToPointer 1v Var"));
		keymaps.push_back(strpair_t("PseudoPointer1", "Pointer C=2 White/Black Superscope"));
	#elif 0
		// Using an Axis for Pseudo-buttons
		keymaps.push_back(strpair_t("J00:Axis0",      "AxisToButtons 1/0 T=50%"));
		keymaps.push_back(strpair_t("J00:Axis1",      "AxisToButtons 3/2 T=50%"));
		keymaps.push_back(strpair_t("PseudoButton0",  "Joypad1 Right"));
		keymaps.push_back(strpair_t("PseudoButton1",  "Joypad1 Left"));
		keymaps.push_back(strpair_t("PseudoButton2",  "Joypad1 Down"));
		keymaps.push_back(strpair_t("PseudoButton3",  "Joypad1 Up"));
	#else
		// Using 'Joypad# Axis'
		keymaps.push_back(strpair_t("J00:Axis0",      "Joypad1 Axis Left/Right T=50%"));
		keymaps.push_back(strpair_t("J00:Axis1",      "Joypad1 Axis Up/Down T=50%"));
	#endif
		keymaps.push_back(strpair_t("J00:B0",         "Joypad1 X"));
		keymaps.push_back(strpair_t("J00:B1",         "Joypad1 A"));
		keymaps.push_back(strpair_t("J00:B2",         "Joypad1 B"));
		keymaps.push_back(strpair_t("J00:B3",         "Joypad1 Y"));
	#if 1
		keymaps.push_back(strpair_t("J00:B6",         "Joypad1 L"));
	#else
		// Show off joypad-meta
		keymaps.push_back(strpair_t("J00:X+B6",       "JS1 Meta1"));
		keymaps.push_back(strpair_t("J00:M1+B1",      "Joypad1 Turbo A"));
	#endif
		keymaps.push_back(strpair_t("J00:B7",         "Joypad1 R"));
		keymaps.push_back(strpair_t("J00:B8",         "Joypad1 Select"));
		keymaps.push_back(strpair_t("J00:B11",        "Joypad1 Start"));
	}

	std::string section = S9xParseDisplayConfig(conf, 1);

	ConfigFile::secvec_t	sec = conf.GetSection((section + " Controls").c_str());
	for (ConfigFile::secvec_t::iterator c = sec.begin(); c != sec.end(); c++)
		keymaps.push_back(*c);
}

static int make_snes9x_dirs (void)
{
	if (strlen(s9x_base_dir) + 1 + sizeof(dirNames[0]) > PATH_MAX + 1)
		return (-1);

	mkdir(s9x_base_dir, 0755);

	for (int i = 0; i < LAST_DIR; i++)
	{
		if (dirNames[i][0])
		{
			char	s[PATH_MAX + 1];
			snprintf(s, PATH_MAX + 1, "%s%s%s", s9x_base_dir, SLASH_STR, dirNames[i]);
			mkdir(s, 0755);
		}
	}

	return (0);
}

const char * S9xGetDirectory (enum s9x_getdirtype dirtype)
{
	static char	s[PATH_MAX + 1];

	if (dirNames[dirtype][0])
		snprintf(s, PATH_MAX + 1, "%s%s%s", s9x_base_dir, SLASH_STR, dirNames[dirtype]);
	else
	{
		switch (dirtype)
		{
			case DEFAULT_DIR:
				strncpy(s, s9x_base_dir, PATH_MAX + 1);
				s[PATH_MAX] = 0;
				break;

			case HOME_DIR:
				strncpy(s, getenv("HOME"), PATH_MAX + 1);
				s[PATH_MAX] = 0;
				break;

			case ROMFILENAME_DIR:
				strncpy(s, Memory.ROMFilename, PATH_MAX + 1);
				s[PATH_MAX] = 0;

				for (int i = strlen(s); i >= 0; i--)
				{
					if (s[i] == SLASH_CHAR)
					{
						s[i] = 0;
						break;
					}
				}

				break;

			default:
				s[0] = 0;
				break;
		}
	}

	return (s);
}

const char * S9xGetFilename (const char *ex, enum s9x_getdirtype dirtype)
{
	static char	s[PATH_MAX + 1];
	char		drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	snprintf(s, PATH_MAX + 1, "%s%s%s%s", S9xGetDirectory(dirtype), SLASH_STR, fname, ex);

	return (s);
}

const char * S9xGetFilenameInc (const char *ex, enum s9x_getdirtype dirtype)
{
	static char	s[PATH_MAX + 1];
	char		drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	unsigned int	i = 0;
	const char		*d;
	struct stat		buf;

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	d = S9xGetDirectory(dirtype);

	do
		snprintf(s, PATH_MAX + 1, "%s%s%s.%03d%s", d, SLASH_STR, fname, i++, ex);
	while (stat(s, &buf) == 0 && i < 1000);

	return (s);
}

const char * S9xBasename (const char *f)
{
	const char	*p;

	if ((p = strrchr(f, '/')) != NULL || (p = strrchr(f, '\\')) != NULL)
		return (p + 1);

	return (f);
}

bool8 S9xOpenSnapshotFile (const char *filename, bool8 read_only, STREAM *file)
{
	char	s[PATH_MAX + 1];
	char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(filename, drive, dir, fname, ext);

	if (*drive || *dir == SLASH_CHAR || (strlen(dir) > 1 && *dir == '.' && *(dir + 1) == SLASH_CHAR))
	{
		strncpy(s, filename, PATH_MAX + 1);
		s[PATH_MAX] = 0;
	}
	else
		snprintf(s, PATH_MAX + 1, "%s%s%s", S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR, fname);

	if (!*ext && strlen(s) <= PATH_MAX - 4)
		strcat(s, ".frz");

	if ((*file = OPEN_STREAM(s, read_only ? "rb" : "wb")))
		return (TRUE);

	return (FALSE);
}

void S9xCloseSnapshotFile (STREAM file)
{
	CLOSE_STREAM(file);
}

bool8 S9xInitUpdate (void)
{
	return (TRUE);
}

bool8 S9xDeinitUpdate (int width, int height)
{
	S9xPutImage(width, height);
	return (TRUE);
}

bool8 S9xContinueUpdate (int width, int height)
{
	return (TRUE);
}

void S9xToggleSoundChannel (int c)
{
	static uint8	sound_switch = 255;

	if (c == 8)
		sound_switch = 255;
	else
		sound_switch ^= 1 << c;

	S9xSetSoundControl(sound_switch);
}

void S9xAutoSaveSRAM (void)
{
	Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR));
}

void S9xSyncSpeed (void)
{
#ifndef NOSOUND
	if (Settings.SoundSync)
	{
		return;
	}
#endif

	if (Settings.DumpStreams)
		return;

#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay && NetPlay.Connected)
	{
	#if defined(NP_DEBUG) && NP_DEBUG == 2
		printf("CLIENT: SyncSpeed @%d\n", S9xGetMilliTime());
	#endif

		S9xNPSendJoypadUpdate(old_joypads[0]);
		for (int J = 0; J < 8; J++)
			joypads[J] = S9xNPGetJoypad(J);

		if (!S9xNPCheckForHeartBeat())
		{
			NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay(100);
		#if defined(NP_DEBUG) && NP_DEBUG == 2
			if (NetPlay.PendingWait4Sync)
				printf("CLIENT: PendingWait4Sync1 @%d\n", S9xGetMilliTime());
		#endif

			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}
		else
		{
			NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay(200);
		#if defined(NP_DEBUG) && NP_DEBUG == 2
			if (NetPlay.PendingWait4Sync)
				printf("CLIENT: PendingWait4Sync2 @%d\n", S9xGetMilliTime());
		#endif

			if (IPPU.SkippedFrames < NetPlay.MaxFrameSkip)
			{
				IPPU.RenderThisFrame = FALSE;
				IPPU.SkippedFrames++;
			}
			else
			{
				IPPU.RenderThisFrame = TRUE;
				IPPU.SkippedFrames = 0;
			}
		}

		if (!NetPlay.PendingWait4Sync)
		{
			NetPlay.FrameCount++;
			S9xNPStepJoypadHistory();
		}

		return;
	}
#endif

	if (Settings.HighSpeedSeek > 0)
		Settings.HighSpeedSeek--;

	if (Settings.TurboMode)
	{
		if ((++IPPU.FrameSkip >= Settings.TurboSkipFrames) && !Settings.HighSpeedSeek)
		{
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		}
		else
		{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}

		return;
	}

	static struct timeval	next1 = { 0, 0 };
	struct timeval			now;

	while (gettimeofday(&now, NULL) == -1) ;

	// If there is no known "next" frame, initialize it now.
	if (next1.tv_sec == 0)
	{
		next1 = now;
		next1.tv_usec++;
	}

	// If we're on AUTO_FRAMERATE, we'll display frames always only if there's excess time.
	// Otherwise we'll display the defined amount of frames.
	unsigned	limit = (Settings.SkipFrames == AUTO_FRAMERATE) ? (timercmp(&next1, &now, <) ? 10 : 1) : Settings.SkipFrames;

	IPPU.RenderThisFrame = (++IPPU.SkippedFrames >= limit) ? TRUE : FALSE;

	if (IPPU.RenderThisFrame)
		IPPU.SkippedFrames = 0;
	else
	{
		// If we were behind the schedule, check how much it is.
		if (timercmp(&next1, &now, <))
		{
			unsigned	lag = (now.tv_sec - next1.tv_sec) * 1000000 + now.tv_usec - next1.tv_usec;
			if (lag >= 500000)
			{
				// More than a half-second behind means probably pause.
				// The next line prevents the magic fast-forward effect.
				next1 = now;
			}
		}
	}

	// Delay until we're completed this frame.
	// Can't use setitimer because the sound code already could be using it. We don't actually need it either.
	while (timercmp(&next1, &now, >))
	{
		// If we're ahead of time, sleep a while.
		unsigned	timeleft = (next1.tv_sec - now.tv_sec) * 1000000 + next1.tv_usec - now.tv_usec;
		usleep(timeleft);

		while (gettimeofday(&now, NULL) == -1) ;
		// Continue with a while-loop because usleep() could be interrupted by a signal.
	}

	// Calculate the timestamp of the next frame.
	next1.tv_usec += Settings.FrameTime;
	if (next1.tv_usec >= 1000000)
	{
		next1.tv_sec += next1.tv_usec / 1000000;
		next1.tv_usec %= 1000000;
	}
}

bool8 S9xMapInput (const char *n, s9xcommand_t *cmd)
{
	int		i, j, d;
	char	*c;
	char	buf[4] = "M1+";

	if (!strncmp(n, "PseudoPointer", 13) && n[13] >= '1' && n[13] <= '8' && n[14] == '\0')
		return (S9xMapPointer(PseudoPointerBase + (n[13] - '1'), *cmd, false));

	if (!strncmp(n, "PseudoButton", 12) && isdigit(n[12]) && (j = strtol(n + 12, &c, 10)) < 256 && (c == NULL || *c == '\0'))
		return (S9xMapButton(PseudoButtonBase + j, *cmd, false));

	if (n[0] != 'J' || !isdigit(n[1]) || !isdigit(n[2]) || n[3] != ':')
		goto unrecog;

	d = ((n[1] - '0') * 10 + (n[2] - '0')) << 24;
	d |= 0x80000000;
	i = 4;
	if (!strncmp(n + i, "X+", 2))
	{
		d |= 0x4000;
		i += 2;
	}
	else
	{
		for (buf[1] = '1'; buf[1] <= '8'; buf[1]++)
		{
			if (!strncmp(n + i, buf, 3))
			{
				d |= 1 << (buf[1] - '1' + 16);
				i += 3;
			}
		}
	}

	if (!strncmp(n + i, "Axis", 4))
	{
		d |= 0x8000;
		i += 4;
	}
	else
	if (n[i] == 'B')
		i++;
	else
		goto unrecog;

	d |= j = strtol(n + i, &c, 10);
	if ((c != NULL && *c != '\0') || j > 0x3fff)
		goto unrecog;

	if (d & 0x8000)
		return (S9xMapAxis(d, *cmd, false));

	return (S9xMapButton(d, *cmd, false));

unrecog:
	return (S9xMapDisplayInput(n, cmd));
}

bool S9xPollButton (uint32 id, bool *pressed)
{
	return (S9xDisplayPollButton(id, pressed));
}

bool S9xPollAxis (uint32 id, int16 *value)
{
	return (S9xDisplayPollAxis(id, value));
}

bool S9xPollPointer (uint32 id, int16 *x, int16 *y)
{
	return (S9xDisplayPollPointer(id, x, y));
}

s9xcommand_t S9xGetPortCommandT (const char *n)
{
	s9xcommand_t	cmd;

	cmd.type         = S9xBadMapping;
	cmd.multi_press  = 0;
	cmd.button_norpt = 0;
	cmd.port[0]      = 0;
	cmd.port[1]      = 0;
	cmd.port[2]      = 0;
	cmd.port[3]      = 0;

	if (!strncmp(n, "JS", 2) && n[2] >= '1' && n[2] <= '8')
	{
		if (!strncmp(n + 3, " Meta", 5) && n[8] >= '1' && n[8] <= '8' && n[9] == '\0')
		{
			cmd.type    = S9xButtonPort;
			cmd.port[1] = 0;
			cmd.port[2] = n[2] - '1';
			cmd.port[3] = 1 << (n[8] - '1');

			return (cmd);
		}
		else
		if (!strncmp(n + 3, " ToggleMeta", 11) && n[14] >= '1' && n[14] <= '8' && n[15] == '\0')
		{
			cmd.type    = S9xButtonPort;
			cmd.port[1] = 1;
			cmd.port[2] = n[2] - '1';
			cmd.port[3] = 1 << (n[13] - '1');

			return (cmd);
		}
	} else
	if (!strcmp(n,"Rewind"))
	{
		cmd.type = S9xButtonPort;
		cmd.port[1] = 2;

		return (cmd);
	}
        else if (!strcmp(n, "Advance"))
        {
                cmd.type = S9xButtonPort;
                cmd.port[1] = 3;
                return (cmd);
        }

	return (S9xGetDisplayCommandT(n));
}

char * S9xGetPortCommandName (s9xcommand_t cmd)
{
	std::string	x;

	switch (cmd.type)
	{
		case S9xButtonPort:
			if (cmd.port[0] != 0)
				break;

			switch (cmd.port[1])
			{
				case 0:
					x = "JS";
					x += (int) cmd.port[2];
					x += " Meta";
					x += (int) cmd.port[3];
					return (strdup(x.c_str()));

				case 1:
					x = "JS";
					x += (int) cmd.port[2];
					x += " ToggleMeta";
					x += (int) cmd.port[3];
					return (strdup(x.c_str()));

				case 2:
					return (strdup("Rewind"));

                                case 3:
                                        return (strdup("Advance"));
			}

			break;

		case S9xAxisPort:
			break;

		case S9xPointerPort:
			break;
	}

	return (S9xGetDisplayCommandName(cmd));
}

void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
#ifdef JOYSTICK_SUPPORT
	switch (cmd.type)
	{
		case S9xButtonPort:
			if (cmd.port[0] != 0)
				break;

			switch (cmd.port[1])
			{
				case 0:
					if (data1)
						js_mod[cmd.port[2]] |=  cmd.port[3];
					else
						js_mod[cmd.port[2]] &= ~cmd.port[3];
					break;

				case 1:
					if (data1)
						js_mod[cmd.port[2]] ^=  cmd.port[3];
					break;

				case 2:
					rewinding = (bool8) data1;
					break;

                                case 3:
                                        frame_advance = (bool8) data1;
			}

			break;

		case S9xAxisPort:
			break;

		case S9xPointerPort:
			break;
	}

	S9xHandleDisplayCommand(cmd, data1, data2);
#endif
}

void S9xSetupDefaultKeymap (void)
{
	s9xcommand_t	cmd;

	S9xUnmapAllControls();

	for (ConfigFile::secvec_t::iterator i = keymaps.begin(); i != keymaps.end(); i++)
	{
		cmd = S9xGetPortCommandT(i->second.c_str());

		if (cmd.type == S9xBadMapping)
		{
			cmd = S9xGetCommandT(i->second.c_str());
			if (cmd.type == S9xBadMapping)
			{
				std::string	s("Unrecognized command '");
				s += i->second + "'";
				perror(s.c_str());
				continue;
			}
		}

		if (!S9xMapInput(i->first.c_str(), &cmd))
		{
			std::string	s("Could not map '");
			s += i->second + "' to '" + i->first + "'";
			perror(s.c_str());
			continue;
		}
	}

	keymaps.clear();
}

void S9xInitInputDevices (void)
{
#ifdef JOYSTICK_SUPPORT
	InitJoysticks();
#endif
}

#ifdef JOYSTICK_SUPPORT

static void InitJoysticks (void)
{
#ifdef JSIOCGVERSION
	int				version;
	unsigned char	axes, buttons;

	if ((js_fd[0] = open(js_device[0], O_RDONLY | O_NONBLOCK)) == -1)
	{
		fprintf(stderr, "joystick: No joystick found.\n");
		return;
	}

	if (ioctl(js_fd[0], JSIOCGVERSION, &version) == -1)
	{
		fprintf(stderr, "joystick: You need at least driver version 1.0 for joystick support.\n");
		close(js_fd[0]);
		return;
	}

	for (int i = 1; i < 8; i++)
		js_fd[i] = open(js_device[i], O_RDONLY | O_NONBLOCK);

#ifdef JSIOCGNAME
	char	name[130];

	bzero(name, 128);

	if (ioctl(js_fd[0], JSIOCGNAME(128), name) > 0)
	{
		printf("Using %s (%s) as joystick1\n", name, js_device[0]);

		for (int i = 1; i < 8; i++)
		{
			if (js_fd[i] > 0)
			{
				ioctl(js_fd[i], JSIOCGNAME(128), name);
				printf ("  and %s (%s) as joystick%d\n", name, js_device[i], i + 1);
			}
		}
	}
	else
#endif
	{
		ioctl(js_fd[0], JSIOCGAXES, &axes);
		ioctl(js_fd[0], JSIOCGBUTTONS, &buttons);
		printf("Using %d-axis %d-button joystick (%s) as joystick1\n", axes, buttons, js_device[0]);

		for (int i = 1; i < 8; i++)
		{
			if (js_fd[i] > 0)
			{
				ioctl(js_fd[i], JSIOCGAXES, &axes);
				ioctl(js_fd[i], JSIOCGBUTTONS, &buttons);
				printf("  and %d-axis %d-button joystick (%s) as joystick%d\n", axes, buttons, js_device[i], i + 1);
			}
		}
	}
#endif
}

static bool8 ReadJoysticks (void)
{
	// track if ANY joystick event happened this frame
	int js_latch = FALSE;
#ifdef JSIOCGVERSION
	struct js_event	js_ev;

	for (int i = 0; i < 8; i++)
	{
		// Try to reopen unplugged sticks
		if (js_unplugged[i])
		{
			js_fd[i] = open(js_device[i], O_RDONLY | O_NONBLOCK);
			if (js_fd[i] >= 0)
			{
				fprintf(stderr,"Joystick %d reconnected.\n",i);
				js_unplugged[i] = FALSE;
				js_latch = TRUE;
			}
		}

		// skip sticks without valid file desc
		if (js_fd[i] < 0) continue;

		while (read(js_fd[i], &js_ev, sizeof(struct js_event)) == sizeof(struct js_event))
		{
			switch (js_ev.type)
			{
				case JS_EVENT_AXIS:
					S9xReportAxis(0x8000c000 | (i << 24) | js_ev.number, js_ev.value);
					S9xReportAxis(0x80008000 | (i << 24) | (js_mod[i] << 16) | js_ev.number, js_ev.value);
					js_latch = TRUE;
					break;

				case JS_EVENT_BUTTON:
				case JS_EVENT_BUTTON | JS_EVENT_INIT:
					S9xReportButton(0x80004000 | (i << 24) | js_ev.number, js_ev.value);
					S9xReportButton(0x80000000 | (i << 24) | (js_mod[i] << 16) | js_ev.number, js_ev.value);
					js_latch = TRUE;
					break;
			}
		}

		/* EAGAIN is returned when the queue is empty */
		if (errno != EAGAIN) {
			// Error reading joystick.
			fprintf(stderr,"Error reading joystick %d!\n",i);

			// Mark for reconnect attempt.
			js_unplugged[i] = TRUE;

			for (unsigned int j = 0; j < 16; j++)
			{
				// Center all axis
				S9xReportAxis(0x8000c000 | (i << 24) | j, 0);
				S9xReportAxis(0x80008000 | (i << 24) | (js_mod[i] << 16) | j, 0);
				// Unpress all buttons.
				S9xReportButton(0x80004000 | (i << 24) | j, 0);
				S9xReportButton(0x80000000 | (i << 24) | (js_mod[i] << 16) | j, 0);
			}

			js_latch = TRUE;
		}
	}
#endif
	return js_latch;
}

#endif

void S9xSamplesAvailable(void *data)
{
#ifndef NOSOUND

    int samples_to_write;
	static uint8 *sound_buffer = NULL;
	static int sound_buffer_size = 0;

#ifdef ALSA
    snd_pcm_sframes_t frames_written, frames;
    frames = snd_pcm_avail(so.pcm_handle);

    if (frames < 0)
    {
        frames = snd_pcm_recover(so.pcm_handle, frames, 1);
        return;
    }
#endif

    if (Settings.DynamicRateControl)
    {
#ifndef ALSA
        S9xUpdateDynamicRate(s_AudioOutput->GetFreeBufferSize(), so.fragment_size * 4);
#else
        S9xUpdateDynamicRate(snd_pcm_frames_to_bytes(so.pcm_handle, frames),
                             so.output_buffer_size);
#endif
    }

    samples_to_write = S9xGetSampleCount();

    if (samples_to_write < 0)
        return;

#ifdef ALSA
    if (Settings.DynamicRateControl && !Settings.SoundSync)
    {
        // Using rate control, we should always keep the emulator's sound buffers empty to
        // maintain an accurate measurement.
        if (frames < samples_to_write/2)
        {
            S9xClearSamples();
            return;
        }
    }

    if (Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        snd_pcm_nonblock(so.pcm_handle, 0);
        frames = samples_to_write/2;
    } else {
        snd_pcm_nonblock(so.pcm_handle, 1);
        frames = MIN(frames, samples_to_write/2);
    }

    int bytes = snd_pcm_frames_to_bytes(so.pcm_handle, frames);
    if (bytes <= 0)
        return;

    if (sound_buffer_size < bytes || sound_buffer == NULL)
    {
        sound_buffer = (uint8 *)realloc(sound_buffer, bytes);
        sound_buffer_size = bytes;
    }
#else //OSS
    if (sound_buffer_size < samples_to_write * 2)
    {
        sound_buffer = (uint8 *)realloc(sound_buffer, samples_to_write * 2);
        sound_buffer_size = samples_to_write * 2;
    }
#endif

#ifndef ALSA
    S9xMixSamples(sound_buffer, samples_to_write);
    s_AudioOutput->Write(sound_buffer, samples_to_write * 2);
#else
    S9xMixSamples(sound_buffer, frames*2);
    frames_written = 0;

    while (frames_written < frames) {
        int result;

        result = snd_pcm_writei(so.pcm_handle,
                                sound_buffer +
                                    snd_pcm_frames_to_bytes(so.pcm_handle, frames_written),
                                frames - frames_written);
        if (result < 0)
        {
            result = snd_pcm_recover(so.pcm_handle, result, 1);

            if (result < 0)
            {
                break;
            }
        }
        else
        {
            frames_written += result;
        }
    }
#endif //ALSA
#endif //NOSOUND
}

bool8 S9xOpenSoundDevice (void)
{
#ifndef NOSOUND
#ifndef ALSA
	int	J, K;

	so.sound_fd = open(sound_device, O_WRONLY | O_NONBLOCK);
	if (so.sound_fd == -1)
	{
		fprintf(stderr, "ERROR: Failed to open sound device %s for writing.\n\t(Try loading snd-pcm-oss module?)\n", sound_device);
		return (FALSE);
	}

	s_AudioOutput = new S9xAudioOutput(
		so.sound_fd,
		Settings.SoundPlaybackRate,
		bool(unixSettings.ThreadSound)
	);

	J = log2(unixSettings.SoundFragmentSize) | (4 << 16);
	if (ioctl(so.sound_fd, SNDCTL_DSP_SETFRAGMENT, &J) == -1)
		return (FALSE);

	J = K = AFMT_S16_NE;
	if (ioctl(so.sound_fd, SNDCTL_DSP_SETFMT,      &J) == -1 || J != K)
		return (FALSE);

	J = K = 1;
	if (ioctl(so.sound_fd, SNDCTL_DSP_STEREO,      &J) == -1 || J != K)
		return (FALSE);

	J = K = Settings.SoundPlaybackRate;
	if (ioctl(so.sound_fd, SNDCTL_DSP_SPEED,       &J) == -1 || J != K)
		return (FALSE);

	J = 0;
	if (ioctl(so.sound_fd, SNDCTL_DSP_GETBLKSIZE,  &J) == -1)
		return (FALSE);

	so.fragment_size = J;
	printf("fragment size: %d\n", J);

#else

    int err;
    unsigned int periods = 8;
    unsigned int buffer_size =  unixSettings.SoundBufferSize * 1000;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_uframes_t alsa_buffer_size, alsa_period_size;
    unsigned int min = 0;
    unsigned int max = 0;

    unsigned int rate = Settings.SoundPlaybackRate;

    err = snd_pcm_open(&so.pcm_handle, sound_device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (err < 0) {
        printf("Failed: %s\n", snd_strerror(err));
        return (FALSE);
    }

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(so.pcm_handle, hw_params);
    snd_pcm_hw_params_set_format(so.pcm_handle, hw_params, SND_PCM_FORMAT_S16);
    snd_pcm_hw_params_set_access(so.pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_rate_resample(so.pcm_handle, hw_params, 0);
    snd_pcm_hw_params_set_channels(so.pcm_handle, hw_params, 2);

    snd_pcm_hw_params_get_rate_min(hw_params, &min, NULL);
    snd_pcm_hw_params_get_rate_max(hw_params, &max, NULL);
    fprintf(stderr, "Alsa available rates: %d to %d\n", min, max);

    if (rate > max && rate < min)
    {
        fprintf(stderr, "Rate %d not available. Using %d instead.\n", rate, max);
        rate = max;
    }
    snd_pcm_hw_params_set_rate_near(so.pcm_handle, hw_params, &rate, NULL);


    snd_pcm_hw_params_get_buffer_time_min(hw_params, &min, NULL);
    snd_pcm_hw_params_get_buffer_time_max(hw_params, &max, NULL);
    fprintf(stderr, "Alsa available buffer sizes: %dms to %dms\n", min / 1000, max / 1000);
    if (buffer_size < min && buffer_size > max)
    {
        fprintf(stderr, "Buffer size %dms not available. Using %d instead.\n", buffer_size / 1000, (min + max) / 2000);
        buffer_size = (min + max) / 2;
    }
    snd_pcm_hw_params_set_buffer_time_near(so.pcm_handle, hw_params, &buffer_size, NULL);


    snd_pcm_hw_params_get_periods_min(hw_params, &min, NULL);
    snd_pcm_hw_params_get_periods_max(hw_params, &max, NULL);
    fprintf(stderr, "Alsa period ranges: %d to %d blocks\n", min, max);
    if (periods > max)
        periods = max;
    snd_pcm_hw_params_set_periods_near(so.pcm_handle, hw_params, &periods, NULL);

    err = snd_pcm_hw_params(so.pcm_handle, hw_params);
    if (err < 0)
    {
        fprintf(stderr, "Alsa error: unable to install hw params\n");
        snd_pcm_drain(so.pcm_handle);
        snd_pcm_close(so.pcm_handle);
        so.pcm_handle = NULL;
        return (FALSE);
    }


    snd_pcm_sw_params_alloca(&sw_params);
    snd_pcm_sw_params_current(so.pcm_handle, sw_params);
    snd_pcm_get_params(so.pcm_handle, &alsa_buffer_size, &alsa_period_size);

    /* Don't start until we're [nearly] full */
    snd_pcm_sw_params_set_start_threshold(so.pcm_handle, sw_params, (alsa_buffer_size / 2));


    err = snd_pcm_sw_params(so.pcm_handle, sw_params);
    if (err < 0) {
        fprintf(stderr, "Alsa error: unable to install sw params\n");
        snd_pcm_drain(so.pcm_handle);
        snd_pcm_close(so.pcm_handle);
        so.pcm_handle = NULL;
        return (FALSE);
    }

    err = snd_pcm_prepare(so.pcm_handle);
    if (err < 0) {
        fprintf(stderr, "Alsa error: unable to prepare audio: %s\n", snd_strerror(err));
        snd_pcm_drain(so.pcm_handle);
        snd_pcm_close(so.pcm_handle);
        so.pcm_handle = NULL;
        return (FALSE);
    }

    so.output_buffer_size = snd_pcm_frames_to_bytes(so.pcm_handle, alsa_buffer_size);
#endif //ALSA
#endif //NOSOUND
	S9xSetSamplesAvailableCallback(S9xSamplesAvailable, NULL);

	return (TRUE);
}


void S9xExit (void)
{
	S9xMovieShutdown();

	S9xSetSoundMute(TRUE);
	Settings.StopEmulation = TRUE;

#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay)
		S9xNPDisconnect();
#endif

#if !defined(NOSOUND) && !defined(ALSA)
	delete s_AudioOutput;
#endif

	Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR));
	S9xResetSaveTimer(FALSE);
	S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	S9xUnmapAllControls();
	S9xDeinitDisplay();
	Memory.Deinit();
	S9xDeinitAPU();

	exit(0);
}

#ifdef DEBUGGER
static void sigbrkhandler (int)
{
	CPU.Flags |= DEBUG_MODE_FLAG;
	signal(SIGINT, (SIG_PF) sigbrkhandler);
}
#endif

int main (int argc, char **argv)
{
	if (argc < 2)
		S9xUsage();

	printf("\n\nSnes9x " VERSION " for unix\n");

	snprintf(default_dir, PATH_MAX + 1, "%s%s%s", getenv("HOME"), SLASH_STR, ".snes9x");
	s9x_base_dir = default_dir;

	memset(&Settings, 0, sizeof(Settings));
	Settings.MouseMaster = TRUE;
	Settings.SuperScopeMaster = TRUE;
	Settings.JustifierMaster = TRUE;
	Settings.MultiPlayer5Master = TRUE;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.SixteenBitSound = TRUE;
	Settings.Stereo = TRUE;
	Settings.SoundPlaybackRate = 48000;
	Settings.SoundInputRate = 31950;
	Settings.Transparency = TRUE;
	Settings.AutoDisplayMessages = TRUE;
	Settings.InitialInfoStringTimeout = 120;
	Settings.HDMATimingHack = 100;
	Settings.BlockInvalidVRAMAccessMaster = TRUE;
	Settings.StopEmulation = TRUE;
	Settings.WrongMovieStateProtection = TRUE;
	Settings.DumpStreamsMaxFrames = -1;
	Settings.StretchScreenshots = 1;
	Settings.SnapshotScreenshots = TRUE;
	Settings.SkipFrames = AUTO_FRAMERATE;
	Settings.TurboSkipFrames = 15;
	Settings.CartAName[0] = 0;
	Settings.CartBName[0] = 0;
#ifdef NETPLAY_SUPPORT
	Settings.ServerName[0] = 0;
#endif

#ifdef JOYSTICK_SUPPORT
	unixSettings.JoystickEnabled = TRUE;
#else
	unixSettings.JoystickEnabled = FALSE;
#endif
	unixSettings.ThreadSound = TRUE;
	unixSettings.SoundBufferSize = 100;
	unixSettings.SoundFragmentSize = 2048;

	unixSettings.rewindBufferSize = 0;
	unixSettings.rewindGranularity = 1;

	memset(&so, 0, sizeof(so));

	rewinding = false;

	CPU.Flags = 0;

	S9xLoadConfigFiles(argv, argc);
	rom_filename = S9xParseArgs(argv, argc);
	S9xDeleteCheats();

	make_snes9x_dirs();

	if (!Memory.Init() || !S9xInitAPU())
	{
		fprintf(stderr, "Snes9x: Memory allocation failure - not enough RAM/virtual memory available.\nExiting...\n");
		Memory.Deinit();
		S9xDeinitAPU();
		exit(1);
	}

	S9xInitSound(0);
	S9xSetSoundMute(TRUE);

	S9xReportControllers();

	uint32	saved_flags = CPU.Flags;
	bool8	loaded = FALSE;

	if (Settings.Multi)
	{
		loaded = Memory.LoadMultiCart(Settings.CartAName, Settings.CartBName);

		if (!loaded)
		{
			char	s1[PATH_MAX + 1], s2[PATH_MAX + 1];
			char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

			s1[0] = s2[0] = 0;

			if (Settings.CartAName[0])
			{
				_splitpath(Settings.CartAName, drive, dir, fname, ext);
				snprintf(s1, PATH_MAX + 1, "%s%s%s", S9xGetDirectory(ROM_DIR), SLASH_STR, fname);
				if (ext[0] && (strlen(s1) <= PATH_MAX - 1 - strlen(ext)))
				{
					strcat(s1, ".");
					strcat(s1, ext);
				}
			}

			if (Settings.CartBName[0])
			{
				_splitpath(Settings.CartBName, drive, dir, fname, ext);
				snprintf(s2, PATH_MAX + 1, "%s%s%s", S9xGetDirectory(ROM_DIR), SLASH_STR, fname);
				if (ext[0] && (strlen(s2) <= PATH_MAX - 1 - strlen(ext)))
				{
					strcat(s2, ".");
					strcat(s2, ext);
				}
			}

			loaded = Memory.LoadMultiCart(s1, s2);
		}
	}
	else
	if (rom_filename)
	{
		loaded = Memory.LoadROM(rom_filename);

		if (!loaded && rom_filename[0])
		{
			char	s[PATH_MAX + 1];
			char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

			_splitpath(rom_filename, drive, dir, fname, ext);
			snprintf(s, PATH_MAX + 1, "%s%s%s", S9xGetDirectory(ROM_DIR), SLASH_STR, fname);
			if (ext[0] && (strlen(s) <= PATH_MAX - 1 - strlen(ext)))
			{
				strcat(s, ".");
				strcat(s, ext);
			}

			loaded = Memory.LoadROM(s);
		}
	}

	if (!loaded)
	{
		fprintf(stderr, "Error opening the ROM file.\n");
		exit(1);
	}

	S9xDeleteCheats();
	S9xCheatsEnable();
	NSRTControllerSetup();
	Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR));

	if (Settings.ApplyCheats)
	{
		S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	}

	S9xParseArgsForCheats(argv, argc);

	CPU.Flags = saved_flags;
	Settings.StopEmulation = FALSE;

#ifdef DEBUGGER
	struct sigaction sa;
	sa.sa_handler = sigbrkhandler;
#ifdef SA_RESTART
	sa.sa_flags = SA_RESTART;
#else
	sa.sa_flags = 0;
#endif
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
#endif

	S9xInitInputDevices();
	S9xInitDisplay(argc, argv);
	S9xSetupDefaultKeymap();
	S9xTextMode();

#ifdef NETPLAY_SUPPORT
	if (strlen(Settings.ServerName) == 0)
	{
		char	*server = getenv("S9XSERVER");
		if (server)
		{
			strncpy(Settings.ServerName, server, 127);
			Settings.ServerName[127] = 0;
		}
	}

	char	*port = getenv("S9XPORT");
	if (Settings.Port >= 0 && port)
		Settings.Port = atoi(port);
	else
	if (Settings.Port < 0)
		Settings.Port = -Settings.Port;

	if (Settings.NetPlay)
	{
		NetPlay.MaxFrameSkip = 10;

		unixSettings.rewindBufferSize = 0;

		if (!S9xNPConnectToServer(Settings.ServerName, Settings.Port, Memory.ROMName))
		{
			fprintf(stderr, "Failed to connect to server %s on port %d.\n", Settings.ServerName, Settings.Port);
			S9xExit();
		}

		fprintf(stderr, "Connected to server %s on port %d as player #%d playing %s.\n", Settings.ServerName, Settings.Port, NetPlay.Player, Memory.ROMName);
	}
#endif

	if (play_smv_filename)
	{
		uint32	flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
		if (S9xMovieOpen(play_smv_filename, TRUE) != SUCCESS)
			exit(1);
		CPU.Flags |= flags;
	}
	else
	if (record_smv_filename)
	{
		uint32	flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
		if (S9xMovieCreate(record_smv_filename, 0xFF, MOVIE_OPT_FROM_RESET, NULL, 0) != SUCCESS)
			exit(1);
		CPU.Flags |= flags;
	}
	else
	{
		if (snapshot_filename)
		{
			uint32	flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
			if (!S9xUnfreezeGame(snapshot_filename))
				exit(1);
			CPU.Flags |= flags;
		}
		if (unixSettings.rewindBufferSize)
		{
			stateMan.init(unixSettings.rewindBufferSize * 1024 * 1024);
		}
	}

	S9xGraphicsMode();

	sprintf(String, "\"%s\" %s: %s", Memory.ROMName, TITLE, VERSION);
	S9xSetTitle(String);

#ifdef JOYSTICK_SUPPORT
	uint32	JoypadSkip = 0;
#endif

	S9xSetSoundMute(FALSE);

#ifdef NETPLAY_SUPPORT
	bool8	NP_Activated = Settings.NetPlay;
#endif

	while (1)
	{
	#ifdef NETPLAY_SUPPORT
		if (NP_Activated)
		{
			if (NetPlay.PendingWait4Sync && !S9xNPWaitForHeartBeatDelay(100))
			{
				S9xProcessEvents(FALSE);
				continue;
			}

			for (int J = 0; J < 8; J++)
				old_joypads[J] = MovieGetJoypad(J);

			for (int J = 0; J < 8; J++)
				MovieSetJoypad(J, joypads[J]);

			if (NetPlay.Connected)
			{
				if (NetPlay.PendingWait4Sync)
				{
					NetPlay.PendingWait4Sync = FALSE;
					NetPlay.FrameCount++;
					S9xNPStepJoypadHistory();
				}
			}
			else
			{
				fprintf(stderr, "Lost connection to server.\n");
				S9xExit();
			}
		}
	#endif

	#ifdef DEBUGGER
		if (!Settings.Paused || (CPU.Flags & (DEBUG_MODE_FLAG | SINGLE_STEP_FLAG)))
	#else
		if (!Settings.Paused)
	#endif
		{
			if(rewinding)
			{
				uint16 joypads[8];
				for (int i = 0; i < 8; i++)
					joypads[i] = MovieGetJoypad(i);

				rewinding = stateMan.pop();

				for (int i = 0; i < 8; i++)
					MovieSetJoypad (i, joypads[i]);
			}
			else if(IPPU.TotalEmulatedFrames % unixSettings.rewindGranularity == 0)
				stateMan.push();

			S9xMainLoop();
		}
                if (Settings.Paused && frame_advance)
                {
                        S9xMainLoop();
                        frame_advance = 0;
                }

	#ifdef NETPLAY_SUPPORT
		if (NP_Activated)
		{
			for (int J = 0; J < 8; J++)
				MovieSetJoypad(J, old_joypads[J]);
		}
	#endif

	#ifdef DEBUGGER
		if (Settings.Paused || (CPU.Flags & DEBUG_MODE_FLAG))
	#else
		if (Settings.Paused)
	#endif
			S9xSetSoundMute(TRUE);

	#ifdef DEBUGGER
		if (CPU.Flags & DEBUG_MODE_FLAG)
			S9xDoDebug();
		else
	#endif
		if (Settings.Paused)
		{
			S9xProcessEvents(FALSE);
			usleep(100000);
		}

	#ifdef JOYSTICK_SUPPORT
		if (unixSettings.JoystickEnabled && (JoypadSkip++ & 1) == 0)
		{
			if (ReadJoysticks() == TRUE) {
				S9xLatchJSEvent();
			}
		}
	#endif

		S9xProcessEvents(FALSE);

	#ifdef DEBUGGER
		if (!Settings.Paused && !(CPU.Flags & DEBUG_MODE_FLAG))
	#else
		if (!Settings.Paused)
	#endif
			S9xSetSoundMute(FALSE);
	}

	return (0);
}
