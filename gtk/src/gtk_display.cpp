/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_2_3_compat.h"
#include <sched.h>

#include "gtk_s9x.h"
#include "gtk_display.h"
#include "gtk_display_driver.h"
#include "gtk_display_driver_gtk.h"

#if defined(USE_XV) && defined(GDK_WINDOWING_X11)
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
static int               endianess = ENDIAN_NORMAL;

/* Scanline constants for the NTSC filter */
static const unsigned int scanline_offsets[] =
{
        0,     /* 0%    */
        3,     /* 12.5% */
        2,     /* 25%   */
        1,     /* 50%   */
        0,     /* 100%  */
};

static const unsigned short scanline_masks[] =
{
        0x0000,     /* 0%    */
        0x18E3,     /* 12.5% */
        0x39E7,     /* 25%   */
        0x7BEF,     /* 50%   */
        0xffff,     /* 100%  */
};

/* Scanline constants for standard scanline filter */
static const uint8 scanline_shifts[] =
{
        3,
        2,
        1,
        0
};

void S9xSetEndianess (int type)
{
    endianess = type;
}

double S9xGetAspect ()
{
    double native_aspect = 256.0 / (gui_config->overscan ? 239.0 : 224.0);
    double aspect;

    switch (gui_config->aspect_ratio / 2)
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

void S9xApplyAspect (int &s_width,  /* Output: x */
                     int &s_height, /* Output: y */
                     int &d_width,  /* Output: width */
                     int &d_height) /* Output: height */
{
    double screen_aspect = (double) d_width / (double) d_height;
    double snes_aspect = S9xGetAspect ();
    bool   integer = gui_config->aspect_ratio & 1;
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
    else if (gui_config->maintain_aspect_ratio && integer)
    {
        for (h = s_height * 2; h <= d_height && (int)(h * (snes_aspect) + 0.5) <= d_width; h += s_height) {}
        h -= s_height;
        w = h * snes_aspect + 0.5;
        x = (d_width  - w) / 2;
        y = (d_height - h) / 2;

        if (w > d_width || h > d_height)
        {
            w = d_width;
            h = d_height;
            x = 0;
            y = 0;
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
}

void S9xRegisterYUVTables (uint8 *y, uint8 *u, uint8 *v)
{
    y_table = y;
    u_table = u;
    v_table = v;
}

/* YUY2 in LSB first format */
static void internal_convert_16_yuv (void *src_buffer,
                                     void *dst_buffer,
                                     int  src_pitch,
                                     int  dst_pitch,
                                     int  width,
                                     int  height)
{
    int    x, y;
    uint16 *src;
    uint8  *dst;
    uint16 p0, p1;

    if (endianess == ENDIAN_SWAPPED)
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
}

static void internal_convert_mask (void         *src_buffer,
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
    if (endianess == ENDIAN_SWAPPED)
#else
    if (endianess == ENDIAN_NORMAL)
#endif
    {
        switch (bpp)
        {
        case 15:
        case 16:
            for (int y = 0; y < height; y++)
            {
                uint16 *data = (uint16 *) ((uint8 *) dst_buffer + y * dst_pitch);
                uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;

                    *data++ = ((pixel & 0xf800) << 0)  >> inv_rshift
                            | ((pixel & 0x07e0) << 5)  >> inv_gshift
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
                for (int y = 0; y < height; y++)
                {
                    uint8 *data = (uint8 *) dst_buffer + y * dst_pitch;
                    uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                    for (int x = 0; x < width; x++)
                    {
                        uint32 pixel = *snes++;
                        *data++ = ((pixel & 0x001f) << 3);
                        *data++ = ((pixel & 0x07e0) >> 3);
                        *data++ = ((pixel & 0xf800) >> 8);
                    }
                }
            }
            else
            {
                for (int y = 0; y < height; y++)
                {
                    uint8 *data = (uint8 *) dst_buffer + y * dst_pitch;

                    uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                    for (int x = 0; x < width; x++)
                    {
                        uint32 pixel = *snes++;
                        *data++ = ((pixel & 0xf800) >> 8);
                        *data++ = ((pixel & 0x07e0) >> 3);
                        *data++ = ((pixel & 0x001f) << 3);
                    }
                }
            }
            break;

        case 32:
            for (int y = 0; y < height; y++)
            {
                uint32 *data = (uint32 *) ((uint8 *) dst_buffer + y * dst_pitch);
                uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;

                    *data++ = ((uint32) (pixel & 0xf800) << 16) >> inv_rshift
                            | ((uint32) (pixel & 0x07e0) << 21) >> inv_gshift
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
            for (int y = 0; y < height; y++)
            {
                uint16 *data = (uint16 *) ((uint8 *) dst_buffer + y * dst_pitch);
                uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;
                    uint16 value;

                    value   = ((pixel & 0xf800) << 0) >> inv_rshift
                            | ((pixel & 0x07e0) << 5) >> inv_gshift
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
                for (int y = 0; y < height; y++)
                {
                    uint8 *data = (uint8 *) dst_buffer + y * dst_pitch;
                    uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                    for (int x = 0; x < width; x++)
                    {
                        uint32 pixel = *snes++;
                        *data++ = ((pixel & 0xf800) >> 8);
                        *data++ = ((pixel & 0x07e0) >> 3);
                        *data++ = ((pixel & 0x001f) << 3);
                    }
                }
            }
            else
            {
                for (int y = 0; y < height; y++)
                {
                    uint8 *data = (uint8 *) dst_buffer + y * dst_pitch;
                    uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                    for (int x = 0; x < width; x++)
                    {
                        uint32 pixel = *snes++;
                        *data++ = ((pixel & 0x001f) << 3);
                        *data++ = ((pixel & 0x07e0) >> 3);
                        *data++ = ((pixel & 0xf800) >> 8);
                    }
                }
            }
            break;

        case 32:
            for (int y = 0; y < height; y++)
            {
                uint32 *data = (uint32 *) ((uint8 *) dst_buffer + y * dst_pitch);
                uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (int x = 0; x < width; x++)
                {
                    uint32 pixel = *snes++;
                    uint32 value;

                    value   = ((uint32) (pixel & 0xf800) << 16) >> inv_rshift
                            | ((uint32) (pixel & 0x07e0) << 21) >> inv_gshift
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
}

static void internal_convert (void *src_buffer,
                              void *dst_buffer,
                              int  src_pitch,
                              int  dst_pitch,
                              int  width,
                              int  height,
                              int  bpp)
{
    if (endianess == ENDIAN_SWAPPED)
    {
        if (bpp == 24)
        {
            for (int y = 0; y < height; y++)
            {
                uint8 *data = (uint8 *) dst_buffer + y * dst_pitch;

                uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (int x = 0; x < width; x++)
                {
                    uint16 pixel = *snes++;
                    *data++ = ((pixel & 0xf800) >> 8) | ((pixel >> 13) & 0x07); /* Red */
                    *data++ = ((pixel & 0x07e0) >> 3) | ((pixel >> 9)  & 0x03); /* Green */
                    *data++ = ((pixel & 0x001f) << 3) | ((pixel >> 2)  & 0x07); /* Blue */
                }
            }
        }

        else if (bpp == 32)
        {
            /* Format in fourcc is xxxxxxxx rrrrrrrr gggggggg bbbbbbbb */
            for (int y = 0; y < height; y++)
            {
                uint8 *data = (uint8 *) dst_buffer + y * dst_pitch;

                uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (int x = 0; x < width; x++)
                {
                    uint16 pixel = *snes++;
                    *data++ = 0xff; /* Null */
                    *data++ = ((pixel & 0xf800) >> 8) | ((pixel >> 13) & 0x07); /* Red */
                    *data++ = ((pixel & 0x07e0) >> 3) | ((pixel >> 9)  & 0x03); /* Green */
                    *data++ = ((pixel & 0x001f) << 3) | ((pixel >> 2)  & 0x07); /* Blue */
                }
            }
        }
    }

    else /* Least significant byte first :-P */
    {
        if (bpp == 24)
        {
            /* Format in fourcc is rrrrrrrr gggggggg bbbbbbbb */
            /* Format in fourcc is xxxxxxxx rrrrrrrr gggggggg bbbbbbbb */
            for (int y = 0; y < height; y++)
            {
                uint8 *data = (uint8 *) dst_buffer + y * dst_pitch;

                uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (int x = 0; x < width; x++)
                {
                    uint16 pixel = *snes++;

                    *data++ = ((pixel & 0x001f) << 3) | ((pixel >> 2)  & 0x07); /* Blue */
                    *data++ = ((pixel & 0x07e0) >> 3) | ((pixel >> 9)  & 0x03); /* Green */
                    *data++ = ((pixel & 0xf800) >> 8) | ((pixel >> 13) & 0x07); /* Red */
                }
            }
        }

        else if (bpp == 32)
        {
            /* Format in fourcc is xxxxxxxx rrrrrrrr gggggggg bbbbbbbb */
            for (int y = 0; y < height; y++)
            {
                uint8 *data = (uint8 *) dst_buffer + y * dst_pitch;

                uint16 *snes = (uint16 *) (((uint8 *) src_buffer) + y * src_pitch);

                for (int x = 0; x < width; x++)
                {
                    uint16 pixel = *snes++;

                    *data++ = ((pixel & 0x001f) << 3) | ((pixel >> 2)  & 0x07); /* Blue */
                    *data++ = ((pixel & 0x07e0) >> 3) | ((pixel >> 9)  & 0x03); /* Green */
                    *data++ = ((pixel & 0xf800) >> 8) | ((pixel >> 13) & 0x07); /* Red */
                    *data++ = 0xff; /* Null */
                }
            }
        }
    }
}

static void S9xForceHires (void *buffer,
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
        for (int y = (height * 2) - 1; y >= 0; y--)
        {
            uint16 *src_line = (uint16 *) ((uint8 *) buffer + (y >> 1) * pitch);
            uint16 *dst_line = (uint16 *) ((uint8 *) buffer + y * pitch);

            for (int x = (width * 2) - 1; x >= 0; x--)
            {
                *(dst_line + x) = *(src_line + (x >> 1));
            }
        }

        width *= 2; height *= 2;
    }
    else if (double_width && !double_height)
    {
        for (int y = (height) - 1; y >= 0; y--)
        {
            uint16 *line = (uint16 *) ((uint8 *) buffer + y * pitch);

            for (int x = (width * 2) - 1; x >= 0; x--)
            {
                *(line + x) = *(line + (x >> 1));
            }
        }

        width *= 2;
    }
    else if (!double_width && double_height)
    {
        for (int y = (height * 2) - 1; y >= 0; y--)
        {
            uint16 *src_line = (uint16 *) ((uint8 *) buffer + (y >> 1) * pitch);
            uint16 *dst_line = (uint16 *) ((uint8 *) buffer + y * pitch);

            memcpy (dst_line, src_line, width * 2);
        }

        height *= 2;
    }
}

static inline uint16 average_565(uint16 colora, uint16 colorb)
{
    return (((colora) & (colorb)) + ((((colora) ^ (colorb)) & 0xF7DE) >> 1));
}

static void S9xMergeHires (void *buffer,
                           int  pitch,
                           int  &width,
                           int  &height)
{
    if (width < 512)
    {
        return;
    }

    for (int y = 0; y < height; y++)
    {
        uint16 *input = (uint16 *) ((uint8 *) buffer + y * pitch);
        uint16 *output = input;

        for (int x = 0; x < (width >> 1); x++)
        {
            *output++ = average_565 (input[0], input[1]);
            input += 2;
        }
    }

    width >>= 1;
}

void filter_2x (void *src,
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
        uint16 *out = (uint16 *) ((uint8 *) dst + (y * 2) * dst_pitch);

        for (x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out++ = pixel;
            *out++ = pixel;
        }

        memcpy ((uint8 *) dst + (y * 2 + 1) * dst_pitch,
                (uint8 *) dst + (y * 2) * dst_pitch,
                width * 2 * 2);
    }
}

void filter_3x (void *src,
                int src_pitch,
                void *dst,
                int dst_pitch,
                int width,
                int height)
{
    int x, y, z;

    for (y = 0; y < height; y++)
    {
        uint16 *in = (uint16 *) ((uint8 *) src + y * src_pitch);
        uint16 *out = (uint16 *) ((uint8 *) dst + (y * 3) * dst_pitch);

        for (x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out++ = pixel;
            *out++ = pixel;
            *out++ = pixel;
        }

        for (z = 1; z <= 2; z++)
        {
            memcpy ((uint8 *) dst + ((y * 3) + z) * dst_pitch,
                    (uint8 *) dst + ((y * 3)) * dst_pitch,
                    width * 2 * 3);
        }
    }
}

void filter_4x (void *src,
                int src_pitch,
                void *dst,
                int dst_pitch,
                int width,
                int height)
{
    int x, y, z;

    for (y = 0; y < height; y++)
    {
        uint16 *in = (uint16 *) ((uint8 *) src + y * src_pitch);
        uint16 *out = (uint16 *) ((uint8 *) dst +  (y * 4) * dst_pitch);

        for (x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out++ = pixel;
            *out++ = pixel;
            *out++ = pixel;
            *out++ = pixel;
        }

        for (z = 1; z <= 3; z++)
        {
            memcpy ((uint8 *) dst + ((y * 4) + z) * dst_pitch,
                    (uint8 *) dst +  (y * 4) * dst_pitch,
                    width * 2 * 4);
        }
    }
}

void filter_scanlines (void *src_buffer,
                       int src_pitch,
                       void *dst_buffer,
                       int dst_pitch,
                       int width,
                       int height)
{
    int x, y;
    uint16 *src, *dst_a, *dst_b;

    uint8 shift = scanline_shifts[gui_config->scanline_filter_intensity];
    uint16 mask = scanline_masks[gui_config->scanline_filter_intensity + 1];

    src = (uint16 *) src_buffer;
    dst_a = (uint16 *) dst_buffer;
    dst_b = ((uint16 *) dst_buffer) + (dst_pitch >> 1);

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            dst_a[x] = src[x];
            dst_b[x] = (src[x] - (src[x] >> shift & mask));
        }

        src += src_pitch >> 1;
        dst_a += dst_pitch;
        dst_b += dst_pitch;
    }
}

void get_filter_scale (int &width, int &height)
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
        // Fall through
    case FILTER_HQ3X:
        if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
            height * 3 <= S9xDisplayDriver::scaled_max_height)
        {
            width *= 3;
            height *= 3;
            break;
        }
        // Fall through
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
        // Fall through
    case FILTER_3XBRZ:
        if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
            height * 3 <= S9xDisplayDriver::scaled_max_height)
        {
            width *= 3;
            height *= 3;
            break;
        }
        // Fall through
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
        // Fall through
    case FILTER_SIMPLE3X:
        if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
            height * 3 <= S9xDisplayDriver::scaled_max_height)
        {
            width *= 3;
            height *= 3;
            break;
        }
        // Fall through
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
}

