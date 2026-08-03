/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _APU_H_
#define _APU_H_

#include "../snes9x.h"

typedef void (*apu_callback) (void *);

#define SPC_SAVE_STATE_BLOCK_SIZE (1024 * 65)
#define SPC_FILE_SIZE             (66048)

bool8 S9xInitAPU (void);
void S9xDeinitAPU (void);
void S9xResetAPU (void);
void S9xSoftResetAPU (void);
uint8 S9xAPUReadPort (int);
void S9xAPUWritePort (int, uint8);
void S9xAPUExecute (void);
void S9xAPUEndScanline (void);
void S9xAPUSetReferenceTime (int32);
void S9xAPUTimingSetSpeedup (int);
void S9xAPULoadState (uint8 *);
void S9xAPULoadBlarggState(uint8 *oldblock);
void S9xAPUSaveState (uint8 *);
void S9xDumpSPCSnapshot (void);
bool8 S9xSPCDump (const char *);

bool8 S9xInitSound (int);
bool8 S9xOpenSoundDevice (void);

bool8 S9xSyncSound (void);
int S9xGetSampleCount (void);
void S9xSetSoundControl (uint8);
void S9xSetSoundMute (bool8);
void S9xLandSamples (void);
void S9xClearSamples (void);
bool8 S9xMixSamples (uint8 *, int);
void S9xSetSamplesAvailableCallback (apu_callback, void *);
void S9xUpdateDynamicRate (int empty = 1, int buffer_size = 2);

// Save/restore resampler state for run-ahead (preserves hermite filter
// continuity across the hidden frame)
void S9xRunAheadSaveAudio (void);
void S9xRunAheadLoadAudio (void);

// Audio waveform debug capture — see Sound > Show Audio Waveform.
// streams: 0=SPC pre-mix, 1=GB pre-mix, 2=final mix, 3..10=SPC voices 1-8.
void S9xAudioWaveformEnable(bool enable);
int  S9xAudioWaveformSnapshot(int stream, short *out_lr, int max_frames);
int  S9xAudioWaveformSampleRate(void);
// Fed by SPC_DSP::voice_output at the DSP's native rate: each voice's
// post-envelope, post-volume L (ch=0) then R (ch=1) contribution.
void S9xAudioWaveformPushVoice(int voice, int ch, int amp);

// SGB BIOS-released mix-mode helpers — host layer (CXAudio2) drains the SPC
// resampler independently of the GB stream so the two stay in sync via the
// shared SNES clock while host throttles on combined buffer state.
// PullSpc: reads up to count words (int16) from spc::resampler output. Returns actual read.
// SpcOutAvail: returns spc::resampler.avail() in output words.
int  S9xPullSpcOutput(int16_t *dst, int count);
int  S9xSpcOutAvailable(void);        // output words after ratio conversion
int  S9xSpcSpaceFilled(void);         // input words buffered (same units as capacity)
int  S9xSpcResamplerCapacity(void);   // spc::resampler.buffer_size in input words
long S9xSpcDroppedSamples(void);      // words discarded at push (full buffer) — overflow crackle
void S9xSpcIoMeters(unsigned int *pushed, unsigned int *consumed); // input words in/out
long S9xApuScanlineMeter(void);       // cumulative emulated scanlines (nominal 15.75k/s)
// Recorder tap on the audiowave rings: frames since *cursor (-1 latches to now).
int  S9xAudioWaveformReadNew(int stream, int *cursor, short *out_lr, int max_frames);
void S9xAudioWaveformPushMix(const int16_t *src, int frames);

// Host-side post-mix step for SGB BIOS mode. Call this on the buffer
// returned by S9xMixSamples to overlay the SPC stream (running the SGB
// BIOS sound engine and SOU_TRN samples) on top of the GB audio, with
// the same gains/clip and boot-handoff click suppression the win32
// driver uses. No-op when not in SGB BIOS released mode.
// sample_count is the interleaved-stereo int16_t count (same units as
// passed to S9xMixSamples).
void S9xMixSpcOverGB(int16_t *dest, int sample_count);

// Per-source volume scalers (0..100, percent) for the SGB BIOS mix
// path. Multiply the fixed per-channel gains so the user can rebalance
// SPC vs GB audio in SGB-with-BIOS mode. 100 = unchanged from default.
extern unsigned int S9xSGBMixVolumeSPC;
extern unsigned int S9xSGBMixVolumeGB;

// Pre-amp gain in whole dB, applied on top of the volume percentages
// above. 0 = unity; the sliders only go up because attenuation is
// already what the percent volumes do.
#define S9X_GAIN_MAX_DB 12
extern unsigned int S9xSGBMixGainSPC;
extern unsigned int S9xSGBMixGainGB;

// Linear factor for a whole-dB pre-amp gain, Q8 (256 = unity) and as a
// double. Input is clamped to 0..S9X_GAIN_MAX_DB.
int    S9xGainQ8(unsigned int db);
double S9xGainLinear(unsigned int db);

// Splice-health counters for the GB/SPC mix path, surfaced by the
// waveform viewer. gb_pad: output samples zero-filled because the GB
// ring came up short mid-block; spc_short: samples the SPC layer missed
// because its resampler ran dry mid-block. Numbers that keep climbing
// while audio is playing correspond to audible gaps/crackle.
extern volatile long S9xSGBMixGBPadSamples;
extern volatile long S9xSGBMixSPCShortSamples;

// Adaptive rate control hooks for the SGB BIOS-released mix mode drain
// thread. Each call observes the stream's own buffer fill level and biases
// its rate to keep it near target, decoupling GB and SPC pitch drift.
// max_dev = trim authority (clamp half-width around 1.0) the caller
// grants: 0.04 when a sound-sync throttle can pace real overspeed,
// 0.30 when none is available and resampling the surplus is the only
// non-crackling option.
void   S9xSpcAdjustRate(double max_dev);
void   S9xSpcResetDrc(void);
double S9xSpcGetTimeRatio(void);
double S9xSpcGetDrcScale(void);   // current production-rate trim (1.0 = untrimmed)
// PI rate-match for once-per-frame frontends in SGB BIOS mode (libretro/gtk/qt).
// Drives spc::drc_scale to hold the SPC resampler buffer near half-full so it
// stays locked to the GB-paced consumption. S9xSpcSyncReset clears the
// integrator (call when leaving BIOS mode).
void   S9xSpcSyncToConsumption(void);
void   S9xSpcSyncReset(void);

#define DSP_INTERPOLATION_NONE     0
#define DSP_INTERPOLATION_LINEAR   1
#define DSP_INTERPOLATION_GAUSSIAN 2
#define DSP_INTERPOLATION_CUBIC    3
#define DSP_INTERPOLATION_SINC     4

#endif
