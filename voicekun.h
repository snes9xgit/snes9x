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

bool S9xVoiceKunAttach(const char *path);   // .cue on disk, or .zip holding cue+bin
void S9xVoiceKunDetach(void);
bool S9xVoiceKunAttached(void);
bool S9xVoiceKunGameSupported(void);        // loaded ROM is in the Voicer-kun game list
const char *S9xVoiceKunLastError(void);     // why the last attach failed
const char *S9xVoiceKunCuePath(void);
int S9xVoiceKunTrackCount(void);
const char *S9xVoiceKunGameTitle(void);     // NULL if nothing attached

void S9xVoiceKunPlayTrack(int track);     // 1-based CD track number
void S9xVoiceKunStop(void);

// Manual mode asks the player to press A once the CD is rolling; we started
// it ourselves, so the input layer holds A for us during this window.
bool S9xVoiceKunAutoConfirm(void);

void S9xVoiceKunGenerate(size_t sample_count);
void S9xVoiceKunSetOutput(Resampler *resampler);

// Fast per-instruction hook data for cpuexec: valid while Settings.VoiceKun.
struct SVoiceKunHook
{
	uint32	PlayPC;      // 24-bit PBPC of the game's play-voice entry (0 = none)
	uint32	StopPC;      // entry that ends/stops the current voice (0 = none)
	int32	ArgStackOff; // >0: 16-bit arg at S+off; 0: low byte of A
	int32	TrackBias;   // CD track = arg + TrackBias
};

extern struct SVoiceKunHook VoiceKunHook;

#endif
