/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __NEW_RESAMPLER_H
#define __NEW_RESAMPLER_H

#include <cstring>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cmath>

// Windowed-sinc polyphase interpolator.
//
// The SPC DSP emits 32040 Hz and the host device almost never runs at that
// rate, so every sample the user hears goes through this. The previous 4-point
// cubic Hermite kernel left the first image (at 32040 - f) only 1.4 dB down at
// 15.5 kHz and 11 dB down at 12 kHz; the higher-order images folded back into
// the audible band as inharmonic tones, which is what "crackling" complaints
// about treble-heavy content were describing.
//
// Kernel is a Kaiser-windowed sinc sampled into RESAMPLER_PHASES fractional
// positions, with linear interpolation between adjacent phases so the phase
// quantisation does not itself set the noise floor. Tap count scales with the
// cutoff so downsampling (PlaybackRate < 32040) gets a real anti-alias filter
// instead of none at all.
#define RESAMPLER_ZEROS   24    // sinc zero crossings per side at full cutoff
#define RESAMPLER_PHASES  256   // fractional-delay table resolution
#define RESAMPLER_MAX_HALF 160  // tap-count ceiling (hit only at 8 kHz output)
#define RESAMPLER_MAX_TAPS (RESAMPLER_MAX_HALF * 2)
#define RESAMPLER_BETA    9.0   // Kaiser beta ~ -90 dB stopband
#define RESAMPLER_CUTOFF  0.918 // passband edge as a fraction of Nyquist
// M_PI is not in <cmath> under MSVC without _USE_MATH_DEFINES.
#define RESAMPLER_PI      3.14159265358979323846

// Selectable kernel ("Audio Fidelity" in the Sound Settings dialog). Both run
// off one delay line centred on the same interpolation base, so they share a
// group delay and can be swapped mid-stream without a click.
enum ResamplerEngine
{
    RESAMPLER_ENGINE_HERMITE = 0,
    RESAMPLER_ENGINE_SINC    = 1
};

struct ResamplerState
{
    int   start;
    int   end;
    float r_frac;
    int16_t hist_l[RESAMPLER_MAX_TAPS];
    int16_t hist_r[RESAMPLER_MAX_TAPS];
};

class Resampler
{
  public:
    volatile int end;
    int buffer_size;
    volatile int start;
    int16_t *buffer;

    float r_step;
    float r_frac;

    // I/O meters in input words: pushed/dropped at push, consumed at read.
    unsigned int dropped = 0;
    unsigned int pushed = 0;
    unsigned int consumed = 0;

    static inline int16_t short_clamp(int n)
    {
        return (int16_t)(((int16_t)n != n) ? (n >> 31) ^ 0x7fff : n);
    }

    static inline int min(int a, int b)
    {
        return ((a) < (b) ? (a) : (b));
    }

    Resampler()
    {
        this->buffer_size = 0;
        buffer = NULL;
        r_step = 1.0;
        table = NULL;
        taps = 0;
        table_cutoff_key = -1;
        want_cutoff_key = (int)(RESAMPLER_CUTOFF * 64.0 + 0.5);
        clear_history();
    }

    Resampler(int num_samples)
    {
        buffer = NULL;
        table = NULL;
        taps = 0;
        table_cutoff_key = -1;
        want_cutoff_key = (int)(RESAMPLER_CUTOFF * 64.0 + 0.5);
        clear_history();
        resize(num_samples);
        r_step = 1.0;
    }

    ~Resampler()
    {
        delete[] buffer;
        buffer = NULL;
        delete[] table;
        table = NULL;
    }

    // Records the wanted kernel; the table itself is (re)built inside read().
    // time_ratio() is called from the sound-callback thread while read() can be
    // running on the SGB drain thread, so it must not touch the table memory.
    inline void time_ratio(double ratio)
    {
        if (ratio <= 0.0)
            return;
        r_step = ratio;
        // Anti-alias at the OUTPUT Nyquist when decimating; when interpolating
        // the input Nyquist is the binding limit and the kernel is unchanged,
        // so the common path never rebuilds.
        double cutoff = RESAMPLER_CUTOFF;
        if (ratio > 1.0)
            cutoff /= ratio;
        // Coarse quantisation: dynamic rate control nudges the ratio by well
        // under a percent on every callback and must not trigger rebuilds.
        int key = (int)(cutoff * 64.0 + 0.5);
        if (key < 1) key = 1;
        want_cutoff_key = key;
    }

    inline void clear(void)
    {
        if (!buffer)
            return;

        start = 0;
        end = 0;
        memset(buffer, 0, buffer_size * 2);

        r_frac = 0.0;
        clear_history();
    }

    inline void dump(int num_samples)
    {
        if (num_samples > 0 && space_filled() >= num_samples)
            start = (start + num_samples) % buffer_size;
    }

    inline void add_silence(int num_samples)
    {
         if (num_samples > 0 && space_empty() < num_samples)
            return;

        int first_block_size = min(num_samples, buffer_size - end);

        memset(buffer + end, 0, first_block_size * 2);

        if (num_samples > first_block_size)
            memset(buffer, 0, (num_samples - first_block_size) * 2);

        end = (end + num_samples) % buffer_size;

        return;
    }

