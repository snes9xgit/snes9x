/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GLX_CONTEXT_HPP
#define __GLX_CONTEXT_HPP

#include "opengl_context.hpp"

#include <glad/glx.h>

class GTKGLXContext : public OpenGLContext
{
  public:
    GTKGLXContext();
    ~GTKGLXContext();
    bool attach(Display *dpy, Window xid);
    bool create_context();
    void resize();
    void swap_buffers();
    void swap_interval(int frames);
    void make_current();
    bool ready();

    GLXContext context;
    GLXFBConfig fbconfig;
    Display *display;
    int screen;
    Window xid;

    int version_major;
    int version_minor;
};

#endif
