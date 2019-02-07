/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_2_3_COMPAT_H
#define __GTK_2_3_COMPAT_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#if GTK_MAJOR_VERSION >= 3

#include <gdk/gdkkeysyms-compat.h>

#ifndef USE_WAYLAND
#undef GDK_WINDOWING_WAYLAND
#endif

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

#else // GTK+ 2.0

#define GDK_WINDOWING_X11
#define GDK_IS_X11_WINDOW(window) true
#define GDK_IS_X11_DISPLAY(display) true
#define gdk_x11_window_get_xid(window) GDK_WINDOW_XWINDOW (window)

inline void gdk_x11_display_error_trap_push(GdkDisplay *dpy)
{
    gdk_error_trap_push();
}

inline void gdk_x11_display_error_trap_pop_ignored(GdkDisplay *dpy)
{
    if (gdk_error_trap_pop())
    {
    }
}

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
