#include <stdio.h>
#include <string.h>

#include "gtk_s9x.h"
#include "gtk_glx_context.h"

GTKGLXContext::GTKGLXContext ()
{
    gdk_display       = NULL;
    parent_gdk_window = NULL;
    gdk_window        = NULL;
    display           = NULL;
    vi                = NULL;
    context           = NULL;

    version_major     = -1;
    version_minor     = -1;
}

GTKGLXContext::~GTKGLXContext ()
{
    if (context)
        glXDestroyContext (display, context);

    if (gdk_window)
        gdk_window_destroy (gdk_window);

    if (vi)
        XFree (vi);
}

bool GTKGLXContext::attach (GtkWidget *widget)
{
    GdkScreen   *gdk_screen;
    GdkWindow   *window;
    GLXFBConfig *fbconfigs;
    int         num_fbconfigs;

    int attribs[] = {
        GLX_DOUBLEBUFFER, True,
        GLX_X_RENDERABLE, True,
        GLX_RED_SIZE,     8,
        GLX_GREEN_SIZE,   8,
        GLX_BLUE_SIZE,    8,
        None
    };

    window = gtk_widget_get_window (widget);
    this->widget = widget;

    if (!GDK_IS_X11_WINDOW (window))
        return false;

    parent_gdk_window = window;
    gdk_display       = gdk_window_get_display (window);
    gdk_screen        = gdk_window_get_screen (window);
    screen            = gdk_x11_screen_get_screen_number (gdk_screen);
    display           = GDK_DISPLAY_XDISPLAY (gdk_display);

    glXQueryVersion (display, &version_major, &version_minor);
    if (version_major < 2 && version_minor < 3)
        return false;

    fbconfigs = glXChooseFBConfig (display, screen, attribs, &num_fbconfigs);
    if (!fbconfigs || num_fbconfigs < 1)
    {
        printf ("Couldn't match a GLX framebuffer config.\n");
        return false;
    }
    fbconfig = fbconfigs[0];
    XFree (fbconfigs);

    vi = glXGetVisualFromFBConfig (display, fbconfig);

    gdk_window_get_geometry (window, &x, &y, &width, &height);
    memset (&window_attr, 0, sizeof (GdkWindowAttr));
    window_attr.width       = width;
    window_attr.height      = height;
    window_attr.wclass      = GDK_INPUT_OUTPUT;
    window_attr.window_type = GDK_WINDOW_CHILD;
    window_attr.visual      = gdk_x11_screen_lookup_visual (gdk_screen, vi->visualid);

    gdk_window = gdk_window_new (window, &window_attr, GDK_WA_VISUAL);
    gdk_window_set_user_data (gdk_window, (gpointer) widget);
    gdk_window_show (gdk_window);
    xid = GDK_COMPAT_WINDOW_XID (gdk_window);

    return true;
}

bool GTKGLXContext::create_context ()
{
    int context_attribs[] = {
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        None
    };

    const char *extensions = glXQueryExtensionsString (display, screen);

    if (strstr (extensions, "GLX_ARB_create_context"))
        context = glXCreateContextAttribsARB (display, fbconfig, NULL, True, context_attribs);
    if (!context)
        context = glXCreateNewContext (display, fbconfig, GLX_RGBA_TYPE, NULL, True);

    if (!context)
    {
        printf ("Couldn't create GLX context.\n");
        return false;
    }

    return true;
}

void GTKGLXContext::resize ()
{
    gdk_window_get_geometry (parent_gdk_window, &x, &y, &width, &height);

    if (window_attr.width == width && window_attr.height == height)
        return;

    window_attr.width  = width;
    window_attr.height = height;

    gdk_window_destroy (gdk_window);
    gdk_window = gdk_window_new (parent_gdk_window, &window_attr, GDK_WA_VISUAL);
    gdk_window_set_user_data (gdk_window, (gpointer) widget);
    gdk_window_show (gdk_window);
    xid = GDK_COMPAT_WINDOW_XID (gdk_window);

    make_current ();
}

void GTKGLXContext::swap_buffers ()
{
    glXSwapBuffers (display, xid);
}

void GTKGLXContext::make_current ()
{
    glXMakeCurrent (display, xid, context);
}

void GTKGLXContext::swap_interval (int frames)
{
    if (epoxy_has_glx_extension (display, screen, "GLX_EXT_swap_control"))
        glXSwapIntervalEXT (display, xid, frames);
    else if (epoxy_has_glx_extension (display, screen, "GLX_SGI_swap_control"))
        glXSwapIntervalSGI (frames);
}


