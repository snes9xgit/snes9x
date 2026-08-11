/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

#include "opengl_context.hpp"

// NSOpenGL-backed OpenGL context for macOS.
//
// The AppKit objects are held as void* so this header stays includable from
// plain C++ translation units; the implementation is Objective-C++
// (mac_opengl_context.mm).
//
// Threading: macOS requires -[NSOpenGLContext setView:] and -update to happen
// on the main (AppKit) thread, while snes9x renders from the emulation
// thread. attach()/create_context()/resize() are therefore main-thread only,
// and every context operation takes the CGL context lock so the two threads
// cannot be inside the context at once.
//
// macOS only exposes a 4.1 core profile (the legacy profile stops at 2.1),
// and its GLSL compiler rejects `#version 140` in a core profile, so callers
// must feed it `#version 150` or newer shader sources.
class MacOpenGLContext : public OpenGLContext
{
  public:
    MacOpenGLContext();
    ~MacOpenGLContext() override;

    // view is the NSView* that QWidget::winId() returns. Main thread only.
    bool attach(void *view);

    bool create_context() override;
    void resize() override;
    void swap_buffers() override;
    void swap_interval(int frames) override;
    void make_current() override;
    bool ready() override;
    void deinit();

  private:
    void *view = nullptr;        // NSView *
    void *gl_context = nullptr;  // NSOpenGLContext *
};
