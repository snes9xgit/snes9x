/* Simple resampler based on bsnes's ruby audio library */

#ifndef __HERMITE_RESAMPLER_H
#define __HERMITE_RESAMPLER_H

#include "resampler.h"

#undef CLAMP
#undef SHORT_CLAMP
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define SHORT_CLAMP(n) ((short) CLAMP((n), -32768, 32767))

class HermiteResampler : public Resampler
{
    protected:

        uint32 r_step;
        uint32 r_frac;
        int   r_left[4], r_right[4];

        static inline int32
        hermite (uint32 mu1, int32 a, int32 b, int32 c, int32 d)
        {
            uint32 mu2, mu3, m0, m1, a0, a1, a2, a3;

            mu2 = (mu1 * mu1) >> 15;
            mu3 = (mu2 * mu1) >> 15;

            m0 = (c - a + 32768) << 14;
            m1 = (d - b + 32768) << 14;

            a0 = (mu3 << 1) - (3 * mu2) + (1 << 15);
            a1 =  mu3       - (mu2 << 1) + mu1;
            a2 =  mu3       -  mu2;
            a3 =  3 * mu2   - (mu3 << 1);

            return (int32) ((
              (a0 * (b + 32768)) +
              ((a1 * m0) >> 15) +
              ((a2 * m1) >> 15) +
              (a3 * (c + 32768))
              ) >> 15) - 32768;
        }

    public:
        HermiteResampler (int num_samples) : Resampler (num_samples)
        {
            clear ();
        }

        ~HermiteResampler ()
        {
        }

        void
        time_ratio (double ratio)
        {
            r_step = (1 << 15) * ratio;
            clear ();
        }

        void
        clear (void)
        {
            ring_buffer::clear ();
            r_frac = (1 << 15);
            r_left [0] = r_left [1] = r_left [2] = r_left [3] = 0;
            r_right[0] = r_right[1] = r_right[2] = r_right[3] = 0;
        }

        void
        read (short *data, int num_samples)
        {
            int i_position = start >> 1;
            int max_samples = buffer_size >> 1;
            short *internal_buffer = (short *) buffer;
            int o_position = 0;
            int consumed = 0;

            while (o_position < num_samples && consumed < buffer_size)
            {
                int s_left = internal_buffer[i_position];
                int s_right = internal_buffer[i_position + 1];
                int hermite_val;

                while (r_frac <= (1 << 15) && o_position < num_samples)
                {
                    hermite_val = hermite (r_frac, r_left [0], r_left [1], r_left [2], r_left [3]);
                    data[o_position]     = SHORT_CLAMP (hermite_val);
                    hermite_val = hermite (r_frac, r_right[0], r_right[1], r_right[2], r_right[3]);
                    data[o_position + 1] = SHORT_CLAMP (hermite_val);

                    o_position += 2;

                    r_frac += r_step;
                }

                if (r_frac > (1 << 15))
                {
                    r_left [0] = r_left [1];
                    r_left [1] = r_left [2];
                    r_left [2] = r_left [3];
                    r_left [3] = s_left;

                    r_right[0] = r_right[1];
                    r_right[1] = r_right[2];
                    r_right[2] = r_right[3];
                    r_right[3] = s_right;

                    r_frac -= (1 << 15);

                    i_position += 2;
                    if (i_position >= max_samples)
                        i_position -= max_samples;
                    consumed += 2;
                }
            }

            size -= consumed << 1;
            start += consumed << 1;
            if (start >= buffer_size)
                start -= buffer_size;
        }

        inline int
        avail (void)
        {
            return ((((uint32) size) << 13) - r_frac) / r_step * 2;
        }
};

#endif /* __HERMITE_RESAMPLER_H */
