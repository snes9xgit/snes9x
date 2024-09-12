/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_DISPLAY_DRIVER_OPENGL_H
#define __GTK_DISPLAY_DRIVER_OPENGL_H

#include "gtk_s9x.h"
#include "gtk_display_driver.h"

#include <glad/gl.h>

#include "common/video/opengl/opengl_context.hpp"

#include "gtk_compat.h"
#ifdef GDK_WINDOWING_X11
#include "common/video/opengl/glx_context.hpp"
#endif
#ifdef GDK_WINDOWING_WAYLAND
#include "common/video/opengl/wayland_egl_context.hpp"
#endif

#include "common/video/opengl/shaders/glsl.h"
#include "common/video/std_chrono_throttle.hpp"

#define BUFFER_OFFSET(i) ((char *)(i))

class S9xOpenGLDisplayDriver : public S9xDisplayDriver
{
  public:
    S9xOpenGLDisplayDriver(Snes9xWindow *window, Snes9xConfig *config);
    void refresh() override;
    int init() override;
    void deinit() override;
    void update(uint16_t *buffer, int width, int height, int stride_in_pixels) override;
    void *get_parameters() override;
    void save(const char *filename) override;
    static int query_availability();
    bool is_ready() override;
    bool can_throttle() override { return true; }
    int get_width() final override { return output_window_width; }
    int get_height() final override { return output_window_height; }
    void shrink() override;
    void regrow() override;

  private:
    bool opengl_defaults();
    void swap_buffers();
    bool load_shaders(const char *);
    void update_texture_size(int width, int height);
    bool create_context();
    void resize();

    GLuint stock_program;
    GLuint coord_buffer;
    GLint texture_width;
    GLint texture_height;
    GLuint texmap;

    bool legacy;
    bool core;
    int version;
    bool initialized;

    bool using_glsl_shaders;
    GLSLShader *glsl_shader;

    GdkWindow *gdk_window;
    int output_window_width;
    int output_window_height;

    OpenGLContext *context;

    Throttle throttle;

#ifdef GDK_WINDOWING_X11
    GTKGLXContext glx;
#endif
#ifdef GDK_WINDOWING_WAYLAND
    WaylandEGLContext wl;
#endif
};

#endif /* __GTK_DISPLAY_DRIVER_OPENGL_H */
