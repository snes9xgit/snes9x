/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// Voicer-kun audio-CD playback (issue #169). See docs/Angelique.md for the
// reverse-engineered spec. The user attaches the game's CD image (Sound >
// Voicer-kun > Attach Audio CD, .cue or .zip); the disc is verified against
// the game's known profile, and when the ROM's play-voice routine runs we
// stream the CD track it computed, mixed like MSU-1 audio.
//
// Two cue layouts are in the wild: one big BIN with every track at an
// offset (Angelique), and one BIN per track (Emit). Both are supported, on
// disk and inside a zip. Games with more than one disc (Emit ships two) can
// have every disc attached at once.

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

struct SVoiceKunHook VoiceKunHook = { 0, 0, 0, 0, 0, 0 };

#define VOICEKUN_MAX_DISCS	2
#define VOICEKUN_FP_LEN		65536	// bytes hashed for the disc fingerprint

// A disc the attach is verified against. The fingerprint is taken from the
// middle of the disc: the first sectors of track 1 are digital silence on
// every disc seen so far, so hashing those matches everything.
struct VoiceKunDisc
{
	int32		tracks;
	long		size;       // total bytes across every BIN the cue names
	uint32		mid_crc;    // crc32 of VOICEKUN_FP_LEN bytes at size/2
	const char	*label;
};

struct VoiceKunGame
{
	uint32			rom_crc32;
	uint32			play_pc;        // entry that requests a voice
	uint32			stop_pc;        // entry that ends the current voice
	int32			arg_stack_off;  // where the track argument sits at play_pc
	int32			track_bias;
	uint32			manual_flag;    // WRAM offset that is 0 in manual mode (0 = skip)
	uint32			arm_pc;         // names the track; 0 when play_pc carries it
	int32			arm_arg_off;
	// Voicer mode drives the deck by IR instead of prompting; the transport
	// keys have to reach the stream or it runs ahead of the story.
	uint32			ir_cmd_pc;      // 0 = game has no IR transport path
	int32			ir_cmd_arg_off;
	// Manual mode asks the player to confirm the CD is rolling; we started it
	// ourselves, so the input layer can stand in for that press. Frames since
	// playback began; confirm_hold 0 leaves the player in charge.
	int32			confirm_first;
	int32			confirm_hold;
	int32			first_voice;    // tracks below this are not narration (0 = all)
	// Some games keep a voice counter that ticks once per voiced scene, even
	// when no CD prompt is shown (EMIT's "continuous play"). With the ROM's
	// chapter table that counter yields both the disc and the CD track, so
	// watching it catches every scene change.
	uint32			voice_id_wram;  // 0 = no counter
	uint32			chapter_rom;    // (disc, first track, count) x n_chapters
	uint32			chapter_base_rom;
	int32			n_chapters;
	int32			n_voices;
	// The device answers the port-2 auto-joypad read with an id in the low
	// nibble; the games poll $421A for it to decide the Voicer is plugged in.
	uint16			port2_id;
	const char		*title;
	int32			n_discs;
	VoiceKunDisc	discs[VOICEKUN_MAX_DISCS];
};

static const VoiceKunGame voicekun_games[] =
{
	// $C98EAC(track): sets the voice-active flag, then drives the IR path or
	// (manual mode) the on-screen "N番目" prompt. $C98EED ends the voice.
	{ 0xEA6AE8A9, 0xC98EAC, 0xC98EED, 4, 0, 0x6844, 0, 0,
	  0, 0,
	  18, 30, 0,
	  0, 0, 0, 0, 0, 0x000D,
	  "Angelique Voice Fantasy (J)", 1,
	  { { 90, 558644688L, 0xCAA36F2D, "CD" } } },

	// EMIT Vol. 1: two audio CDs, 98 tracks each. A scene change runs
	// $C1388E "pause at the start of track %02d" (arg at S+4, displayed as
	// arg+1) to cue the disc, a 3..1 countdown, then $C15D02 acts on PLAY -
	// so the track is armed at the cue prompt and started at the dispatcher.
	// Starting at the cue instead plays while the game is still saying
	// "pause here". $C13B08 ends the voice.
	// No auto-press here - injecting keypresses into the deck UI skips
	// story (measured: a whole voice scene lost).
	{ 0xD1AACC2D, 0xC15D02, 0xC13B08, 0, 1, 0x22B2, 0xC1388E, 4,
	  0xC90D2B, 4,
	  0, 0, 0,
	  0, 0x403D7, 0x403FB, 10, 74, 0x000D,
	  "EMIT Vol. 1: Toki no Maigo (J)", 2,
	  { { 98, 394863168L, 0x57FD0BE3, "CD 1" },
	    { 98, 508817568L, 0x4DAC56EF, "CD 2" } } },
};

