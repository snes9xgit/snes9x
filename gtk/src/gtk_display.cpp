/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_compat.h"
#include "threadpool.h"

#include "gtk_s9x.h"
#include "gtk_display.h"
#include "gtk_display_driver.h"
#include "gtk_display_driver_gtk.h"

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "ppu.h"
#include "gfx.h"
#include "netplay.h"
#include "controls.h"
#include "movie.h"

#if defined(USE_XV) && defined(GDK_WINDOWING_X11)
#include "gtk_display_driver_xv.h"
#endif

#include "gtk_display_driver_opengl.h"
#ifdef USE_SLANG
#include "gtk_display_driver_vulkan.h"
#endif

#include "snes9x_imgui.h"

void filter_scanlines(uint8 *, int, uint8 *, int, int, int);
void filter_2x(uint8 *, int, uint8 *, int, int, int);
void filter_3x(uint8 *, int, uint8 *, int, int, int);
void filter_4x(uint8 *, int, uint8 *, int, int, int);

static struct
{
    int index;
    void (*filter_func)(uint8 *, int, uint8 *, int, int, int);
    int xscale;
    int yscale;
} filter_data[NUM_FILTERS] = {
    { FILTER_NONE,       nullptr,               1, 1 },
    { FILTER_SUPEREAGLE, SuperEagle,            2, 2 },
    { FILTER_2XSAI,      _2xSaI,                2, 2 },
    { FILTER_SUPER2XSAI, Super2xSaI,            2, 2 },
    { FILTER_EPX,        EPX_16_unsafe,         2, 2 },
    { FILTER_EPX_SMOOTH, EPX_16_smooth_unsafe,  2, 2 },
    { FILTER_NTSC,       nullptr,               1, 1 },
    { FILTER_SCANLINES,  filter_scanlines,      1, 2 },
    { FILTER_SIMPLE2X,   filter_2x,             2, 2 },
    { FILTER_SIMPLE3X,   filter_3x,             3, 3 },
    { FILTER_SIMPLE4X,   filter_4x,             4, 4 },
    { FILTER_HQ2X,       HQ2X_16,               2, 2 },
    { FILTER_HQ3X,       HQ3X_16,               3, 3 },
    { FILTER_HQ4X,       HQ4X_16,               4, 4 },
    { FILTER_2XBRZ,      filter_2xBRZ,          2, 2 },
    { FILTER_3XBRZ,      filter_3xBRZ,          3, 3 },
    { FILTER_4XBRZ,      filter_4xBRZ,          4, 4 },
};

static S9xDisplayDriver *driver;
static snes_ntsc_t snes_ntsc;
static int burst_phase = 0;
static uint8 *y_table, *u_table, *v_table;
static std::vector<uint8_t> scaled_image;
static std::unique_ptr<threadpool> pool;

/* Scanline constants for the NTSC filter */
static const unsigned int scanline_offsets[] = {
    0, /* 0%    */
    3, /* 12.5% */
    2, /* 25%   */
    1, /* 50%   */
    0, /* 100%  */
};

static const unsigned short scanline_masks[] = {
    0x0000, /* 0%    */
    0x18E3, /* 12.5% */
    0x39E7, /* 25%   */
    0x7BEF, /* 50%   */
    0xffff, /* 100%  */
};

/* Scanline constants for standard scanline filter */
static const uint8 scanline_shifts[] = {
    3,
    2,
    1,
    0
};

double S9xGetAspect()
{
    double native_aspect = 256.0 / (gui_config->overscan ? 239.0 : 224.0);
    double aspect;

    switch (gui_config->aspect_ratio)
    {
    case 0: /* Square pixels */
    case 1:
        aspect = native_aspect;
        break;

    case 2: /* 4:3 */
    case 3:
        aspect = native_aspect * 7 / 6;
        break;

    case 4: /* NTSC 64:49 */
    case 5:
    default:
        aspect = native_aspect * 8 / 7;
        break;
    }

    return aspect;
}

