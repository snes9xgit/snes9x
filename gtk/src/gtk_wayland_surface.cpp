/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "gtk_s9x.h"
#include "gtk_wayland_surface.h"
#include "wayland-idle-inhibit-unstable-v1.h"

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
}

bool WaylandSurface::attach(GtkWidget *widget)
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

    if (idle_inhibit_manager && gui_config->prevent_screensaver)
    {
        printf("Inhibiting screensaver.\n");
        zwp_idle_inhibit_manager_v1_create_inhibitor(idle_inhibit_manager, child);
    }

    auto scale = gdk_window_get_scale_factor(window);
    wl_surface_set_buffer_scale(child, scale);

    return true;
}

void WaylandSurface::resize()
{
    gdk_window_get_geometry(gdk_window, &x, &y, &width, &height);
    auto scale = gdk_window_get_scale_factor(gdk_window);
    wl_subsurface_set_position(subsurface, x, y);
    wl_surface_set_buffer_scale(child, scale);
}