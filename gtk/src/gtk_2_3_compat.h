#ifndef __GTK_2_3_COMPAT_H
#define __GTK_2_3_COMPAT_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#if GTK_MAJOR_VERSION >= 3
#define GDK_COMPAT_WINDOW_XID(window) (gdk_x11_window_get_xid (window))
#else
#define GDK_WINDOWING_X11
#define GDK_IS_X11_WINDOW(window) TRUE
#define GDK_IS_X11_DISPLAY(display) TRUE
#define GDK_COMPAT_WINDOW_XID(window) (GDK_WINDOW_XWINDOW (window))
#endif

#endif