S9xRect S9xApplyAspect(int src_width,
                       int src_height,
                       int dst_width,
                       int dst_height)
{
    double screen_aspect = (double)dst_width / (double)dst_height;
    double snes_aspect = S9xGetAspect();
    bool integer = gui_config->aspect_ratio & 1;
    double granularity = 1.0 / (double)MAX(dst_width, dst_height);
    int x, y, w, h;

    if (!gui_config->scale_to_fit)
    {
        if (gui_config->maintain_aspect_ratio)
        {
            w = src_height * snes_aspect + 0.5;
            h = src_height;
            x = (dst_width - w) / 2;
            y = (dst_height - src_height) / 2;
        }
        else
        {
            w = src_width;
            h = src_height;
            x = (dst_width - w) / 2;
            y = (dst_height - h) / 2;
        }
    }
    else if (gui_config->maintain_aspect_ratio && integer)
    {
        for (h = src_height * 2; h <= dst_height && (int)(h * (snes_aspect) + 0.5) <= dst_width; h += src_height)
        {
        }
        h -= src_height;
        w = h * snes_aspect + 0.5;
        x = (dst_width - w) / 2;
        y = (dst_height - h) / 2;

        if (w > dst_width || h > dst_height)
        {
            w = dst_width;
            h = dst_height;
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
            w = dst_height * snes_aspect + 0.5;
            h = dst_height;
            x = (dst_width - w) / 2;
            y = 0;
        }
        else
        {
            w = dst_width;
            h = dst_width / snes_aspect + 0.5;
            x = 0;
            y = (dst_height - h) / 2;
        }
    }

    else
    {
        x = 0;
        y = 0;
        w = dst_width;
        h = dst_height;
    }

    return { x, y, w, h };
}

void S9xRegisterYUVTables(uint8 *y, uint8 *u, uint8 *v)
{
    y_table = y;
    u_table = u;
    v_table = v;
}