static void internal_filter (uint8 *src_buffer,
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
        // Fall through
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
        // Fall through
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
        if (width * 4 <= S9xDisplayDriver::scaled_max_width &&
            height * 4 <= S9xDisplayDriver::scaled_max_height)
        {
            filter_4xBRZ (src_buffer,
                          src_pitch,
                          dst_buffer,
                          dst_pitch,
                          width,
                          height);
            break;
        }
        // Fall through
    case FILTER_3XBRZ:
        if (width * 3 <= S9xDisplayDriver::scaled_max_width &&
            height * 3 <= S9xDisplayDriver::scaled_max_height)
        {
            filter_3xBRZ (src_buffer,
                          src_pitch,
                          dst_buffer,
                          dst_pitch,
                          width,
                          height);
            break;
        }
        // Fall through
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
        // Fall through
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
        // Fall through
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
            snes_ntsc_blit_hires_scanlines (&snes_ntsc,
                                            (SNES_NTSC_IN_T *) src_buffer,
                                            src_pitch >> 1,
                                            0, /* Burst_phase */
                                            width,
                                            height,
                                            (void *) dst_buffer,
                                            dst_pitch);
        else
            snes_ntsc_blit_scanlines (&snes_ntsc,
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
}

static void thread_worker (gpointer data,
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

    job->complete = true;
}

static void
create_thread_pool ()
{
    if (pool == NULL)
    {
        pool = g_thread_pool_new (thread_worker,
                                  NULL,
                                  gui_config->num_threads - 1,
                                  true,
                                  NULL);
    }
}

static void wait_for_jobs_to_complete ()
{
    while (1)
    {
        int complete = 1;
        for (int i = 0; i < gui_config->num_threads; i++)
            complete = complete && job[i].complete;

        if (complete)
            break;

        sched_yield ();
    }
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
    create_thread_pool ();

    for (int i = 0; i < gui_config->num_threads; i++)
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
        job[i].complete = false;

        if (i == gui_config->num_threads - 1)
            job[i].height = height - ((gui_config->num_threads - 1) * (height / gui_config->num_threads));

        g_thread_pool_push (pool, (gpointer) &(job[i]), NULL);
    }

    wait_for_jobs_to_complete ();
}

