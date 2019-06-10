/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_WAYLAND_EGL_CONTEXT_H
#define __GTK_WAYLAND_EGL_CONTEXT_H

#include <epoxy/egl.h>
#include <epoxy/egl_generated.h>
#include <wayland-egl.h>

#include "gtk_opengl_context.h"

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

    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_subcompositor *subcompositor;

    struct wl_surface *parent;
    struct wl_surface *child;
    struct wl_subsurface *subsurface;
    struct wl_region *region;

    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLContext egl_context;
    EGLConfig egl_config;

    wl_egl_window *egl_window;

    typedef EGLBoolean (*PEGLGETSYNCVALUESCHROMIUM)(EGLDisplay, EGLSurface, EGLuint64KHR *, EGLuint64KHR *, EGLuint64KHR *);
    PEGLGETSYNCVALUESCHROMIUM eglGetSyncValuesCHROMIUM;
    bool use_sync_control;
    EGLuint64KHR ust, msc, sbc;
};

#endif
