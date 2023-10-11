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
#include <cmath>

class Resampler
{
  public:
    volatile int end;
    int buffer_size;
    volatile int start;
    int16_t *buffer;

    float r_step;
    float r_frac;
    int   r_left[4], r_right[4];

    static inline int16_t short_clamp(int n)
    {
        return (int16_t)(((int16_t)n != n) ? (n >> 31) ^ 0x7fff : n);
    }

    static inline int min(int a, int b)
    {
        return ((a) < (b) ? (a) : (b));
    }

    static inline float hermite(float mu1, float a, float b, float c, float d)
    {
        float mu2, mu3, m0, m1, a0, a1, a2, a3;

        mu2 = mu1 * mu1;
        mu3 = mu2 * mu1;

        m0 = (c - a) * 0.5;
        m1 = (d - b) * 0.5;

        a0 = +2 * mu3 - 3 * mu2 + 1;
        a1 = mu3 - 2 * mu2 + mu1;
        a2 = mu3 - mu2;
        a3 = -2 * mu3 + 3 * mu2;

        return (a0 * b) + (a1 * m0) + (a2 * m1) + (a3 * c);
    }

    Resampler()
    {
        this->buffer_size = 0;
        buffer = NULL;
        r_step = 1.0;
    }

    Resampler(int num_samples)
    {
        buffer = NULL;
        resize(num_samples);
        r_step = 1.0;
    }

    ~Resampler()
    {
        delete[] buffer;
        buffer = NULL;
    }

    inline void time_ratio(double ratio)
    {
        r_step = ratio;
    }

    inline void clear(void)
    {
        if (!buffer)
            return;

        start = 0;
        end = 0;
        memset(buffer, 0, buffer_size * 2);

        r_frac = 0.0;
        r_left[0] = r_left[1] = r_left[2] = r_left[3] = 0;
        r_right[0] = r_right[1] = r_right[2] = r_right[3] = 0;
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

        return true;
    }

    inline void push_sample(int16_t l, int16_t r)
    {
        if (space_empty() >= 2)
        {
            buffer[end] = l;
            buffer[end + 1] = r;
            end = (end + 2) % buffer_size;
        }
    }

    inline bool push(int16_t *src, int num_samples)
    {
        if (space_empty() < num_samples)
            return false;

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
        int o_position = 0;

        while (o_position < num_samples && space_filled() >= 2)
        {
            int s_left = buffer[start];
            int s_right = buffer[start + 1];
            int hermite_val[2];

            while (r_frac <= 1.0 && o_position < num_samples)
            {
                hermite_val[0] = (int)hermite(r_frac, (float)r_left[0], (float)r_left[1], (float)r_left[2], (float)r_left[3]);
                hermite_val[1] = (int)hermite(r_frac, (float)r_right[0], (float)r_right[1], (float)r_right[2], (float)r_right[3]);
                data[o_position] = short_clamp(hermite_val[0]);
                data[o_position + 1] = short_clamp(hermite_val[1]);

                o_position += 2;

                r_frac += r_step;
            }

            if (r_frac > 1.0)
            {
                r_left[0] = r_left[1];
                r_left[1] = r_left[2];
                r_left[2] = r_left[3];
                r_left[3] = s_left;

                r_right[0] = r_right[1];
                r_right[1] = r_right[2];
                r_right[2] = r_right[3];
                r_right[3] = s_right;

                r_frac -= 1.0;

                start += 2;
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
};

#endif /* __NEW_RESAMPLER_H */