static void internal_threaded_convert_mask (void *src_buffer,
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
    create_thread_pool ();

    for (int i = 0; i < gui_config->num_threads; i++)
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
        job[i].complete = false;

        if (i == gui_config->num_threads - 1)
            job[i].height = height - ((gui_config->num_threads - 1) * (height / gui_config->num_threads));

        g_thread_pool_push (pool, (gpointer) &(job[i]), NULL);
    }

    wait_for_jobs_to_complete ();
}

static void internal_threaded_filter (uint8 *src_buffer,
                                      int   src_pitch,
                                      uint8 *dst_buffer,
                                      int   dst_pitch,
                                      int   &width,
                                      int   &height)
{
    int dst_width = width, dst_height = height;

    /* If the threadpool doesn't exist, create it */
    create_thread_pool ();

    get_filter_scale (dst_width, dst_height);

    int yscale = dst_height / height;
    int coverage = 0;

    for (int i = 0; i < gui_config->num_threads; i++)
    {
        job[i].operation_type = JOB_FILTER;
        job[i].complete = false;
        job[i].width = width;
        job[i].src_pitch = src_pitch;
        job[i].dst_pitch = dst_pitch;
        job[i].src_buffer = src_buffer + (src_pitch * coverage);
        job[i].dst_buffer = dst_buffer + (dst_pitch * coverage * yscale);
        job[i].height = (height / gui_config->num_threads) & ~3; /* Cut to multiple of 4 */
        if (i == gui_config->num_threads - 1)
            job[i].height = height - coverage;
        coverage += job[i].height;

        g_thread_pool_push (pool, (gpointer) &(job[i]), NULL);
    }

    wait_for_jobs_to_complete ();

    get_filter_scale (width, height);
}

