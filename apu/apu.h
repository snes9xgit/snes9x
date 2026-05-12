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
// streams: 0=SPC pre-mix, 1=GB pre-mix, 2=final mix.
void S9xAudioWaveformEnable(bool enable);
int  S9xAudioWaveformSnapshot(int stream, short *out_lr, int max_frames);
int  S9xAudioWaveformSampleRate(void);

// SGB BIOS-released mix-mode helpers — host layer (CXAudio2) drains the SPC
// resampler independently of the GB stream so the two stay in sync via the
// shared SNES clock while host throttles on combined buffer state.
// PullSpc: reads up to count words (int16) from spc::resampler output. Returns actual read.
// SpcOutAvail: returns spc::resampler.avail() in output words.
int  S9xPullSpcOutput(int16_t *dst, int count);
int  S9xSpcOutAvailable(void);
void S9xAudioWaveformPushMix(const int16_t *src, int frames);

#define DSP_INTERPOLATION_NONE     0
#define DSP_INTERPOLATION_LINEAR   1
#define DSP_INTERPOLATION_GAUSSIAN 2
#define DSP_INTERPOLATION_CUBIC    3
#define DSP_INTERPOLATION_SINC     4

#endif