/* YUY2 in LSB first format */
static void internal_convert_16_yuv(void *src_buffer,
                                    void *dst_buffer,
                                    int src_pitch,
                                    int dst_pitch,
                                    int width,
                                    int height,
                                    int bpp)
{
    int x, y;
    uint16 *src;
    uint8 *dst;
    uint16 p0, p1;

    for (y = 0; y < height; y++)
    {
        src = (uint16 *)(((uint8 *)src_buffer) + src_pitch * y);
        dst = (uint8 *)(((uint8 *)dst_buffer) + dst_pitch * y);

        for (x = 0; x < width / 2; x++)
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

static void internal_convert_mask(void *src_buffer,
                                  void *dst_buffer,
                                  int src_pitch,
                                  int dst_pitch,
                                  int width,
                                  int height,
                                  unsigned int inv_rshift,
                                  unsigned int inv_gshift,
                                  unsigned int inv_bshift,
                                  unsigned int bpp)
{
    switch (bpp)
    {
    case 15:
    case 16:
        for (int y = 0; y < height; y++)
        {
            uint16 *data = (uint16 *)((uint8 *)dst_buffer + y * dst_pitch);
            uint16 *snes = (uint16 *)(((uint8 *)src_buffer) + y * src_pitch);

            for (int x = 0; x < width; x++)
            {
                uint32 pixel = *snes++;

                *data++ = ((pixel & 0xf800) << 0) >> inv_rshift | ((pixel & 0x07e0) << 5) >> inv_gshift | ((pixel & 0x001f) << 11) >> inv_bshift;
            }
        }
        break;

    case 32:
        for (int y = 0; y < height; y++)
        {
            uint32 *data = (uint32 *)((uint8 *)dst_buffer + y * dst_pitch);
            uint16 *snes = (uint16 *)(((uint8 *)src_buffer) + y * src_pitch);

            for (int x = 0; x < width; x++)
            {
                uint32 pixel = *snes++;

                *data++ = ((uint32)(pixel & 0xf800) << 16) >> inv_rshift | ((uint32)(pixel & 0x07e0) << 21) >> inv_gshift | ((uint32)(pixel & 0x001f) << 27) >> inv_bshift;
            }
        }
        break;
    }
}

static void internal_convert(void *src_buffer,
                             void *dst_buffer,
                             int src_pitch,
                             int dst_pitch,
                             int width,
                             int height,
                             int bpp)
{

    if (bpp == 32)
    {
        /* Format in fourcc is xxxxxxxx rrrrrrrr gggggggg bbbbbbbb */
        for (int y = 0; y < height; y++)
        {
            uint8 *data = (uint8 *)dst_buffer + y * dst_pitch;

            uint16 *snes = (uint16 *)(((uint8 *)src_buffer) + y * src_pitch);

            for (int x = 0; x < width; x++)
            {
                uint16 pixel = *snes++;

                *data++ = ((pixel & 0x001f) << 3) | ((pixel >> 2) & 0x07);  /* Blue */
                *data++ = ((pixel & 0x07e0) >> 3) | ((pixel >> 9) & 0x03);  /* Green */
                *data++ = ((pixel & 0xf800) >> 8) | ((pixel >> 13) & 0x07); /* Red */
                *data++ = 0xff;                                             /* Null */
            }
        }
    }
}

static void S9xForceHires(void *buffer, int pitch, int &width, int &height)
{
    if (width <= 256)
    {
        for (int y = (height)-1; y >= 0; y--)
        {
            uint16 *line = (uint16 *)((uint8 *)buffer + y * pitch);

            for (int x = (width * 2) - 1; x >= 0; x--)
            {
                *(line + x) = *(line + (x >> 1));
            }
        }

        width *= 2;
    }
}

static inline uint16 average_565(uint16 colora, uint16 colorb)
{
    return (((colora) & (colorb)) + ((((colora) ^ (colorb)) & 0xF7DE) >> 1));
}

static void S9xMergeHires(void *buffer, int pitch, int &width, int &height)
{
    if (width < 512)
        return;

    for (int y = 0; y < height; y++)
    {
        uint16 *input = (uint16 *)((uint8 *)buffer + y * pitch);
        uint16 *output = input;

        for (int x = 0; x < (width >> 1); x++)
        {
            *output++ = average_565(input[0], input[1]);
            input += 2;
        }
    }

    width >>= 1;
}

void filter_2x(uint8 *src,
               int src_pitch,
               uint8 *dst,
               int dst_pitch,
               int width,
               int height)
{
    for (int y = 0; y < height; y++)
    {
        uint16 *in = (uint16 *)((uint8 *)src + y * src_pitch);
        uint16 *out = (uint16 *)((uint8 *)dst + (y * 2) * dst_pitch);

        for (int x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out++ = pixel;
            *out++ = pixel;
        }

        memcpy((uint8 *)dst + (y * 2 + 1) * dst_pitch,
               (uint8 *)dst + (y * 2) * dst_pitch,
               width * 2 * 2);
    }
}

void filter_3x(uint8 *src,
               int src_pitch,
               uint8 *dst,
               int dst_pitch,
               int width,
               int height)
{
    for (int y = 0; y < height; y++)
    {
        uint16 *in = (uint16 *)((uint8 *)src + y * src_pitch);
        uint16 *out = (uint16 *)((uint8 *)dst + (y * 3) * dst_pitch);

        for (int x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out++ = pixel;
            *out++ = pixel;
            *out++ = pixel;
        }

        for (int line = 1; line <= 2; line++)
        {
            memcpy((uint8 *)dst + ((y * 3) + line) * dst_pitch,
                   (uint8 *)dst + ((y * 3)) * dst_pitch,
                   width * 2 * 3);
        }
    }
}

void filter_4x(uint8 *src,
               int src_pitch,
               uint8 *dst,
               int dst_pitch,
               int width,
               int height)
{
    for (int y = 0; y < height; y++)
    {
        uint16 *in = (uint16 *)((uint8 *)src + y * src_pitch);
        uint16 *out = (uint16 *)((uint8 *)dst + (y * 4) * dst_pitch);

        for (int x = 0; x < width; x++)
        {
            uint16 pixel = *in++;

            *out++ = pixel;
            *out++ = pixel;
            *out++ = pixel;
            *out++ = pixel;
        }

        for (int line = 1; line <= 3; line++)
        {
            memcpy((uint8 *)dst + ((y * 4) + line) * dst_pitch,
                   (uint8 *)dst + (y * 4) * dst_pitch,
                   width * 2 * 4);
        }
    }
}

void filter_scanlines(uint8 *src_buffer,
                      int src_pitch,
                      uint8 *dst_buffer,
                      int dst_pitch,
                      int width,
                      int height)
{
    uint8 shift = scanline_shifts[gui_config->scanline_filter_intensity];
    uint16 mask = scanline_masks[gui_config->scanline_filter_intensity + 1];

    uint16 *src = (uint16 *)src_buffer;
    uint16 *dst_a = (uint16 *)dst_buffer;
    uint16 *dst_b = ((uint16 *)dst_buffer) + (dst_pitch >> 1);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            dst_a[x] = src[x];
            dst_b[x] = (src[x] - (src[x] >> shift & mask));
        }

        src += src_pitch >> 1;
        dst_a += dst_pitch;
        dst_b += dst_pitch;
    }
}

