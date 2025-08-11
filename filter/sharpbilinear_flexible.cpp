/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <cstdint>
#include <cmath>
#include <algorithm>

#define CLAMP_U8(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

// ---- Gamma tables (unchanged behavior) --------------------------------------

static uint8_t gamma_r_encode[32];
static uint8_t gamma_g_encode[64];
static uint8_t gamma_decode[256];

static void init_gamma_tables()
{
    constexpr float gamma = 1.6f;
    constexpr float inv_gamma = 1.0f / gamma;

    for (int i = 0; i < 32; ++i)
        gamma_r_encode[i] = uint8_t(CLAMP_U8(int(std::pow((i << 3) / 255.0f, gamma) * 255.0f + 0.5f), 0, 255));
    for (int i = 0; i < 64; ++i)
        gamma_g_encode[i] = uint8_t(CLAMP_U8(int(std::pow((i << 2) / 255.0f, gamma) * 255.0f + 0.5f), 0, 255));
    for (int i = 0; i < 256; ++i)
        gamma_decode[i] = uint8_t(CLAMP_U8(int(std::pow(i / 255.0f, inv_gamma) * 255.0f + 0.5f), 0, 255));
}

// ---- RGB565 helpers ---------------------------------------------------------

static inline uint16_t build_rgb565_fast(int r, int g, int b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static inline void unpack_rgb565_gamma(const uint8_t* src, int pitch, int x, int y, int& r, int& g, int& b)
{
    const uint8_t* pixel = src + y * pitch + x * 2;
    const uint16_t color = uint16_t(pixel[0]) | (uint16_t(pixel[1]) << 8);

    const int r5 = (color >> 11) & 0x1F;
    const int g6 = (color >> 5) & 0x3F;
    const int b5 = color & 0x1F;

    r = gamma_r_encode[r5];
    g = gamma_g_encode[g6];
    b = gamma_r_encode[b5]; // reuse red gamma table for blue
}

// ---- Fixed-point smoothstep weights at 4× sample locations ------------------
// smoothstep(0,1,x) at x in {0, 1/4, 1/2, 3/4} = {0, 5/32, 1/2, 27/32}
// Scale by 256 for 8.8 fixed point.
static constexpr uint16_t W[4] = { 0,  40, 128, 216 }; // w = smoothstep
static constexpr uint16_t IW[4] = { 256, 216, 128,  40 }; // 256 - w

void ApplySharpBilinear4x(uint8_t* __restrict dst, int dst_pitch,
    const uint8_t* __restrict src,
    int src_width, int src_height, int src_pitch)
{
    const int dst_width = src_width << 2; // *4
    const int dst_height = src_height << 2; // *4

    static bool gamma_ready = false;
    if (!gamma_ready)
    {
        init_gamma_tables();
        gamma_ready = true;
    }

    // Iterate over source texels; each emits a 4×4 block in the destination.
    for (int sy = 0; sy < src_height; ++sy)
    {
        // Clamp source rows to avoid reading past the bottom edge.
        const int sy0 = (sy < src_height - 1) ? sy : (src_height - 2);
        const int sy1 = sy0 + 1;

        // Precompute destination row base once per source row.
        const int dy_base = sy << 2; // sy * 4

        for (int sx = 0; sx < src_width; ++sx)
        {
            // Clamp source cols to avoid reading past the right edge.
            const int sx0 = (sx < src_width - 1) ? sx : (src_width - 2);
            const int sx1 = sx0 + 1;

            // Unpack the 2×2 neighborhood exactly once per 4×4 block.
            int r00, g00, b00;
            int r10, g10, b10;
            int r01, g01, b01;
            int r11, g11, b11;

            unpack_rgb565_gamma(src, src_pitch, sx0, sy0, r00, g00, b00);
            unpack_rgb565_gamma(src, src_pitch, sx1, sy0, r10, g10, b10);
            unpack_rgb565_gamma(src, src_pitch, sx0, sy1, r01, g01, b01);
            unpack_rgb565_gamma(src, src_pitch, sx1, sy1, r11, g11, b11);

            // Emit the 4×4 destination block using separable bilinear in 8.8 fixed-point.
            const int dx_base = sx << 2; // sx * 4

            // For each of the 4 subcolumns (dx), do horizontal mixes top/bottom once,
            // then vertical mix for each of the 4 subrows (dy).
            int rtop[4], gtop[4], btop[4];
            int rbot[4], gbot[4], bbot[4];

            for (int dx = 0; dx < 4; ++dx)
            {
                const uint16_t wx = W[dx];
                const uint16_t iwx = IW[dx];

                // Top row horizontal blend
                rtop[dx] = (r00 * iwx + r10 * wx + 128) >> 8;
                gtop[dx] = (g00 * iwx + g10 * wx + 128) >> 8;
                btop[dx] = (b00 * iwx + b10 * wx + 128) >> 8;

                // Bottom row horizontal blend
                rbot[dx] = (r01 * iwx + r11 * wx + 128) >> 8;
                gbot[dx] = (g01 * iwx + g11 * wx + 128) >> 8;
                bbot[dx] = (b01 * iwx + b11 * wx + 128) >> 8;
            }

            for (int dy = 0; dy < 4; ++dy)
            {
                const uint16_t wy = W[dy];
                const uint16_t iwy = IW[dy];

                // Destination row pointer for this subrow
                const int y = dy_base + dy;
                uint8_t* __restrict dst_row = dst + y * dst_pitch;

                for (int dx = 0; dx < 4; ++dx)
                {
                    const int x = dx_base + dx;

                    // Final vertical blend
                    int r = (rtop[dx] * iwy + rbot[dx] * wy + 128) >> 8;
                    int g = (gtop[dx] * iwy + gbot[dx] * wy + 128) >> 8;
                    int b = (btop[dx] * iwy + bbot[dx] * wy + 128) >> 8;

                    // Gamma decode back to display space and pack
                    const uint16_t out = build_rgb565_fast(
                        gamma_decode[CLAMP_U8(r, 0, 255)],
                        gamma_decode[CLAMP_U8(g, 0, 255)],
                        gamma_decode[CLAMP_U8(b, 0, 255)]
                    );

                    uint8_t* __restrict dst_px = dst_row + (x << 1); // x*2
                    dst_px[0] = uint8_t(out & 0xFF);
                    dst_px[1] = uint8_t((out >> 8) & 0xFF);
                }
            }
        }
    }
}

void sharpbilinear_4x(uint8_t* srcPtr, int srcPitch,
    uint8_t* dstPtr, int dstPitch,
    int width, int height)
{
    ApplySharpBilinear4x(dstPtr, dstPitch, srcPtr, width, height, srcPitch);
}
