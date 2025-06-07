/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __WGL_CONTEXT_HPP
#define __WGL_CONTEXT_HPP

#include "opengl_context.hpp"

class WGLContext : public OpenGLContext
{
  public:
    WGLContext();
    ~WGLContext() override;
    bool attach(HWND xid);
    bool create_context() override;
    void resize() override;
    void swap_buffers() override;
    void swap_interval(int frames) override;
    void make_current() override;
    bool ready() override;
    void deinit();

    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;

    int version_major;
    int version_minor;
};

#endif
