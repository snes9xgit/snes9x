#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cmath>

#define FP_SHIFT 8
#define FP_ONE   (1 << FP_SHIFT)
#define CLAMP_U8(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

static uint8_t gamma_r_encode[32];
static uint8_t gamma_g_encode[64];
static uint8_t gamma_decode[256];

static void init_gamma_tables()
{
    constexpr float gamma = 1.8f;
    constexpr float inv_gamma = 1.0f / gamma;

    for (int i = 0; i < 32; ++i)
        gamma_r_encode[i] = uint8_t(CLAMP_U8(int(std::pow((i << 3) / 255.0f, gamma) * 255.0f + 0.5f), 0, 255));
    for (int i = 0; i < 64; ++i)
        gamma_g_encode[i] = uint8_t(CLAMP_U8(int(std::pow((i << 2) / 255.0f, gamma) * 255.0f + 0.5f), 0, 255));
    for (int i = 0; i < 256; ++i)
        gamma_decode[i] = uint8_t(CLAMP_U8(int(std::pow(i / 255.0f, inv_gamma) * 255.0f + 0.5f), 0, 255));
}

inline int hermite_weight(int t_fp)
{
    int t = (t_fp > FP_ONE) ? FP_ONE : t_fp;
    int inv = FP_ONE - t;
    return ((inv * inv * (FP_ONE + (t >> 1))) >> FP_SHIFT) >> FP_SHIFT; // Softer (approx. 2/3)
}

inline uint16_t build_rgb565_fast(int r, int g, int b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

extern "C"
void ApplyBicubic4x(
    uint8_t* __restrict dst,
    int dst_width,
    int dst_height,
    int dst_pitch,
    const uint8_t* __restrict src,
    int src_width,
    int src_height,
    int src_pitch)
{
    constexpr int scale = 4;
    const uint16_t* __restrict src16 = reinterpret_cast<const uint16_t*>(src);
    uint16_t* __restrict dst16 = reinterpret_cast<uint16_t*>(dst);
    int src_pitch16 = src_pitch / 2;
    int dst_pitch16 = dst_pitch / 2;

    static bool gamma_ready = false;
    if (!gamma_ready) {
        init_gamma_tables();
        gamma_ready = true;
    }

    int weights[scale][scale][4][4];
    uint16_t inv_weight_sums[scale][scale];

    for (int dy = 0; dy < scale; ++dy) {
        int sy_fp = ((dy << FP_SHIFT) / scale);
        for (int dx = 0; dx < scale; ++dx) {
            int sx_fp = ((dx << FP_SHIFT) / scale);
            int sum = 0;
            for (int m = 0; m < 4; ++m) {
                int wy = hermite_weight(std::abs(((m - 1) << FP_SHIFT) - sy_fp));
                for (int n = 0; n < 4; ++n) {
                    int wx = hermite_weight(std::abs(((n - 1) << FP_SHIFT) - sx_fp));
                    int weight = (wx * wy) >> FP_SHIFT;
                    weights[dy][dx][m][n] = weight;
                    sum += weight;
                }
            }
            inv_weight_sums[dy][dx] = (sum == 0) ? 65536 : ((65536 + (sum >> 1)) / sum);
        }
    }

    for (int y = 0; y < src_height; ++y) {
        for (int x = 0; x < src_width; ++x) {
            bool is_center = x > 2 && x < src_width - 3 && y > 2 && y < src_height - 3;
            uint16_t neighborhood[4][4];
            uint8_t r8[4][4], g8[4][4], b8[4][4];

            if (is_center) {
                const uint16_t* row0 = src16 + (y - 1) * src_pitch16;
                const uint16_t* row1 = src16 + (y + 0) * src_pitch16;
                const uint16_t* row2 = src16 + (y + 1) * src_pitch16;
                const uint16_t* row3 = src16 + (y + 2) * src_pitch16;
                for (int i = 0; i < 4; ++i) {
                    int dx = i - 1;
                    neighborhood[0][i] = row0[x + dx];
                    neighborhood[1][i] = row1[x + dx];
                    neighborhood[2][i] = row2[x + dx];
                    neighborhood[3][i] = row3[x + dx];
                }
            }
            else {
                int clamped_x[4];
                int clamped_y[4];
                const uint16_t* src_rows[4];

                for (int j = 0; j < 4; ++j) {
                    clamped_y[j] = std::clamp(y + j - 1, 0, src_height - 1);
                    src_rows[j] = src16 + clamped_y[j] * src_pitch16;
                }
                for (int i = 0; i < 4; ++i) {
                    clamped_x[i] = std::clamp(x + i - 1, 0, src_width - 1);
                }

                for (int j = 0; j < 4; ++j) {
                    for (int i = 0; i < 4; ++i) {
                        neighborhood[j][i] = src_rows[j][clamped_x[i]];
                    }
                }
            }

            for (int j = 0; j < 4; ++j) {
                for (int i = 0; i < 4; ++i) {
                    uint16_t p = neighborhood[j][i];
                    r8[j][i] = gamma_r_encode[(p >> 11) & 0x1F];
                    g8[j][i] = gamma_g_encode[(p >> 5) & 0x3F];
                    b8[j][i] = gamma_r_encode[p & 0x1F];
                }
            }

            for (int dy = 0; dy < scale; ++dy) {
                for (int dx = 0; dx < scale; ++dx) {
                    int r_sum = 0, g_sum = 0, b_sum = 0;
                    int (*w)[4] = weights[dy][dx];
                    for (int j = 0; j < 4; ++j) {
                        for (int i = 0; i < 4; ++i) {
                            int weight = w[j][i];
                            r_sum += r8[j][i] * weight;
                            g_sum += g8[j][i] * weight;
                            b_sum += b8[j][i] * weight;
                        }
                    }
                    uint16_t norm = inv_weight_sums[dy][dx];
                    int r_final = gamma_decode[(r_sum * norm) >> 16];
                    int g_final = gamma_decode[(g_sum * norm) >> 16];
                    int b_final = gamma_decode[(b_sum * norm) >> 16];

                    dst16[(y * scale + dy) * dst_pitch16 + (x * scale + dx)] =
                        build_rgb565_fast(r_final, g_final, b_final);
                }
            }
        }
    }
}

extern "C"
void filter_bicubic4x_standard(uint8_t* srcPtr, int srcPitch, uint8_t* dstPtr, int dstPitch,
    int width, int height)
{
    ApplyBicubic4x(dstPtr, width * 4, height * 4, dstPitch,
        srcPtr, width, height, srcPitch);
}
