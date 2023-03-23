/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <cstdlib>
#include <cstdio>

#include "wgl_context.hpp"

WGLContext::WGLContext()
{
    hwnd = nullptr;
    hdc = nullptr;
    hglrc = nullptr;
    version_major = -1;
    version_minor = -1;
}

WGLContext::~WGLContext()
{
    deinit();
}

void WGLContext::deinit()
{
    if (wglMakeCurrent)
        wglMakeCurrent(nullptr, nullptr);
    if (hglrc)
        wglDeleteContext(hglrc);
    if (hdc)
        ReleaseDC(hwnd, hdc);
}

bool WGLContext::attach(HWND target)
{
    hwnd = target;
    hdc  = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    auto pfdIndex = ChoosePixelFormat(hdc, &pfd);
    if (!pfdIndex)
        return false;

    if (!SetPixelFormat(hdc, pfdIndex, &pfd))
    {
        // Shouldn't happen
    }

    return true;
}

bool WGLContext::create_context()
{
    hglrc = wglCreateContext(hdc);
    if (!hglrc)
        return false;

    if (!wglMakeCurrent(hdc, hglrc))
        return false;

    gladLoaderLoadWGL(hdc);

    resize();

    return true;
}

void WGLContext::resize()
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    this->width = rect.right - rect.left;
    this->height = rect.bottom - rect.top;
    make_current();
}

void WGLContext::swap_buffers()
{
    SwapBuffers(hdc);
}

bool WGLContext::ready()
{
    return true;
}

void WGLContext::make_current()
{
    wglMakeCurrent(hdc, hglrc);
}

void WGLContext::swap_interval(int frames)
{
    wglSwapIntervalEXT(frames);
}
