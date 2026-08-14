#include "SoftwareFilters.hpp"
#include "snes9x.h"
#include "filter/2xsai.h"
#include "filter/epx.h"
#include "filter/filter_epx_unsafe.h"
#include "filter/hq2x.h"
#include "filter/xbrz.h"
#include "filter/snes_ntsc.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <vector>

// The output-image-processing filters from the win32/gtk ports. The frame is
// RGB565, pitches are in bytes, and the source is GFX.Screen, whose 32 rows of
// padding above and below (gfx.cpp) the EPX Smooth filter relies on.

enum
{
    FILTER_NONE = 0,
    FILTER_SUPEREAGLE,
    FILTER_2XSAI,
    FILTER_SUPER2XSAI,
    FILTER_EPX,
    FILTER_EPX_SMOOTH,
    FILTER_NTSC_COMPOSITE,
    FILTER_NTSC_SVIDEO,
    FILTER_NTSC_RGB,
    FILTER_NTSC_MONOCHROME,
    FILTER_SCANLINES,
    FILTER_SIMPLE2X,
    FILTER_SIMPLE3X,
    FILTER_SIMPLE4X,
    FILTER_HQ2X,
    FILTER_HQ3X,
    FILTER_HQ4X,
    FILTER_2XBRZ,
    FILTER_3XBRZ,
    FILTER_4XBRZ,
    NUM_FILTERS
};

static void filter_scanlines(uint8_t *src, int src_pitch, uint8_t *dst, int dst_pitch, int width, int height)
{
    const uint8_t shift = 1;      // darken the in-between lines by 50%
    const uint16_t mask = 0x7bef;

    for (int y = 0; y < height; y++)
    {
        auto *in = (uint16_t *)(src + y * src_pitch);
        auto *out_a = (uint16_t *)(dst + (y * 2) * dst_pitch);
        auto *out_b = (uint16_t *)(dst + (y * 2 + 1) * dst_pitch);

        for (int x = 0; x < width; x++)
        {
            out_a[x] = in[x];
            out_b[x] = in[x] - (in[x] >> shift & mask);
        }
    }
}

template <int scale>
static void filter_simple(uint8_t *src, int src_pitch, uint8_t *dst, int dst_pitch, int width, int height)
{
    for (int y = 0; y < height; y++)
    {
        auto *in = (uint16_t *)(src + y * src_pitch);
        auto *out = (uint16_t *)(dst + (y * scale) * dst_pitch);

        for (int x = 0; x < width; x++)
            for (int i = 0; i < scale; i++)
                *out++ = in[x];

        for (int line = 1; line < scale; line++)
            memcpy(dst + (y * scale + line) * dst_pitch,
                   dst + (y * scale) * dst_pitch,
                   width * scale * 2);
    }
}

#define CONVERT_16_TO_32(pixel) \
    (((((pixel) >> 11)       ) << 19) | \
     ((((pixel) >> 5) & 0x3f ) << 10) | \
      (((pixel)       & 0x1f ) << 3))

#define CONVERT_32_TO_16(pixel) \
    (((((pixel) & 0xf80000) >> 8) | \
      (((pixel) & 0x00fc00) >> 5) | \
      (((pixel) & 0x0000f8) >> 3)) & 0xffff)

static void filter_xbrz(int factor, uint8_t *src, int src_pitch, uint8_t *dst, int dst_pitch, int width, int height)
{
    static std::vector<uint32_t> in32;
    static std::vector<uint32_t> out32;

    in32.resize(width * height);
    out32.resize(in32.size() * factor * factor);

    for (int y = 0; y < height; y++)
    {
        auto *in = (uint16_t *)(src + y * src_pitch);
        uint32_t *out = &in32[y * width];
        for (int x = 0; x < width; x++)
            out[x] = CONVERT_16_TO_32(in[x]);
    }

    xbrz::scale(factor, in32.data(), out32.data(), width, height, xbrz::ColorFormat::RGB, xbrz::ScalerCfg(), 0, height);

    const int out_width = width * factor;
    for (int y = 0; y < height * factor; y++)
    {
        const uint32_t *in = &out32[y * out_width];
        auto *out = (uint16_t *)(dst + y * dst_pitch);
        for (int x = 0; x < out_width; x++)
            out[x] = CONVERT_32_TO_16(in[x]);
    }
}

