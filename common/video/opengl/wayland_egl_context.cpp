/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "wayland_egl_context.hpp"

WaylandEGLContext::WaylandEGLContext()
{
    egl_display = NULL;
    egl_surface = NULL;
    egl_context = NULL;
    egl_config = NULL;
    egl_window = NULL;
    x = 0;
    y = 0;
}

WaylandEGLContext::~WaylandEGLContext()
{
    if (egl_context)
        eglDestroyContext(egl_display, egl_context);

    if (egl_surface)
        eglDestroySurface(egl_display, egl_surface);

    if (egl_window)
        wl_egl_window_destroy(egl_window);
}

bool WaylandEGLContext::attach(wl_display *display, wl_surface *surface, WaylandSurface::Metrics m)
{
    wayland_surface = std::make_unique<WaylandSurface>();
    wayland_surface->attach(display, surface, m);

    return true;
}

bool WaylandEGLContext::create_context()
{
    EGLint surface_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
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

    if (!gladLoaderLoadEGL(nullptr))
    {
        printf("Couldn't load EGL.\n");
        return false;
    }

    egl_display = eglGetDisplay((EGLNativeDisplayType)wayland_surface->display);
    int major, minor;
    eglInitialize(egl_display, &major, &minor);

    // Load the rest of the functions only after calling eglInitialize.
    if (!gladLoaderLoadEGL(egl_display))
    {
        printf("Couldn't load EGL functions.\n");
    }

    if (!eglChooseConfig(egl_display, surface_attribs, &egl_config, 1, &num_configs))
    {
        printf("Couldn't find matching config.\n");
        return false;
    }

    std::tie(width, height) = wayland_surface->get_size();
    egl_window = wl_egl_window_create(wayland_surface->child, width, height);
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

    eglBindAPI(EGL_OPENGL_API);
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

    return true;
}

void WaylandEGLContext::resize(WaylandSurface::Metrics m)
{
    std::tie(width, height) = wayland_surface->get_size_for_metrics(m);
    wl_egl_window_resize(egl_window, width, height, 0, 0);
    wayland_surface->resize(m);
    make_current();
}

void WaylandEGLContext::swap_buffers()
{
    eglSwapBuffers(egl_display, egl_surface);
    wl_surface_commit(wayland_surface->child);
}

bool WaylandEGLContext::ready()
{
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

void WaylandEGLContext::shrink()
{
    wayland_surface->shrink();
}

void WaylandEGLContext::regrow()
{
    wayland_surface->regrow();
}