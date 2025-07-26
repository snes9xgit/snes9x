#include <cstdint>
#include <cmath>
#include <algorithm>

#define CLAMP_U8(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

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

extern "C"
void ApplySharpBilinear4x(uint8_t* __restrict dst, int dst_pitch, const uint8_t* __restrict src,
    int src_width, int src_height, int src_pitch)
{
    const int dst_width = src_width * 4;
    const int dst_height = src_height * 4;

    static bool gamma_ready = false;
    if (!gamma_ready)
    {
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

            int r00, g00, b00;
            int r10, g10, b10;
            int r01, g01, b01;
            int r11, g11, b11;

            unpack_rgb565_gamma(src, src_pitch, sx, sy, r00, g00, b00);
            unpack_rgb565_gamma(src, src_pitch, sx + 1, sy, r10, g10, b10);
            unpack_rgb565_gamma(src, src_pitch, sx, sy + 1, r01, g01, b01);
            unpack_rgb565_gamma(src, src_pitch, sx + 1, sy + 1, r11, g11, b11);

            float fx1 = 1.0f - fx;
            float fy1 = 1.0f - fy;

            // Always blend horizontally and vertically
            float r_h = r00 * fx1 + r10 * fx;
            float g_h = g00 * fx1 + g10 * fx;
            float b_h = b00 * fx1 + b10 * fx;

            float r_v = r01 * fx1 + r11 * fx;
            float g_v = g01 * fx1 + g11 * fx;
            float b_v = b01 * fx1 + b11 * fx;

            float r = r_h * fy1 + r_v * fy;
            float g = g_h * fy1 + g_v * fy;
            float b = b_h * fy1 + b_v * fy;

            uint16_t out = build_rgb565_fast(
                gamma_decode[CLAMP_U8(int(r + 0.5f), 0, 255)],
                gamma_decode[CLAMP_U8(int(g + 0.5f), 0, 255)],
                gamma_decode[CLAMP_U8(int(b + 0.5f), 0, 255)]
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
