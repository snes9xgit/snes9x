#pragma once
#include <gdk/gdkwayland.h>
#include <wayland-egl.h>
#include <epoxy/egl.h>

#include "gtk_opengl_context.h"

struct WaylandEGLContext : OpenGLContext
{
    WaylandEGLContext ();
    ~WaylandEGLContext ();
    bool attach (GtkWidget *widget);
    bool create_context ();
    void resize ();
    void swap_buffers ();
    void swap_interval (int frames);
    void make_current ();

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
    EGLConfig  egl_config;

    wl_egl_window *egl_window;
};
