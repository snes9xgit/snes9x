/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _VOICEKUN_H_
#define _VOICEKUN_H_

#include "snes9x.h"

class Resampler;

// Koei Voicer-kun games ship an audio CD of voice tracks; the ROM computes
// the absolute CD track before driving the IR hardware, so we hook that PC
// and stream the track straight from a user-attached CUE/BIN image.

bool S9xVoiceKunAttach(const char *path);   // .cue on disk, or .zip holding cue+bin(s)
void S9xVoiceKunDetach(void);               // ejects every attached disc
bool S9xVoiceKunAttached(void);
bool S9xVoiceKunCanAttachMore(void);        // game has a disc that is not attached yet
bool S9xVoiceKunGameSupported(void);        // loaded ROM is in the Voicer-kun game list
const char *S9xVoiceKunLastError(void);     // why the last attach failed
const char *S9xVoiceKunCuePath(void);
int S9xVoiceKunTrackCount(void);            // tracks on the disc attached last
int S9xVoiceKunDiscCount(void);
const char *S9xVoiceKunDiscLabel(void);     // label of the disc attached last
const char *S9xVoiceKunGameTitle(void);     // NULL if nothing attached

void S9xVoiceKunPlayTrack(int track);                 // 1-based CD track number
void S9xVoiceKunPlayDiscTrack(int disc, int track);   // disc < 0 = first attached
void S9xVoiceKunStop(void);

// Manual mode asks the player to press A once the CD is rolling; we started
// it ourselves, so the input layer holds A for us during this window.
bool S9xVoiceKunAutoConfirm(void);
uint16 S9xVoiceKunPort2Id(void);    // device id bits for port 2, 0 = none

// Emulated IR receive line (port 2 D1) for the games' learn wizard.
bool S9xVoiceKunIRActive(void);
bool S9xVoiceKunIRBit(void);
void S9xVoiceKunIRSetActive(bool on);

void S9xVoiceKunGenerate(size_t sample_count);
void S9xVoiceKunSetOutput(Resampler *resampler);

// Fast per-instruction hook data for cpuexec: valid while Settings.VoiceKun.
struct SVoiceKunHook
{
	uint32	PlayPC;      // 24-bit PBPC of the game's play-voice entry (0 = none)
	uint32	StopPC;      // entry that ends/stops the current voice (0 = none)
	int32	ArgStackOff; // >0: 16-bit arg at S+off; 0: low byte of A
	int32	TrackBias;   // CD track = arg + TrackBias
	// Games that name the track and start it at separate points (Emit prompts
	// "pause at track N", counts down, then acts) arm here and play at PlayPC.
	uint32	ArmPC;       // 0 = PlayPC carries the argument itself
	int32	ArmArgOff;
	// Voicer mode transmits deck keypresses instead of prompting the player.
	// The game pauses the disc to let text catch up and resumes it, so the
	// stream has to follow those commands or it drains ahead of the story.
	uint32	IRCmdPC;     // transport keys: 0 stop 1 play 2 pause 3 prev 4 next
	int32	IRCmdArgOff;
};

void S9xVoiceKunArmTrack(int track);
void S9xVoiceKunDeckCommand(int cmd);   // a transport key the game transmitted
void S9xVoiceKunPollVoiceId(int vid);
int S9xVoiceKunVoiceIdAddr(void);   // WRAM offset of the voice counter, 0 = none

extern struct SVoiceKunHook VoiceKunHook;

#endif
