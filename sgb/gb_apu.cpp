/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// Game Boy APU — CH1/CH2 (square), CH3 (wave), CH4 (LFSR noise), plus
// a 512 Hz frame sequencer driving length/envelope/sweep ticks.
//
// Structure (per T-cycle):
//   1. Tick each enabled channel's frequency timer; advance its
//      internal position (duty step, wave pos, LFSR shift) when the
//      timer underflows.
//   2. Tick the frame sequencer; it steps 0..7 at 8192 T-cycles per
//      step (= 512 Hz at 4.194 MHz). Different steps fire different
//      counters:
//        0,2,4,6 → length counters
//        2,6     → CH1 sweep
//        7       → envelopes
//   3. Sum all 4 channel outputs, apply per-channel panning from
//      NR51 and master volume from NR50, accumulate into box-filter
//      buffer; emit one stereo sample every cycles_per_sample.
//
// Output samples are pushed into a ring buffer drained by ApuDrain().

#include "gb_apu.h"

#include <algorithm>
#include <cstring>

namespace SGB {

namespace {

// Duty waveform patterns per Pan Docs (bits emitted left-to-right as
// duty_pos 0,1,2,...,7).
constexpr uint8_t DUTY_TABLE[4][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 1 },   // 12.5%
	{ 1, 0, 0, 0, 0, 0, 0, 1 },   // 25%
	{ 1, 0, 0, 0, 0, 1, 1, 1 },   // 50%
	{ 0, 1, 1, 1, 1, 1, 1, 0 }    // 75%
};

constexpr int32_t FRAME_SEQ_PERIOD = 8192;   // T-cycles per 512 Hz tick
constexpr int32_t CPU_CLOCK_HZ     = 4194304;

// -------------------------------------------------------------------
// Channel helpers
// -------------------------------------------------------------------

inline uint16_t SquareFreq(const ApuSquare &c)
{
	return static_cast<uint16_t>(((c.nrx4 & 0x07) << 8) | c.nrx3);
}

inline uint16_t WaveFreq(const ApuWave &c)
{
	return static_cast<uint16_t>(((c.nr34 & 0x07) << 8) | c.nr33);
}

inline int32_t SquarePeriod(uint16_t freq)
{
	// (2048 - f) * 4 T-cycles between duty steps.
	return static_cast<int32_t>((2048 - freq) * 4);
}

inline int32_t WavePeriod(uint16_t freq)
{
	// Half the square period per wave sample.
	return static_cast<int32_t>((2048 - freq) * 2);
}

inline int32_t NoisePeriod(uint8_t nr43)
{
	// divisor: 0 → 8, 1 → 16, 2 → 32, ..., 7 → 112.
	static const int32_t DIV_TABLE[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
	const int32_t div   = DIV_TABLE[nr43 & 0x07];
	const int32_t shift = (nr43 >> 4) & 0x0F;
	return div << shift;
}

// -------------------------------------------------------------------
// CH1 sweep
// -------------------------------------------------------------------

// Compute next sweep frequency. Returns the candidate; caller compares
// to 2047 to decide channel-disable.
uint16_t SweepCalcFreq(ApuSquare &c)
{
	const uint8_t shift  = c.nrx0 & 0x07;
	const bool    negate = (c.nrx0 & 0x08) != 0;
	const uint32_t delta = c.sweep_shadow_freq >> shift;

	int32_t next = static_cast<int32_t>(c.sweep_shadow_freq);
	if (negate)
	{
		next -= static_cast<int32_t>(delta);
		c.sweep_negate_calc_used = true;
	}
	else
	{
		next += static_cast<int32_t>(delta);
	}
	if (next < 0) next = 0;
	return static_cast<uint16_t>(next);
}

void SweepTick(ApuSquare &c)
{
	if (--c.sweep_timer > 0) return;

	const uint8_t period = (c.nrx0 >> 4) & 0x07;
	c.sweep_timer = period ? period : 8;

	if (!c.sweep_enabled || period == 0) return;

	const uint16_t next = SweepCalcFreq(c);
	if (next > 2047) { c.enabled = false; return; }

	if ((c.nrx0 & 0x07) != 0)
	{
		c.freq              = next;
		c.sweep_shadow_freq = next;
		c.nrx3              = static_cast<uint8_t>(next & 0xFF);
		c.nrx4              = static_cast<uint8_t>((c.nrx4 & 0xF8) | ((next >> 8) & 0x07));

		// Second overflow check — disables channel even though result
		// isn't stored. Matches Pan Docs sweep behavior.
		if (SweepCalcFreq(c) > 2047) c.enabled = false;
	}
}

// -------------------------------------------------------------------
// Envelope
// -------------------------------------------------------------------

void EnvelopeTick(uint8_t nrx2, uint8_t &volume, int32_t &timer, bool &running)
{
	if (!running) return;

	if (--timer > 0) return;
	const uint8_t period = nrx2 & 0x07;
	timer = period ? period : 8;
	if (period == 0) { running = false; return; }

	const bool increase = (nrx2 & 0x08) != 0;
	if (increase && volume < 15)      ++volume;
	else if (!increase && volume > 0) --volume;
	else                              running = false;
}

// -------------------------------------------------------------------
// Length counter
// -------------------------------------------------------------------

void LengthTick(uint32_t &length, bool length_enabled, bool &enabled)
{
	if (length_enabled && length > 0)
	{
		if (--length == 0) enabled = false;
	}
}

// -------------------------------------------------------------------
// Trigger (kick a channel via NRx4 bit 7)
// -------------------------------------------------------------------

void TriggerSquare(ApuSquare &c, bool is_ch1)
{
	if (c.dac_enabled) c.enabled = true;

	if (c.length == 0) c.length = 64;

	c.freq       = SquareFreq(c);
	c.freq_timer = SquarePeriod(c.freq);

	c.env_volume  = (c.nrx2 >> 4) & 0x0F;
	c.env_timer   = (c.nrx2 & 0x07) ? (c.nrx2 & 0x07) : 8;
	c.env_running = true;

	if (is_ch1)
	{
		c.sweep_shadow_freq = c.freq;
		const uint8_t period = (c.nrx0 >> 4) & 0x07;
		c.sweep_timer = period ? period : 8;
		c.sweep_enabled = (period != 0) || ((c.nrx0 & 0x07) != 0);
		c.sweep_negate_calc_used = false;
		if ((c.nrx0 & 0x07) != 0 && SweepCalcFreq(c) > 2047) c.enabled = false;
	}
}

void TriggerWave(ApuWave &c)
{
	if (c.dac_enabled) c.enabled = true;
	if (c.length == 0) c.length = 256;
	c.freq       = WaveFreq(c);
	c.freq_timer = WavePeriod(c.freq);
	c.pos        = 0;
}

void TriggerNoise(ApuNoise &c)
{
	if (c.dac_enabled) c.enabled = true;
	if (c.length == 0) c.length = 64;
	c.lfsr       = 0x7FFF;
	c.freq_timer = NoisePeriod(c.nr43);
	c.env_volume = (c.nr42 >> 4) & 0x0F;
	c.env_timer  = (c.nr42 & 0x07) ? (c.nr42 & 0x07) : 8;
	c.env_running = true;
}

// -------------------------------------------------------------------
// Per-T-cycle channel advance
// -------------------------------------------------------------------

void SquareClock(ApuSquare &c)
{
	if (--c.freq_timer > 0) return;
	c.freq_timer = SquarePeriod(c.freq);
	c.duty_pos   = static_cast<uint8_t>((c.duty_pos + 1) & 7);
}

void WaveClock(ApuWave &c)
{
	if (--c.freq_timer > 0) return;
	c.freq_timer = WavePeriod(c.freq);
	c.pos        = static_cast<uint8_t>((c.pos + 1) & 0x1F);
	const uint8_t byte = c.ram[c.pos >> 1];
	c.sample_buf = (c.pos & 1) ? static_cast<uint8_t>(byte & 0x0F)
	                           : static_cast<uint8_t>(byte >> 4);
}

void NoiseClock(ApuNoise &c)
{
	if (--c.freq_timer > 0) return;
	c.freq_timer = NoisePeriod(c.nr43);
	const uint16_t bit   = static_cast<uint16_t>((c.lfsr ^ (c.lfsr >> 1)) & 1);
	uint16_t       next  = static_cast<uint16_t>((c.lfsr >> 1) | (bit << 14));
	if (c.nr43 & 0x08) next = static_cast<uint16_t>((next & ~0x40) | (bit << 6));  // 7-bit mode
	c.lfsr = next;
}

// -------------------------------------------------------------------
// Channel output — 0..15 DAC level, inverted to -15..+15 range.
// Returns 0..15 "digital" level; DAC conversion to signed is done in the mixer.
// -------------------------------------------------------------------

uint8_t SquareOutput(const ApuSquare &c)
{
	if (!c.enabled || !c.dac_enabled) return 0;
	const uint8_t duty = (c.nrx1 >> 6) & 0x03;
	const uint8_t bit  = DUTY_TABLE[duty][c.duty_pos];
	return static_cast<uint8_t>(bit * c.env_volume);
}

uint8_t WaveOutput(const ApuWave &c)
{
	if (!c.enabled || !c.dac_enabled) return 0;
	const uint8_t shift_table[4] = { 4, 0, 1, 2 };
	const uint8_t shift = shift_table[(c.nr32 >> 5) & 0x03];
	return static_cast<uint8_t>(c.sample_buf >> shift);
}

uint8_t NoiseOutput(const ApuNoise &c)
{
	if (!c.enabled || !c.dac_enabled) return 0;
	const uint8_t bit = static_cast<uint8_t>((~c.lfsr) & 1);
	return static_cast<uint8_t>(bit * c.env_volume);
}

// -------------------------------------------------------------------
// Mixer — produces int16 stereo from the four channel outputs.
// -------------------------------------------------------------------

void Mix(const Apu &a, int32_t &out_l, int32_t &out_r)
{
	const uint8_t levels[4] = {
		SquareOutput(a.ch1),
		SquareOutput(a.ch2),
		WaveOutput(a.ch3),
		NoiseOutput(a.ch4)
	};

	int32_t l = 0, r = 0;
	for (int ch = 0; ch < 4; ++ch)
	{
		// DAC: 0..15 → +7..-8 (roughly) as signed. Use linear centering
		// around the middle (7.5), scaled to int16 range later.
		const int32_t lvl = static_cast<int32_t>(levels[ch]) * 2 - 15;  // -15..+15
		if (a.nr51 & (1 << ch))       r += lvl;           // right side (bits 0-3)
		if (a.nr51 & (1 << (ch + 4))) l += lvl;           // left  side (bits 4-7)
	}

	const int32_t vol_r = static_cast<int32_t>(a.nr50 & 0x07) + 1;         // 1..8
	const int32_t vol_l = static_cast<int32_t>((a.nr50 >> 4) & 0x07) + 1;  // 1..8

	// Each channel contributes -15..+15; 4 channels max = -60..+60.
	// Scale × volume (1..8) × ~70 to fill int16 (max 60 * 8 * 70 ≈ 33600).
	constexpr int32_t GAIN = 70;
	out_l = l * vol_l * GAIN;
	out_r = r * vol_r * GAIN;
	if (out_l >  32767) out_l =  32767;
	if (out_l < -32768) out_l = -32768;
	if (out_r >  32767) out_r =  32767;
	if (out_r < -32768) out_r = -32768;
}

// -------------------------------------------------------------------
// Frame sequencer
// -------------------------------------------------------------------

void FrameSequencerStep(Apu &a)
{
	const uint8_t step = a.frame_seq_step;

	// Length (even steps).
	if ((step & 1) == 0)
	{
		LengthTick(a.ch1.length, a.ch1.length_enabled, a.ch1.enabled);
		LengthTick(a.ch2.length, a.ch2.length_enabled, a.ch2.enabled);
		LengthTick(a.ch3.length, a.ch3.length_enabled, a.ch3.enabled);
		LengthTick(a.ch4.length, a.ch4.length_enabled, a.ch4.enabled);
	}

	// Sweep (steps 2 and 6).
	if (step == 2 || step == 6)
	{
		SweepTick(a.ch1);
	}

	// Envelope (step 7).
	if (step == 7)
	{
		EnvelopeTick(a.ch1.nrx2, a.ch1.env_volume, a.ch1.env_timer, a.ch1.env_running);
		EnvelopeTick(a.ch2.nrx2, a.ch2.env_volume, a.ch2.env_timer, a.ch2.env_running);
		EnvelopeTick(a.ch4.nr42, a.ch4.env_volume, a.ch4.env_timer, a.ch4.env_running);
	}

	a.frame_seq_step = static_cast<uint8_t>((step + 1) & 7);
}

// -------------------------------------------------------------------
// Ring buffer output — push one stereo sample.
// -------------------------------------------------------------------

void PushSample(Apu &a, int16_t l, int16_t r)
{
	const uint32_t size = APU_SAMPLE_BUF_SIZE;
	const uint32_t next = (a.sample_head + 1) % size;
	if (next == a.sample_tail)
	{
		// Buffer full — drop. In practice snes9x's audio thread should
		// drain in time; overruns indicate an unconsumed stream.
		return;
	}
	a.sample_buf[a.sample_head * 2 + 0] = l;
	a.sample_buf[a.sample_head * 2 + 1] = r;
	a.sample_head = next;
}

// Emit one integrated sample to the ring buffer, reset accumulator.
void FlushSample(Apu &a)
{
	int16_t out_l = 0, out_r = 0;
	if (a.sample_accum_cnt > 0)
	{
		out_l = static_cast<int16_t>(a.sample_accum_l / static_cast<int32_t>(a.sample_accum_cnt));
		out_r = static_cast<int16_t>(a.sample_accum_r / static_cast<int32_t>(a.sample_accum_cnt));
	}
	a.sample_accum_l   = 0;
	a.sample_accum_r   = 0;
	a.sample_accum_cnt = 0;
	PushSample(a, out_l, out_r);
}

} // anonymous

// ===================================================================
// Public API
// ===================================================================

void ApuReset(Apu &a)
{
	a.ch1 = ApuSquare{};
	a.ch2 = ApuSquare{};
	a.ch3 = ApuWave{};
	a.ch4 = ApuNoise{};
	a.ch4.lfsr = 0x7FFF;

	a.nr50 = 0;
	a.nr51 = 0;
	a.master_enabled = false;

	a.frame_seq_timer = FRAME_SEQ_PERIOD;
	a.frame_seq_step  = 0;

	a.sample_accum_l   = 0;
	a.sample_accum_r   = 0;
	a.sample_accum_cnt = 0;
	a.sample_timer     = a.cycles_per_sample;
	a.sample_head      = 0;
	a.sample_tail      = 0;

	std::memset(a.sample_buf, 0, sizeof a.sample_buf);
}

// Compute cycles_per_sample as integer FLOOR (not rounded) plus the
// Bresenham remainder so the long-run average is exactly clock_hz/
// output_rate cycles per sample. Pitch is exact — no 0.4% drift from
// integer rounding.
static inline void RecomputeSampleRate(Apu &a)
{
	if (a.output_rate < 1) a.output_rate = 32000;
	a.cycles_per_sample  = a.clock_hz / a.output_rate;       // floor
	a.cps_remainder_step = a.clock_hz % a.output_rate;       // [0, rate)
	if (a.cycles_per_sample < 1) a.cycles_per_sample = 1;
	// Preserve cps_remainder_acc across rate changes — the running
	// fractional carry shouldn't reset, otherwise we'd briefly emit
	// at the integer-only rate until the carry rebuilds.
}

void ApuSetOutputRate(Apu &a, int32_t rate)
{
	if (rate <= 0) rate = 32000;
	if (rate == a.output_rate) return;  // idempotent — preserve sample_timer
	a.output_rate = rate;
	RecomputeSampleRate(a);
	// Don't touch sample_timer — it's a countdown into the next sample
	// boundary; letting it tick down naturally avoids a one-sample gap.
}

void ApuSetClockHz(Apu &a, int32_t hz)
{
	if (hz <= 0)         return;
	if (hz == a.clock_hz) return;  // idempotent
	a.clock_hz = hz;
	RecomputeSampleRate(a);
}

// Event-driven advance. Per-T-cycle loops were the hottest code in the
// SGB core (APU fires for every CPU instruction). This version walks
// forward in chunks bounded by the *next* event — whichever of the
// channel freq-timers, frame-sequencer timer, or sample-output timer
// expires soonest. During any such chunk every channel's output level
// is constant, so we mix once and multiply by the chunk width to
// contribute to the sample accumulator. Net result: O(events) instead
// of O(cycles), with the same sample output.
void ApuStep(Apu &a, int32_t tcycles)
{
	while (tcycles > 0)
	{
		// Determine the size of the next constant-output chunk. Master
		// disable has a single event (sample emit) — timers don't tick.
		int32_t chunk = tcycles;
		if (a.sample_timer < chunk) chunk = a.sample_timer;

		if (a.master_enabled)
		{
			if (a.frame_seq_timer                   < chunk) chunk = a.frame_seq_timer;
			if (a.ch1.enabled && a.ch1.freq_timer   < chunk) chunk = a.ch1.freq_timer;
			if (a.ch2.enabled && a.ch2.freq_timer   < chunk) chunk = a.ch2.freq_timer;
			if (a.ch3.enabled && a.ch3.freq_timer   < chunk) chunk = a.ch3.freq_timer;
			if (a.ch4.enabled && a.ch4.freq_timer   < chunk) chunk = a.ch4.freq_timer;
		}
		if (chunk < 1) chunk = 1;  // safety against zero-period corner cases

		// Integrate channel output × chunk into the sample accumulator.
		if (a.master_enabled)
		{
			int32_t l, r;
			Mix(a, l, r);
			a.sample_accum_l   += l * chunk;
			a.sample_accum_r   += r * chunk;
		}
		a.sample_accum_cnt += static_cast<uint32_t>(chunk);

		// Advance timers by the same chunk size.
		a.sample_timer -= chunk;

		if (a.master_enabled)
		{
			a.frame_seq_timer -= chunk;
			if (a.ch1.enabled) a.ch1.freq_timer -= chunk;
			if (a.ch2.enabled) a.ch2.freq_timer -= chunk;
			if (a.ch3.enabled) a.ch3.freq_timer -= chunk;
			if (a.ch4.enabled) a.ch4.freq_timer -= chunk;

			// Handle expiries. Channel periods are positive, so a single
			// reload per event suffices; for larger skips (e.g. a very
			// short NR43 divisor), the next loop iteration's chunk math
			// reconverges.
			if (a.ch1.enabled && a.ch1.freq_timer <= 0)
			{
				a.ch1.freq_timer += SquarePeriod(a.ch1.freq);
				a.ch1.duty_pos    = static_cast<uint8_t>((a.ch1.duty_pos + 1) & 7);
			}
			if (a.ch2.enabled && a.ch2.freq_timer <= 0)
			{
				a.ch2.freq_timer += SquarePeriod(a.ch2.freq);
				a.ch2.duty_pos    = static_cast<uint8_t>((a.ch2.duty_pos + 1) & 7);
			}
			if (a.ch3.enabled && a.ch3.freq_timer <= 0)
			{
				a.ch3.freq_timer += WavePeriod(a.ch3.freq);
				a.ch3.pos         = static_cast<uint8_t>((a.ch3.pos + 1) & 0x1F);
				const uint8_t byte = a.ch3.ram[a.ch3.pos >> 1];
				a.ch3.sample_buf  = (a.ch3.pos & 1) ? static_cast<uint8_t>(byte & 0x0F)
				                                    : static_cast<uint8_t>(byte >> 4);
			}
			if (a.ch4.enabled && a.ch4.freq_timer <= 0)
			{
				a.ch4.freq_timer += NoisePeriod(a.ch4.nr43);
				const uint16_t bit  = static_cast<uint16_t>((a.ch4.lfsr ^ (a.ch4.lfsr >> 1)) & 1);
				uint16_t       next = static_cast<uint16_t>((a.ch4.lfsr >> 1) | (bit << 14));
				if (a.ch4.nr43 & 0x08)
					next = static_cast<uint16_t>((next & ~0x40) | (bit << 6));
				a.ch4.lfsr = next;
			}

			if (a.frame_seq_timer <= 0)
			{
				a.frame_seq_timer += FRAME_SEQ_PERIOD;
				FrameSequencerStep(a);
			}
		}

		if (a.sample_timer <= 0)
		{
			// Bresenham: reload with `cycles_per_sample` floor, plus
			// occasionally an extra cycle when remainder accumulates
			// past output_rate. Long-run average reload = clock_hz/
			// output_rate exactly — no integer-rounding drift.
			a.sample_timer += a.cycles_per_sample;
			a.cps_remainder_acc += a.cps_remainder_step;
			if (a.cps_remainder_acc >= a.output_rate)
			{
				a.cps_remainder_acc -= a.output_rate;
				a.sample_timer += 1;
			}
			FlushSample(a);
		}

		tcycles -= chunk;
	}
}

int32_t ApuDrain(Apu &a, int16_t *out, int32_t max_samples)
{
	int32_t got = 0;
	while (got < max_samples && a.sample_tail != a.sample_head)
	{
		out[got * 2 + 0] = a.sample_buf[a.sample_tail * 2 + 0];
		out[got * 2 + 1] = a.sample_buf[a.sample_tail * 2 + 1];
		a.sample_tail    = (a.sample_tail + 1) % APU_SAMPLE_BUF_SIZE;
		++got;
	}
	return got;
}

// ===================================================================
// Register access
// ===================================================================

uint8_t ApuRead(Apu &a, uint16_t addr)
{
	// Wave RAM is always accessible; CH3 running is a special-case on
	// DMG (reads from CH3's internal pos) — we return the backing RAM
	// which is correct when CH3 is off. Real DMG quirks deferred.
	if (addr >= 0xFF30 && addr <= 0xFF3F)
	{
		return a.ch3.ram[addr - 0xFF30];
	}

	switch (addr)
	{
		case 0xFF10: return static_cast<uint8_t>(a.ch1.nrx0 | 0x80);
		case 0xFF11: return static_cast<uint8_t>(a.ch1.nrx1 | 0x3F);
		case 0xFF12: return a.ch1.nrx2;
		case 0xFF13: return 0xFF;
		case 0xFF14: return static_cast<uint8_t>(a.ch1.nrx4 | 0xBF);

		case 0xFF16: return static_cast<uint8_t>(a.ch2.nrx1 | 0x3F);
		case 0xFF17: return a.ch2.nrx2;
		case 0xFF18: return 0xFF;
		case 0xFF19: return static_cast<uint8_t>(a.ch2.nrx4 | 0xBF);

		case 0xFF1A: return static_cast<uint8_t>(a.ch3.nr30 | 0x7F);
		case 0xFF1B: return 0xFF;
		case 0xFF1C: return static_cast<uint8_t>(a.ch3.nr32 | 0x9F);
		case 0xFF1D: return 0xFF;
		case 0xFF1E: return static_cast<uint8_t>(a.ch3.nr34 | 0xBF);

		case 0xFF20: return 0xFF;
		case 0xFF21: return a.ch4.nr42;
		case 0xFF22: return a.ch4.nr43;
		case 0xFF23: return static_cast<uint8_t>(a.ch4.nr44 | 0xBF);

		case 0xFF24: return a.nr50;
		case 0xFF25: return a.nr51;
		case 0xFF26:
		{
			uint8_t status = a.master_enabled ? 0x80 : 0x00;
			status |= 0x70;  // bits 4-6 always read as 1
			if (a.ch1.enabled) status |= 0x01;
			if (a.ch2.enabled) status |= 0x02;
			if (a.ch3.enabled) status |= 0x04;
			if (a.ch4.enabled) status |= 0x08;
			return status;
		}
	}
	return 0xFF;
}

void ApuWrite(Apu &a, uint16_t addr, uint8_t value)
{
	// Wave RAM writes pass through regardless of master enable.
	if (addr >= 0xFF30 && addr <= 0xFF3F)
	{
		a.ch3.ram[addr - 0xFF30] = value;
		return;
	}

	// NR52 master enable is writable even when the rest of the APU is off.
	if (addr == 0xFF26)
	{
		const bool new_on = (value & 0x80) != 0;
		if (!new_on && a.master_enabled)
		{
			// Powering down: zero writable regs, disable channels. Wave
			// RAM survives — real DMG keeps it across APU power cycles.
			uint8_t wave_ram_save[16];
			std::memcpy(wave_ram_save, a.ch3.ram, 16);

			a.ch1 = ApuSquare{};
			a.ch2 = ApuSquare{};
			a.ch3 = ApuWave{};
			a.ch4 = ApuNoise{};
			a.ch4.lfsr = 0x7FFF;
			a.nr50 = 0;
			a.nr51 = 0;
			a.frame_seq_step = 0;

			std::memcpy(a.ch3.ram, wave_ram_save, 16);
		}
		a.master_enabled = new_on;
		return;
	}

	// When master is off, writes to the other APU regs are ignored on DMG.
	if (!a.master_enabled) return;

	switch (addr)
	{
		case 0xFF10:
		{
			// Clearing the negate bit after a sweep calculation that used
			// it disables the channel — the "sweep subtraction clear" quirk.
			const bool was_negate = (a.ch1.nrx0 & 0x08) != 0;
			a.ch1.nrx0 = value;
			if (was_negate && !(value & 0x08) && a.ch1.sweep_negate_calc_used)
			{
				a.ch1.enabled = false;
			}
			return;
		}
		case 0xFF11:
			a.ch1.nrx1 = value;
			a.ch1.length = static_cast<uint32_t>(64 - (value & 0x3F));
			return;
		case 0xFF12:
			a.ch1.nrx2        = value;
			a.ch1.dac_enabled = (value & 0xF8) != 0;
			if (!a.ch1.dac_enabled) a.ch1.enabled = false;
			return;
		case 0xFF13:
			a.ch1.nrx3 = value;
			a.ch1.freq = SquareFreq(a.ch1);
			return;
		case 0xFF14:
			a.ch1.nrx4            = value;
			a.ch1.length_enabled  = (value & 0x40) != 0;
			a.ch1.freq            = SquareFreq(a.ch1);
			if (value & 0x80) TriggerSquare(a.ch1, /*is_ch1=*/true);
			return;

		case 0xFF16:
			a.ch2.nrx1 = value;
			a.ch2.length = static_cast<uint32_t>(64 - (value & 0x3F));
			return;
		case 0xFF17:
			a.ch2.nrx2        = value;
			a.ch2.dac_enabled = (value & 0xF8) != 0;
			if (!a.ch2.dac_enabled) a.ch2.enabled = false;
			return;
		case 0xFF18:
			a.ch2.nrx3 = value;
			a.ch2.freq = SquareFreq(a.ch2);
			return;
		case 0xFF19:
			a.ch2.nrx4            = value;
			a.ch2.length_enabled  = (value & 0x40) != 0;
			a.ch2.freq            = SquareFreq(a.ch2);
			if (value & 0x80) TriggerSquare(a.ch2, /*is_ch1=*/false);
			return;

		case 0xFF1A:
			a.ch3.nr30        = value;
			a.ch3.dac_enabled = (value & 0x80) != 0;
			if (!a.ch3.dac_enabled) a.ch3.enabled = false;
			return;
		case 0xFF1B:
			a.ch3.nr31   = value;
			a.ch3.length = static_cast<uint32_t>(256 - value);
			return;
		case 0xFF1C:
			a.ch3.nr32 = value;
			return;
		case 0xFF1D:
			a.ch3.nr33 = value;
			a.ch3.freq = WaveFreq(a.ch3);
			return;
		case 0xFF1E:
			a.ch3.nr34           = value;
			a.ch3.length_enabled = (value & 0x40) != 0;
			a.ch3.freq           = WaveFreq(a.ch3);
			if (value & 0x80) TriggerWave(a.ch3);
			return;

		case 0xFF20:
			a.ch4.nr41   = value;
			a.ch4.length = static_cast<uint32_t>(64 - (value & 0x3F));
			return;
		case 0xFF21:
			a.ch4.nr42        = value;
			a.ch4.dac_enabled = (value & 0xF8) != 0;
			if (!a.ch4.dac_enabled) a.ch4.enabled = false;
			return;
		case 0xFF22:
			a.ch4.nr43 = value;
			return;
		case 0xFF23:
			a.ch4.nr44           = value;
			a.ch4.length_enabled = (value & 0x40) != 0;
			if (value & 0x80) TriggerNoise(a.ch4);
			return;

		case 0xFF24: a.nr50 = value; return;
		case 0xFF25: a.nr51 = value; return;
	}
}

} // namespace SGB
