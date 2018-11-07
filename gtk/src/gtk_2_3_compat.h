#ifndef __GTK_2_3_COMPAT_H
#define __GTK_2_3_COMPAT_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#if GTK_MAJOR_VERSION >= 3

#include <gdk/gdkkeysyms-compat.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

#else // GTK+ 2.0

#define GDK_WINDOWING_X11
#define GDK_IS_X11_WINDOW(window) TRUE
#define GDK_IS_X11_DISPLAY(display) TRUE
#define gdk_x11_window_get_xid(window) GDK_WINDOW_XWINDOW (window)

inline void gdk_window_get_geometry (GdkWindow *window,
                                     gint *x,
                                     gint *y,
                                     gint *width,
                                     gint *height)
{
    gdk_window_get_geometry (window, x, y, width, height, NULL);
}

#endif

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#endif
