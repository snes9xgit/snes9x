/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_DISPLAY_DRIVER_H
#define __GTK_DISPLAY_DRIVER_H

#include "gtk_s9x.h"

class S9xDisplayDriver
{
  public:
    virtual ~S9xDisplayDriver()
    {
    }
    virtual void refresh(int width, int height) = 0;
    virtual int init() = 0;
    virtual void deinit() = 0;
    virtual void update(uint16_t *buffer, int width, int height, int stride_in_pixels) = 0;
    virtual void *get_parameters() = 0;
    virtual void save(const char *filename) = 0;
    virtual bool is_ready() = 0;

  protected:
    Snes9xWindow *window;
    Snes9xConfig *config;
    GtkWidget *drawing_area;
};

#endif /* __GTK_DISPLAY_DRIVER_H*/
