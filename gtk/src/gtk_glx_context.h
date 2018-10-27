#pragma once
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <epoxy/glx.h>

#include "gtk_opengl_context.h"

struct GTKGLXContext : OpenGLContext
{
    GTKGLXContext ();
    ~GTKGLXContext ();
    bool attach (GtkWidget *widget);
    bool create_context ();
    void resize ();
    void swap_buffers ();
    void swap_interval (int frames);
    void make_current ();

    GtkWidget *widget;

    GdkDisplay *gdk_display;
    GdkWindow *parent_gdk_window;
    GdkWindow *gdk_window;
    GdkWindowAttr window_attr;

    GLXContext context;
    GLXFBConfig fbconfig;
    Display *display;
    int screen;
    XVisualInfo *vi;
    Window xid;

    int version_major;
    int version_minor;
};
