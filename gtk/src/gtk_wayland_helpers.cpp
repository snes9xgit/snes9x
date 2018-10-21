#include <stdio.h>
#include <string.h>

#include "gtk_wayland_helpers.h"

static void wl_global (void *data,
                       struct wl_registry *wl_registry,
                       uint32_t name,
                       const char *interface,
                       uint32_t version)
{
    struct wlgl_helper *wl = (struct wlgl_helper *) data;

    if (!strcmp (interface, "wl_compositor"))
        wl->compositor = (struct wl_compositor *) wl_registry_bind (wl_registry, name, &wl_compositor_interface, 1);
    else if (!strcmp (interface, "wl_subcompositor"))
        wl->subcompositor = (struct wl_subcompositor *) wl_registry_bind (wl_registry, name, &wl_subcompositor_interface, 1);
}

static void wl_global_remove (void *data,
                              struct wl_registry *wl_registry,
                              uint32_t name)
{
}

static const struct wl_registry_listener wl_registry_listener = {
    wl_global,
    wl_global_remove
};

wlgl_helper::wlgl_helper ()
{
    display       = NULL;
    registry      = NULL;
    compositor    = NULL;
    subcompositor = NULL;
    parent        = NULL;
    child         = NULL;
    subsurface    = NULL;
    egl_display   = NULL;
    egl_surface   = NULL;
    egl_context   = NULL;
    egl_config    = NULL;
    egl_window    = NULL;
}

wlgl_helper::~wlgl_helper ()
{
    if (subsurface)
        wl_subsurface_destroy (subsurface);

    if (region)
        wl_region_destroy (region);

    if (child)
        wl_surface_destroy (child);

    if (egl_context)
        eglDestroyContext (egl_display, egl_context);

    if (egl_surface)
        eglDestroySurface (egl_display, egl_surface);

    if (egl_window)
        wl_egl_window_destroy (egl_window);
}

bool wlgl_helper::attach (GdkWindow *window)
{
    int x, y, w, h;

    if (!GDK_IS_WAYLAND_WINDOW (window))
        return false;

    gdk_window = window;
    gdk_window_get_geometry (gdk_window, &x, &y, &w, &h);

    display  = gdk_wayland_display_get_wl_display (gdk_window_get_display (gdk_window));
    parent   = gdk_wayland_window_get_wl_surface (gdk_window);
    registry = wl_display_get_registry (display);

    wl_registry_add_listener (registry, &wl_registry_listener, this);
    wl_display_roundtrip (display);

    if (!compositor || !subcompositor)
        return false;

    child = wl_compositor_create_surface (compositor);
    region = wl_compositor_create_region (compositor);
    subsurface = wl_subcompositor_get_subsurface (subcompositor, child, parent);

    wl_surface_set_input_region (child, region);
    wl_subsurface_set_desync (subsurface);
    wl_subsurface_set_position (subsurface, x, y);

    return true;
}

bool wlgl_helper::create_egl_context (int width, int height)
{
    int scale = gdk_window_get_scale_factor (gdk_window);

    EGLint surface_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_NONE
    };

    EGLint context_attribs[] = {
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
        EGL_NONE
    };

    EGLint num_configs = 0;

    if (!subsurface)
        return false;

    egl_display = eglGetDisplay ((EGLNativeDisplayType) display);
    eglInitialize (egl_display, NULL, NULL);

    if (!eglChooseConfig (egl_display, surface_attribs, &egl_config, 1, &num_configs))
    {
        printf ("Couldn't find matching config.\n");
        return false;
    }
    eglBindAPI (EGL_OPENGL_API);

    egl_window  = wl_egl_window_create (child, width * scale, height * scale);
    if (!egl_window)
    {
        printf ("Couldn't create window.\n");
        return false;
    }

    egl_surface = eglCreateWindowSurface (egl_display, egl_config, (EGLNativeWindowType) egl_window, NULL);
    if (!egl_surface)
    {
        printf ("Couldn't create surface.\n");
        return false;
    }

    egl_context = eglCreateContext (egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);
    if (!egl_context)
    {
        printf ("Couldn't create context.\n");
        return false;
    }

    return true;
}

void wlgl_helper::resize (int width, int height)
{
    int x, y, w, h, scale;

    gdk_window_get_geometry (gdk_window, &x, &y, &w, &h);
    scale = gdk_window_get_scale_factor (gdk_window);

    wl_egl_window_resize (egl_window, w * scale, h * scale, 0, 0);
    wl_subsurface_set_position (subsurface, x, y);

    make_current ();
}

void wlgl_helper::swap_buffers ()
{
    eglSwapBuffers (egl_display, egl_surface);
    wl_surface_commit (child);
}

void wlgl_helper::make_current ()
{
    eglMakeCurrent (egl_display, egl_surface, egl_surface, egl_context);
}

void wlgl_helper::swap_interval (int frames)
{
    eglSwapInterval (egl_display, frames);
}


