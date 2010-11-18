#ifndef __GTK_DISPLAY_DRIVER_GTK_H
#define __GTK_DISPLAY_DRIVER_GTK_H

#include "gtk_s9x.h"
#include "gtk_display_driver.h"

class S9xGTKDisplayDriver : public S9xDisplayDriver
{
    public:
        S9xGTKDisplayDriver (Snes9xWindow *window, Snes9xConfig *config);
        void refresh (int width, int height);
        int init (void);
        void deinit (void);
        void clear_buffers (void);
        void update (int width, int height);
        uint16 *get_next_buffer (void);
        uint16 *get_current_buffer (void);
        void push_buffer (uint16 *src);
        void reconfigure (int width, int height);

    private:
        void clear (void);
        void output (void *src,
                     int src_pitch,
                     int x,
                     int y,
                     int width,
                     int height,
                     int dst_width,
                     int dst_height);

        GdkPixbuf *pixbuf;
        int gdk_buffer_width;
        int gdk_buffer_height;
        int last_known_width;
        int last_known_height;
};

#endif /* __GTK_DISPLAY_DRIVER_GTK_H */
