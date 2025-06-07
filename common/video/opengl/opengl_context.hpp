/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

class OpenGLContext
{
  public:
    virtual ~OpenGLContext() = default;
    virtual bool create_context() = 0;
    virtual void resize() = 0;
    virtual void swap_buffers() = 0;
    virtual void swap_interval(int frames) = 0;
    virtual void make_current() = 0;
    virtual bool ready()
    {
        return true;
    };

    int x;
    int y;
    int width;
    int height;
};