struct VoiceKunTrack
{
	int		file;       // index into VoiceKunImage::files
	long	start;      // byte offset of INDEX 01 within that file
	long	end;        // byte offset one past the last sample
};

// One attached CD image.
struct VoiceKunImage
{
	std::string					path;       // the .cue or .zip the user picked
	bool						is_zip;
	int							profile;    // index into VoiceKunGame::discs
	std::vector<std::string>	files;      // cue FILE names, in cue order
	std::vector<long>			sizes;
	std::vector<VoiceKunTrack>	tracks;     // [0] = CD track 1
};

static std::vector<VoiceKunImage>	images;
static const VoiceKunGame			*game      = NULL;
static Resampler					*resampler = NULL;
static std::string					last_error = "";

// Playback state: which image/file is feeding the stream right now.
static int					cur_image = -1;
static FILE					*cur_fp   = NULL;   // disk images stream from here
static std::vector<uint8>	cur_mem;            // zip images decode a whole file here
static int					cur_file  = -1;     // file index cur_fp/cur_mem holds

static bool		playing     = false;
static bool		deck_paused = false;    // held mid-track, play_pos kept
// Voicer mode: the game works the deck by remote and only cues once per run of
// scenes. Manual mode prompts for every track, so there the disc must not roll
// on by itself or it gets ahead of the story.
static bool		deck_driven = false;
static int		armed_track = 0;    // named by ArmPC, started at PlayPC
static int		cur_track   = 0;    // track streaming, or the one just finished
// The track the game last asked for, which is not where the deck ends up once
// it rolls on. Prompts repeat, so they are matched against the request.
static int		cued_track  = 0;
static uint32	track_done  = 0;    // frame the current track ran out on

static long		play_pos  = 0;
static long		play_end  = 0;
static size_t	partial_frames = 0;

// A finished track can be asked for again - scenes are replayable from the
// game's index - but the prompt that asks for it also repeats every few
// seconds while a voice is current, so ignore the same track for a while.
#define VOICEKUN_REPLAY_GUARD	1500

static uint32	confirm_base = 0;   // frame the press schedule starts from
static bool		confirm_on   = false;

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

static const char *BaseName(const char *p)
{
	const char	*s = strrchr(p, '/');
	const char	*b = strrchr(p, '\\');
	if (b > s)
		s = b;
	return s ? s + 1 : p;
}

static long MSFToBytes(int m, int s, int f)
{
	return ((long)(m * 60 + s) * 75 + f) * 2352;
}

// A cue entry before file sizes are known: which FILE it belongs to and the
// INDEX 00/01 offsets within that file.
struct CueEntry
{
	int		file;
	long	index0;     // -1 when absent
	long	index1;
};

// Minimal CUE parser: audio tracks only, INDEX 00/01 in MSF, one or many
// FILE statements. Offsets are relative to the FILE the track sits under.
static bool ParseCue(const std::string &content, std::vector<std::string> &files,
					 std::vector<CueEntry> &entries)
{
	files.clear();
	entries.clear();

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
			size_t	q1 = line.find('"', b);
			size_t	q2 = (q1 == std::string::npos) ? std::string::npos : line.find('"', q1 + 1);
			if (q2 == std::string::npos)
			{
				last_error = "malformed FILE line in cue sheet";
				return false;
			}
			files.push_back(line.substr(q1 + 1, q2 - q1 - 1));
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
			if (sscanf(p + 5, "%d %d:%d:%d", &idx, &mm, &ss, &ff) == 4 &&
				cur_track > 0 && !files.empty())
			{
				long	off = MSFToBytes(mm, ss, ff);
				if (idx == 0)
					cur_index0 = off;
				else
				if (idx == 1)
				{
					CueEntry	e;
					e.file   = (int)files.size() - 1;
					e.index0 = cur_index0;
					e.index1 = off;
					entries.push_back(e);
				}
			}
		}
	}

	if (non_audio)
	{
		last_error = "cue sheet contains non-audio tracks";
		return false;
	}
	if (entries.empty() || files.empty())
	{
		last_error = "no audio tracks found in cue sheet";
		return false;
	}
	return true;
}

