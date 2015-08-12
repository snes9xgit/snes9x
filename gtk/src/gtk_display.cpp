#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <sched.h>

#include "gtk_s9x.h"
#include "gtk_display.h"
#include "gtk_display_driver.h"
#include "gtk_display_driver_gtk.h"
#include "snes_ntsc.h"
#ifdef USE_XV
#include "gtk_display_driver_xv.h"
#endif
#ifdef USE_OPENGL
#include "gtk_display_driver_opengl.h"
#endif

static S9xDisplayDriver  *driver;
static snes_ntsc_t       snes_ntsc;
static thread_job_t      job[8];
static GThreadPool       *pool;
static uint8             *y_table, *u_table, *v_table;
static int               endianess = ENDIAN_LSB;

/* Scanline constants for the NTSC filter */
static unsigned int scanline_offsets[] =
{
        0,     /* 0%    */
        3,     /* 12.5% */
        2,     /* 25%   */
        1,     /* 50%   */
        0,     /* 100%  */
};

static unsigned short scanline_masks[] =
{
        0x0000,     /* 0%    */
        0x0C63,     /* 12.5% */
        0x1CE7,     /* 25%   */
        0x3DEF,     /* 50%   */
        0xffff,     /* 100%  */
};

extern unsigned int scanline_offset;
extern unsigned short scanline_mask;

/* Scanline constants for standard scanline filter */
static uint8 scanline_shifts[] =
{
        3,
        2,
        1,
        0
};

void
S9xSetEndianess (int type)
{
    endianess = type;

    return;
}

double
S9xGetAspect (void)
{
    double native_aspect = 256.0 / (gui_config->overscan ? 239.0 : 224.0);
    double aspect;

    switch (gui_config->aspect_ratio)
    {
        case 0: /* Square pixels */
            aspect = native_aspect;
            break;

        case 1: /* 4:3 */
            aspect = native_aspect * 7 / 6;
            break;

        case 2:
        default: /* Correct */
            aspect = native_aspect * 8 / 7;
            break;
    }

    return aspect;
}

void
S9xApplyAspect (int &s_width,  /* Output: x */
                int &s_height, /* Output: y */
                int &d_width,  /* Output: width */
                int &d_height) /* Output: height */
{
    double screen_aspect = (double) d_width / (double) d_height;
    double snes_aspect = S9xGetAspect ();
    double granularity = 1.0 / (double) MAX (d_width, d_height);
    int x, y, w, h;

    if (!gui_config->scale_to_fit)
    {
        if (gui_config->maintain_aspect_ratio)
        {
            w = s_height * snes_aspect + 0.5;
            h = s_height;
            x = (d_width - w) / 2;
            y = (d_height - s_height) / 2;
        }
        else
        {
            w = s_width;
            h = s_height;
            x = (d_width - w) / 2;
            y = (d_height - h) / 2;

        }
    }

    else if (gui_config->maintain_aspect_ratio &&
            !(screen_aspect <= snes_aspect * (1.0 + granularity) &&
              screen_aspect >= snes_aspect * (1.0 - granularity)))
    {
        if (screen_aspect > snes_aspect)
        {
            w = d_height * snes_aspect + 0.5;
            h = d_height;
            x = (d_width - w) / 2;
            y = 0;

        }

        else
        {
            w = d_width;
            h = d_width / snes_aspect + 0.5;
            x = 0;
            y = (d_height - h) / 2;

        }
    }

    else
    {
        x = 0;
        y = 0;
        w = d_width;
        h = d_height;
    }

    s_width = x;
    s_height = y;
    d_width = w;
    d_height = h;

    return;
}

void
S9xRegisterYUVTables (uint8 *y, uint8 *u, uint8 *v)
{
    y_table = y;
    u_table = u;
    v_table = v;

    return;
}

/* YUY2 in LSB first format */
static void
internal_convert_16_yuv (void *src_buffer,
                         void *dst_buffer,
                         int  src_pitch,
                         int  dst_pitch,
                         int  width,
                         int  height)
{
    register int    x, y;
    register uint16 *src;
    register uint8  *dst;
    register uint16 p0, p1;

    if (endianess == ENDIAN_MSB)
    {
        for (y = 0; y < height; y++)
        {
            src = (uint16 *) (((uint8 *) src_buffer) + src_pitch * y);
            dst = (uint8  *) (((uint8 *) dst_buffer) + dst_pitch * y);

            for (x = 0; x < width >> 1; x++)
            {
                p0 = *src++;
                p1 = *src++;

                *dst++ = (v_table[p0] + v_table[p1]) >> 1;
                *dst++ = y_table[p1];
                *dst++ = (u_table[p0] + u_table[p1]) >> 1;
                *dst++ = y_table[p0];
            }
        }
    }

    else
    {
        for (y = 0; y < height; y++)
        {
            src = (uint16 *) (((uint8 *) src_buffer) + src_pitch * y);
            dst = (uint8  *) (((uint8 *) dst_buffer) + dst_pitch * y);

            for (x = 0; x < width >> 1; x++)
            {
                p0 = *src++;
                p1 = *src++;

                *dst++ = y_table[p0];
                *dst++ = (u_table[p0] + u_table[p1]) >> 1;
                *dst++ = y_table[p1];
                *dst++ = (v_table[p0] + v_table[p1]) >> 1;
            }
        }
    }

    return;
}