void S9xFilter (uint8 *src_buffer,
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
}

void S9xConvertYUV (void *src_buffer,
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
}

void S9xConvert (void *src,
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
}

void S9xConvertMask (void *src,
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
}

void S9xDisplayRefresh (int width, int height)
{
    driver->refresh (width, height);
}

static void ntsc_filter_init ()
{
    snes_ntsc_scanline_offset = scanline_offsets [gui_config->ntsc_scanline_intensity];
    snes_ntsc_scanline_mask   = scanline_masks [gui_config->ntsc_scanline_intensity];

    snes_ntsc_init (&snes_ntsc, &gui_config->ntsc_setup);
}

void S9xDisplayReconfigure ()
{
    ntsc_filter_init ();

    if (pool)
    {
        g_thread_pool_set_max_threads (pool, gui_config->num_threads - 1, NULL);
    }
}

void S9xQueryDrivers ()
{
    GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (top_level->get_window()));

    gui_config->allow_xv = false;
#if defined(USE_XV) && defined(GDK_WINDOWING_X11)
    if (GDK_IS_X11_DISPLAY (display))
        gui_config->allow_xv = S9xXVDisplayDriver::query_availability ();
#endif

#ifdef USE_OPENGL
    gui_config->allow_opengl = S9xOpenGLDisplayDriver::query_availability ();