void apply_filter_scale(int &width, int &height)
{
    if (gui_config->scale_method == FILTER_NTSC)
    {
        width = SNES_NTSC_OUT_WIDTH(256);
        height *= 2;
        return;
    }

    const auto &filter = filter_data[gui_config->scale_method];
    width *= filter.xscale;
    height *= filter.yscale;
    return;
}

static void internal_filter(uint8 *src_buffer,
                            int src_pitch,
                            uint8 *dst_buffer,
                            int dst_pitch,
                            int width,
                            int height)
{
    if (gui_config->scale_method == FILTER_NONE)
    {
        return;
    }

    if (gui_config->scale_method == FILTER_NTSC)
    {
        if (width > 256)
            snes_ntsc_blit_hires_scanlines(&snes_ntsc,
                                           (SNES_NTSC_IN_T *)src_buffer,
                                           src_pitch >> 1,
                                           burst_phase,
                                           width,
                                           height,
                                           (void *)dst_buffer,
                                           dst_pitch);
        else
            snes_ntsc_blit_scanlines(&snes_ntsc,
                                     (SNES_NTSC_IN_T *)src_buffer,
                                     src_pitch >> 1,
                                     burst_phase,
                                     width,
                                     height,
                                     (void *)dst_buffer,
                                     dst_pitch);
    }
    else
    {
        filter_data[gui_config->scale_method].filter_func(src_buffer,
                                                          src_pitch,
                                                          dst_buffer,
                                                          dst_pitch,
                                                          width,
                                                          height);
    }
}

static void create_thread_pool()
{
    if (!pool)
    {
        pool = std::make_unique<threadpool>();
        pool->start(gui_config->num_threads);
    }
}

static void internal_threaded_convert(void *src_buffer,
                          void *dst_buffer,
                          int src_pitch,
                          int dst_pitch,
                          int width,
                          int height,
                          int bpp)
{
    create_thread_pool();

    auto func = (bpp == -1) ? internal_convert_16_yuv : internal_convert;
    int coverage = 0;

    for (int i = 0; i < gui_config->num_threads; i++)
    {
        int job_height = (height / gui_config->num_threads) & 3;
            job_height = height - coverage;

        pool->queue([=] {
            func((uint8 *)src_buffer + (src_pitch * coverage),
                 (uint8 *)dst_buffer + (dst_pitch * coverage),
                 src_pitch,
                 dst_pitch,
                 width,
                 job_height,
                 bpp);
        });
    }

    pool->wait_idle();
}

static void internal_threaded_convert_mask(void *src_buffer,
                                           void *dst_buffer,
                                           int src_pitch,
                                           int dst_pitch,
                                           int width,
                                           int height,
                                           int inv_rmask,
                                           int inv_gmask,
                                           int inv_bmask,
                                           int bpp)
{
    create_thread_pool();

    int lines_handled = 0;
    int job_height = height / gui_config->num_threads;

    for (int i = 0; i < gui_config->num_threads; i++)
    {
        if (i == gui_config->num_threads - 1)
            job_height = height - lines_handled;
        pool->queue([=] {
            internal_convert_mask((uint8 *)src_buffer + (src_pitch * lines_handled),
                                  (uint8 *)dst_buffer + (dst_pitch * lines_handled),
                                  src_pitch,
                                  dst_pitch,
                                  width,
                                  job_height,
                                  inv_rmask,
                                  inv_gmask,
                                  inv_bmask,
                                  bpp);
        });
    }

    pool->wait_idle();
}

static void internal_threaded_filter(uint8 *src_buffer,
                                     int src_pitch,
                                     uint8 *dst_buffer,
                                     int dst_pitch,
                                     int width,
                                     int height)
{
    // NTSC filter has internal state, so it can't be threaded properly.
    if (gui_config->scale_method == FILTER_NTSC)
        return internal_filter(src_buffer, src_pitch, dst_buffer, dst_pitch, width, height);

    /* If the threadpool doesn't exist, create it */
    create_thread_pool();

    int yscale = filter_data[gui_config->scale_method].yscale;
    int coverage = 0;

    for (int i = 0; i < gui_config->num_threads; i++)
    {
        int job_height = height / gui_config->num_threads & ~3;
        if (i == gui_config->num_threads - 1)
            job_height = height - coverage;

        pool->queue([=] {
            internal_filter(src_buffer + (src_pitch * coverage),
                            src_pitch,
                            dst_buffer + (dst_pitch * coverage * yscale),
                            dst_pitch,
                            width,
                            job_height);
        });
        coverage += job_height;
    }

    pool->wait_idle();
}

