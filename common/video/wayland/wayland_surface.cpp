/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <tuple>
#include <wayland-util.h>

#include "fractional-scale-v1.h"
#include "wayland_surface.hpp"
#include "wayland-idle-inhibit-unstable-v1.h"
#include "viewporter-client-protocol.h"

static void wl_global(void *data,
                      struct wl_registry *wl_registry,
                      uint32_t name,
                      const char *interface,
                      uint32_t version)
{
    auto wl = (WaylandSurface *)data;

    auto bind = [=](auto **pointer, auto *interface_struct) {
        if (!strcmp(interface, interface_struct->name))
            *pointer = reinterpret_cast<typeof(*pointer)>(
                wl_registry_bind(wl_registry, name, interface_struct, version));
    };

    bind(&wl->compositor,               &wl_compositor_interface);
    bind(&wl->subcompositor,            &wl_subcompositor_interface);
    bind(&wl->idle_inhibit_manager,     &zwp_idle_inhibit_manager_v1_interface);
    bind(&wl->viewporter,               &wp_viewporter_interface);
    bind(&wl->fractional_scale_manager, &wp_fractional_scale_manager_v1_interface);
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

WaylandSurface::~WaylandSurface()
{
    if (idle_inhibitor)
        zwp_idle_inhibitor_v1_destroy(idle_inhibitor);

    if (idle_inhibit_manager)
        zwp_idle_inhibit_manager_v1_destroy(idle_inhibit_manager);

    if (subsurface)
        wl_subsurface_destroy(subsurface);

    if (region)
        wl_region_destroy(region);

    if (child)
        wl_surface_destroy(child);

    if (viewporter)
        wp_viewporter_destroy(viewporter);

    if (viewport)
        wp_viewport_destroy(viewport);

    if (fractional_scale_manager)
        wp_fractional_scale_manager_v1_destroy(fractional_scale_manager);

    if (fractional_scale)
        wp_fractional_scale_v1_destroy(fractional_scale);
}

static void preferred_scale(void *data,
                            struct wp_fractional_scale_v1 *wp_fractional_scale_v1,
                            uint32_t scale)
{
    ((WaylandSurface *)data)->actual_scale = scale / 120.0;
}

wp_fractional_scale_v1_listener fractional_scale_v1_listener =
{
    preferred_scale
};

bool WaylandSurface::attach(wl_display *display, wl_surface *surface, Metrics m)
{
    metrics = m;
    this->display = display;
    parent = surface;
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
    wl_subsurface_set_position(subsurface, m.x, m.y);

    if (fractional_scale_manager)
    {
        fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale(fractional_scale_manager, child);
        wp_fractional_scale_v1_add_listener(fractional_scale, &fractional_scale_v1_listener, this);
    }

    if (idle_inhibit_manager)
    {
        printf("Inhibiting screensaver.\n");
        zwp_idle_inhibit_manager_v1_create_inhibitor(idle_inhibit_manager, child);
    }

    wl_display_roundtrip(display);

    resize(m);

    return true;
}

std::tuple<int, int> WaylandSurface::get_size()
{
    return get_size_for_metrics(metrics);
}

std::tuple<int, int> WaylandSurface::get_size_for_metrics(Metrics m)
{
    if (actual_scale == 0.0)
    {
        return { m.width * m.scale, m.height * m.scale };
    }

    return { round(m.width * actual_scale), round(m.height * actual_scale) };
}

void WaylandSurface::shrink()
{
    resize({ metrics.x, metrics.y, 1, 1, metrics.scale});
}

void WaylandSurface::regrow()
{
    resize(metrics);
}

void WaylandSurface::resize(Metrics m)
{
    metrics = m;

    wl_subsurface_set_position(subsurface, m.x, m.y);

    if (!viewport)
        viewport = wp_viewporter_get_viewport(viewporter, child);

    wp_viewport_set_source(viewport,
        wl_fixed_from_int(-1), wl_fixed_from_int(-1),
        wl_fixed_from_int(-1), wl_fixed_from_int(-1));
    wp_viewport_set_destination(viewport, m.width, m.height);

    wl_surface_commit(child);
    wl_surface_commit(parent);
}