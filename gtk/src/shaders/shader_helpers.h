#ifndef __SHADER_HELPERS_H
#define __SHADER_HELPERS_H

#include <epoxy/gl.h>

typedef struct _STGA
{
    _STGA()
    {
        data = (unsigned char*)0;
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
    unsigned char* data;
} STGA;

bool loadPngImage(const char* name,
                  int& outWidth,
                  int& outHeight,
                  bool& outHasAlpha,
                  GLubyte** outData);

bool loadTGA(const char* filename, STGA& tgaFile);

void glLogErrors (void);

bool srgb_available (void);
bool float_texture_available (void);
bool gl_version_at_least (int maj, int min);

#endif // __SHADER_HELPERS_H
