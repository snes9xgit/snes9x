/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_GLX_CONTEXT_H
#define __GTK_GLX_CONTEXT_H

#include "gtk_2_3_compat.h"
#include <epoxy/glx.h>

#include "gtk_opengl_context.h"

class GTKGLXContext : public OpenGLContext
{
  public:
    GTKGLXContext ();
    ~GTKGLXContext ();
    bool attach (GtkWidget *widget);
    bool create_context ();
    void resize ();
    void swap_buffers ();
    void swap_interval (int frames);
    void make_current ();
    bool ready();

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

    bool use_oml_sync_control;
    int64_t ust, msc, sbc;
};

#endif
