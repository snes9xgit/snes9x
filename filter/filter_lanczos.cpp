#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FP_SHIFT 8
#define FP_ONE   (1 << FP_SHIFT)
#define CLAMP_U8(x) ((x) < 0 ? 0 : ((x) > 255 ? 255 : (x)))

inline float sinc(float x)
{
    if (x == 0.0f) return 1.0f;
    x *= float(M_PI);
    return std::sinf(x) / x;
}

inline int lanczos2_weight_fp(int t_fp)
{
    constexpr float radius = 0.6f;
    float t = float(t_fp) / float(FP_ONE);
    if (std::abs(t) >= radius) return 0;
    float w = sinc(t) * sinc(t / radius);
    return int(w * FP_ONE + 0.5f);
}

inline uint16_t build_rgb565_fast(int r, int g, int b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

extern "C"
void ApplyLanczos4x(
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
    constexpr int kernel_size = 4;
    constexpr int half_kernel = kernel_size / 2;
    const uint16_t* __restrict src16 = reinterpret_cast<const uint16_t*>(src);
    uint16_t* __restrict dst16 = reinterpret_cast<uint16_t*>(dst);
    int src_pitch16 = src_pitch / 2;
    int dst_pitch16 = dst_pitch / 2;

    constexpr float black_factor = 0.45f;

    // Build weight tables
    int16_t w_table[2][scale * scale * kernel_size * kernel_size];
    uint16_t inv_weight_sums[scale][scale];

    for (int dy = 0; dy < scale; ++dy) {
        int sy_fp = ((dy << FP_SHIFT) / scale);
        for (int dx = 0; dx < scale; ++dx) {
            int sx_fp = ((dx << FP_SHIFT) / scale);
            int base_sum = 0;
            int16_t* base_block = &w_table[0][(dy * scale + dx) * kernel_size * kernel_size];
            int16_t* black_block = &w_table[1][(dy * scale + dx) * kernel_size * kernel_size];

            for (int j = 0; j < kernel_size; ++j) {
                int wy = lanczos2_weight_fp(((j - half_kernel) << FP_SHIFT) - sy_fp);
                for (int i = 0; i < kernel_size; ++i) {
                    int wx = lanczos2_weight_fp(((i - half_kernel) << FP_SHIFT) - sx_fp);
                    int weight = (wx * wy) >> FP_SHIFT;
                    base_block[j * kernel_size + i] = static_cast<int16_t>(weight);
                    black_block[j * kernel_size + i] = static_cast<int16_t>(weight * black_factor + 0.5f);
                    base_sum += weight;
                }
            }

            inv_weight_sums[dy][dx] = (base_sum == 0) ? 65536 : ((65536 + (base_sum >> 1)) / base_sum);
        }
    }

    for (int y = 0; y < src_height; ++y) {
        for (int x = 0; x < src_width; ++x) {
            uint8_t rgb[kernel_size][kernel_size][3];
            uint8_t is_black[kernel_size][kernel_size];
            bool has_black = false;

            for (int j = 0; j < kernel_size; ++j) {
                int sy = std::clamp(y + j - half_kernel, 0, src_height - 1);
                const uint16_t* row = src16 + sy * src_pitch16;
                for (int i = 0; i < kernel_size; ++i) {
                    int sx = std::clamp(x + i - half_kernel, 0, src_width - 1);
                    uint16_t p = row[sx];
                    uint8_t r = (p >> 11) & 0x1F;
                    uint8_t g = (p >> 5) & 0x3F;
                    uint8_t b = p & 0x1F;
                    r = (r << 3) | (r >> 2);
                    g = (g << 2) | (g >> 4);
                    b = (b << 3) | (b >> 2);
                    rgb[j][i][0] = r;
                    rgb[j][i][1] = g;
                    rgb[j][i][2] = b;
                    bool black = (r | g | b) == 0;
                    is_black[j][i] = black;
                    has_black |= black;
                }
            }

            for (int dy = 0; dy < scale; ++dy) {
                for (int dx = 0; dx < scale; ++dx) {
                    int r_sum = 0, g_sum = 0, b_sum = 0, weight_sum = 0;
                    int block_idx = (dy * scale + dx) * kernel_size * kernel_size;
                    const int16_t* base_block = &w_table[0][block_idx];
                    const int16_t* black_block = &w_table[1][block_idx];

                    if (!has_black) {
                        // Fast path
                        for (int j = 0; j < kernel_size; ++j) {
                            for (int i = 0; i < kernel_size; ++i) {
                                int w = base_block[j * kernel_size + i];
                                r_sum += rgb[j][i][0] * w;
                                g_sum += rgb[j][i][1] * w;
                                b_sum += rgb[j][i][2] * w;
                            }
                        }
                        int norm = inv_weight_sums[dy][dx];
                        int r_final = CLAMP_U8((r_sum * norm) >> 16);
                        int g_final = CLAMP_U8((g_sum * norm) >> 16);
                        int b_final = CLAMP_U8((b_sum * norm) >> 16);
                        dst16[(y * scale + dy) * dst_pitch16 + (x * scale + dx)] =
                            build_rgb565_fast(r_final, g_final, b_final);
                    }
                    else {
                        // Slow path with true black weight attenuation
                        for (int j = 0; j < kernel_size; ++j) {
                            for (int i = 0; i < kernel_size; ++i) {
                                int index = j * kernel_size + i;
                                int w = base_block[index] - ((base_block[index] - black_block[index]) * is_black[j][i]);
                                r_sum += rgb[j][i][0] * w;
                                g_sum += rgb[j][i][1] * w;
                                b_sum += rgb[j][i][2] * w;
                                weight_sum += w;
                            }
                        }
                        int norm = (weight_sum == 0) ? 1 : ((65536 + (weight_sum >> 1)) / weight_sum);
                        int r_final = CLAMP_U8((r_sum * norm) >> 16);
                        int g_final = CLAMP_U8((g_sum * norm) >> 16);
                        int b_final = CLAMP_U8((b_sum * norm) >> 16);
                        dst16[(y * scale + dy) * dst_pitch16 + (x * scale + dx)] =
                            build_rgb565_fast(r_final, g_final, b_final);
                    }
                }
            }
        }
    }
}

extern "C"
void filter_lanczos4x(uint8_t* srcPtr, int srcPitch, uint8_t* dstPtr, int dstPitch,
    int width, int height)
{
    ApplyLanczos4x(dstPtr, width * 4, height * 4, dstPitch,
        srcPtr, width, height, srcPitch);
}
