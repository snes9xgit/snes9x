/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_DISPLAY_DRIVER_XV_H
#define __GTK_DISPLAY_DRIVER_XV_H

#include "gtk_s9x.h"
#include "gtk_display_driver.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>

const uint32 FOURCC_YUY2 = 0x32595559;

class S9xXVDisplayDriver : public S9xDisplayDriver
{
  public:
    S9xXVDisplayDriver(Snes9xWindow *window, Snes9xConfig *config);
    void refresh();
    int init();
    void deinit();
    void update(uint16_t *buffer, int width, int height, int stride_in_pixels);
    void *get_parameters() { return nullptr; }
    void save(const char *filename) {}
    static int query_availability();
    bool is_ready() { return true; }
    int get_width() { return output_window_width; }
    int get_height() { return output_window_height; }

  private:
    void clear();
    void update_image_size(int width, int height);
    void resize_window(int width, int height);
    void create_window(int width, int height);

    Display *display;
    Window xwindow;
    Colormap xcolormap;
    XVisualInfo *vi;
    GdkWindow *gdk_window;

    XvImage *xv_image;
    int xv_portid;
    XShmSegmentInfo shm;

    int output_window_width;
    int output_window_height;
    int last_known_width;
    int last_known_height;
    int xv_image_width;
    int xv_image_height;
    int format;
    int bpp;
    int bytes_per_pixel;
    int rshift, gshift, bshift;

    uint8 y_table[65536];
    uint8 u_table[65536];
    uint8 v_table[65536];
};

#endif /* __GTK_DISPLAY_DRIVER_XV_H */
