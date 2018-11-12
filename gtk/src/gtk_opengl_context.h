#ifndef __GTK_OPENGL_CONTEXT_H
#define __GTK_OPENGL_CONTEXT_H

#include "gtk_2_3_compat.h"

class OpenGLContext
{
  public:
    virtual bool attach (GtkWidget *widget) = 0;
    virtual bool create_context () = 0;
    virtual void resize () = 0;
    virtual void swap_buffers () = 0;
    virtual void swap_interval (int frames) = 0;
    virtual void make_current () = 0;

    int x;
    int y;
    int width;
    int height;
};

#endif
