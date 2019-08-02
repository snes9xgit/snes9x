/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __SHADER_HELPERS_H
#define __SHADER_HELPERS_H

#include "shader_platform.h"
#include <vector>
#include <string>
#include <sstream>

typedef struct _STGA
{
    _STGA()
    {
        data = (unsigned char *)0;
        width = 0;
        height = 0;
        byteCount = 0;
    }

    ~_STGA()
    {
        delete[] data;
        data = 0;
    }

    void destroy()
    {
        delete[] data;
        data = 0;
    }

    int width;
    int height;
    unsigned char byteCount;
    unsigned char *data;
} STGA;

bool loadPngImage(const char *name, int &outWidth, int &outHeight,
                  bool &grayscale, bool &outHasAlpha, GLubyte **outData);
bool loadTGA(const char *filename, STGA &tgaFile);
void gl_log_errors();
bool gl_srgb_available();
int gl_version();
bool gl_float_texture_available();
void reduce_to_path(char* filename);

#endif // __SHADER_HELPERS_H
