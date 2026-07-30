/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// Voicer-kun audio-CD playback (issue #169). See docs/Angelique.md for the
// reverse-engineered spec. The user attaches the game's CUE/BIN CD image
// (Sound > Voice Kun > Attach Audio CD, .cue or .zip); the disc is
// verified against the game's known profile, and when the ROM's play-voice
// routine runs we stream the CD track it computed, mixed like MSU-1 audio.

#include "voicekun.h"
#include "memmap.h"
#include "display.h"
#include "ppu.h"
#include "apu/resampler.h"
#ifdef UNZIP_SUPPORT
#  ifdef SYSTEM_ZIP
#    include <minizip/unzip.h>
#  else
#    include "unzip/unzip.h"
#  endif
#endif
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

struct SVoiceKunHook VoiceKunHook = { 0, 0, 0, 0 };

// Games that use the Voicer-kun, with the disc profile the attach is
// verified against. The hook PC is the routine that receives the voice id
// in A with the CD track derivable as A + bias.
struct VoiceKunGame
{
	uint32		rom_crc32;
	uint32		play_pc;        // entry that requests a voice
	uint32		stop_pc;        // entry that ends the current voice
	int32		arg_stack_off;  // where the track argument sits at play_pc
	int32		track_bias;
	uint32		manual_flag;    // WRAM offset that is 0 in manual mode (0 = skip)
	int32		disc_tracks;    // audio tracks on the CD
	long		disc_size;      // BIN size in bytes (raw 2352-byte sectors)
	uint32		disc_head_crc;  // crc32 of the BIN's first 64 KB
	const char	*title;
};

static const VoiceKunGame voicekun_games[] =
{
	// $C98EAC(track): sets the voice-active flag, then drives the IR path or
	// (manual mode) the on-screen "N番目" prompt. $C98EED ends the voice.
	{ 0xEA6AE8A9, 0xC98EAC, 0xC98EED, 4, 0, 0x6844, 90, 558644688L, 0xD7978EEB,
	  "Angelique Voice Fantasy (J)" },
};

struct VoiceKunTrack
{
	long	start;      // byte offset of INDEX 01 in the BIN
	long	end;        // byte offset one past the last sample
};

static std::string					cue_path;
static std::vector<VoiceKunTrack>	tracks;      // [0] = CD track 1
static FILE							*bin_file  = NULL;   // .cue attach: stream from disk
static std::vector<uint8>			bin_mem;             // .zip attach: whole BIN in memory
static const VoiceKunGame			*game      = NULL;
static Resampler					*resampler = NULL;
static std::string					last_error = "";

static bool		playing   = false;
static long		play_pos  = 0;
static long		play_end  = 0;
static size_t	partial_frames = 0;

// Auto-confirm window, in emulated frames since the track started.
#define VOICEKUN_CONFIRM_START	18
#define VOICEKUN_CONFIRM_END	48
static uint32	confirm_from = 0;
static uint32	confirm_to   = 0;

const char *S9xVoiceKunLastError(void)
{
	return last_error.c_str();
}

static const VoiceKunGame *FindGame(void)
{
	for (size_t i = 0; i < sizeof(voicekun_games) / sizeof(voicekun_games[0]); i++)
		if (voicekun_games[i].rom_crc32 == Memory.ROMCRC32)
			return &voicekun_games[i];
	return NULL;
}

bool S9xVoiceKunGameSupported(void)
{
	return FindGame() != NULL;
}

