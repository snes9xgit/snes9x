/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_WAYLAND_EGL_CONTEXT_H
#define __GTK_WAYLAND_EGL_CONTEXT_H

#include "gtk_opengl_context.h"
#include "gtk_wayland_surface.h"
#include "gtk_compat.h"

#include <epoxy/egl.h>
#include <memory>
#include <wayland-egl.h>

class WaylandEGLContext : public OpenGLContext
{
  public:
    WaylandEGLContext();
    ~WaylandEGLContext();
    bool attach(GtkWidget *widget);
    bool create_context();
    void resize();
    void swap_buffers();
    void swap_interval(int frames);
    void make_current();
    bool ready();

    GdkWindow *gdk_window;

    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLContext egl_context;
    EGLConfig egl_config;

    wl_egl_window *egl_window;

    std::unique_ptr<WaylandSurface> wayland_surface;
};

#endif
