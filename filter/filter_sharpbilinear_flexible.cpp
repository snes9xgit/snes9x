#include <cstdint>
#include <cmath>
#include <algorithm>

#define CLAMP_U8(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

static uint8_t gamma_r_encode[32];
static uint8_t gamma_g_encode[64];
static uint8_t gamma_decode[256];

static void init_gamma_tables()
{
    constexpr float gamma = 1.4f;
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
    b = gamma_r_encode[b5]; // reuse red table for blue
}

extern "C"
void ApplySharpBilinear4x(
    uint8_t* __restrict dst,
    int dst_pitch,
    const uint8_t* __restrict src,
    int src_width,
    int src_height,
    int src_pitch)
{
    const int dst_width = src_width * 4;
    const int dst_height = src_height * 4;

    static bool gamma_ready = false;
    if (!gamma_ready) {
        init_gamma_tables();
        gamma_ready = true;
    }

    for (int y = 0; y < dst_height; ++y)
    {
        float src_yf = y / 4.0f;
        int sy = static_cast<int>(src_yf);
        float fy = src_yf - sy;
        sy = std::clamp(sy, 0, src_height - 2);

        for (int x = 0; x < dst_width; ++x)
        {
            float src_xf = x / 4.0f;
            int sx = static_cast<int>(src_xf);
            float fx = src_xf - sx;
            sx = std::clamp(sx, 0, src_width - 2);

            int r0, g0, b0, rx, gx, bx, ry, gy, by;

            unpack_rgb565_gamma(src, src_pitch, sx, sy, r0, g0, b0);
            unpack_rgb565_gamma(src, src_pitch, sx + 1, sy, rx, gx, bx);
            unpack_rgb565_gamma(src, src_pitch, sx, sy + 1, ry, gy, by);

            // Blend in gamma space
            int rh = r0 + static_cast<int>((rx - r0) * fx + 0.5f);
            int gh = g0 + static_cast<int>((gx - g0) * fx + 0.5f);
            int bh = b0 + static_cast<int>((bx - b0) * fx + 0.5f);

            int rv = r0 + static_cast<int>((ry - r0) * fy + 0.5f);
            int gv = g0 + static_cast<int>((gy - g0) * fy + 0.5f);
            int bv = b0 + static_cast<int>((by - b0) * fy + 0.5f);

            // Average
            int r = (rh + rv) >> 1;
            int g = (gh + gv) >> 1;
            int b = (bh + bv) >> 1;

            // Gamma decode and clamp
            uint16_t out = build_rgb565_fast(
                gamma_decode[r],
                gamma_decode[g],
                gamma_decode[b]
            );

            uint8_t* dst_px = dst + y * dst_pitch + x * 2;
            dst_px[0] = out & 0xFF;
            dst_px[1] = (out >> 8) & 0xFF;
        }
    }
}

extern "C"
void filter_sharpbilinear_4x(uint8_t* srcPtr, int srcPitch, uint8_t* dstPtr, int dstPitch,
    int width, int height)
{
    ApplySharpBilinear4x(dstPtr, dstPitch, srcPtr, width, height, srcPitch);
}