void S9xFilter(uint8 *src_buffer,
               int src_pitch,
               uint8 *dst_buffer,
               int dst_pitch,
               int &width,
               int &height)
{
    burst_phase = (burst_phase + 1) % 3;
    if (gui_config->multithreading)
        internal_threaded_filter(src_buffer,
                                 src_pitch,
                                 dst_buffer,
                                 dst_pitch,
                                 width,
                                 height);
    else
        internal_filter(src_buffer,
                        src_pitch,
                        dst_buffer,
                        dst_pitch,
                        width,
                        height);

    apply_filter_scale(width, height);
}

void S9xConvertYUV(void *src_buffer,
                   void *dst_buffer,
                   int src_pitch,
                   int dst_pitch,
                   int width,
                   int height)
{
    if (gui_config->multithreading)
        internal_threaded_convert(src_buffer,
                                  dst_buffer,
                                  src_pitch,
                                  dst_pitch,
                                  width,
                                  height,
                                  -1);
    else
        internal_convert_16_yuv(src_buffer,
                                dst_buffer,
                                src_pitch,
                                dst_pitch,
                                width,
                                height,
                                -1);
}

void S9xConvert(void *src,
                void *dst,
                int src_pitch,
                int dst_pitch,
                int width,
                int height,
                int bpp)
{
    if (gui_config->multithreading)
        internal_threaded_convert(src,
                                  dst,
                                  src_pitch,
                                  dst_pitch,
                                  width,
                                  height,
                                  bpp);
    else
        internal_convert(src,
                         dst,
                         src_pitch,
                         dst_pitch,
                         width,
                         height,
                         bpp);
}

