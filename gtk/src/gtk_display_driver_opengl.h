/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_DISPLAY_DRIVER_OPENGL_H
#define __GTK_DISPLAY_DRIVER_OPENGL_H

#include "gtk_s9x.h"
#include "gtk_display_driver.h"

#include <epoxy/gl.h>

#include "gtk_opengl_context.h"

#include "gtk_2_3_compat.h"
#ifdef GDK_WINDOWING_X11
#include "gtk_glx_context.h"
#endif
#ifdef GDK_WINDOWING_WAYLAND
#include "gtk_wayland_egl_context.h"
#endif

#include "shaders/glsl.h"

#define BUFFER_OFFSET(i) ((char *) (i))

class S9xOpenGLDisplayDriver : public S9xDisplayDriver
{
    public:
        S9xOpenGLDisplayDriver (Snes9xWindow *window, Snes9xConfig *config);
        void refresh (int width, int height);
        int init ();
        void deinit ();
        void clear_buffers ();
        void update (int width, int height, int yoffset);
        uint16 *get_next_buffer ();
        uint16 *get_current_buffer ();
        void push_buffer (uint16 *src);
        void *get_parameters ();
        void save (const char *filename);
        static int query_availability ();
        bool is_ready ();

    private:
        bool opengl_defaults ();
        void swap_buffers ();
        bool load_shaders (const char *);
        void update_texture_size (int width, int height);
        bool create_context ();
        void resize ();

        GLuint            stock_program;
        GLuint            coord_buffer;
        GLint             texture_width;
        GLint             texture_height;
        GLuint            texmap;
        GLuint            pbo;

        bool              legacy;
        bool              core;
        int               version;
        bool              npot;
        bool              using_pbos;
        bool              initialized;

        bool              using_glsl_shaders;
        GLSLShader        *glsl_shader;

        GdkWindow         *gdk_window;
        int               output_window_width;
        int               output_window_height;

        OpenGLContext     *context;

#ifdef GDK_WINDOWING_X11
        GTKGLXContext     glx;
#endif
#ifdef GDK_WINDOWING_WAYLAND
        WaylandEGLContext wl;
#endif
};

#endif /* __GTK_DISPLAY_DRIVER_OPENGL_H */