#else
    gui_config->allow_opengl = false;
#endif

    gui_config->allow_xrandr = false;
#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_DISPLAY (display))
    {
        Display *dpy = gdk_x11_display_get_xdisplay (gtk_widget_get_display (GTK_WIDGET (top_level->get_window())));
        Window xid   = gdk_x11_window_get_xid (gtk_widget_get_window (GTK_WIDGET (top_level->get_window())));

        gui_config->allow_xrandr = true;
        gui_config->xrr_screen_resources = XRRGetScreenResourcesCurrent (dpy, xid);
        gui_config->xrr_crtc_info        = XRRGetCrtcInfo (dpy,
                                                        gui_config->xrr_screen_resources,
                                                        gui_config->xrr_screen_resources->crtcs[0]);
    }
#endif
}

bool8 S9xDeinitUpdate (int width, int height)
{
    int yoffset = 0;

    if (top_level->last_height > height)
    {
        memset (GFX.Screen + (GFX.Pitch >> 1) * height,
                0,
                GFX.Pitch * (top_level->last_height - height));
    }

    top_level->last_height = height;
    top_level->last_width = width;

    if (gui_config->overscan)
    {
        if (height == SNES_HEIGHT)
        {
            yoffset = -8;
            height = SNES_HEIGHT_EXTENDED;
        }
        if (height == SNES_HEIGHT * 2)
        {
            yoffset = -16;
            height = SNES_HEIGHT_EXTENDED * 2;
        }
    }
    else
    {
        if (height == SNES_HEIGHT_EXTENDED)
        {
            yoffset = 7;
            height = SNES_HEIGHT;
        }
        if (height == SNES_HEIGHT_EXTENDED * 2)
        {
            yoffset = 14;
            height = SNES_HEIGHT * 2;
        }
    }

    if (!Settings.Paused && !NetPlay.Paused
    )

    {
        if (gui_config->hires_effect == HIRES_SCALE)
        {
            S9xForceHires (GFX.Screen,
                           S9xDisplayDriver::image_width *
                               S9xDisplayDriver::image_bpp,
                           width,
                           height);
            top_level->last_width = width;
        }
        else if (gui_config->hires_effect == HIRES_MERGE)
        {
            S9xMergeHires (GFX.Screen,
                           S9xDisplayDriver::image_width *
                               S9xDisplayDriver::image_bpp,
                           width,
                           height);
            top_level->last_width = width;
        }

        GFX.Screen = driver->get_next_buffer ();
    }

    driver->update (width, height, yoffset);

    return true;
}