static void filter_ntsc(int filter, uint8_t *src, int src_pitch, uint8_t *dst, int dst_pitch, int width, int height)
{
    static snes_ntsc_t ntsc;
    static snes_ntsc_setup_t setup;
    static int current_preset = -1;
    static int burst_phase = 0;

    if (current_preset != filter)
    {
        switch (filter)
        {
        default:
        case FILTER_NTSC_COMPOSITE:  setup = snes_ntsc_composite;  break;
        case FILTER_NTSC_SVIDEO:     setup = snes_ntsc_svideo;     break;
        case FILTER_NTSC_RGB:        setup = snes_ntsc_rgb;        break;
        case FILTER_NTSC_MONOCHROME: setup = snes_ntsc_monochrome; break;
        }
        snes_ntsc_init(&ntsc, &setup);
        current_preset = filter;
    }

    // Subtle scanlines on the doubled lines, matching the win32 default (12.5%)
    snes_ntsc_scanline_offset = 3;
    snes_ntsc_scanline_mask = 0x18e3;

    if (!setup.merge_fields)
        burst_phase = (burst_phase + 1) % 3;

    if (width > 256)
        snes_ntsc_blit_hires_scanlines(&ntsc, (SNES_NTSC_IN_T *)src, src_pitch >> 1,
                                       burst_phase, width, height, dst, dst_pitch);
    else
        snes_ntsc_blit_scanlines(&ntsc, (SNES_NTSC_IN_T *)src, src_pitch >> 1,
                                 burst_phase, width, height, dst, dst_pitch);
}

static const struct
{
    const char *name;
    void (*func)(uint8_t *, int, uint8_t *, int, int, int);
    int xscale;
    int yscale;
} filter_table[NUM_FILTERS] = {
    { "None",                       nullptr,              1, 1 },
    { "SuperEagle",                 SuperEagle,           2, 2 },
    { "2xSaI",                      _2xSaI,               2, 2 },
    { "Super2xSaI",                 Super2xSaI,           2, 2 },
    { "EPX",                        EPX_16,               2, 2 },
    { "EPX Smooth",                 EPX_16_smooth_unsafe, 2, 2 },
    { "Blargg's NTSC (Composite)",  nullptr,              1, 2 },
    { "Blargg's NTSC (S-Video)",    nullptr,              1, 2 },
    { "Blargg's NTSC (RGB)",        nullptr,              1, 2 },
    { "Blargg's NTSC (Monochrome)", nullptr,              1, 2 },
    { "Scanlines",                  filter_scanlines,     1, 2 },
    { "Simple2x",                   filter_simple<2>,     2, 2 },
    { "Simple3x",                   filter_simple<3>,     3, 3 },
    { "Simple4x",                   filter_simple<4>,     4, 4 },
    { "HQ2x",                       HQ2X_16,              2, 2 },
    { "HQ3x",                       HQ3X_16,              3, 3 },
    { "HQ4x",                       HQ4X_16,              4, 4 },
    { "2xBRZ",                      [](uint8_t *s, int sp, uint8_t *d, int dp, int w, int h) { filter_xbrz(2, s, sp, d, dp, w, h); }, 2, 2 },
    { "3xBRZ",                      [](uint8_t *s, int sp, uint8_t *d, int dp, int w, int h) { filter_xbrz(3, s, sp, d, dp, w, h); }, 3, 3 },
    { "4xBRZ",                      [](uint8_t *s, int sp, uint8_t *d, int dp, int w, int h) { filter_xbrz(4, s, sp, d, dp, w, h); }, 4, 4 },
};

int S9xSoftwareFilterCount()
{
    return NUM_FILTERS;
}

const char *S9xSoftwareFilterName(int filter)
{
    if (filter < 0 || filter >= NUM_FILTERS)
        return filter_table[FILTER_NONE].name;
    return filter_table[filter].name;
}

int S9xSoftwareFilterFromName(const std::string &name)
{
    auto iequals = [](const std::string &a, const char *b) {
        return std::equal(a.begin(), a.end(), b, b + strlen(b), [](char x, char y) {
            return std::tolower((unsigned char)x) == std::tolower((unsigned char)y);
        });
    };

    for (int i = 0; i < NUM_FILTERS; i++)
        if (iequals(name, filter_table[i].name))
            return i;
    return FILTER_NONE;
}

void S9xSoftwareFilterScale(int filter, int &width, int &height)
{
    if (filter <= FILTER_NONE || filter >= NUM_FILTERS)
        return;

    if (filter >= FILTER_NTSC_COMPOSITE && filter <= FILTER_NTSC_MONOCHROME)
    {
        // The hires blitter also outputs SNES_NTSC_OUT_WIDTH(256) pixels per line
        width = SNES_NTSC_OUT_WIDTH(256);
        height *= 2;
        return;
    }

    width *= filter_table[filter].xscale;
    height *= filter_table[filter].yscale;
}

void S9xApplySoftwareFilter(int filter,
                            uint8_t *src, int src_pitch,
                            uint8_t *dst, int dst_pitch,
                            int width, int height)
{
    if (filter <= FILTER_NONE || filter >= NUM_FILTERS)
        return;

    if (filter >= FILTER_NTSC_COMPOSITE && filter <= FILTER_NTSC_MONOCHROME)
        filter_ntsc(filter, src, src_pitch, dst, dst_pitch, width, height);
    else
        filter_table[filter].func(src, src_pitch, dst, dst_pitch, width, height);
}
