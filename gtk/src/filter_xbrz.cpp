/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_s9x.h"
#include "../filter/xbrz.h"
#include <vector>

#define CONVERT_16_TO_32(pixel) \
    (((((pixel) >> 11)        ) << /*RedShift+3*/  19) | \
     ((((pixel) >> 5)   & 0x3f) << /*GreenShift+3*/10) | \
      (((pixel)         & 0x1f) << /*BlueShift+3*/ 3))

#define CONVERT_32_TO_16(pixel) \
    (((((pixel) & 0xf80000) >> 8) | \
      (((pixel) & 0xfc00)   >> 5) | \
      (((pixel) & 0xf8)     >> 3)) & 0xffff)

inline
void copyImage16To32(const uint16_t* src, int width, int height, int srcPitch,
                     uint32_t* trg, int yFirst, int yLast)
{
    yFirst = std::max(yFirst, 0);
    yLast  = std::min(yLast, height);
    if (yFirst >= yLast || height <= 0 || width <= 0) return;

    for (int y = yFirst; y < yLast; ++y)
    {
        uint32_t* trgLine = trg + y * width;
        const uint16_t* srcLine = reinterpret_cast<const uint16_t*>(reinterpret_cast<const char*>(src) + y * srcPitch);

        for (int x = 0; x < width; ++x)
            trgLine[x] = CONVERT_16_TO_32(srcLine[x]);
    }
}

//stretch image and convert from ARGB to RGB565/555
inline
void stretchImage32To16(const uint32_t* src, int srcWidth, int srcHeight,
                        uint16_t* trg, int trgWidth, int trgHeight, int trgPitch,
                        int yFirst, int yLast)
{
    yFirst = std::max(yFirst, 0);
    yLast  = std::min(yLast, trgHeight);
    if (yFirst >= yLast || srcHeight <= 0 || srcWidth <= 0) return;

    for (int y = yFirst; y < yLast; ++y)
    {
        uint16_t* trgLine = reinterpret_cast<uint16_t*>(reinterpret_cast<char*>(trg) + y * trgPitch);
        const int ySrc = srcHeight * y / trgHeight;
        const uint32_t* srcLine = src + ySrc * srcWidth;
        for (int x = 0; x < trgWidth; ++x)
        {
            const int xSrc = srcWidth * x / trgWidth;
            trgLine[x] = CONVERT_32_TO_16(srcLine[xSrc]);
        }
    }
}


void xBRZ(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, int scalingFactor)
{

    std::vector<uint32_t> renderBuffer; //raw image
    std::vector<uint32_t> xbrzBuffer;   //scaled image

    if (width  <= 0 || height <= 0)
        return;

    int trgWidth = width * scalingFactor;
    int trgHeight = height * scalingFactor;

    if (gui_config->multithreading && gui_config->num_threads > 1)
    {
        renderBuffer.resize(width * (height + 4));
        xbrzBuffer.resize(renderBuffer.size() * scalingFactor * scalingFactor);

        copyImage16To32(reinterpret_cast<const uint16_t *>(srcPtr - srcPitch * 2),
                        width, 
                        height + 4, 
                        srcPitch, 
                        &renderBuffer[0], 
                        0, 
                        height + 4);
        xbrz::scale(scalingFactor, &renderBuffer[0], &xbrzBuffer[0], width, height + 4, xbrz::ColorFormat::RGB, xbrz::ScalerCfg(), 2, height + 2);

        stretchImage32To16(&xbrzBuffer[trgWidth * 2 * scalingFactor], trgWidth, trgHeight,
                           reinterpret_cast<uint16_t *>(dstPtr), trgWidth, trgHeight, dstPitch, 0, height * scalingFactor);
    }
    else
    {
        renderBuffer.resize(width * height);
        xbrzBuffer.resize(renderBuffer.size() * scalingFactor * scalingFactor);

        copyImage16To32(reinterpret_cast<const uint16_t *>(srcPtr), width, height, srcPitch,
                        &renderBuffer[0], 0, height);

        xbrz::scale(scalingFactor, &renderBuffer[0], &xbrzBuffer[0], width, height, xbrz::ColorFormat::RGB, xbrz::ScalerCfg(), 0, height);
        stretchImage32To16(&xbrzBuffer[0], width * scalingFactor, height * scalingFactor,
                          reinterpret_cast<uint16_t *>(dstPtr), trgWidth, trgHeight, dstPitch, 0, height * scalingFactor);
    }
}

void filter_2xBRZ(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
    xBRZ(srcPtr, srcPitch, dstPtr, dstPitch, width, height, 2);
}

void filter_3xBRZ(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
    xBRZ(srcPtr, srcPitch, dstPtr, dstPitch, width, height, 3);
}

void filter_4xBRZ(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
    xBRZ(srcPtr, srcPitch, dstPtr, dstPitch, width, height, 4);
}