static void S9xInitDriver ()
{
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_DISPLAY (gdk_display_get_default ()))
    {
        gui_config->hw_accel = HWA_OPENGL;
    }
#endif
    switch (gui_config->hw_accel)
    {
#ifdef USE_OPENGL
        case HWA_OPENGL:

            driver = new S9xOpenGLDisplayDriver (top_level,
                                                 gui_config);

            break;
#endif
#if defined(USE_XV) && defined(GDK_WINDOWING_X11)
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
}

S9xDisplayDriver *S9xDisplayGetDriver ()
{
    return driver;
}

void S9xDisplayClearBuffers ()
{
    driver->clear_buffers ();
}

void S9xDeinitDisplay ()
{
    driver->deinit ();
    delete driver;

    driver = NULL;
    GFX.Screen = NULL;

    if (pool)
        g_thread_pool_free (pool, false, true);
}

void S9xReinitDisplay ()
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
}

bool8 S9xContinueUpdate (int width, int height)
{
    S9xDeinitUpdate (width, height);
    return true;
}

bool8 S9xInitUpdate ()
{
    return true;
}

void S9xSetPalette ()
{
}

void S9xTextMode ()
{
}

void S9xGraphicsMode ()
{
}

#include "var8x10font.h"
static const int font_width = 8;
static const int font_height = 10;

