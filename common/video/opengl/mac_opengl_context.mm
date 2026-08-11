/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "mac_opengl_context.hpp"

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

#include <cstdio>

// OpenGL is deprecated on macOS but still the only cross-vendor 3D API
// available without dragging in MoltenVK. Silence the deprecation spam so
// real warnings stay visible.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace
{

NSOpenGLContext *as_context(void *p)
{
    return (__bridge NSOpenGLContext *)p;
}

NSView *as_view(void *p)
{
    return (__bridge NSView *)p;
}

// RAII wrapper for the CGL context lock. The emulation thread renders while
// the main thread may be servicing a resize, and NSOpenGLContext is not safe
// to touch from both at once.
class ContextLock
{
  public:
    explicit ContextLock(void *ctx) : cgl(nullptr)
    {
        if (!ctx)
            return;
        cgl = [as_context(ctx) CGLContextObj];
        if (cgl)
            CGLLockContext(cgl);
    }

    ~ContextLock()
    {
        if (cgl)
            CGLUnlockContext(cgl);
    }

    ContextLock(const ContextLock &) = delete;
    ContextLock &operator=(const ContextLock &) = delete;

  private:
    CGLContextObj cgl;
};

} // namespace

MacOpenGLContext::MacOpenGLContext()
{
}

MacOpenGLContext::~MacOpenGLContext()
{
    deinit();
}

bool MacOpenGLContext::attach(void *view_)
{
    if (!view_)
        return false;

    view = view_;

    NSView *v = as_view(view);
    // The layer backing is what actually gets composited; without it the
    // context draws into a surface the window server never shows.
    [v setWantsBestResolutionOpenGLSurface:YES];

    return true;
}

bool MacOpenGLContext::create_context()
{
    if (gl_context)
        return true;

    if (!view)
        return false;

    // 4.1 is the highest macOS ever shipped, and only in a core profile.
    NSOpenGLPixelFormatAttribute attributes[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFAColorSize,     24,
        NSOpenGLPFAAlphaSize,     8,
        NSOpenGLPFADepthSize,     0,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        // Lets the context survive an eGPU/discrete GPU going away, and
        // allows software fallback rather than failing outright.
        NSOpenGLPFAAllowOfflineRenderers,
        0
    };

    NSOpenGLPixelFormat *format =
        [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];

    if (!format)
    {
        printf("Couldn't find a suitable NSOpenGL pixel format.\n");
        return false;
    }

    NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:format
                                                         shareContext:nil];
    if (!context)
    {
        printf("Couldn't create an NSOpenGLContext.\n");
        return false;
    }

    gl_context = (__bridge_retained void *)context;

    NSView *v = as_view(view);
    [context setView:v];
    [context makeCurrentContext];

    NSRect backing = [v convertRectToBacking:[v bounds]];
    width = backing.size.width;
    height = backing.size.height;

    return true;
}

void MacOpenGLContext::resize()
{
    if (!gl_context || !view)
        return;

    ContextLock lock(gl_context);

    NSView *v = as_view(view);
    NSRect backing = [v convertRectToBacking:[v bounds]];
    width = backing.size.width;
    height = backing.size.height;

    // -update re-reads the view geometry; it is the NSOpenGL equivalent of
    // glXWaitX()/wglMakeCurrent after a resize.
    [as_context(gl_context) update];
}

void MacOpenGLContext::swap_buffers()
{
    if (!gl_context)
        return;

    ContextLock lock(gl_context);
    [as_context(gl_context) flushBuffer];
}

void MacOpenGLContext::swap_interval(int frames)
{
    if (!gl_context)
        return;

    ContextLock lock(gl_context);
    GLint interval = frames;
    [as_context(gl_context) setValues:&interval
                         forParameter:NSOpenGLContextParameterSwapInterval];
}

void MacOpenGLContext::make_current()
{
    if (!gl_context)
        return;

    [as_context(gl_context) makeCurrentContext];
}

bool MacOpenGLContext::ready()
{
    return gl_context != nullptr;
}

void MacOpenGLContext::deinit()
{
    if (!gl_context)
        return;

    NSOpenGLContext *context = (__bridge_transfer NSOpenGLContext *)gl_context;
    gl_context = nullptr;

    if ([NSOpenGLContext currentContext] == context)
        [NSOpenGLContext clearCurrentContext];

    [context clearDrawable];

    view = nullptr;
}