// Turn cue entries + file sizes into the track table. A track runs to the
// next track's pregap when they share a file, otherwise to end of file.
static bool BuildTracks(const std::vector<CueEntry> &entries, const std::vector<long> &sizes,
						std::vector<VoiceKunTrack> &out)
{
	out.clear();
	out.resize(entries.size());
	for (size_t i = 0; i < entries.size(); i++)
	{
		const CueEntry	&e = entries[i];
		if (e.file < 0 || e.file >= (int)sizes.size() ||
			e.index1 < 0 || e.index1 >= sizes[e.file])
		{
			last_error = "cue sheet track offsets are inconsistent";
			out.clear();
			return false;
		}
		out[i].file  = e.file;
		out[i].start = e.index1;
		if (i + 1 < entries.size() && entries[i + 1].file == e.file)
		{
			long	next = (entries[i + 1].index0 >= 0) ? entries[i + 1].index0
			                                            : entries[i + 1].index1;
			if (next <= e.index1)
			{
				last_error = "cue sheet track offsets are inconsistent";
				out.clear();
				return false;
			}
			out[i].end = next;
		}
		else
			out[i].end = sizes[e.file];
	}
	return true;
}

// Locate the file holding the disc's midpoint and the offset within it.
static void FingerprintSpot(const std::vector<long> &sizes, int *file, long *off, long *total)
{
	long	sum = 0;
	for (size_t i = 0; i < sizes.size(); i++)
		sum += sizes[i];
	*total = sum;

	long	target = sum / 2, acc = 0;
	*file = 0;
	*off  = 0;
	for (size_t i = 0; i < sizes.size(); i++)
	{
		if (acc + sizes[i] > target)
		{
			*file = (int)i;
			*off  = target - acc;
			// keep the whole window inside this file
			if (*off + VOICEKUN_FP_LEN > sizes[i])
				*off = sizes[i] > VOICEKUN_FP_LEN ? sizes[i] - VOICEKUN_FP_LEN : 0;
			return;
		}
		acc += sizes[i];
	}
}

