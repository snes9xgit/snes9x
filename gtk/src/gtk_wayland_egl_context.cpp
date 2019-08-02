/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "gtk_s9x.h"
#include "gtk_wayland_egl_context.h"

static void wl_global(void *data,
                      struct wl_registry *wl_registry,
                      uint32_t name,
                      const char *interface,
                      uint32_t version)
{
    WaylandEGLContext *wl = (WaylandEGLContext *)data;

    if (!strcmp(interface, "wl_compositor"))
        wl->compositor = (struct wl_compositor *)wl_registry_bind(wl_registry, name, &wl_compositor_interface, 3);
    else if (!strcmp(interface, "wl_subcompositor"))
        wl->subcompositor = (struct wl_subcompositor *)wl_registry_bind(wl_registry, name, &wl_subcompositor_interface, 1);
}

static void wl_global_remove(void *data,
                             struct wl_registry *wl_registry,
                             uint32_t name)
{
}

static const struct wl_registry_listener wl_registry_listener = {
    wl_global,
    wl_global_remove
};

WaylandEGLContext::WaylandEGLContext()
{
    display = NULL;
    registry = NULL;
    compositor = NULL;
    subcompositor = NULL;
    parent = NULL;
    child = NULL;
    region = NULL;
    subsurface = NULL;
    egl_display = NULL;
    egl_surface = NULL;
    egl_context = NULL;
    egl_config = NULL;
    egl_window = NULL;
    use_sync_control = false;
    ust = msc = sbc = 0;
}

WaylandEGLContext::~WaylandEGLContext()
{
    if (subsurface)
        wl_subsurface_destroy(subsurface);

    if (region)
        wl_region_destroy(region);

    if (child)
        wl_surface_destroy(child);

    if (egl_context)
        eglDestroyContext(egl_display, egl_context);

    if (egl_surface)
        eglDestroySurface(egl_display, egl_surface);

    if (egl_window)
        wl_egl_window_destroy(egl_window);
}

bool WaylandEGLContext::attach(GtkWidget *widget)
{
    GdkWindow *window = gtk_widget_get_window(widget);

    if (!GDK_IS_WAYLAND_WINDOW(window))
        return false;

    gdk_window = window;
    gdk_window_get_geometry(gdk_window, &x, &y, &width, &height);

    display = gdk_wayland_display_get_wl_display(gdk_window_get_display(gdk_window));
    parent = gdk_wayland_window_get_wl_surface(gdk_window);
    registry = wl_display_get_registry(display);

    wl_registry_add_listener(registry, &wl_registry_listener, this);
    wl_display_roundtrip(display);

    if (!compositor || !subcompositor)
        return false;

    child = wl_compositor_create_surface(compositor);
    region = wl_compositor_create_region(compositor);
    subsurface = wl_subcompositor_get_subsurface(subcompositor, child, parent);

    wl_surface_set_input_region(child, region);
    wl_subsurface_set_desync(subsurface);
    wl_subsurface_set_position(subsurface, x, y);

    return true;
}

bool WaylandEGLContext::create_context()
{
    int scale = gdk_window_get_scale_factor(gdk_window);
    gdk_window_get_geometry(gdk_window, &x, &y, &width, &height);

    EGLint surface_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_NONE
    };

    EGLint core_context_attribs[] = {
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_NONE
    };

    EGLint compatibility_context_attribs[] = {
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
        EGL_NONE
    };

    EGLint num_configs = 0;

    if (!subsurface)
        return false;

    egl_display = eglGetDisplay((EGLNativeDisplayType)display);
    eglInitialize(egl_display, NULL, NULL);

    if (!eglChooseConfig(egl_display, surface_attribs, &egl_config, 1, &num_configs))
    {
        printf("Couldn't find matching config.\n");
        return false;
    }
    eglBindAPI(EGL_OPENGL_API);

    egl_window = wl_egl_window_create(child, width * scale, height * scale);
    if (!egl_window)
    {
        printf("Couldn't create window.\n");
        return false;
    }

    egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)egl_window, NULL);
    if (!egl_surface)
    {
        printf("Couldn't create surface.\n");
        return false;
    }

    egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, core_context_attribs);
    if (!egl_context)
    {
        egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, compatibility_context_attribs);
        if (!egl_context)
        {
            printf("Couldn't create context.\n");
            return false;
        }
    }

    if (gui_config->use_sync_control && epoxy_has_egl_extension(egl_display, "EGL_CHROMIUM_sync_control"))
    {
        eglGetSyncValuesCHROMIUM = (PEGLGETSYNCVALUESCHROMIUM)eglGetProcAddress("eglGetSyncValuesCHROMIUM");
        if (eglGetSyncValuesCHROMIUM)
            use_sync_control = true;
    }

    wl_surface_set_buffer_scale(child, scale);
    gdk_window_invalidate_rect(gdk_window, NULL, false);

    return true;
}

void WaylandEGLContext::resize()
{
    int scale;

    gdk_window_get_geometry(gdk_window, &x, &y, &width, &height);
    scale = gdk_window_get_scale_factor(gdk_window);

    wl_egl_window_resize(egl_window, width * scale, height * scale, 0, 0);
    wl_subsurface_set_position(subsurface, x, y);

    make_current();
}

void WaylandEGLContext::swap_buffers()
{
    if (use_sync_control)
        eglGetSyncValuesCHROMIUM(egl_display, egl_surface, &ust, &msc, &sbc);

    eglSwapBuffers(egl_display, egl_surface);
    wl_surface_commit(child);
}

bool WaylandEGLContext::ready()
{
    if (use_sync_control)
    {
        EGLuint64KHR ust, msc, sbc;
        eglGetSyncValuesCHROMIUM(egl_display, egl_surface, &ust, &msc, &sbc);
        if (sbc != this->sbc || msc - this->msc > 2)
            return true;
        return false;
    }

    return true;
}

void WaylandEGLContext::make_current()
{
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
}

void WaylandEGLContext::swap_interval(int frames)
{
    eglSwapInterval(egl_display, frames);
}
