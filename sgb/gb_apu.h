/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SGB_GB_APU_H_
#define _SGB_GB_APU_H_

#include <cstdint>

namespace SGB {

// GB APU — 4 channels, a 512 Hz frame sequencer, stereo mixer.
// Internal samples come out at T-cycle resolution and are box-filtered
// down to a configurable output rate (default 32000 Hz).

constexpr uint32_t APU_SAMPLE_BUF_SIZE = 16384;

// CH1/CH2 — Square wave with envelope; CH1 also has frequency sweep.
struct ApuSquare
{
	// Raw registers — returned by ApuRead with the correct read masks.
	uint8_t  nrx0 = 0;   // CH1 sweep; unused for CH2
	uint8_t  nrx1 = 0;   // duty + length
	uint8_t  nrx2 = 0;   // envelope
	uint8_t  nrx3 = 0;   // freq low
	uint8_t  nrx4 = 0;   // trigger + length enable + freq high

	// Derived state.
	uint16_t freq        = 0;    // 11-bit combined freq
	int32_t  freq_timer  = 0;    // T-cycles until next duty advance
	uint8_t  duty_pos    = 0;    // 0..7

	uint32_t length      = 0;    // 0..64; channel disables on underflow

	uint8_t  env_volume  = 0;    // 0..15
	int32_t  env_timer   = 0;    // frame-sequencer ticks until next step
	bool     env_running = false;

	// Sweep (CH1 only).
	uint16_t sweep_shadow_freq      = 0;
	int32_t  sweep_timer            = 0;
	bool     sweep_enabled          = false;
	bool     sweep_negate_calc_used = false;  // for "negate-then-clear" quirk

	bool     length_enabled = false;
	bool     dac_enabled    = false;
	bool     enabled        = false;
};

// CH3 — wave table from 32 4-bit samples in wave RAM (NR30..NR34 + $FF30-$FF3F).
struct ApuWave
{
	uint8_t  nr30 = 0;
	uint8_t  nr31 = 0;
	uint8_t  nr32 = 0;
	uint8_t  nr33 = 0;
	uint8_t  nr34 = 0;

	uint8_t  ram[16]   = {0};    // 32 packed 4-bit samples

	uint16_t freq       = 0;
	int32_t  freq_timer = 0;
	uint8_t  pos        = 0;     // 0..31
	uint8_t  sample_buf = 0;     // latched current 4-bit sample

	uint32_t length     = 0;     // 0..256

	bool     length_enabled = false;
	bool     dac_enabled    = false;
	bool     enabled        = false;
};

// CH4 — LFSR-based noise with envelope.
struct ApuNoise
{
	uint8_t  nr41 = 0;
	uint8_t  nr42 = 0;
	uint8_t  nr43 = 0;
	uint8_t  nr44 = 0;

	uint16_t lfsr       = 0x7FFF;
	int32_t  freq_timer = 0;

	uint32_t length     = 0;     // 0..64

	uint8_t  env_volume = 0;
	int32_t  env_timer  = 0;
	bool     env_running = false;

	bool     length_enabled = false;
	bool     dac_enabled    = false;
	bool     enabled        = false;
};

struct Apu
{
	ApuSquare ch1, ch2;
	ApuWave   ch3;
	ApuNoise  ch4;

	uint8_t   nr50 = 0;          // master L/R volume + VIN enables
	uint8_t   nr51 = 0;          // per-channel panning
	bool      master_enabled = false;  // NR52 bit 7

	// Free-running 512 Hz frame sequencer.
	int32_t   frame_seq_timer = 0;
	uint8_t   frame_seq_step  = 0;

	// Box-filter resampling state.
	int32_t   sample_accum_l    = 0;
	int32_t   sample_accum_r    = 0;
	uint32_t  sample_accum_cnt  = 0;
	int32_t   sample_timer      = 0;
	int32_t   cycles_per_sample = 131;  // ~4.194 MHz / 32000 Hz (floor)
	int32_t   output_rate       = 32000;
	// Actual GB clock rate the host is driving us at. Defaults to DMG
	// (4.194 MHz); SGB1 mode uses SNES master / 5 = 4.295 MHz, which
	// is 2.4% faster — without tracking this, SGB1 audio would play
	// back chipmunk-pitched (samples emitted faster than the host rate
	// expects). Used together with output_rate to compute cycles_per_sample.
	int32_t   clock_hz          = 4194304;
	// Bresenham fractional accumulator for cycles_per_sample. Integer
	// cps alone gives clock_hz/cps Hz output, which differs from the
	// true clock_hz/output_rate by up to ~0.5% (e.g. 4194304/48000 =
	// 87.38, integer 87 → effective 48210 Hz at host 48000 Hz, audible
	// as a "hurried" pitch shift). On each sample emit, sample_timer
	// reloads with `cps + (1 if remainder_acc rolls over, else 0)`,
	// making the long-run average exactly clock_hz/output_rate cycles
	// per sample. Pitch is now exact.
	int32_t   cps_remainder_step = 0;       // = clock_hz % output_rate
	int32_t   cps_remainder_acc  = 0;       // [0, output_rate)

	// Ring buffer of mixed stereo int16 samples (interleaved L,R).
	int16_t   sample_buf[APU_SAMPLE_BUF_SIZE * 2];
	uint32_t  sample_head = 0;
	uint32_t  sample_tail = 0;

	// DC-blocking high-pass state, one pole per output channel. Removes the
	// DC the DAC-bias mixer introduces (so ordinary playback keeps its exact
	// AC level and channel enable/disable doesn't thump) while passing the
	// audio band — including digitized-voice PCM. Transient like the sample
	// accumulators: reset on Reset, never serialized.
	float     hp_xprev_l = 0.0f, hp_yprev_l = 0.0f;
	float     hp_xprev_r = 0.0f, hp_yprev_r = 0.0f;
};

void ApuReset(Apu &a);
void ApuStep(Apu &a, int32_t tcycles);

uint8_t ApuRead(Apu &a, uint16_t addr);
void    ApuWrite(Apu &a, uint16_t addr, uint8_t value);

int32_t ApuDrain(Apu &a, int16_t *out, int32_t max_samples);

// Configure the downsample target. Idempotent — re-applying the same
// rate is a no-op so callers can safely push it every frame from the
// host audio path without disturbing in-flight sample accumulation.
void ApuSetOutputRate(Apu &a, int32_t rate);

// Tell the APU the actual GB clock rate it's being stepped at. Affects
// cycles_per_sample math so audio plays at correct pitch regardless of
// run mode (DMG/SGB2 = 4.194 MHz, SGB1 = 4.295 MHz). Idempotent.
void ApuSetClockHz(Apu &a, int32_t hz);

} // namespace SGB

#endif
