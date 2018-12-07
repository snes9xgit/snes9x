/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_DISPLAY_DRIVER_GTK_H
#define __GTK_DISPLAY_DRIVER_GTK_H

#include "gtk_s9x.h"
#include "gtk_display_driver.h"

class S9xGTKDisplayDriver : public S9xDisplayDriver
{
    public:
        S9xGTKDisplayDriver (Snes9xWindow *window, Snes9xConfig *config);
        void refresh (int width, int height);
        int init ();
        void deinit ();
        void clear_buffers ();
        void update (int width, int height, int yoffset);
        uint16 *get_next_buffer ();
        uint16 *get_current_buffer ();
        void push_buffer (uint16 *src);
        void *get_parameters () { return NULL; }
        void save (const char *filename) { }
        bool is_ready () { return true; }

    private:
        void clear ();
        void output (void *src,
                     int src_pitch,
                     int x,
                     int y,
                     int width,
                     int height,
                     int dst_width,
                     int dst_height);

        int last_known_width;
        int last_known_height;
};

#endif /* __GTK_DISPLAY_DRIVER_GTK_H */
