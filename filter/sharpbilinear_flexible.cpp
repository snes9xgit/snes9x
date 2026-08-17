/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <cstdint>
#include <cmath>
#include <algorithm>

#define CLAMP_U8(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

// Smoothstep as in GLSL: cubic easing
static inline float smoothstep01(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

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

static inline uint16_t build_rgb565_fast(int r, int g, int b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static inline void unpack_rgb565_gamma(const uint8_t* src, int pitch, int x, int y, int& r, int& g, int& b)
{
    const uint8_t* pixel = src + y * pitch + x * 2;
    uint16_t color = pixel[0] | (pixel[1] << 8);

    int r5 = (color >> 11) & 0x1F;
    int g6 = (color >> 5) & 0x3F;
    int b5 = color & 0x1F;

    r = gamma_r_encode[r5];
    g = gamma_g_encode[g6];
    b = gamma_r_encode[b5]; // reuse red gamma table for blue
}

// --------- NEW: safe floor div/mod by 4 (handles negatives correctly) ---------
// We need this because centered mapping uses (x-2)/(y-2), which can be negative
// for the first two output pixels. Right-shift of negative is implementation-defined,
// so do it explicitly and keep remainder in [0..3].
static inline void floor_divmod_4(int v, int& q, int& r)
{
    if (v >= 0)
    {
        q = v >> 2;
        r = v & 3;
    }
    else
    {
        // floor division: q = floor(v/4)
        int a = -v;
        q = -((a + 3) >> 2);
        r = v - (q << 2); // remainder in [0..3]
        // Safety: due to any oddities, clamp remainder to 0..3
        if (r < 0) r = 0;
        if (r > 3) r = 3;
    }
}
// ---------------------------------------------------------------------------

extern "C"
void ApplySharpBilinear4x(uint8_t* __restrict dst, int dst_pitch, const uint8_t* __restrict src,
    int src_width, int src_height, int src_pitch)
{
    const int dst_width = src_width << 2;   // *4
    const int dst_height = src_height << 2; // *4

    static bool gamma_ready = false;
    if (!gamma_ready)
    {
        init_gamma_tables();
        gamma_ready = true;
    }

    // Gain > 1.0 = sharper (you can keep tweaking this one constant)
    constexpr float gain = 1.1f;

    // --------- CHANGED: centered fractional positions for 4× ---------
    // Pixel-center mapping is:
    //   src_x = (x + 0.5)/4 - 0.5
    // which corresponds to fractional phases:
    //   {0.125, 0.375, 0.625, 0.875}
    const float fracLUT[4] = { 0.125f, 0.375f, 0.625f, 0.875f };
    // ----------------------------------------------------------------

    // Precompute 1D weights for x/y phases
    float WX[4], IWX[4];
    float WY[4], IWY[4];
    for (int i = 0; i < 4; ++i)
    {
        float xf = std::clamp(0.5f + (fracLUT[i] - 0.5f) * gain, 0.0f, 1.0f);
        float w = smoothstep01(xf);
        WX[i] = w;  IWX[i] = 1.0f - w;
        WY[i] = w;  IWY[i] = 1.0f - w;
    }

    for (int y = 0; y < dst_height; ++y)
    {
        // --------- CHANGED: centered phase mapping via (y - 2) ---------
        // This implements src_y = (y + 0.5)/4 - 0.5 using integer ops.
        int sy, dy;
        floor_divmod_4(y - 2, sy, dy);
        // Clamp for safe +1 sampling
        sy = std::clamp(sy, 0, src_height - 2);
        // ----------------------------------------------------------------

        const float wy = WY[dy];
        const float iwy = IWY[dy];

        uint8_t* __restrict dst_row = dst + y * dst_pitch;

        for (int x = 0; x < dst_width; ++x)
        {
            // --------- CHANGED: centered phase mapping via (x - 2) ---------
            int sx, dx;
            floor_divmod_4(x - 2, sx, dx);
            sx = std::clamp(sx, 0, src_width - 2);
            // ----------------------------------------------------------------

            const float wx = WX[dx];
            const float iwx = IWX[dx];

            int r00, g00, b00;
            int r10, g10, b10;
            int r01, g01, b01;
            int r11, g11, b11;

            unpack_rgb565_gamma(src, src_pitch, sx, sy, r00, g00, b00);
            unpack_rgb565_gamma(src, src_pitch, sx + 1, sy, r10, g10, b10);
            unpack_rgb565_gamma(src, src_pitch, sx, sy + 1, r01, g01, b01);
            unpack_rgb565_gamma(src, src_pitch, sx + 1, sy + 1, r11, g11, b11);

            // Horizontal blends
            float r_h = r00 * iwx + r10 * wx;
            float g_h = g00 * iwx + g10 * wx;
            float b_h = b00 * iwx + b10 * wx;

            float r_v = r01 * iwx + r11 * wx;
            float g_v = g01 * iwx + g11 * wx;
            float b_v = b01 * iwx + b11 * wx;

            // Vertical blend
            float rf = r_h * iwy + r_v * wy;
            float gf = g_h * iwy + g_v * wy;
            float bf = b_h * iwy + b_v * wy;

            uint16_t out = build_rgb565_fast(
                gamma_decode[CLAMP_U8(int(rf + 0.5f), 0, 255)],
                gamma_decode[CLAMP_U8(int(gf + 0.5f), 0, 255)],
                gamma_decode[CLAMP_U8(int(bf + 0.5f), 0, 255)]
            );

            uint8_t* __restrict dst_px = dst_row + (x << 1);
            dst_px[0] = out & 0xFF;
            dst_px[1] = (out >> 8) & 0xFF;
        }
    }
}

void sharpbilinear_4x(uint8_t* srcPtr, int srcPitch,
    uint8_t* dstPtr, int dstPitch,
    int width, int height)
{
    ApplySharpBilinear4x(dstPtr, dstPitch, srcPtr, width, height, srcPitch);
}