void S9xConvertMask(void *src,
                    void *dst,
                    int src_pitch,
                    int dst_pitch,
                    int width,
                    int height,
                    int rshift,
                    int gshift,
                    int bshift,
                    int bpp)
{
    if (gui_config->multithreading)
        internal_threaded_convert_mask(src,
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
        internal_convert_mask(src,
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

void S9xDisplayRefresh()
{
    driver->refresh();
}

static void ntsc_filter_init()
{
    snes_ntsc_scanline_offset = scanline_offsets[gui_config->ntsc_scanline_intensity];
    snes_ntsc_scanline_mask = scanline_masks[gui_config->ntsc_scanline_intensity];

    snes_ntsc_init(&snes_ntsc, &gui_config->ntsc_setup);
}

void S9xDisplayReconfigure()
{
    ntsc_filter_init();

    if (pool)
    {
        pool->stop();
        pool = nullptr;
    }
}

void S9xQueryDrivers()
{
    GdkDisplay *gdk_display = top_level->window->get_display()->gobj();

    gui_config->allow_xv = false;
#if defined(USE_XV) && defined(GDK_WINDOWING_X11)
    if (GDK_IS_X11_DISPLAY(gdk_display))
        gui_config->allow_xv = S9xXVDisplayDriver::query_availability();
#endif

    gui_config->allow_opengl = S9xOpenGLDisplayDriver::query_availability();

    gui_config->allow_xrandr = false;
#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_DISPLAY(gdk_display))
    {
        Display *dpy = gdk_x11_display_get_xdisplay(gdk_display);
        Window xid = gdk_x11_window_get_xid(top_level->window->get_window()->gobj());

        gui_config->allow_xrandr = true;
        if (!gui_config->xrr_screen_resources)
            gui_config->xrr_screen_resources = XRRGetScreenResourcesCurrent(dpy, xid);
        if (!gui_config->xrr_crtc_info)
            gui_config->xrr_crtc_info = XRRGetCrtcInfo(dpy,
                                                       gui_config->xrr_screen_resources,
                                                       gui_config->xrr_screen_resources->crtcs[0]);
    }
#endif

    auto &dd = gui_config->display_drivers;
    dd.clear();
    dd.push_back("none");
    if (gui_config->allow_opengl)
        dd.push_back("opengl");
    if (gui_config->allow_xv)
        dd.push_back("xv");
#ifdef USE_SLANG
    dd.push_back("vulkan");
#endif
}

bool8 S9xDeinitUpdate(int width, int height)
{
    int yoffset = 0;

    if (width <= 0 || height <= 0)
        return false;

    if (top_level->last_height > height)
    {
        memset(GFX.Screen + GFX.RealPPL * height,
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

    uint16_t *screen_view = GFX.Screen + (yoffset * (int)GFX.RealPPL);

    if (!Settings.Paused && !NetPlay.Paused)

    {
        if (gui_config->hires_effect == HIRES_SCALE)
        {
            S9xForceHires(screen_view, GFX.Pitch, width, height);
            top_level->last_width = width;
        }
        else if (gui_config->hires_effect == HIRES_MERGE)
        {
            S9xMergeHires(screen_view, GFX.Pitch, width, height);
            top_level->last_width = width;
        }
    }

    if (gui_config->scale_method > 0)
    {
        int scaled_width = width;
        int scaled_height = height;

        apply_filter_scale(scaled_width, scaled_height);

        if ((int)scaled_image.size() < (scaled_width * scaled_height * 2))
        {
            scaled_image.resize(scaled_width * scaled_height * 2);
        }

        S9xFilter((uint8_t *)screen_view, GFX.Pitch, &scaled_image[0], scaled_width * 2, width, height);
        driver->update((uint16_t *)&scaled_image[0], width, height, scaled_width);

        return true;
    }

    driver->update(screen_view, width, height, GFX.RealPPL);

    return true;
}

static void S9xInitDriver()
{
    // Only OpenGL is supported on Wayland
#ifdef GDK_WINDOWING_WAYLAND
    if (is_wayland())
    {
        if (gui_config->display_driver != "vulkan")
            gui_config->display_driver = "opengl";
    }
#endif

    if ("opengl" == gui_config->display_driver)
    {
        driver = new S9xOpenGLDisplayDriver(top_level, gui_config);
    }
#ifdef USE_SLANG
    else if ("vulkan" == gui_config->display_driver)
    {
        driver = new S9xVulkanDisplayDriver(top_level, gui_config);
    }
#endif
#if defined(USE_XV) && defined(GDK_WINDOWING_X11)
    else if ("xv" == gui_config->display_driver)
    {
        driver = new S9xXVDisplayDriver(top_level, gui_config);
    }
#endif
    else
    {
        driver = new S9xGTKDisplayDriver(top_level, gui_config);
    }

    if (driver->init())
    {
        delete driver;

        driver = new S9xGTKDisplayDriver(top_level, gui_config);
        driver->init();
        gui_config->display_driver = "none";

        Gtk::MessageDialog dialog("Couldn't load display driver. Using default.");
        dialog.run();
    }

    pool = NULL;
}

S9xDisplayDriver *S9xDisplayGetDriver()
{
    return driver;
}

void S9xDeinitDisplay()
{
    if (driver)
        driver->deinit();
    delete driver;
    driver = nullptr;

    if (pool)
    {
        pool->stop();
        pool = nullptr;
    }
}

void S9xReinitDisplay()
{
    uint16_t *buffer = NULL;
    int width, height;

    buffer = new uint16_t[512 * 512];
    memmove(buffer, GFX.Screen, 512 * 478 * 2);

    width = top_level->last_width;
    height = top_level->last_height;

    S9xDeinitDisplay();
    S9xInitDriver();
    S9xDisplayReconfigure();

    top_level->last_width = width;
    top_level->last_height = height;

    memmove(GFX.Screen, buffer, 512 * 478 * 2);

    delete[] buffer;
}

bool8 S9xContinueUpdate(int width, int height)
{
    S9xDeinitUpdate(width, height);
    return true;
}

bool8 S9xInitUpdate()
{
    return true;
}

void S9xTextMode()
{
}

void S9xGraphicsMode()
{
}

void S9xInitDisplay(int argc, char **argv)
{
    S9xBlit2xSaIFilterInit();
#ifdef USE_HQ2X
    S9xBlitHQ2xFilterInit();
#endif /* USE_HQ2SX */
    S9xQueryDrivers();
    S9xInitDriver();
    S9xGraphicsInit();
    S9xDisplayReconfigure();

    S9xCustomDisplayString = [](const char *str, int linesFromBottom, int pixelsFromLeft, bool allowWrap, int type) {
        if (!S9xImGuiRunning())
            S9xVariableDisplayString(str, linesFromBottom, pixelsFromLeft, allowWrap, type);
    };
}

bool S9xDisplayDriverIsReady()
{
    if (!driver)
        return false;
    return driver->is_ready();
}