    inline bool pull(int16_t *dst, int num_samples)
    {
        if (space_filled() < num_samples)
            return false;

        int first_block_size = buffer_size - start;

        memcpy(dst, buffer + start, min(num_samples, first_block_size) * 2);

        if (num_samples > first_block_size)
            memcpy(dst + first_block_size, buffer, (num_samples - first_block_size) * 2);

        start = (start + num_samples) % buffer_size;
        consumed += num_samples;

        return true;
    }

    inline void push_sample(int16_t l, int16_t r)
    {
        if (space_empty() >= 2)
        {
            buffer[end] = l;
            buffer[end + 1] = r;
            end = (end + 2) % buffer_size;
            pushed += 2;
        }
        else
            dropped += 2;
    }

    inline bool push(int16_t *src, int num_samples)
    {
        if (space_empty() < num_samples)
        {
            dropped += num_samples;
            return false;
        }

        int first_block_size = min(num_samples, buffer_size - end);

        memcpy(buffer + end, src, first_block_size * 2);

        if (num_samples > first_block_size)
            memcpy(buffer, src + first_block_size, (num_samples - first_block_size) * 2);

        end = (end + num_samples) % buffer_size;

        return true;
    }

    void read(int16_t *data, int num_samples)
    {
        //If we are outputting the exact same ratio as the input, pull directly from the input buffer
        if (r_step == 1.0)
        {
            pull(data, num_samples);
            return;
        }

        assert((num_samples & 1) == 0); // resampler always processes both stereo samples

        // Both kernels run off the same delay line and the same interpolation
        // base, so they have identical group delay and switching mid-stream is
        // seamless — no history reset, no click. That is what makes the Sound
        // Settings combo safe to apply live.
        if (!table || want_cutoff_key != table_cutoff_key)
            build_table(want_cutoff_key / 64.0, want_cutoff_key);

        engine = want_engine;

        int o_position = 0;

        while (o_position < num_samples && space_filled() >= 2)
        {
            int s_left = buffer[start];
            int s_right = buffer[start + 1];

            while (r_frac <= 1.0 && o_position < num_samples)
            {
                float acc_l, acc_r;

                if (engine == RESAMPLER_ENGINE_HERMITE)
                {
                    // Interpolate between hist[b] and hist[b+1] — the same base
                    // sample the sinc kernel is centred on.
                    const int b = taps / 2 - 1;
                    acc_l = hermite(r_frac, (float)hist_l[b - 1], (float)hist_l[b],
                                            (float)hist_l[b + 1], (float)hist_l[b + 2]);
                    acc_r = hermite(r_frac, (float)hist_r[b - 1], (float)hist_r[b],
                                            (float)hist_r[b + 1], (float)hist_r[b + 2]);
                }
                else
                {
                    float fp = r_frac * (float)RESAMPLER_PHASES;
                    int   pi = (int)fp;
                    float pf = fp - (float)pi;
                    if (pi >= RESAMPLER_PHASES) { pi = RESAMPLER_PHASES - 1; pf = 1.0f; }
                    if (pi < 0)                 { pi = 0;                    pf = 0.0f; }

                    const float *c0 = table + (size_t)pi * taps;
                    const float *c1 = c0 + taps;

                    acc_l = 0.0f; acc_r = 0.0f;
                    for (int j = 0; j < taps; j++)
                    {
                        float c = c0[j] + pf * (c1[j] - c0[j]);
                        acc_l += c * (float)hist_l[j];
                        acc_r += c * (float)hist_r[j];
                    }
                }

                data[o_position]     = short_clamp((int)lrintf(acc_l));
                data[o_position + 1] = short_clamp((int)lrintf(acc_r));

                o_position += 2;

                r_frac += r_step;
            }

            if (r_frac > 1.0)
            {
                memmove(hist_l, hist_l + 1, (taps - 1) * sizeof(int16_t));
                memmove(hist_r, hist_r + 1, (taps - 1) * sizeof(int16_t));
                hist_l[taps - 1] = (int16_t)s_left;
                hist_r[taps - 1] = (int16_t)s_right;

                r_frac -= 1.0;

                start += 2;
                consumed += 2;
                if (start >= buffer_size)
                    start -= buffer_size;
            }
        }
    }

    inline int space_empty(void) const
    {
        return buffer_size - 2 - space_filled();
    }

    inline int space_filled(void) const
    {
        int size = end - start;
        if (size < 0)
            size += buffer_size;
        return size;
    }

    inline int avail(void)
    {
        int size = space_filled();
        //If we are outputting the exact same ratio as the input, find out directly from the input buffer
        if (r_step == 1.0)
            return size;

        return (int)trunc(((size >> 1) - r_frac) / r_step) * 2;
    }

    void resize(int num_samples)
    {
        if (buffer)
            delete[] buffer;
        // Only allow even buffer sizes
        if (num_samples & 1)
            num_samples++;
        buffer_size = num_samples;
        buffer = new int16_t[buffer_size];
        clear();
    }

