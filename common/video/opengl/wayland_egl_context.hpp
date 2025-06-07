/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "opengl_context.hpp"
#include "common/video/wayland/wayland_surface.hpp"

#include "glad/egl.h"
#include <memory>
#include <wayland-egl.h>

class WaylandEGLContext : public OpenGLContext
{
  public:
    WaylandEGLContext();
    ~WaylandEGLContext() override;
    bool attach(wl_display *display, wl_surface *surface, WaylandSurface::Metrics m);
    bool create_context() override;
    void resize() override {};
    void resize(WaylandSurface::Metrics m);
    void swap_buffers() override;
    void swap_interval(int frames) override;
    void make_current() override;
    void shrink();
    void regrow();
    bool ready() override;

    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLContext egl_context;
    EGLConfig egl_config;

    wl_egl_window *egl_window;

    std::unique_ptr<WaylandSurface> wayland_surface;
};