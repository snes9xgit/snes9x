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
#define GL_FRAGMENT_SHADER           0x8B30
#define GL_VERTEX_SHADER             0x8B31
#define BUFFER_OFFSET(i)             ((char *) NULL + (i))

#ifdef __BIG_ENDIAN__
/* We have to reverse the bytes on MSB systems. This can be slow */
/* GL_UNSIGNED_INT_8_8_8_8_REV = 0x8367 */
#define PBO_BGRA_NATIVE_ORDER        0x8367
#else
#define PBO_BGRA_NATIVE_ORDER        GL_UNSIGNED_BYTE
#endif
#define PBO_GET_FORMAT(x) (((x) == PBO_FMT_16 || (x) == PBO_FMT_32) ? GL_BGRA : GL_RGB)
#define PBO_GET_PACKING(x) (((x) == PBO_FMT_16) ? GL_UNSIGNED_SHORT_1_5_5_5_REV : (((x) == PBO_FMT_24) ? GL_UNSIGNED_BYTE : PBO_BGRA_NATIVE_ORDER))

/* The following are procedure pointer types.
 * These aren't necessarily guaranteed to be in GL 1.1 */
typedef void      (*gl_proc)                  (void);
typedef gl_proc   (*getProcAddressProc)       (const GLubyte *name);

typedef GLint     (*glSwapIntervalProc)       (GLint interval);
typedef void      (*glXSwapIntervalEXTProc)   (Display *dpy,
                                               GLXDrawable drawable,
                                               int interval);
/* Procedures for pixel buffer objects */
typedef void      (*glGenBuffersProc)         (GLsizei n, GLuint *buffers);
typedef void      (*glDeleteBuffersProc)      (GLsizei n, const GLuint *buffers);
typedef void      (*glBindBufferProc)         (GLenum target, GLuint buffer);
typedef void      (*glBufferDataProc)         (GLenum target,
                                               GLsizeiptr size,
                                               const GLvoid *data,
                                               GLenum usage);
typedef void      (*glBufferSubDataProc)      (GLenum target,
                                               GLintptr offset,
                                               GLsizeiptr size,
                                               const GLvoid *data);
typedef GLvoid    *(*glMapBufferProc)         (GLenum target, GLenum access);
typedef GLboolean (*glUnmapBufferProc)        (GLenum target);
/* Procedures for GLSL */
typedef GLuint    (*glCreateProgramProc)      (void);
typedef GLuint    (*glCreateShaderProc)       (GLenum type);
typedef void      (*glCompileShaderProc)      (GLuint shader);
typedef void      (*glDeleteShaderProc)       (GLuint shader);
typedef void      (*glDeleteProgramProc)      (GLuint program);
typedef void      (*glAttachShaderProc)       (GLuint program, GLuint shader);
typedef void      (*glDetachShaderProc)       (GLuint program, GLuint shader);
typedef void      (*glLinkProgramProc)        (GLuint program);
typedef void      (*glUseProgramProc)         (GLuint program);
typedef void      (*glShaderSourceProc)       (GLuint shader,
                                               GLsizei count,
                                               const GLchar* *string,
                                               const GLint *length);
typedef GLint     (*glGetUniformLocationProc) (GLuint program,
                                               const GLchar *name);
typedef void      (*glUniform2fvProc)         (GLint location,
                                               GLsizei count,
                                               const GLfloat *value);

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
        int opengl_defaults (void);
        void swap_control (int enable);
        void gl_swap (void);
        int load_pixel_buffer_functions (void);
        int load_shader_functions (void);
        int load_shaders (const char *);
        gl_proc get_aliased_extension (const char **name);
        void update_texture_size (int width, int height);
        int init_glx (void);
        void create_window (int width, int height);
        void resize_window (int width, int height);

        getProcAddressProc       glGetProcAddress;
        glGenBuffersProc         glGenBuffers;
        glBindBufferProc         glBindBuffer;
        glBufferDataProc         glBufferData;
        glBufferSubDataProc      glBufferSubData;
        glMapBufferProc          glMapBuffer;
        glUnmapBufferProc        glUnmapBuffer;
        glDeleteBuffersProc      glDeleteBuffers;
        glCreateProgramProc      glCreateProgram;
        glCreateShaderProc       glCreateShader;
        glCompileShaderProc      glCompileShader;
        glDeleteShaderProc       glDeleteShader;
        glDeleteProgramProc      glDeleteProgram;
        glAttachShaderProc       glAttachShader;
        glDetachShaderProc       glDetachShader;
        glLinkProgramProc        glLinkProgram;
        glUseProgramProc         glUseProgram;
        glShaderSourceProc       glShaderSource;
        glGetUniformLocationProc glGetUniformLocation;
        glUniform2fvProc         glUniform2fv;

        GLint                    texture_width;
        GLint                    texture_height;
        GLfloat                  vertices[8];
        GLfloat                  texcoords[8];
        GLuint                   texmap;
        GLuint                   pbo;
        GLenum                   tex_target;
        GLuint                   program;
        GLuint                   fragment_shader;
        GLuint                   vertex_shader;

        int                      dyn_resizing;
        int                      using_pbos;
        int                      using_shaders;
        int                      initialized;

        Display                  *display;
        Window                   xwindow;
        Colormap                 xcolormap;
        XVisualInfo              *vi;
        GdkWindow                *gdk_window;
        GLXContext               glx_context;
        int                      output_window_width;
        int                      output_window_height;
};

#endif /* __GTK_DISPLAY_DRIVER_OPENGL_H */