// Match a parsed image against the game's disc profiles; returns the profile
// index or -1, and sets last_error on a miss.
static int MatchProfile(int n_tracks, long total, uint32 mid_crc)
{
	bool	size_seen = false;
	for (int d = 0; d < game->n_discs; d++)
	{
		const VoiceKunDisc	&p = game->discs[d];
		if (p.tracks != n_tracks)
			continue;
		if (p.size != total)
		{
			size_seen = true;
			continue;
		}
		if (p.mid_crc != mid_crc)
		{
			size_seen = true;
			continue;
		}
		return d;
	}
	last_error = size_seen
		? "data does not match this game's audio CD (wrong disc or bad dump)"
		: "track count does not match this game's audio CD";
	return -1;
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

static bool AttachFromCue(const char *path, VoiceKunImage &img)
{
	std::string	content;
	if (!ReadFileAll(path, content))
	{
		last_error = "could not open the cue sheet";
		return false;
	}

	std::vector<std::string>	names;
	std::vector<CueEntry>		entries;
	if (!ParseCue(content, names, entries))
		return false;

	std::string	full(path);
	size_t		sep = full.find_last_of("/\\");
	std::string	dir = (sep == std::string::npos) ? "" : full.substr(0, sep + 1);

	// resolve every FILE next to the cue and collect its size
	for (size_t i = 0; i < names.size(); i++)
	{
		std::string	p = dir + BaseName(names[i].c_str());
		FILE		*f = fopen(p.c_str(), "rb");
		if (!f)
		{
			last_error = "a data file named by the cue sheet is missing";
			return false;
		}
		fseek(f, 0, SEEK_END);
		img.sizes.push_back(ftell(f));
		fclose(f);
		img.files.push_back(p);
	}

	int		fp_file;
	long	fp_off, total;
	FingerprintSpot(img.sizes, &fp_file, &fp_off, &total);

	std::vector<uint8>	buf(VOICEKUN_FP_LEN);
	FILE				*f = fopen(img.files[fp_file].c_str(), "rb");
	if (!f)
	{
		last_error = "a data file named by the cue sheet is missing";
		return false;
	}
	fseek(f, fp_off, SEEK_SET);
	size_t	got = fread(&buf[0], 1, buf.size(), f);
	fclose(f);

	img.profile = MatchProfile((int)entries.size(), total, CRC32Buf(buf.data(), got));
	if (img.profile < 0)
		return false;

	return BuildTracks(entries, img.sizes, img.tracks);
}

#ifdef UNZIP_SUPPORT
// Seek the zip to a member by basename (case-insensitive) or by extension.
static bool ZipLocate(unzFile zf, const char *want_ext, const char *want_name,
					  unz_file_info *info_out)
{
	for (int port = unzGoToFirstFile(zf); port == UNZ_OK; port = unzGoToNextFile(zf))
	{
		unz_file_info	info;
		char			name[512];
		if (unzGetCurrentFileInfo(zf, &info, name, sizeof(name) - 1, NULL, 0, NULL, 0) != UNZ_OK)
			continue;

		const char	*base = BaseName(name);
		if (want_name && strcasecmp(base, want_name))
			continue;
		if (want_ext)
		{
			size_t	nl = strlen(base), el = strlen(want_ext);
			if (nl < el || strcasecmp(base + nl - el, want_ext))
				continue;
		}
		if (info_out)
			*info_out = info;
		return true;
	}
	return false;
}

// Read the member the zip is positioned on, optionally only the first
// want bytes starting at skip (the member is inflated from its start).
static bool ZipReadCurrent(unzFile zf, std::vector<uint8> &out, long skip, long want)
{
	if (unzOpenCurrentFile(zf) != UNZ_OK)
		return false;

	bool	ok = true;
	if (skip > 0)
	{
		std::vector<uint8>	junk(65536);
		while (skip > 0)
		{
			int	n = (int)((skip < (long)junk.size()) ? skip : (long)junk.size());
			if (unzReadCurrentFile(zf, &junk[0], n) != n)
			{
				ok = false;
				break;
			}
			skip -= n;
		}
	}
	if (ok)
	{
		out.resize((size_t)want);
		int	n = out.empty() ? 0 : unzReadCurrentFile(zf, &out[0], (unsigned)out.size());
		if (n < 0)
			ok = false;
		else
			out.resize((size_t)n);
	}
	unzCloseCurrentFile(zf);
	return ok;
}

static bool AttachFromZip(const char *path, VoiceKunImage &img)
{
	unzFile	zf = unzOpen(path);
	if (!zf)
	{
		last_error = "could not open the zip file";
		return false;
	}

	unz_file_info		cue_info;
	std::vector<uint8>	cue_raw;
	if (!ZipLocate(zf, ".cue", NULL, &cue_info) ||
		!ZipReadCurrent(zf, cue_raw, 0, (long)cue_info.uncompressed_size))
	{
		last_error = "no .cue file inside the zip";
		unzClose(zf);
		return false;
	}

	std::string					content((const char *)cue_raw.data(), cue_raw.size());
	std::vector<std::string>	names;
	std::vector<CueEntry>		entries;
	if (!ParseCue(content, names, entries))
	{
		unzClose(zf);
		return false;
	}

	// every FILE the cue names must be in the zip; remember its size
	for (size_t i = 0; i < names.size(); i++)
	{
		unz_file_info	info;
		const char		*base = BaseName(names[i].c_str());
		if (!ZipLocate(zf, NULL, base, &info))
		{
			last_error = "a data file named by the cue sheet is not in the zip";
			unzClose(zf);
			return false;
		}
		img.files.push_back(base);
		img.sizes.push_back((long)info.uncompressed_size);
	}

	int		fp_file;
	long	fp_off, total;
	FingerprintSpot(img.sizes, &fp_file, &fp_off, &total);

	std::vector<uint8>	buf;
	if (!ZipLocate(zf, NULL, img.files[fp_file].c_str(), NULL) ||
		!ZipReadCurrent(zf, buf, fp_off, VOICEKUN_FP_LEN))
	{
		last_error = "could not read the disc data from the zip";
		unzClose(zf);
		return false;
	}
	unzClose(zf);

	img.profile = MatchProfile((int)entries.size(), total, CRC32Buf(buf.data(), buf.size()));
	if (img.profile < 0)
		return false;

	return BuildTracks(entries, img.sizes, img.tracks);
}
#endif

static void CloseSource(void)
{
	if (cur_fp)
	{
		fclose(cur_fp);
		cur_fp = NULL;
	}
	cur_mem.clear();
	std::vector<uint8>().swap(cur_mem);
	cur_image = -1;
	cur_file  = -1;
}

bool S9xVoiceKunAttach(const char *path)
{
	last_error = "";

	game = FindGame();
	if (!game)
	{
		last_error = "the loaded ROM is not a supported Voicer-kun game";
		printf("Voicer-kun: attach failed: %s\n", last_error.c_str());
		return false;
	}

	if ((int)images.size() >= game->n_discs)
	{
		last_error = "every disc this game uses is already attached";
		printf("Voicer-kun: attach failed: %s\n", last_error.c_str());
		return false;
	}

	VoiceKunImage	img;
	img.path    = path;
	img.profile = -1;

	size_t	len = strlen(path);
	img.is_zip = len > 4 && !strcasecmp(path + len - 4, ".zip");

	bool	ok;
	if (img.is_zip)
	{
#ifdef UNZIP_SUPPORT
		ok = AttachFromZip(path, img);
#else
		last_error = "zip support is not compiled in";
		ok = false;
#endif
	}
	else
		ok = AttachFromCue(path, img);

	if (!ok)
	{
		printf("Voicer-kun: attach failed: %s\n", last_error.c_str());
		if (images.empty())
			game = NULL;
		return false;
	}

	for (size_t i = 0; i < images.size(); i++)
		if (images[i].profile == img.profile)
		{
			last_error = "that disc is already attached";
			printf("Voicer-kun: attach failed: %s\n", last_error.c_str());
			return false;
		}

	images.push_back(img);

	VoiceKunHook.PlayPC      = game->play_pc;
	VoiceKunHook.StopPC      = game->stop_pc;
	VoiceKunHook.ArgStackOff = game->arg_stack_off;
	VoiceKunHook.TrackBias   = game->track_bias;
	VoiceKunHook.ArmPC       = game->arm_pc;
	VoiceKunHook.ArmArgOff   = game->arm_arg_off;
	VoiceKunHook.IRCmdPC     = game->ir_cmd_pc;
	VoiceKunHook.IRCmdArgOff = game->ir_cmd_arg_off;

	partial_frames = 0;
	if (resampler)
		resampler->clear();

	Settings.VoiceKun = TRUE;
	S9xVoiceKunIRSetActive(true);

	printf("Voicer-kun: attached %s (%s, %d tracks, %s)\n", path,
		game->discs[img.profile].label, (int)img.tracks.size(), game->title);
	return true;
}

void S9xVoiceKunDetach(void)
{
	CloseSource();
	images.clear();
	game    = NULL;
	playing = false;
	VoiceKunHook.PlayPC = VoiceKunHook.StopPC = VoiceKunHook.ArmPC = 0;
	VoiceKunHook.IRCmdPC = 0;
	deck_driven = deck_paused = false;
	S9xVoiceKunIRSetActive(false);
	armed_track = cur_track = cued_track = 0;
	Settings.VoiceKun = FALSE;
}

bool S9xVoiceKunAttached(void)
{
	return !images.empty();
}

bool S9xVoiceKunCanAttachMore(void)
{
	const VoiceKunGame	*g = FindGame();
	return g && (int)images.size() < g->n_discs;
}

const char *S9xVoiceKunCuePath(void)
{
	return images.empty() ? "" : images[0].path.c_str();
}

int S9xVoiceKunTrackCount(void)
{
	return images.empty() ? 0 : (int)images.back().tracks.size();
}

int S9xVoiceKunDiscCount(void)
{
	return (int)images.size();
}

const char *S9xVoiceKunDiscLabel(void)
{
	if (images.empty() || !game)
		return "";
	return game->discs[images.back().profile].label;
}

const char *S9xVoiceKunGameTitle(void)
{
	return game ? game->title : NULL;
}

// Point the stream at a track of an attached image.
static bool OpenTrack(int image, const VoiceKunTrack &t)
{
	const VoiceKunImage	&img = images[image];

	if (image != cur_image || t.file != cur_file)
	{
		CloseSource();
		if (img.is_zip)
		{
#ifdef UNZIP_SUPPORT
			unzFile	zf = unzOpen(img.path.c_str());
			if (!zf)
				return false;
			bool	ok = ZipLocate(zf, NULL, img.files[t.file].c_str(), NULL) &&
			             ZipReadCurrent(zf, cur_mem, 0, img.sizes[t.file]);
			unzClose(zf);
			if (!ok)
			{
				cur_mem.clear();
				return false;
			}
#else
			return false;
#endif
		}
		else
		{
			cur_fp = fopen(img.files[t.file].c_str(), "rb");
			if (!cur_fp)
				return false;
		}
		cur_image = image;
		cur_file  = t.file;
	}

	play_pos = t.start;
	play_end = t.end;
	if (cur_fp)
		fseek(cur_fp, play_pos, SEEK_SET);
	else
	if (play_end > (long)cur_mem.size())
		play_end = (long)cur_mem.size();
	return true;
}

// Resolve the game's voice counter through the chapter table: which disc the
// voice lives on and its CD track. Returns false if the id is out of range.
static bool ResolveVoice(int vid, int *disc, int *track)
{
	if (!game || !game->chapter_rom || vid < 0 || vid >= game->n_voices)
		return false;

	const uint8	*tbl  = Memory.ROM + game->chapter_rom;
	const uint8	*base = Memory.ROM + game->chapter_base_rom;

	int	c = -1;
	for (int i = 0; i < game->n_chapters; i++)
		if (vid >= base[i])
			c = i;
	if (c < 0)
		return false;

	*disc  = tbl[c * 3];
	*track = tbl[c * 3 + 1] + (vid - base[c]) + 1;
	return true;
}

// The counter is a working variable that flickers through other values as
// the game sets it up, so only act once it has held still for a moment.
#define VOICEKUN_ID_SETTLE	30

void S9xVoiceKunPollVoiceId(int vid)
{
	static int		seen = -1, acted = -1;
	static uint32	since = 0;

	if (vid != seen)
	{
		seen  = vid;
		since = IPPU.TotalEmulatedFrames;
		return;
	}
	if (vid == acted || IPPU.TotalEmulatedFrames - since < VOICEKUN_ID_SETTLE)
		return;

	int	disc, track;
	if (!ResolveVoice(vid, &disc, &track))
		return;
	acted = vid;
	S9xVoiceKunPlayDiscTrack(disc, track);
}

int S9xVoiceKunVoiceIdAddr(void)
{
	return (game && !images.empty()) ? (int)game->voice_id_wram : 0;
}

void S9xVoiceKunArmTrack(int track)
{
	// The prompt repeats while the player is being nagged; only a genuinely
	// new track means a scene change, and only that re-arms the presses.
	armed_track = track;
}

void S9xVoiceKunPlayTrack(int track)
{
	if (images.empty())
		return;

	// PlayPC without an argument of its own starts whatever ArmPC named.
	if (track <= 0)
	{
		if (armed_track <= 0)
			return;
		track = armed_track;
	}

	// With one disc attached the choice is made; the caller-visible disc
	// selection for multi-disc games is resolved by S9xVoiceKunPlayDiscTrack.
	S9xVoiceKunPlayDiscTrack(images.size() == 1 ? images[0].profile : -1, track);
}

void S9xVoiceKunPlayDiscTrack(int disc, int track)
{
	int	image = -1;
	for (size_t i = 0; i < images.size(); i++)
		if (disc < 0 || images[i].profile == disc)
		{
			image = (int)i;
			break;
		}

	if (image < 0)
	{
		printf("Voicer-kun: track %d wants a disc that is not attached\n", track);
		return;
	}

	const VoiceKunImage	&img = images[image];
	if (track < 1 || track > (int)img.tracks.size())
		return;

	// Some discs open with an announcement / total-time track the game cues
	// while the player verifies the disc; that is not narration.
	if (game && game->first_voice > 0 && track < game->first_voice)
	{
		printf("Voicer-kun: track %d is not a voice track, not playing\n", track);
		return;
	}

	// Prompts repeat while a voice is current, so the same track arriving
	// again is a redraw rather than a fresh request - both while it plays and
	// for a spell after it ends. Later than that (an index revisit) it is a
	// real request and plays again.
	// The prompt for a track repeats while that voice is current, and the deck
	// may have rolled past it by now, so match the request rather than where
	// the head sits. A paused deck still counts as current.
	if (track == cued_track &&
		(playing || deck_paused ||
		 IPPU.TotalEmulatedFrames - track_done < VOICEKUN_REPLAY_GUARD))
		return;

	if (!OpenTrack(image, img.tracks[track - 1]))
	{
		printf("Voicer-kun: could not read track %d\n", track);
		return;
	}

	playing      = true;
	deck_paused  = false;
	cur_track    = cued_track = track;
	confirm_base = IPPU.TotalEmulatedFrames;
	confirm_on   = true;

	char	msg[160];
	if (game && game->n_discs > 1)
		snprintf(msg, sizeof(msg), "Voicer-kun: playing %s track %d/%d (%.0fs)",
			game->discs[img.profile].label, track, (int)img.tracks.size(),
			(play_end - play_pos) / (2352.0 * 75.0));
	else
		snprintf(msg, sizeof(msg), "Voicer-kun: playing track %d/%d (%.0fs)",
			track, (int)img.tracks.size(), (play_end - play_pos) / (2352.0 * 75.0));
	S9xSetInfoString(msg);
	printf("%s [f=%u]\n", msg, IPPU.TotalEmulatedFrames);
}

void S9xVoiceKunStop(void)
{
	if (playing)
		printf("Voicer-kun: voice ended\n");
	playing      = false;
	deck_paused  = false;
	cur_track    = cued_track = 0;
	confirm_on   = false;
}

// A real deck rolls on into the next track, and both games count on that. In
// 連続再生 (continuous play) the game cues once and then just reads scene after
// scene, so a disc that stops at the end of a track goes silent for the rest of
// the run; Voicer mode pauses and resumes across the same boundary. A later cue
// for a specific track overrides wherever the roll-on landed.
static bool DeckAdvance(void)
{
	if (cur_image < 0 || cur_track < 1)
		return false;

	const VoiceKunImage	&img = images[cur_image];
	if (cur_track >= (int)img.tracks.size() ||
	    !OpenTrack(cur_image, img.tracks[cur_track]))
		return false;

	cur_track++;
	printf("Voicer-kun: deck rolled into track %d [f=%u]\n",
		cur_track, IPPU.TotalEmulatedFrames);
	return true;
}

// Voicer mode works the deck by remote instead of prompting the player, and
// pauses it to let the text catch up. Ignoring those keys drains the stream
// ahead of the story, after which every resume lands past the end of a track
// and the scene plays silent.
void S9xVoiceKunDeckCommand(int cmd)
{
	if (images.empty())
		return;

	deck_driven = true;

	switch (cmd)
	{
	  case 0:	// stop
		playing = deck_paused = false;
		cur_track = cued_track = 0;
		break;

	  case 1:	// play - resume where the pause left the head
		if (deck_paused)
		{
			playing     = true;
			deck_paused = false;
		}
		break;

	  case 2:	// pause - hold position, keep play_pos
		if (playing)
		{
			playing     = false;
			deck_paused = true;
		}
		break;

	  default:	// prev/next: the cue prompt already names the track
		break;
	}
}

bool S9xVoiceKunAutoConfirm(void)
{
	if (!confirm_on || !game || game->confirm_hold <= 0)
		return false;

	// Only stand in for the player in manual mode; with the Voicer driver
	// active the game never waits for the keypress.
	if (game->manual_flag && Memory.RAM[game->manual_flag & 0x1ffff] != 0)
		return false;

	int32	t = (int32)(IPPU.TotalEmulatedFrames - confirm_base);
	if (t < game->confirm_first)
		return false;
	t -= game->confirm_first;

	if (t >= game->confirm_hold)
	{
		confirm_on = false;
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// Synthetic IR receiver.
//
// Having accepted the device, the games run a learn wizard: they ask the
// player to point a CD remote at the Voicer-kun and press each key, and
// record the waveform off port 2's D1 line into a 322-byte slot per key.
// There is no real deck on the other end here - we play the disc ourselves -
// so the codes only have to be well-formed and distinct from each other.
//
// The learner idles until D1 goes high, then samples the line once per pass
// of a NOP-padded loop, so the waveform is expressed in reads: a header, then
// bits distinguished by the length of their low gap.
// ---------------------------------------------------------------------------

#define VOICEKUN_IR_BITS	16

static bool		ir_on       = false;
static uint32	ir_frame    = 0;    // frame of the previous D1 read
static int		ir_pos      = 0;    // read index inside the current burst
static int		ir_code     = 0;    // bumped per capture so keys differ
static int		ir_runs[2 + VOICEKUN_IR_BITS * 2 + 2][2];
static int		ir_nruns    = 0;

static void IRBuildBurst(int code)
{
	int	n = 0;
	ir_runs[n][0] = 0; ir_runs[n++][1] = 20;    // idle before the burst
	ir_runs[n][0] = 1; ir_runs[n++][1] = 90;    // header mark
	ir_runs[n][0] = 0; ir_runs[n++][1] = 45;    // header space
	for (int b = 0; b < VOICEKUN_IR_BITS; b++)
	{
		ir_runs[n][0] = 1; ir_runs[n++][1] = 10;
		ir_runs[n][0] = 0; ir_runs[n++][1] = ((code >> b) & 1) ? 25 : 10;
	}
	ir_runs[n][0] = 1; ir_runs[n++][1] = 10;    // stop mark
	ir_runs[n][0] = 0; ir_runs[n++][1] = 1 << 20;
	ir_nruns = n;
}

void S9xVoiceKunIRSetActive(bool on)
{
	ir_on = on;
	ir_pos = 0;
}

bool S9xVoiceKunIRActive(void)
{
	return ir_on && game && !images.empty();
}

// Level of the emulated D1 line for this read.
bool S9xVoiceKunIRBit(void)
{
	if (!S9xVoiceKunIRActive())
		return false;

	// A gap between reads means the previous capture finished; the next key
	// gets its own code.
	uint32	f = IPPU.TotalEmulatedFrames;
	if (f - ir_frame > 1 || !ir_nruns)
	{
		ir_pos = 0;
		IRBuildBurst(++ir_code);
	}
	ir_frame = f;

	int	pos = ir_pos++, acc = 0;
	for (int i = 0; i < ir_nruns; i++)
	{
		acc += ir_runs[i][1];
		if (pos < acc)
			return ir_runs[i][0] != 0;
	}
	return false;
}

// Reported in the port-2 controller id bits while a disc is attached, so the
// game's "is the Voicer-kun plugged in?" poll of $421A succeeds.
uint16 S9xVoiceKunPort2Id(void)
{
	return (game && !images.empty()) ? game->port2_id : 0;
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
			if (cur_fp)
			{
				if (fread(frame, 1, 4, cur_fp) == 4)
					play_pos += 4;
				else
					playing = false;
			}
			else
			{
				memcpy(frame, &cur_mem[play_pos], 4);
				play_pos += 4;
			}
			if (play_pos + 4 > play_end && !(deck_driven && DeckAdvance()))
			{
				playing    = false;
				track_done = IPPU.TotalEmulatedFrames;
			}
		}
		else
		{
			if (playing)
				track_done = IPPU.TotalEmulatedFrames;
			playing = false;
		}

		resampler->push_sample(frame[0], frame[1]);
		partial_frames -= 3204;
	}
}