static inline int CharWidth (uint8 c)
{
    return font_width - var8x10font_kern[c - 32][0] - var8x10font_kern[c - 32][1];
}

static int StringWidth (const char *str)
{
    int length = strlen (str);
    int pixcount = 0;

    if (length > 0)
        pixcount++;

    for (int i = 0; i < length; i++)
    {
        pixcount += (CharWidth (str[i]) - 1);
    }

    return pixcount;
}

static void GTKDisplayChar(int x, int y, uint8 c, bool monospace = false, int overlap = 0)
{
    int cindex = c - 32;
    int crow   = cindex >> 4;
    int ccol   = cindex & 15;
    int cwidth = font_width - (monospace ? 0 : (var8x10font_kern[cindex][0] + var8x10font_kern[cindex][1]));

    int	line   = crow * font_height;
    int	offset = ccol * font_width + (monospace ? 0 : var8x10font_kern[cindex][0]);
    int scale = IPPU.RenderedScreenWidth / SNES_WIDTH;

    uint16 *s = GFX.Screen + y * GFX.RealPPL + x * scale;

    for (int h = 0; h < font_height; h++, line++, s += GFX.RealPPL - cwidth * scale)
    {
        for (int w = 0; w < cwidth; w++, s++)
        {
            if (var8x10font[line][offset + w] == '#')
                *s = Settings.DisplayColor;
            else if (var8x10font[line][offset + w] == '.')
                *s = 0x0000;
//            else if (!monospace && w >= overlap)
//                *s = (*s & 0xf7de) >> 1;
//                *s = (*s & 0xe79c) >> 2;

            if (scale > 1)
            {
                s[1] = s[0];
                s++;
            }
        }
    }
}

static void S9xGTKDisplayString(const char *string, int linesFromBottom,
                                int pixelsFromLeft, bool allowWrap, int type)
{
    bool monospace = true;
    if (type == S9X_NO_INFO)
    {
        if (linesFromBottom <= 0)
            linesFromBottom = 1;

        if (linesFromBottom >= 5)
            linesFromBottom -= 3;

        if (pixelsFromLeft > 128)
            pixelsFromLeft = SNES_WIDTH - StringWidth (string);

        monospace = false;
    }

    int dst_x = pixelsFromLeft;
    int dst_y = IPPU.RenderedScreenHeight - (font_height) * linesFromBottom;
    int	len = strlen(string);

    if (IPPU.RenderedScreenHeight % 224 && !gui_config->overscan)
        dst_y -= 8;
    else if (gui_config->overscan)
        dst_y += 8;

    int overlap = 0;

    for (int i = 0 ; i < len ; i++)
    {
        int cindex = string[i] - 32;
        int char_width = font_width - (monospace ? 1 : (var8x10font_kern[cindex][0] + var8x10font_kern[cindex][1]));

        if (dst_x + char_width > SNES_WIDTH || (uint8)string[i] < 32)
        {
            if (!allowWrap)
                break;

            linesFromBottom--;
            dst_y = IPPU.RenderedScreenHeight - font_height * linesFromBottom;
            dst_x = pixelsFromLeft;

            if (dst_y >= IPPU.RenderedScreenHeight)
                break;
        }

        if ((uint8) string[i] < 32)
            continue;

        GTKDisplayChar(dst_x, dst_y, string[i], monospace, overlap);

        dst_x  += char_width - 1;
        overlap = 1;
    }
}

void S9xInitDisplay (int argc, char **argv)
{
    Settings.SupportHiRes = true;
    S9xBlit2xSaIFilterInit ();
#ifdef USE_HQ2X
    S9xBlitHQ2xFilterInit ();
#endif /* USE_HQ2SX */
    S9xQueryDrivers ();
    S9xInitDriver ();
    S9xGraphicsInit ();
    S9xDisplayReconfigure ();
    S9xCustomDisplayString = S9xGTKDisplayString;
}

bool S9xDisplayDriverIsReady ()
{
    if (!driver)
        return false;
    return driver->is_ready ();
}
