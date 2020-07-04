/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gtk_s9x.h"
#include "gtk_glx_context.h"

GTKGLXContext::GTKGLXContext()
{
    display = NULL;
    context = NULL;

    version_major = -1;
    version_minor = -1;
    use_oml_sync_control = false;
    ust = msc = sbc = 0;
}

GTKGLXContext::~GTKGLXContext()
{
    if (context)
        glXDestroyContext(display, context);
}

bool GTKGLXContext::attach(Display *dpy, Window xid)
{
    GLXFBConfig *fbconfigs;
    int num_fbconfigs;

    int attribs[] = {
        GLX_DOUBLEBUFFER, True,
        GLX_X_RENDERABLE, True,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        None
    };

    this->xid = xid;
    display = dpy;

    glXQueryVersion(display, &version_major, &version_minor);
    if (version_major < 2 && version_minor < 3)
        return false;

    fbconfigs = glXChooseFBConfig(display, screen, attribs, &num_fbconfigs);
    if (!fbconfigs || num_fbconfigs < 1)
    {
        printf("Couldn't match a GLX framebuffer config.\n");
        return false;
    }
    fbconfig = fbconfigs[0];

    return true;
}

bool GTKGLXContext::create_context()
{
    int context_attribs[] = {
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        None
    };

    const char *extensions = glXQueryExtensionsString(display, screen);

    XSetErrorHandler([](Display *dpy, XErrorEvent *event) -> int {
        printf("XError: type: %d code: %d\n", event->type, event->error_code);
        return X_OK;
    });
    if (strstr(extensions, "GLX_ARB_create_context"))
        context = glXCreateContextAttribsARB(display, fbconfig, NULL, True, context_attribs);
    if (!context)
        context = glXCreateNewContext(display, fbconfig, GLX_RGBA_TYPE, NULL, True);
    XSetErrorHandler(nullptr);

    if (!context)
    {
        printf("Couldn't create GLX context.\n");
        return false;
    }

    if (strstr(extensions, "GLX_OML_sync_control") && gui_config->use_sync_control)
        use_oml_sync_control = true;

    return true;
}

void GTKGLXContext::resize()
{
    while (!ready())
        usleep(100);

    unsigned int width;
    unsigned int height;
    glXQueryDrawable(display, xid, GLX_WIDTH, &width);
    glXQueryDrawable(display, xid, GLX_HEIGHT, &height);

    this->width = width;
    this->height = height;
    make_current();
}

void GTKGLXContext::swap_buffers()
{
    if (use_oml_sync_control)
        glXGetSyncValuesOML(display, xid, &ust, &msc, &sbc);

    glXSwapBuffers(display, xid);
}

bool GTKGLXContext::ready()
{
    if (use_oml_sync_control)
    {
        int64 ust, msc, sbc;
        glXGetSyncValuesOML(display, xid, &ust, &msc, &sbc);

        if (sbc != this->sbc || msc - this->msc > 2)
            return true;
        return false;
    }

    return true;
}

void GTKGLXContext::make_current()
{
    glXMakeCurrent(display, xid, context);
}

void GTKGLXContext::swap_interval(int frames)
{
    if (epoxy_has_glx_extension(display, screen, "GLX_EXT_swap_control"))
        glXSwapIntervalEXT(display, xid, frames);
    else if (epoxy_has_glx_extension(display, screen, "GLX_SGI_swap_control"))
        glXSwapIntervalSGI(frames);
#ifdef GLX_MESA_swap_control
    else if (epoxy_has_glx_extension(display, screen, "GLX_MESA_swap_control"))
        glXSwapIntervalMESA(frames);
#endif
}