static uint32 CRC32Buf(const uint8 *buf, size_t len)
{
	static uint32	table[256];
	static bool		init = false;
	if (!init)
	{
		for (uint32 n = 0; n < 256; n++)
		{
			uint32	c = n;
			for (int k = 0; k < 8; k++)
				c = (c & 1) ? 0xEDB88320 ^ (c >> 1) : (c >> 1);
			table[n] = c;
		}
		init = true;
	}
	uint32	crc = 0xFFFFFFFF;
	for (size_t i = 0; i < len; i++)
		crc = table[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
	return crc ^ 0xFFFFFFFF;
}

static long MSFToBytes(int m, int s, int f)
{
	return ((long)(m * 60 + s) * 75 + f) * 2352;
}

// Minimal CUE parser: single FILE, audio tracks only, INDEX 00/01 in MSF.
// Returns the FILE entry's name in bin_name; track ends resolve later.
static bool ParseCue(const std::string &content, std::string &bin_name,
					 std::vector<long> &starts, std::vector<long> &index0s)
{
	starts.clear();
	index0s.clear();
	bin_name.clear();

	size_t	pos = 0;
	int		cur_track = 0;
	long	cur_index0 = -1;
	bool	non_audio = false;

	while (pos < content.size())
	{
		size_t	eol = content.find('\n', pos);
		if (eol == std::string::npos)
			eol = content.size();
		std::string	line = content.substr(pos, eol - pos);
		pos = eol + 1;

		size_t	b = line.find_first_not_of(" \t\r");
		if (b == std::string::npos)
			continue;
		const char	*p = line.c_str() + b;

		if (!strncasecmp(p, "FILE", 4))
		{
			if (!bin_name.empty())
			{
				last_error = "multi-file cue sheets are not supported";
				return false;
			}
			size_t	q1 = line.find('"', b);
			size_t	q2 = (q1 == std::string::npos) ? std::string::npos : line.find('"', q1 + 1);
			if (q2 == std::string::npos)
			{
				last_error = "malformed FILE line in cue sheet";
				return false;
			}
			bin_name = line.substr(q1 + 1, q2 - q1 - 1);
		}
		else
		if (!strncasecmp(p, "TRACK", 5))
		{
			cur_track = atoi(p + 5);
			cur_index0 = -1;
			if (!strstr(p, "AUDIO"))
				non_audio = true;
		}
		else
		if (!strncasecmp(p, "INDEX", 5))
		{
			int	idx, mm, ss, ff;
			if (sscanf(p + 5, "%d %d:%d:%d", &idx, &mm, &ss, &ff) == 4 && cur_track > 0)
			{
				long	off = MSFToBytes(mm, ss, ff);
				if (idx == 0)
					cur_index0 = off;
				else
				if (idx == 1)
				{
					starts.push_back(off);
					index0s.push_back(cur_index0);
				}
			}
		}
	}

	if (non_audio)
	{
		last_error = "cue sheet contains non-audio tracks";
		return false;
	}
	if (starts.empty() || bin_name.empty())
	{
		last_error = "no audio tracks found in cue sheet";
		return false;
	}
	return true;
}

// Build the track table and verify the disc against the game profile.
static bool ValidateDisc(const std::vector<long> &starts, const std::vector<long> &index0s,
						 long bin_size, const uint8 *head, size_t head_len)
{
	if ((int)starts.size() != game->disc_tracks)
	{
		last_error = "track count mismatch - this is not the game's audio CD";
		return false;
	}
	if (bin_size != game->disc_size)
	{
		last_error = "data size mismatch - this is not the game's audio CD";
		return false;
	}
	if (CRC32Buf(head, head_len) != game->disc_head_crc)
	{
		last_error = "data fingerprint mismatch - this is not the game's audio CD";
		return false;
	}

	tracks.resize(starts.size());
	for (size_t i = 0; i < starts.size(); i++)
	{
		if (starts[i] < 0 || starts[i] >= bin_size ||
			(i > 0 && starts[i] <= starts[i - 1]))
		{
			last_error = "cue sheet track offsets are inconsistent";
			tracks.clear();
			return false;
		}
		tracks[i].start = starts[i];
		if (i + 1 < starts.size())
			tracks[i].end = (index0s[i + 1] >= 0) ? index0s[i + 1] : starts[i + 1];
		else
			tracks[i].end = bin_size;
	}
	return true;
}

static bool ReadFileAll(const char *path, std::string &out)
{
	FILE	*f = fopen(path, "rb");
	if (!f)
		return false;
	fseek(f, 0, SEEK_END);
	long	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	out.resize(size);
	bool	ok = size == 0 || fread(&out[0], 1, size, f) == (size_t)size;
	fclose(f);
	return ok;
}

static bool AttachFromCue(const char *path)
{
	std::string	content;
	if (!ReadFileAll(path, content))
	{
		last_error = "could not open the cue sheet";
		return false;
	}

	std::string			bin_name;
	std::vector<long>	starts, index0s;
	if (!ParseCue(content, bin_name, starts, index0s))
		return false;

	// resolve the BIN next to the CUE
	std::string	full(path);
	size_t		sep = full.find_last_of("/\\");
	std::string	bin_path = ((sep == std::string::npos) ? "" : full.substr(0, sep + 1)) + bin_name;

	FILE	*f = fopen(bin_path.c_str(), "rb");
	if (!f)
	{
		last_error = "could not open the data file named by the cue sheet";
		return false;
	}
	fseek(f, 0, SEEK_END);
	long	bin_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8	head[65536];
	size_t	head_len = fread(head, 1, sizeof(head), f);
	if (!ValidateDisc(starts, index0s, bin_size, head, head_len))
	{
		fclose(f);
		return false;
	}

	bin_file = f;
	return true;
}

#ifdef UNZIP_SUPPORT
// Load a member into out; member name match is by basename, case-insensitive.
static bool ZipReadMember(unzFile zf, const char *want_ext, const char *want_name,
						  std::vector<uint8> &out, std::string *found_name)
{
	for (int port = unzGoToFirstFile(zf); port == UNZ_OK; port = unzGoToNextFile(zf))
	{
		unz_file_info	info;
		char			name[256];
		if (unzGetCurrentFileInfo(zf, &info, name, sizeof(name) - 1, NULL, 0, NULL, 0) != UNZ_OK)
			continue;

		const char	*base = strrchr(name, '/');
		base = base ? base + 1 : name;

		if (want_name && strcasecmp(base, want_name))
			continue;
		if (want_ext)
		{
			size_t	nl = strlen(base), el = strlen(want_ext);
			if (nl < el || strcasecmp(base + nl - el, want_ext))
				continue;
		}

		if (unzOpenCurrentFile(zf) != UNZ_OK)
			return false;
		out.resize((size_t)info.uncompressed_size);
		bool	ok = out.empty() ||
			unzReadCurrentFile(zf, &out[0], (unsigned)out.size()) == (int)out.size();
		unzCloseCurrentFile(zf);
		if (ok && found_name)
			*found_name = base;
		return ok;
	}
	return false;
}

static bool AttachFromZip(const char *path)
{
	unzFile	zf = unzOpen(path);
	if (!zf)
	{
		last_error = "could not open the zip file";
		return false;
	}

	std::vector<uint8>	cue_raw;
	if (!ZipReadMember(zf, ".cue", NULL, cue_raw, NULL))
	{
		last_error = "no .cue file inside the zip";
		unzClose(zf);
		return false;
	}

	std::string			content((const char *)cue_raw.data(), cue_raw.size());
	std::string			bin_name;
	std::vector<long>	starts, index0s;
	if (!ParseCue(content, bin_name, starts, index0s))
	{
		unzClose(zf);
		return false;
	}

	if (!ZipReadMember(zf, NULL, bin_name.c_str(), bin_mem, NULL))
	{
		last_error = "the data file named by the cue sheet is not in the zip";
		unzClose(zf);
		bin_mem.clear();
		return false;
	}
	unzClose(zf);

	size_t	head_len = bin_mem.size() < 65536 ? bin_mem.size() : 65536;
	if (!ValidateDisc(starts, index0s, (long)bin_mem.size(), bin_mem.data(), head_len))
	{
		bin_mem.clear();
		return false;
	}
	return true;
}
#endif

bool S9xVoiceKunAttach(const char *path)
{
	S9xVoiceKunDetach();
	last_error = "";

	bool	ok = false;
	size_t	len = strlen(path);
	bool	is_zip = len > 4 && !strcasecmp(path + len - 4, ".zip");

	game = FindGame();
	if (!game)
		last_error = "the loaded ROM is not a supported Voice Kun game";
	else
	if (is_zip)
	{
#ifdef UNZIP_SUPPORT
		ok = AttachFromZip(path);
#else
		last_error = "zip support is not compiled in";
#endif
	}
	else
		ok = AttachFromCue(path);

	if (!ok)
	{
		tracks.clear();
		game = NULL;
		printf("Voice Kun: attach failed: %s\n", last_error.c_str());
		return false;
	}

	cue_path = path;
	VoiceKunHook.PlayPC      = game->play_pc;
	VoiceKunHook.StopPC      = game->stop_pc;
	VoiceKunHook.ArgStackOff = game->arg_stack_off;
	VoiceKunHook.TrackBias   = game->track_bias;

	partial_frames = 0;
	if (resampler)
		resampler->clear();

	Settings.VoiceKun = TRUE;

	printf("Voice Kun: attached %s (%d tracks, %s)\n", path, (int)tracks.size(), game->title);
	return true;
}

void S9xVoiceKunDetach(void)
{
	if (bin_file)
	{
		fclose(bin_file);
		bin_file = NULL;
	}
	bin_mem.clear();
	std::vector<uint8>().swap(bin_mem);   // actually release the memory
	tracks.clear();
	cue_path.clear();
	game    = NULL;
	playing = false;
	VoiceKunHook.PlayPC = VoiceKunHook.StopPC = 0;
	Settings.VoiceKun = FALSE;
}

bool S9xVoiceKunAttached(void)
{
	return (bin_file != NULL || !bin_mem.empty());
}

const char *S9xVoiceKunCuePath(void)
{
	return cue_path.c_str();
}

int S9xVoiceKunTrackCount(void)
{
	return (int)tracks.size();
}

const char *S9xVoiceKunGameTitle(void)
{
	return game ? game->title : NULL;
}

void S9xVoiceKunPlayTrack(int track)
{
	if (!S9xVoiceKunAttached() || track < 1 || track > (int)tracks.size())
		return;

	play_pos = tracks[track - 1].start;
	play_end = tracks[track - 1].end;
	if (bin_file)
		fseek(bin_file, play_pos, SEEK_SET);
	playing = true;

	confirm_from = IPPU.TotalEmulatedFrames + VOICEKUN_CONFIRM_START;
	confirm_to   = IPPU.TotalEmulatedFrames + VOICEKUN_CONFIRM_END;

	char	msg[128];
	snprintf(msg, sizeof(msg), "Voice Kun: playing track %d/%d (%.0fs)",
		track, (int)tracks.size(), (play_end - play_pos) / (2352.0 * 75.0));
	S9xSetInfoString(msg);
	printf("%s\n", msg);
}

void S9xVoiceKunStop(void)
{
	if (playing)
		printf("Voice Kun: voice ended\n");
	playing = false;
	confirm_from = confirm_to = 0;
}

bool S9xVoiceKunAutoConfirm(void)
{
	if (!confirm_to || !game)
		return false;

	// Only stand in for the player in manual mode; with the Voicer driver
	// active the game never waits for the keypress.
	if (game->manual_flag && Memory.RAM[game->manual_flag & 0x1ffff] != 0)
		return false;

	uint32	f = IPPU.TotalEmulatedFrames;
	if (f >= confirm_to)
	{
		confirm_from = confirm_to = 0;
		return false;
	}
	return (f >= confirm_from);
}

void S9xVoiceKunSetOutput(Resampler *r)
{
	resampler = r;
}

// Pumped from the SPC DSP output hook, same cadence as MSU-1: convert the
// 32040 Hz sample budget into 44100 Hz CD frames, push silence when idle.
void S9xVoiceKunGenerate(size_t sample_count)
{
	if (!resampler)
		return;

	partial_frames += 4410 * (sample_count / 2);

	while (partial_frames >= 3204)
	{
		int16	frame[2] = { 0, 0 };

		if (playing && play_pos + 4 <= play_end)
		{
			if (bin_file)
			{
				if (fread(frame, 1, 4, bin_file) == 4)
					play_pos += 4;
				else
					playing = false;
			}
			else
			{
				memcpy(frame, &bin_mem[play_pos], 4);
				play_pos += 4;
			}
			if (play_pos + 4 > play_end)
				playing = false;
		}
		else
			playing = false;

		resampler->push_sample(frame[0], frame[1]);
		partial_frames -= 3204;
	}
}
