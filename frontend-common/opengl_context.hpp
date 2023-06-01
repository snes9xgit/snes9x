/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_OPENGL_CONTEXT_H
#define __GTK_OPENGL_CONTEXT_H

class OpenGLContext
{
  public:
    virtual ~OpenGLContext(){};
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

#endif
