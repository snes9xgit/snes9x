/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "opengl_context.hpp"

#include <glad/glx.h>

class GTKGLXContext : public OpenGLContext
{
  public:
    GTKGLXContext();
    ~GTKGLXContext() override;
    bool attach(Display *dpy, Window xid);
    bool create_context() override;
    void resize() override;
    void swap_buffers() override;
    void swap_interval(int frames) override;
    void make_current() override;
    bool ready() override;

    GLXContext context;
    GLXFBConfig fbconfig;
    Display *display;
    int screen;
    Window xid;

    int version_major;
    int version_minor;
};