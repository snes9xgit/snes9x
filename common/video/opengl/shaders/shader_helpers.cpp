/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <png.h>
#include <stdlib.h>
#include <string.h>

#include "shader_helpers.h"
#include "shader_platform.h"

static void gl_error_callback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam)
{
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        fprintf(stderr, "GL: %s type = 0x%x, severity = 0x%x, \n %s\n",
                (type == GL_DEBUG_TYPE_ERROR ? "*ERROR*" : ""), type, severity,
                message);
    }
    else
    {
        fprintf(stderr, "GL type = 0x%x, severity = 0x%x, \n %s\n", type,
                severity, message);
    }

    return;
}

int gl_version()
{
    static int version = -1;

    if (version < 0)
    {
        const char *version_string = (const char *)glGetString(GL_VERSION);
        static int major_version = 1;
        static int minor_version = 0;

        sscanf(version_string, "%d.%d", &major_version, &minor_version);
        version = major_version * 10 + minor_version;
    }

    return version;
}

bool gl_srgb_available(void)
{
    if (gl_version() >= 30)
        return true;

    const char *extensions = (const char *)glGetString(GL_EXTENSIONS);

    if (strstr(extensions, "texture_sRGB") &&
        strstr(extensions, "framebuffer_sRGB"))
        return true;

    return false;
}

bool gl_float_texture_available(void)
{
    if (gl_version() >= 32)
        return true;

    const char *extensions = (const char *)glGetString(GL_EXTENSIONS);

    if (strstr(extensions, "texture_float"))
        return true;

    return false;
}

void gl_log_errors(void)
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback((GLDEBUGPROC)gl_error_callback, 0);
}

bool loadPngImage(const char *name, int &outWidth, int &outHeight,
                  bool &grayscale, bool &outHasAlpha, GLubyte **outData)
{
#ifdef HAVE_LIBPNG
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    FILE *fp;

    if ((fp = fopen(name, "rb")) == NULL)
        return false;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL)
    {
        fclose(fp);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);

    png_set_sig_bytes(png_ptr, sig_read);

    png_read_png(png_ptr, info_ptr,
                 PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING |
                     PNG_TRANSFORM_EXPAND,
                 (png_voidp)NULL);

    outWidth = png_get_image_width(png_ptr, info_ptr);
    outHeight = png_get_image_height(png_ptr, info_ptr);

    switch (png_get_color_type(png_ptr, info_ptr))
    {
    case PNG_COLOR_TYPE_RGBA:
        outHasAlpha = true;
        grayscale = false;
        break;
    case PNG_COLOR_TYPE_RGB:
        outHasAlpha = false;
        grayscale = false;
        break;
    case PNG_COLOR_TYPE_GRAY:
        grayscale = true;
        break;
    default:
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return false;
    }

    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    *outData = (unsigned char *)malloc(row_bytes * outHeight);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (int i = 0; i < outHeight; i++)
    {
        memcpy(*outData + (row_bytes * i), row_pointers[i], row_bytes);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

    fclose(fp);

    return true;
#else
    return false;
#endif
}

bool loadTGA(const char *filename, STGA &tgaFile)
{
    FILE *file;
    unsigned char type[4];
    unsigned char info[6];

    file = fopen(filename, "rb");

    if (!file)
        return false;

    fread(&type, sizeof(char), 3, file);
    fseek(file, 12, SEEK_SET);
    fread(&info, sizeof(char), 6, file);

    // image type either 2 (color) or 3 (greyscale)
    if (type[1] != 0 || (type[2] != 2 && type[2] != 3))
    {
        fclose(file);
        return false;
    }

    tgaFile.width = info[0] + info[1] * 256;
    tgaFile.height = info[2] + info[3] * 256;
    tgaFile.byteCount = info[4] / 8;

    if (tgaFile.byteCount != 3 && tgaFile.byteCount != 4)
    {
        fclose(file);
        return false;
    }

    long imageSize = tgaFile.width * tgaFile.height * tgaFile.byteCount;

    unsigned char *tempBuf = new unsigned char[imageSize];
    tgaFile.data = new unsigned char[tgaFile.width * tgaFile.height * 4];

    fread(tempBuf, sizeof(unsigned char), imageSize, file);

    // swap line order and convert to RGBA
    for (int i = 0; i < tgaFile.height; i++)
    {
        unsigned char *source = tempBuf + tgaFile.width *
                                          (tgaFile.height - 1 - i) *
                                          tgaFile.byteCount;
        unsigned char *destination = tgaFile.data + tgaFile.width * i * 4;
        for (int j = 0; j < tgaFile.width; j++)
        {
            destination[0] = source[2];
            destination[1] = source[1];
            destination[2] = source[0];
            destination[3] = tgaFile.byteCount == 4 ? source[3] : 0xff;
            source += tgaFile.byteCount;
            destination += 4;
        }
    }
    delete[] tempBuf;
    tgaFile.byteCount = 4;

    fclose(file);

    return true;
}

void reduce_to_path(char *filename)
{
    for (int i = strlen(filename); i >= 0; i--)
    {
        if (filename[i] == '\\' || filename[i] == '/')
        {
            filename[i] = 0;
            break;
        }
    }
}


