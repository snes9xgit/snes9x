#ifndef __GTK_DISPLAY_DRIVER_XV_H
#define __GTK_DISPLAY_DRIVER_XV_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>

#include "gtk_s9x.h"
#include "gtk_display_driver.h"

#define FOURCC_YUY2 0x32595559

class S9xXVDisplayDriver : public S9xDisplayDriver
{
    public:
        S9xXVDisplayDriver (Snes9xWindow *window, Snes9xConfig *config);
        void refresh (int width, int height);
        int init (void);
        void deinit (void);
        void clear_buffers (void);
        void update (int width, int height);
        uint16 *get_next_buffer (void);
        uint16 *get_current_buffer (void);
        void push_buffer (uint16 *src);
        void reconfigure (int width, int height);
        static int query_availability (void);

    private:
        void clear (void);
        void update_image_size (int width, int height);
        void resize_window (int width, int height);
        void create_window (int width, int height);

        Display *display;
        Window xwindow;
        GC xgc;
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
        int desired_width;
        int desired_height;
        int format;
        int bpp;
        int bytes_per_pixel;
        int rshift, gshift, bshift;

        uint8 y_table[65536];
        uint8 u_table[65536];
        uint8 v_table[65536];
};

#endif /* __GTK_DISPLAY_DRIVER_XV_H */
