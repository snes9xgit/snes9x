/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <wayland-util.h>

#include "fractional-scale-v1.h"
#include "gtk_s9x.h"
#include "gtk_wayland_surface.h"
#include "wayland-idle-inhibit-unstable-v1.h"
#include "viewporter-client-protocol.h"

static void wl_global(void *data,
                      struct wl_registry *wl_registry,
                      uint32_t name,
                      const char *interface,
                      uint32_t version)
{
    auto wl = (WaylandSurface *)data;

    if (!strcmp(interface, "wl_compositor"))
        wl->compositor = (struct wl_compositor *)wl_registry_bind(wl_registry, name, &wl_compositor_interface, 3);
    else if (!strcmp(interface, "wl_subcompositor"))
        wl->subcompositor = (struct wl_subcompositor *)wl_registry_bind(wl_registry, name, &wl_subcompositor_interface, 1);
    else if (!strcmp(interface, zwp_idle_inhibit_manager_v1_interface.name))
        wl->idle_inhibit_manager = (struct zwp_idle_inhibit_manager_v1 *)wl_registry_bind(wl_registry, name, &zwp_idle_inhibit_manager_v1_interface, 1);
    else if (!strcmp(interface, wp_viewporter_interface.name))
        wl->viewporter = (struct wp_viewporter *)wl_registry_bind(wl_registry, name, &wp_viewporter_interface, 1);
    else if (!strcmp(interface, wp_fractional_scale_manager_v1_interface.name))
        wl->fractional_scale_manager = (struct wp_fractional_scale_manager_v1 *)wl_registry_bind(wl_registry, name, &wp_fractional_scale_manager_v1_interface, 1);
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

WaylandSurface::WaylandSurface()
{
    display = nullptr;
    registry = nullptr;
    compositor = nullptr;
    subcompositor = nullptr;
    parent = nullptr;
    child = nullptr;
    region = nullptr;
    subsurface = nullptr;
    idle_inhibit_manager = nullptr;
    idle_inhibitor = nullptr;
    viewporter = nullptr;
    viewport = nullptr;
    fractional_scale_manager = nullptr;
    fractional_scale = nullptr;
    actual_scale = 0.0;
}

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

bool WaylandSurface::attach(GtkWidget *widget)
{
    GdkWindow *window = gtk_widget_get_window(widget);

    if (!GDK_IS_WAYLAND_WINDOW(window))
        return false;

    gdk_window = window;
    gdk_window_get_geometry(gdk_window, &x, &y, &width, &height);
    gdk_scale = gdk_window_get_scale_factor(gdk_window);

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

    if (fractional_scale_manager)
    {
        fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale(fractional_scale_manager, child);
        wp_fractional_scale_v1_add_listener(fractional_scale, &fractional_scale_v1_listener, this);
    }

    if (idle_inhibit_manager && gui_config->prevent_screensaver)
    {
        printf("Inhibiting screensaver.\n");
        zwp_idle_inhibit_manager_v1_create_inhibitor(idle_inhibit_manager, child);
    }

    resize();

    return true;
}

std::tuple<int, int> WaylandSurface::get_size()
{
    gdk_window_get_geometry(gdk_window, &x, &y, &width, &height);

    if (actual_scale == 0.0)
    {
        gdk_scale = gdk_window_get_scale_factor(gdk_window);

        return { width * gdk_scale, height * gdk_scale };
    }

    return { width * actual_scale, height * actual_scale };
}

void WaylandSurface::resize()
{
    int w, h; std::tie(w, h) = get_size();
    wl_subsurface_set_position(subsurface, x, y);

    if (!viewport)
        viewport = wp_viewporter_get_viewport(viewporter, child);

    wp_viewport_set_source(viewport,
        wl_fixed_from_int(0), wl_fixed_from_int(0),
        wl_fixed_from_int(w), wl_fixed_from_int(h));
    wp_viewport_set_destination(viewport, width, height);

    wl_surface_commit(child);
}