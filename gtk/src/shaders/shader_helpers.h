#ifndef __IMAGE_FILE_FORMATS_H
#define __IMAGE_FILE_FORMATS_H

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

#endif // __IMAGE_FILE_FORMATS_H