static void
internal_convert_mask (void         *src_buffer,
                       void         *dst_buffer,
                       int          src_pitch,
                       int          dst_pitch,
                       int          width,
                       int          height,
                       unsigned int inv_rshift,
                       unsigned int inv_gshift,
                       unsigned int inv_bshift,
                       unsigned int bpp)
{
#ifdef __BIG_ENDIAN__
    if (endianess == ENDIAN_MSB)
#else
    if (endianess == ENDIAN_LSB)
#endif
    {
        switch (bpp)
        {
            case 15:
            case 16:

                for (register int y = 0; y < height; y++)
                {
                    register uint16 *data =
                        (uint16 *) ((uint8 *) dst_buffer + y * dst_pitch);

                    register uint16 *snes =
                        (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                    for (register int x = 0; x < width; x++)
                    {
                        uint32 pixel = *snes++;

                        *data++ = ((pixel & 0x7c00) << 1) >> inv_rshift
                                | ((pixel & 0x03e0) << 6) >> inv_gshift
                                | ((pixel & 0x001f) << 11) >> inv_bshift;
                    }
                }

                break;

            case 24:

#ifdef __BIG_ENDIAN__
                if (inv_rshift > 8)
#else
                if (!(inv_rshift > 8))
#endif
                {
                    for (register int y = 0; y < height; y++)
                    {
                        register uint8 *data =
                            (uint8 *) dst_buffer + y * dst_pitch;

                        register uint16 *snes =
                            (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                        for (register int x = 0; x < width; x++)
                        {
                            uint32 pixel = *snes++;
                            *data++ = ((pixel & 0x001f) << 3);
                            *data++ = ((pixel & 0x03e0) >> 2);
                            *data++ = ((pixel & 0x7c00) >> 7);
                        }
                    }
                }

                else
                {
                    for (register int y = 0; y < height; y++)
                    {
                        register uint8 *data =
                            (uint8 *) dst_buffer + y * dst_pitch;

                        register uint16 *snes =
                            (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                        for (register int x = 0; x < width; x++)
                        {
                            uint32 pixel = *snes++;
                            *data++ = ((pixel & 0x7c00) >> 7);
                            *data++ = ((pixel & 0x03e0) >> 2);
                            *data++ = ((pixel & 0x001f) << 3);
                        }
                    }
                }

                break;

            case 32:

                for (register int y = 0; y < height; y++)
                {
                    register uint32 *data =
                        (uint32 *) ((uint8 *) dst_buffer + y * dst_pitch);

                    register uint16 *snes =
                        (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                    for (register int x = 0; x < width; x++)
                    {
                        uint32 pixel = *snes++;

                        *data++ = ((uint32) (pixel & 0x7c00) << 17) >> inv_rshift
                                | ((uint32) (pixel & 0x03e0) << 22) >> inv_gshift
                                | ((uint32) (pixel & 0x001f) << 27) >> inv_bshift;
                    }
                }

                break;
        }
    }

    else /* Byte-order is inverted from native */
    {
        switch (bpp)
        {
            case 15:
            case 16:

                for (register int y = 0; y < height; y++)
                {
                    register uint16 *data =
                        (uint16 *) ((uint8 *) dst_buffer + y * dst_pitch);

                    register uint16 *snes =
                        (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                    for (register int x = 0; x < width; x++)
                    {
                        uint32 pixel = *snes++;
                        register uint16 value;

                        value   = ((pixel & 0x7c00) << 1) >> inv_rshift
                                | ((pixel & 0x03e0) << 6) >> inv_gshift
                                | ((pixel & 0x001f) << 11) >> inv_bshift;

                        *data++ = ((value & 0xff) << 8)
                                | ((value & 0xff00) >> 8);
                    }
                }

                break;

            case 24:

#ifdef __BIG_ENDIAN__
                if (inv_rshift > 8)
#else
                    if (!(inv_rshift > 8))
#endif
                    {
                        for (register int y = 0; y < height; y++)
                        {
                            register uint8 *data =
                                (uint8 *) dst_buffer + y * dst_pitch;

                            register uint16 *snes =
                                (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                            for (register int x = 0; x < width; x++)
                            {
                                uint32 pixel = *snes++;
                                *data++ = ((pixel & 0x7c00) >> 7);
                                *data++ = ((pixel & 0x03e0) >> 2);
                                *data++ = ((pixel & 0x001f) << 3);
                            }
                        }
                    }

                    else
                    {
                        for (register int y = 0; y < height; y++)
                        {
                            register uint8 *data =
                                (uint8 *) dst_buffer + y * dst_pitch;

                            register uint16 *snes =
                                (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                            for (register int x = 0; x < width; x++)
                            {
                                uint32 pixel = *snes++;
                                *data++ = ((pixel & 0x001f) << 3);
                                *data++ = ((pixel & 0x03e0) >> 2);
                                *data++ = ((pixel & 0x7c00) >> 7);
                            }
                        }
                    }

                break;

            case 32:

                for (register int y = 0; y < height; y++)
                {
                    register uint32 *data =
                        (uint32 *) ((uint8 *) dst_buffer + y * dst_pitch);

                    register uint16 *snes =
                        (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                    for (register int x = 0; x < width; x++)
                    {
                        uint32 pixel = *snes++;
                        register uint32 value;

                        value   = ((uint32) (pixel & 0x7c00) << 17) >> inv_rshift
                                | ((uint32) (pixel & 0x03e0) << 22) >> inv_gshift
                                | ((uint32) (pixel & 0x001f) << 27) >> inv_bshift;

                        *data++ = ((value & 0x000000ff) << 24)
                                | ((value & 0x0000ff00) << 8)
                                | ((value & 0x00ff0000) >> 8)
                                | ((value & 0xff000000) >> 24);
                    }
                }

                break;
        }
    }

    return;
}

static void
internal_convert (void *src_buffer,
                  void *dst_buffer,
                  int  src_pitch,
                  int  dst_pitch,
                  int  width,
                  int  height,
                  int  bpp)
{
    if (endianess == ENDIAN_MSB)
    {
        if (bpp == 15)
        {
            /* Format in fourcc is xrrrrrgg gggbbbbb */
            for (register int y = 0; y < height; y++)
            {
                register uint8 *data =
                    (uint8 *) dst_buffer + y * dst_pitch;

                register uint16 *snes =
                    (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (register int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;
                    *data++ = (pixel & 0x7c00) >> 8
                            | (pixel & 0x0300) >> 8; /* Top 2 green, 5 red */
                    *data++ = (pixel & 0x00c0)
                            | (pixel & 0x001f); /* Top 3 of last 4 green 5 blue */
                }
            }
        }

        else if (bpp == 16)
        {
            /* Format in fourcc is rrrrrggg gggbbbbb */
            for (register int y = 0; y < height; y++)
            {
                register uint8 *data =
                    (uint8 *) dst_buffer + y * dst_pitch;

                register uint16 *snes =
                    (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (register int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;

                    *data++ = (pixel & 0x7c00) >> 7
                            | (pixel & 0x0300) >> 7; /* 5 red, first 3 green */
                    *data++ = (pixel & 0x00c0)
                            | (pixel & 0x001f); /* last 3 green, 5 blue */
                }
            }
        }

        else if (bpp == 24)
        {
            /* Format in fourcc is rrrrrrrr gggggggg bbbbbbbb */
            for (register int y = 0; y < height; y++)
            {
                register uint8 *data =
                    (uint8 *) dst_buffer + y * dst_pitch;

                register uint16 *snes =
                    (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (register int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;

                    *data++ = (pixel & 0x7c00) >> 7; /* Red */
                    *data++ = (pixel & 0x03e0) >> 2; /* Green */
                    *data++ = (pixel & 0x001f) << 3; /* Blue */
                }
            }
        }

        else if (bpp == 32)
        {
            /* Format in fourcc is xxxxxxxx rrrrrrrr gggggggg bbbbbbbb */
            for (register int y = 0; y < height; y++)
            {
                register uint8 *data =
                    (uint8 *) dst_buffer + y * dst_pitch;

                register uint16 *snes =
                    (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (register int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;

                    *data++ = 0xff; /* Null */
                    *data++ = (pixel & 0x7c00) >> 7; /* Red */
                    *data++ = (pixel & 0x03e0) >> 2; /* Green */
                    *data++ = (pixel & 0x001f) << 3; /* Blue */
                }
            }
        }
    }

    else /* Least significant byte first :-P */
    {
        if (bpp == 15)
        {
            /* Format in fourcc is xrrrrrgg gggbbbbb */

            for (register int y = 0; y < height; y++)
            {
                register uint8 *data =
                    (uint8 *) dst_buffer + y * dst_pitch;

                register uint16 *snes =
                    (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (register int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;
                    *data++ = (pixel & 0x00c0)
                            | (pixel & 0x001f); /* Top 3 of last 4 green 5 blue */
                    *data++ = (pixel & 0x7c00) >> 8
                            | (pixel & 0x0300) >> 8; /* Top 2 green, 5 red */
                }
            }
        }

        else if (bpp == 16)
        {
            /* Format in fourcc is rrrrrggg gggbbbbb */
            for (register int y = 0; y < height; y++)
            {
                register uint8 *data =
                    (uint8 *) dst_buffer + y * dst_pitch;

                register uint16 *snes =
                    (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (register int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;

                    *data++ = (pixel & 0x00c0)
                            | (pixel & 0x001f); /* last 3 green, 5 blue */
                    *data++ = (pixel & 0x7c00) >> 7
                            | (pixel & 0x0300) >> 7; /* 5 red, first 3 green */
                }
            }
        }

        else if (bpp == 24)
        {
            /* Format in fourcc is rrrrrrrr gggggggg bbbbbbbb */
            for (register int y = 0; y < height; y++)
            {
                register uint8 *data =
                    (uint8 *) dst_buffer + y * dst_pitch;

                register uint16 *snes =
                    (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (register int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;

                    *data++ = (pixel & 0x001f) << 3; /* Blue */
                    *data++ = (pixel & 0x03e0) >> 2; /* Green */
                    *data++ = (pixel & 0x7c00) >> 7; /* Red */
                }
            }
        }

        else if (bpp == 32)
        {
            /* Format in fourcc is xxxxxxxx rrrrrrrr gggggggg bbbbbbbb */
            for (register int y = 0; y < height; y++)
            {
                register uint8 *data =
                    (uint8 *) dst_buffer + y * dst_pitch;

                register uint16 *snes =
                    (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (register int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;

                    *data++ = (pixel & 0x001f) << 3; /* Blue */
                    *data++ = (pixel & 0x03e0) >> 2; /* Green */
                    *data++ = (pixel & 0x7c00) >> 7; /* Red */
                    *data++ = 0xff; /* Null */
                }
            }
        }
    }

    return;
}

static void
S9xForceHires (void *buffer,
               int  pitch,
               int  &width,
               int  &height)
{
    int double_width  = 0,
        double_height = 0;

    if (width <= 256)
        double_width++;

    /*if (height <= 224)
        double_height++; */

    if (double_width && double_height)
    {
        for (register int y = (height * 2) - 1; y >= 0; y--)
        {
            register uint16 *src_line = (uint16 *) ((uint8 *) buffer + (y >> 1) * pitch);
            register uint16 *dst_line = (uint16 *) ((uint8 *) buffer + y * pitch);

            for (register int x = (width * 2) - 1; x >= 0; x--)
            {
                *(dst_line + x) = *(src_line + (x >> 1));
            }
        }

        width *= 2; height *= 2;
    }
    else if (double_width && !double_height)
    {
        for (register int y = (height) - 1; y >= 0; y--)
        {
            register uint16 *line = (uint16 *) ((uint8 *) buffer + y * pitch);

            for (register int x = (width * 2) - 1; x >= 0; x--)
            {
                *(line + x) = *(line + (x >> 1));
            }
        }

        width *= 2;
    }
    else if (!double_width && double_height)
    {
        for (register int y = (height * 2) - 1; y >= 0; y--)
        {
            register uint16 *src_line = (uint16 *) ((uint8 *) buffer + (y >> 1) * pitch);
            register uint16 *dst_line = (uint16 *) ((uint8 *) buffer + y * pitch);

            memcpy (dst_line, src_line, width * 2);
        }

        height *= 2;
    }

    return;
}

#undef  AVERAGE_1555
#define AVERAGE_1555(el0, el1) (((el0) & (el1)) + ((((el0) ^ (el1)) & 0x7BDE) >> 1))
static void
S9xMergeHires (void *buffer,
               int  pitch,
               int  &width,
               int  &height)
{
    if (width < 512)
    {
        return;
    }

    for (register int y = 0; y < height; y++)
    {
        register uint16 *input = (uint16 *) ((uint8 *) buffer + y * pitch);
        register uint16 *output = input;

        for (register int x = 0; x < (width >> 1); x++)
        {
            *output++ = AVERAGE_1555 (input[0], input[1]);
            input += 2;
        }
    }

    width >>= 1;

    return;
}

#if 0
static void
S9xBlendHires (void *buffer, int pitch, int &width, int &height)
{
    uint16 tmp[512];

    if (width < 512)
    {
        width <<= 1;

        for (int y = 0; y < height; y++)
        {
            uint16 *input = (uint16 *) ((uint8 *) buffer + y * pitch);

            tmp[0] = input[0];
            for (int x = 1; x < width; x++)
                tmp[x] = AVERAGE_1555 (input[(x - 1) >> 1], input[(x >> 1)]);

            memcpy (input, tmp, width << 1);
        }
    }
    else for (int y = 0; y < height; y++)
    {
        uint16 *input = (uint16 *) ((uint8 *) buffer + y * pitch);

        tmp[0] = input[0];
        for (int x = 1; x < width; x++)
            tmp[x] = AVERAGE_1555 (input[x - 1], input[x]);

        memcpy (input, tmp, pitch);
    }

    return;
}
#endif

void
filter_2x (void *src,
           int src_pitch,
           void *dst,
           int dst_pitch,
           int width,
           int height)
{
    int x, y;

    for (y = 0; y < height; y++)
    {
        uint16 *in = (uint16 *) ((uint8 *) src + y * src_pitch);
        uint16 *out1 = (uint16 *) ((uint8 *) dst + (y * 2) * dst_pitch);
        uint16 *out2 = (uint16 *) ((uint8 *) dst + ((y * 2) + 1) * dst_pitch);

        for (x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out1++ = pixel;
            *out1++ = pixel;

            *out2++ = pixel;
            *out2++ = pixel;
        }
    }

    return;
}

void
filter_3x (void *src,
           int src_pitch,
           void *dst,
           int dst_pitch,
           int width,
           int height)
{
    int x, y;

    for (y = 0; y < height; y++)
    {
        uint16 *in = (uint16 *) ((uint8 *) src + y * src_pitch);
        uint16 *out1 = (uint16 *) ((uint8 *) dst + (y * 3) * dst_pitch);
        uint16 *out2 = (uint16 *) ((uint8 *) dst + ((y * 3) + 1) * dst_pitch);
        uint16 *out3 = (uint16 *) ((uint8 *) dst + ((y * 3) + 2) * dst_pitch);

        for (x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out1++ = pixel;
            *out1++ = pixel;
            *out1++ = pixel;

            *out2++ = pixel;
            *out2++ = pixel;
            *out2++ = pixel;

            *out3++ = pixel;
            *out3++ = pixel;
            *out3++ = pixel;
        }
    }

    return;
}

void
filter_4x (void *src,
           int src_pitch,
           void *dst,
           int dst_pitch,
           int width,
           int height)
{
    int x, y;

    for (y = 0; y < height; y++)
    {
        uint16 *in = (uint16 *) ((uint8 *) src + y * src_pitch);
        uint16 *out1 = (uint16 *) ((uint8 *) dst +  (y * 4) * dst_pitch);
        uint16 *out2 = (uint16 *) ((uint8 *) dst + ((y * 4) + 1) * dst_pitch);
        uint16 *out3 = (uint16 *) ((uint8 *) dst + ((y * 4) + 2) * dst_pitch);
        uint16 *out4 = (uint16 *) ((uint8 *) dst + ((y * 4) + 3) * dst_pitch);

        for (x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out1++ = pixel;
            *out1++ = pixel;
            *out1++ = pixel;
            *out1++ = pixel;

            *out2++ = pixel;
            *out2++ = pixel;
            *out2++ = pixel;
            *out2++ = pixel;

            *out3++ = pixel;
            *out3++ = pixel;
            *out3++ = pixel;
            *out3++ = pixel;

            *out4++ = pixel;
            *out4++ = pixel;
            *out4++ = pixel;
            *out4++ = pixel;
        }
    }

    return;
}

void
filter_scanlines (void *src_buffer,
                  int src_pitch,
                  void *dst_buffer,
                  int dst_pitch,
                  int width,
                  int height)
{
    register int x, y;
    register uint16 *src, *dst_a, *dst_b;

    uint8 shift = scanline_shifts[gui_config->scanline_filter_intensity];

    src = (uint16 *) src_buffer;
    dst_a = (uint16 *) dst_buffer;
    dst_b = ((uint16 *) dst_buffer) + (dst_pitch >> 1);

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            register uint8 rs, gs, bs, /* Source components */
                           rh, gh, bh; /* High (bright) components */

            rs = ((src[x] >> 10) & 0x1f);
            gs = ((src[x] >> 5)  & 0x1f);
            bs = ((src[x])       & 0x1f);

            rh = rs + (rs >> shift);
            gh = gs + (gs >> shift);
            bh = bs + (bs >> shift);

            rh = (rh > 31) ? 31 : rh;
            gh = (gh > 31) ? 31 : gh;
            bh = (bh > 31) ? 31 : bh;

            dst_a[x] = (rh << 10) + (gh << 5) + (bh);
            dst_b[x] = ((rs + rs - rh) << 10) +
                       ((gs + gs - gh) << 5)  +
                        (bs + bs - bh);
        }

        src += src_pitch >> 1;
        dst_a += dst_pitch;
        dst_b += dst_pitch;
    }

    return;
}


void
get_filter_scale (int &width, int &height)
{
    switch (gui_config->scale_method)
    {
        case FILTER_SUPEREAGLE:
            width *= 2;
            height *= 2;
            break;

        case FILTER_2XSAI:
            width *= 2;
            height *= 2;
            break;

        case FILTER_SUPER2XSAI:
            width *= 2;
            height *= 2;
            break;

#ifdef USE_HQ2X
        case FILTER_HQ4X:
            if (((width * 4) <= S9xDisplayDriver::scaled_max_width) &&
                ((height * 4) <= S9xDisplayDriver::scaled_max_height))
            {
                width *= 4;
                height *= 4;
                break;
            }

        case FILTER_HQ3X:
            if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
                    height * 3 <= S9xDisplayDriver::scaled_max_height)
            {
                width *= 3;
                height *= 3;
                break;
            }

        case FILTER_HQ2X:
            width *= 2;
            height *= 2;
            break;
#endif /* USE_HQ2X */

#ifdef USE_XBRZ
        case FILTER_4XBRZ:
            if (((width * 4) <= S9xDisplayDriver::scaled_max_width) &&
                ((height * 4) <= S9xDisplayDriver::scaled_max_height))
            {
                width *= 4;
                height *= 4;
                break;
            }

        case FILTER_3XBRZ:
            if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
                    height * 3 <= S9xDisplayDriver::scaled_max_height)
            {
                width *= 3;
                height *= 3;
                break;
            }

        case FILTER_2XBRZ:
            width *= 2;
            height *= 2;
            break;
#endif /* USE_XBRZ */

        case FILTER_SIMPLE4X:
            if (((width * 4) <= S9xDisplayDriver::scaled_max_width) &&
                ((height * 4) <= S9xDisplayDriver::scaled_max_height))
            {
                width *= 4;
                height *= 4;
                break;
            }

        case FILTER_SIMPLE3X:
            if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
                    height * 3 <= S9xDisplayDriver::scaled_max_height)
            {
                width *= 3;
                height *= 3;
                break;
            }

        case FILTER_SIMPLE2X:
            width *= 2;
            height *= 2;
            break;

        case FILTER_EPX:
            width *= 2;
            height *= 2;
            break;

        case FILTER_NTSC:
            if (width > 256)
                width = SNES_NTSC_OUT_WIDTH (width / 2);
            else
                width = SNES_NTSC_OUT_WIDTH (width);
            height *= 2;
            break;

        case FILTER_SCANLINES:
            height *= 2;
            break;

        case FILTER_EPX_SMOOTH:
            width *= 2;
            height *= 2;
            break;
    }

    return;
}

static void
internal_filter (uint8 *src_buffer,
                 int   src_pitch,
                 uint8 *dst_buffer,
                 int   dst_pitch,
                 int   &width,
                 int   &height)
{
    switch (gui_config->scale_method)
    {
        case FILTER_SUPEREAGLE:
            SuperEagle (src_buffer,
                        src_pitch,
                        dst_buffer,
                        dst_pitch,
                        width,
                        height);

            break;

        case FILTER_2XSAI:
            _2xSaI (src_buffer,
                    src_pitch,
                    dst_buffer,
                    dst_pitch,
                    width,
                    height);

            break;

        case FILTER_SUPER2XSAI:
            Super2xSaI (src_buffer,
                        src_pitch,
                        dst_buffer,
                        dst_pitch,
                        width,
                        height);

            break;

#ifdef USE_HQ2X
        case FILTER_HQ4X:

            if (((width * 4) <= S9xDisplayDriver::scaled_max_width) &&
                    ((height * 4) <= S9xDisplayDriver::scaled_max_height))
            {
                HQ4X_16 (src_buffer,
                         src_pitch,
                         dst_buffer,
                         dst_pitch,
                         width,
                         height);

                break;
            }
            /* Fall back to 3x */

        case FILTER_HQ3X:

            if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
                    height * 3 <= S9xDisplayDriver::scaled_max_height)
            {
                HQ3X_16 (src_buffer,
                         src_pitch,
                         dst_buffer,
                         dst_pitch,
                         width,
                         height);

                break;
            }
            /* Fall back to 2x */

        case FILTER_HQ2X:

            HQ2X_16 (src_buffer,
                     src_pitch,
                     dst_buffer,
                     dst_pitch,
                     width,
                     height);

            break;
#endif /* USE_HQ2X */

#ifdef USE_XBRZ
        case FILTER_4XBRZ:

            filter_4xBRZ (src_buffer,
                     src_pitch,
                     dst_buffer,
                     dst_pitch,
                     width,
                     height);

            break;

        case FILTER_3XBRZ:

            filter_3xBRZ (src_buffer,
                     src_pitch,
                     dst_buffer,
                     dst_pitch,
                     width,
                     height);

            break;

        case FILTER_2XBRZ:

            filter_2xBRZ (src_buffer,
                 src_pitch,
                 dst_buffer,
                 dst_pitch,
                 width,
                 height);

            break;
#endif /* USE_XBRZ */


        case FILTER_SIMPLE4X:

            if (((width * 4) <= S9xDisplayDriver::scaled_max_width) &&
                    ((height * 4) <= S9xDisplayDriver::scaled_max_height))
            {
                filter_4x (src_buffer,
                           src_pitch,
                           dst_buffer,
                           dst_pitch,
                           width,
                           height);

                break;
            }

        case FILTER_SIMPLE3X:

            if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
                    height * 3 <= S9xDisplayDriver::scaled_max_height)
            {
                filter_3x (src_buffer,
                          src_pitch,
                          dst_buffer,
                          dst_pitch,
                          width,
                          height);

                break;
            }

        case FILTER_SIMPLE2X:

            filter_2x (src_buffer,
                       src_pitch,
                       dst_buffer,
                       dst_pitch,
                       width,
                       height);

            break;

        case FILTER_EPX:

            EPX_16_unsafe (src_buffer,
                           src_pitch,
                           dst_buffer,
                           dst_pitch,
                           width,
                           height);

            break;

        case FILTER_EPX_SMOOTH:

            EPX_16_smooth_unsafe (src_buffer,
                                  src_pitch,
                                  dst_buffer,
                                  dst_pitch,
                                  width,
                                  height);

            break;

        case FILTER_NTSC:
            if (width > 256)
                snes_ntsc_blit_hires (&snes_ntsc,
                                      (SNES_NTSC_IN_T *) src_buffer,
                                      src_pitch >> 1,
                                      0, /* Burst_phase */
                                      width,
                                      height,
                                      (void *) dst_buffer,
                                      dst_pitch);
            else
                snes_ntsc_blit (&snes_ntsc,
                                (SNES_NTSC_IN_T *) src_buffer,
                                src_pitch >> 1,
                                0, /* Burst_phase */
                                width,
                                height,
                                (void *) dst_buffer,
                                dst_pitch);
            break;

        case FILTER_SCANLINES:
            filter_scanlines (src_buffer,
                              src_pitch,
                              dst_buffer,
                              dst_pitch,
                              width,
                              height);
            break;

    }

    get_filter_scale (width, height);

    return;
}

static void
thread_worker (gpointer data,
               gpointer user_data)
{
    thread_job_t *job = ((thread_job_t *) data);

    switch (job->operation_type)
    {
        case JOB_FILTER:
            internal_filter (job->src_buffer,
                             job->src_pitch,
                             job->dst_buffer,
                             job->dst_pitch,
                             job->width,
                             job->height);
            break;

        case JOB_CONVERT:
            internal_convert (job->src_buffer,
                              job->dst_buffer,
                              job->src_pitch,
                              job->dst_pitch,
                              job->width,
                              job->height,
                              job->bpp);
            break;

        case JOB_CONVERT_YUV:
            internal_convert_16_yuv (job->src_buffer,
                                     job->dst_buffer,
                                     job->src_pitch,
                                     job->dst_pitch,
                                     job->width,
                                     job->height);
            break;

        case JOB_CONVERT_MASK:
            internal_convert_mask (job->src_buffer,
                                   job->dst_buffer,
                                   job->src_pitch,
                                   job->dst_pitch,
                                   job->width,
                                   job->height,
                                   job->inv_rmask,
                                   job->inv_bmask,
                                   job->inv_gmask,
                                   job->bpp);
            break;
    }

    job->complete = 1;

    return;
}

static void
create_thread_pool (void)
{
    if (pool == NULL)
    {
        pool = g_thread_pool_new (thread_worker,
                                  NULL,
                                  gui_config->num_threads - 1,
                                  TRUE,
                                  NULL);
    }

    return;
}

static void
internal_threaded_convert (void *src_buffer,
                           void *dst_buffer,
                           int  src_pitch,
                           int  dst_pitch,
                           int  width,
                           int  height,
                           int  bpp)
{
    int i, flag;

    create_thread_pool ();

    for (i = 0; i < gui_config->num_threads - 1; i++)
    {
        job[i].operation_type = (bpp == -1 ? JOB_CONVERT_YUV : JOB_CONVERT);
        job[i].src_buffer =
            ((uint8 *) src_buffer) + (src_pitch * i * (height / gui_config->num_threads));
        job[i].src_pitch = src_pitch;
        job[i].dst_buffer =
            ((uint8 *) dst_buffer) + (dst_pitch * i * (height / gui_config->num_threads));
        job[i].dst_pitch = dst_pitch;
        job[i].width = width;
        job[i].height = height / gui_config->num_threads;
        job[i].bpp = bpp;
        job[i].complete = 0;

        g_thread_pool_push (pool, (gpointer) &(job[i]), NULL);
    }

    job[i].operation_type = (bpp == -1 ? JOB_CONVERT_YUV : JOB_CONVERT);
    job[i].src_buffer =
        ((uint8 *) src_buffer) + (src_pitch * i * (height / gui_config->num_threads));
    job[i].src_pitch = src_pitch;
    job[i].dst_buffer =
        ((uint8 *) dst_buffer) + (dst_pitch * i * (height / gui_config->num_threads));
    job[i].dst_pitch = dst_pitch;
    job[i].width = width;
    job[i].height = height - ((gui_config->num_threads - 1) * (height / gui_config->num_threads));
    job[i].bpp = bpp;

    thread_worker ((gpointer) &(job[i]), NULL);

    while (1)
    {
        flag = 1;
        for (i = 0; i < gui_config->num_threads - 1; i++)
            flag = flag && job[i].complete;

        if (flag)
            break;

        sched_yield ();
    }

    return;
}

static void
internal_threaded_convert_mask (void *src_buffer,
                                void *dst_buffer,
                                int  src_pitch,
                                int  dst_pitch,
                                int  width,
                                int  height,
                                int  inv_rmask,
                                int  inv_gmask,
                                int  inv_bmask,
                                int  bpp)
{
    int i, flag;

    create_thread_pool ();

    for (i = 0; i < gui_config->num_threads - 1; i++)
    {
        job[i].operation_type = (bpp == -1 ? JOB_CONVERT_YUV : JOB_CONVERT);
        job[i].src_buffer =
            ((uint8 *) src_buffer) + (src_pitch * i * (height / gui_config->num_threads));
        job[i].src_pitch = src_pitch;
        job[i].dst_buffer =
            ((uint8 *) dst_buffer) + (dst_pitch * i * (height / gui_config->num_threads));
        job[i].dst_pitch = dst_pitch;
        job[i].width = width;
        job[i].height = height / gui_config->num_threads;
        job[i].bpp = bpp;
        job[i].inv_rmask = inv_rmask;
        job[i].inv_gmask = inv_gmask;
        job[i].inv_bmask = inv_bmask;
        job[i].complete = 0;

        g_thread_pool_push (pool, (gpointer) &(job[i]), NULL);
    }

    job[i].operation_type = (bpp == -1 ? JOB_CONVERT_YUV : JOB_CONVERT);
    job[i].src_buffer =
        ((uint8 *) src_buffer) + (src_pitch * i * (height / gui_config->num_threads));
    job[i].src_pitch = src_pitch;
    job[i].dst_buffer =
        ((uint8 *) dst_buffer) + (dst_pitch * i * (height / gui_config->num_threads));
    job[i].dst_pitch = dst_pitch;
    job[i].width = width;
    job[i].height = height - ((gui_config->num_threads - 1) * (height / gui_config->num_threads));
    job[i].bpp = bpp;
    job[i].inv_rmask = inv_rmask;
    job[i].inv_gmask = inv_gmask;
    job[i].inv_bmask = inv_bmask;

    thread_worker ((gpointer) &(job[i]), NULL);

    while (1)
    {
        flag = 1;
        for (i = 0; i < gui_config->num_threads - 1; i++)
            flag = flag && job[i].complete;

        if (flag)
            break;

        sched_yield ();
    }

    return;
}

static void
internal_threaded_filter (uint8 *src_buffer,
                          int   src_pitch,
                          uint8 *dst_buffer,
                          int   dst_pitch,
                          int   &width,
                          int   &height)
{
    int i, flag;
    int dwidth = width, dheight = height;
    int src_coverage = 0, dst_coverage = 0;
    int height_scale;

    get_filter_scale (dwidth, dheight);
    height_scale = dheight / height;

    /* If the threadpool doesn't exist, create it */
    create_thread_pool ();

    for (i = 0; i < gui_config->num_threads - 1; i++)
    {
        job[i].operation_type = JOB_FILTER;
        job[i].complete = 0;
        job[i].width = width;
        job[i].src_pitch = src_pitch;
        job[i].dst_pitch = dst_pitch;
        job[i].src_buffer = src_buffer + (src_pitch * src_coverage);
        job[i].dst_buffer = dst_buffer + (dst_pitch * dst_coverage);

        job[i].height = (height / gui_config->num_threads) & ~3; /* Cut to multiple of 4 */
        src_coverage += job[i].height;
        dst_coverage += job[i].height * height_scale;


        g_thread_pool_push (pool, (gpointer) &(job[i]), NULL);
    }

    job[i].operation_type = JOB_FILTER;
    job[i].width = width;
    job[i].src_pitch = src_pitch;
    job[i].dst_pitch = dst_pitch;
    job[i].src_buffer = src_buffer + (src_pitch * src_coverage);
    job[i].dst_buffer = dst_buffer + (dst_pitch * dst_coverage);

    job[i].height = height - src_coverage;

    thread_worker ((gpointer) &(job[i]), NULL);

    while (1)
    {
        flag = 1;
        for (i = 0; i < gui_config->num_threads - 1; i++)
            flag = flag && job[i].complete;

        if (flag)
            break;

        sched_yield ();
    }

    get_filter_scale (width, height);

    return;
}

void
S9xFilter (uint8 *src_buffer,
           int   src_pitch,
           uint8 *dst_buffer,
           int   dst_pitch,
           int   &width,
           int   &height)
{

    if (gui_config->multithreading)
        internal_threaded_filter (src_buffer,
                                  src_pitch,
                                  dst_buffer,
                                  dst_pitch,
                                  width,
                                  height);
    else
        internal_filter (src_buffer,
                         src_pitch,
                         dst_buffer,
                         dst_pitch,
                         width,
                         height);

    return;
}

void
S9xConvertYUV (void *src_buffer,
               void *dst_buffer,
               int  src_pitch,
               int  dst_pitch,
               int  width,
               int  height)
{
    if (gui_config->multithreading)
        internal_threaded_convert (src_buffer,
                                   dst_buffer,
                                   src_pitch,
                                   dst_pitch,
                                   width,
                                   height,
                                   -1);
    else
        internal_convert_16_yuv (src_buffer,
                                 dst_buffer,
                                 src_pitch,
                                 dst_pitch,
                                 width,
                                 height);
    return;
}

void
S9xConvert (void *src,
            void *dst,
            int  src_pitch,
            int  dst_pitch,
            int  width,
            int  height,
            int  bpp)
{
    if (gui_config->multithreading)
        internal_threaded_convert (src,
                                   dst,
                                   src_pitch,
                                   dst_pitch,
                                   width,
                                   height,
                                   bpp);
    else
        internal_convert (src,
                          dst,
                          src_pitch,
                          dst_pitch,
                          width,
                          height,
                          bpp);
    return;
}

void
S9xConvertMask (void *src,
                void *dst,
                int  src_pitch,
                int  dst_pitch,
                int  width,
                int  height,
                int  rshift,
                int  gshift,
                int  bshift,
                int  bpp)
{
    if (gui_config->multithreading)
        internal_threaded_convert_mask (src,
                                        dst,
                                        src_pitch,
                                        dst_pitch,
                                        width,
                                        height,
                                        rshift,
                                        gshift,
                                        bshift,
                                        bpp);
    else
        internal_convert_mask (src,
                               dst,
                               src_pitch,
                               dst_pitch,
                               width,
                               height,
                               rshift,
                               gshift,
                               bshift,
                               bpp);
    return;
}

void
S9xDisplayRefresh (int width, int height)
{
    driver->refresh (width, height);

    return;
}

static void
ntsc_filter_init (void)
{
    scanline_offset = scanline_offsets [gui_config->ntsc_scanline_intensity];
    scanline_mask   = scanline_masks [gui_config->ntsc_scanline_intensity];

    snes_ntsc_init (&snes_ntsc, &gui_config->ntsc_setup);

    return;
}

void
S9xDisplayReconfigure (void)
{
    ntsc_filter_init ();

    if (pool)
    {
        g_thread_pool_set_max_threads (pool, gui_config->num_threads - 1, NULL);
    }

    return;
}

void
S9xQueryDrivers (void)
{
#ifdef USE_XV
    gui_config->allow_xv = S9xXVDisplayDriver::query_availability ();
#else
    gui_config->allow_xv = 0;
#endif

#ifdef USE_OPENGL
    gui_config->allow_opengl = S9xOpenGLDisplayDriver::query_availability ();
#else
    gui_config->allow_opengl = 0;
#endif

#ifdef USE_XRANDR
    int error_base_p, event_base_p;
    Display *display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    gui_config->allow_xrandr = 1;

    if (!XRRQueryExtension (display, &event_base_p, &error_base_p))
    {
        gui_config->allow_xrandr = 0;
        gui_config->change_display_resolution = FALSE;
    }

    if (gui_config->allow_xrandr)
    {
        gui_config->xrr_config = XRRGetScreenInfo (display,
                                                   DefaultRootWindow (display));
        gui_config->xrr_original_size =
            XRRConfigCurrentConfiguration (gui_config->xrr_config,
                                           &(gui_config->xrr_rotation));
        gui_config->xrr_sizes = XRRConfigSizes (gui_config->xrr_config,
                                            &(gui_config->xrr_num_sizes));
    }
#else
    gui_config->allow_xrandr = 0;
#endif

    return;
}

bool8
S9xDeinitUpdate (int width, int height)
{
    if (!Settings.Paused
#ifdef NETPLAY_SUPPORT
            && !NetPlay.Paused
#endif
    )

    {
        if (gui_config->overscan)
            height = (height > SNES_HEIGHT_EXTENDED) ?
                         SNES_HEIGHT_EXTENDED * 2 :
                         SNES_HEIGHT_EXTENDED;
        else
            if (height > SNES_HEIGHT_EXTENDED)
                height = 448;
            else
                height = 224;

        if (gui_config->hires_effect == HIRES_SCALE)
        {
            S9xForceHires (GFX.Screen,
                           S9xDisplayDriver::image_width *
                               S9xDisplayDriver::image_bpp,
                           width,
                           height);
        }
        else if (gui_config->hires_effect == HIRES_MERGE)
        {
            S9xMergeHires (GFX.Screen,
                           S9xDisplayDriver::image_width *
                               S9xDisplayDriver::image_bpp,
                           width,
                           height);
        }

        GFX.Screen = driver->get_next_buffer ();
    }

    top_level->last_width = width;
    top_level->last_height = height;

    driver->update (width, height);

    return TRUE;
}

static void
S9xInitDriver (void)
{
    switch (gui_config->hw_accel)
    {
#ifdef USE_OPENGL
        case HWA_OPENGL:

            driver = new S9xOpenGLDisplayDriver (top_level,
                                                 gui_config);

            break;
#endif
#ifdef USE_XV
        case HWA_XV:

            driver = new S9xXVDisplayDriver (top_level, gui_config);

            break;
#endif
        default:

            driver = new S9xGTKDisplayDriver (top_level, gui_config);
    }

    if (driver->init ())
    {
        if (gui_config->hw_accel > 0)
        {
            driver->deinit ();
            delete driver;

            gui_config->hw_accel = HWA_NONE;

            S9xInitDriver ();
        }
        else
        {
            fprintf (stderr, "Error: Couldn't initialize any display output.\n");
            exit (1);
        }
    }

    pool = NULL;

    return;
}

void
S9xInitDisplay (int argc, char **argv)
{
    Settings.SupportHiRes = TRUE;
    S9xSetRenderPixelFormat (RGB555);
    S9xBlit2xSaIFilterInit ();
#ifdef USE_HQ2X
    S9xBlitHQ2xFilterInit ();
#endif /* USE_HQ2SX */
    S9xQueryDrivers ();
    S9xInitDriver ();
    S9xGraphicsInit ();
    S9xDisplayReconfigure ();

    return;
}

void
S9xDisplayClearBuffers (void)
{
    driver->clear_buffers ();

    return;
}

void
S9xDeinitDisplay (void)
{
    driver->deinit ();
    delete driver;

    driver = NULL;
    GFX.Screen = NULL;

    if (pool)
        g_thread_pool_free (pool, FALSE, TRUE);

    return;
}

void
S9xReinitDisplay (void)
{
    uint16 *buffer = NULL;
    int    width, height;

    buffer = (uint16 *) malloc (S9xDisplayDriver::image_size);
    memmove (buffer,
             driver->get_current_buffer (),
             S9xDisplayDriver::image_size);

    width = top_level->last_width;
    height = top_level->last_height;

    S9xDeinitDisplay ();
    S9xInitDriver ();
    S9xDisplayReconfigure ();

    top_level->last_width = width;
    top_level->last_height = height;

    driver->push_buffer (buffer);

    free (buffer);

    return;
}

bool8
S9xContinueUpdate (int width, int height)
{
    return TRUE;
}

bool8
S9xInitUpdate (void)
{
    return TRUE;
}

void
S9xSetPalette (void)
{
    return;
}

void
S9xTextMode (void)
{
    return;
}

void
S9xGraphicsMode (void)
{
    return;
}


