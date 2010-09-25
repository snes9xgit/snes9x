#ifndef __GTK_DISPLAY_DRIVER_OPENGL_H
#define __GTK_DISPLAY_DRIVER_OPENGL_H

#include "gtk_s9x.h"
#include "gtk_display_driver.h"

#include <GL/gl.h>
#include <GL/glx.h>

#define PBO_FMT_16 0
#define PBO_FMT_24 1
#define PBO_FMT_32 2

#define GL_PIXEL_UNPACK_BUFFER       0x88EC
#define GL_WRITE_ONLY                0x88B9
#define GL_STREAM_DRAW               0x88E0
#define GL_TEXTURE_RECTANGLE         0x84F5
#define BUFFER_OFFSET(i)             ((char *) NULL + (i))

#ifdef __BIG_ENDIAN__
/* We have to reverse the bytes on MSB systems. This can be slow */
/* GL_UNSIGNED_INT_8_8_8_8_REV = 0x8367 */
#define PBO_BGRA_NATIVE_ORDER        0x8367
#else
#define PBO_BGRA_NATIVE_ORDER        GL_UNSIGNED_BYTE
#endif

/* The following are procedure pointer types.
 * These aren't necessarily guaranteed to be in GL 1.1 */
typedef int (*glSwapIntervalProc) (int interval);

typedef void (*gl_proc) (void);
typedef gl_proc (*getProcAddressProc) (const GLubyte *name);

typedef void (*glGenBuffersProc) (GLsizei n, GLuint *buffers);
typedef void (*glDeleteBuffersProc) (GLsizei n, const GLuint *buffers);
typedef void (*glBindBufferProc) (GLenum target, GLuint buffer);
typedef void (*glBufferDataProc) (GLenum       target,
                                  GLsizeiptr   size,
                                  const GLvoid *data,
                                  GLenum       usage);
typedef void (*glBufferSubDataProc) (GLenum       target,
                                     GLintptr     offset,
                                     GLsizeiptr   size,
                                     const GLvoid *data);
typedef GLvoid *(*glMapBufferProc) (GLenum target, GLenum access);
typedef GLboolean (*glUnmapBufferProc) (GLenum target);

class S9xOpenGLDisplayDriver : public S9xDisplayDriver
{
    public:
        S9xOpenGLDisplayDriver (Snes9xWindow *window, Snes9xConfig *config);
        void refresh (int width, int height);
        int init (void);
        void deinit (void);
        void clear_buffers (void);
        void update (int width, int height);
        uint16 *get_next_buffer (void);
        uint16 *get_current_buffer (void);
        void push_buffer (uint16 *src);
        void reconfigure (int width, int height);
        static int query_availability (void);

    private:
        void opengl_defaults (void);
        void swap_control (int enable);
        void gl_lock (void);
        void gl_unlock (void);
        void gl_swap (void);
        int load_pixel_buffer_functions (void);
        gl_proc get_aliased_extension (const char **name);
        void update_texture_size (int width, int height);

        getProcAddressProc  glGetProcAddress;
        glGenBuffersProc    glGenBuffers;
        glBindBufferProc    glBindBuffer;
        glBufferDataProc    glBufferData;
        glBufferSubDataProc glBufferSubData;
        glMapBufferProc     glMapBuffer;
        glUnmapBufferProc   glUnmapBuffer;
        glDeleteBuffersProc glDeleteBuffers;

        GLint               texture_width;
        GLint               texture_height;
        GLfloat             vertices[8];
        GLfloat             texcoords[8];
        GLuint              texmap;
        GLuint              pbo;
        GLenum              tex_target;

        int                 dyn_resizing;
        int                 filtering;
        int                 using_pbos;
        GLXContext          glx_context;
};

#endif /* __GTK_DISPLAY_DRIVER_OPENGL_H */