    // Save/restore resampler control state (ring-buffer positions, phase
    // accumulator, filter history). Used by run-ahead to undo the effect of
    // the hidden frame's audio generation on the resampler without resetting
    // the filter warm-up state (which would cause audio artifacts on every
    // iteration).
    void save_state(ResamplerState &s) const
    {
        s.start = start;
        s.end = end;
        s.r_frac = r_frac;
        memcpy(s.hist_l, hist_l, sizeof(hist_l));
        memcpy(s.hist_r, hist_r, sizeof(hist_r));
    }

    void load_state(const ResamplerState &s)
    {
        start = s.start;
        end = s.end;
        r_frac = s.r_frac;
        memcpy(hist_l, s.hist_l, sizeof(hist_l));
        memcpy(hist_r, s.hist_r, sizeof(hist_r));
    }

    int filter_taps(void) const { return taps; }

    // "Audio Fidelity" selector. Like time_ratio() this only records the
    // request; read() performs the switch, so neither the table nor the delay
    // line is mutated from the sound-callback thread while the SGB drain
    // thread is inside read().
    void set_engine(int e)
    {
        want_engine = (e == RESAMPLER_ENGINE_HERMITE) ? RESAMPLER_ENGINE_HERMITE
                                                      : RESAMPLER_ENGINE_SINC;
    }

    int get_engine(void) const { return engine; }

  private:
    static inline float hermite(float mu1, float a, float b, float c, float d)
    {
        float mu2, mu3, m0, m1, a0, a1, a2, a3;

        mu2 = mu1 * mu1;
        mu3 = mu2 * mu1;

        m0 = (c - a) * 0.5f;
        m1 = (d - b) * 0.5f;

        a0 = +2 * mu3 - 3 * mu2 + 1;
        a1 = mu3 - 2 * mu2 + mu1;
        a2 = mu3 - mu2;
        a3 = -2 * mu3 + 3 * mu2;

        return (a0 * b) + (a1 * m0) + (a2 * m1) + (a3 * c);
    }

    float  *table;             // (RESAMPLER_PHASES + 1) rows of `taps` coefficients
    int     taps;
    int     table_cutoff_key;  // cutoff the current table was built for
    int     want_cutoff_key;   // cutoff requested by time_ratio()
    int     engine = RESAMPLER_ENGINE_SINC;
    int     want_engine = RESAMPLER_ENGINE_SINC;
    int16_t hist_l[RESAMPLER_MAX_TAPS];
    int16_t hist_r[RESAMPLER_MAX_TAPS];

    inline void clear_history(void)
    {
        memset(hist_l, 0, sizeof(hist_l));
        memset(hist_r, 0, sizeof(hist_r));
    }

    static double bessel_i0(double x)
    {
        double sum = 1.0, term = 1.0;
        for (int k = 1; k < 64; k++)
        {
            term *= (x * x) / (4.0 * k * k);
            sum += term;
            if (term < sum * 1e-17)
                break;
        }
        return sum;
    }

    // h(t) = fc * sinc(fc * t) * kaiser(t / half), sampled at the TAPS integer
    // offsets around each fractional phase. Rows are normalised to unity DC so
    // the phase sweep does not amplitude-modulate the signal.
    void build_table(double cutoff, int key)
    {
        int half = (int)ceil(RESAMPLER_ZEROS / cutoff);
        if (half > RESAMPLER_MAX_HALF)
            half = RESAMPLER_MAX_HALF;
        if (half < 2)
            half = 2;

        const int new_taps = half * 2;
        float *new_table = new float[(size_t)(RESAMPLER_PHASES + 1) * new_taps];
        const double i0b = bessel_i0(RESAMPLER_BETA);

        for (int p = 0; p <= RESAMPLER_PHASES; p++)
        {
            const double mu = (double)p / RESAMPLER_PHASES;
            float *row = new_table + (size_t)p * new_taps;
            double sum = 0.0;

            for (int j = 0; j < new_taps; j++)
            {
                const double t = mu + (double)(half - 1) - (double)j;
                const double x = t / (double)half;

                double w = 0.0;
                if (x > -1.0 && x < 1.0)
                    w = bessel_i0(RESAMPLER_BETA * sqrt(1.0 - x * x)) / i0b;

                const double a = RESAMPLER_PI * cutoff * t;
                const double s = (fabs(a) < 1e-9) ? cutoff : cutoff * sin(a) / a;

                const double c = w * s;
                row[j] = (float)c;
                sum += c;
            }

            if (sum != 0.0)
                for (int j = 0; j < new_taps; j++)
                    row[j] = (float)(row[j] / sum);
        }

        delete[] table;
        table = new_table;
        table_cutoff_key = key;
        // Only a tap-count change invalidates the delay line; a same-length
        // rebuild keeps its contents so re-tuning the cutoff mid-stream does
        // not put a step in the output.
        if (new_taps != taps)
        {
            taps = new_taps;
            clear_history();
        }
    }
};

#endif /* __NEW_RESAMPLER_H */
