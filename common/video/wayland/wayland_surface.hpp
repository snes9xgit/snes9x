/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

#include "viewporter-client-protocol.h"
#include "fractional-scale-v1.h"
#include <tuple>

class WaylandSurface
{
  public:
    WaylandSurface();
    ~WaylandSurface();

    struct Metrics {
        int x, y, width, height, scale;
    };

    bool attach(wl_display *display, wl_surface *surface, Metrics source_metrics);
    void resize(Metrics new_metrics);
    void shrink();
    void regrow();
    std::tuple<int, int> get_size();
    std::tuple<int, int> get_size_for_metrics(Metrics m);

    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_subcompositor *subcompositor;

    struct wl_surface *parent;
    struct wl_surface *child;
    struct wl_subsurface *subsurface;
    struct wl_region *region;

    Metrics metrics;
    double actual_scale;

    struct zwp_idle_inhibit_manager_v1 *idle_inhibit_manager;
    struct zwp_idle_inhibitor_v1 *idle_inhibitor;

    struct wp_viewporter *viewporter;
    struct wp_viewport *viewport;

    struct wp_fractional_scale_manager_v1 *fractional_scale_manager;
    struct wp_fractional_scale_v1 *fractional_scale;
};
