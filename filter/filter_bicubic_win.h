#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void ApplyBicubic4x(uint8_t* dst, int dst_width, int dst_height, int dst_pitch, const uint8_t* src, int src_width, int src_height, int src_pitch);
void filter_bicubic4x_standard(uint8_t* srcPtr, int srcPitch, uint8_t* dstPtr, int dstPitch, int srcWidth, int srcHeight);

#ifdef __cplusplus
}
#endif
