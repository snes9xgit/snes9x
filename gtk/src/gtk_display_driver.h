#ifndef __GTK_DISPLAY_DRIVER_H
#define __GTK_DISPLAY_DRIVER_H

#include "gtk_s9x.h"

class S9xDisplayDriver
{
    public:
        virtual ~S9xDisplayDriver() {}
        virtual void refresh (int width, int height) = 0;
        virtual int init (void) = 0;
        virtual void deinit (void) = 0;
        virtual void clear_buffers (void) = 0;
        virtual void update (int width, int height) = 0;
        virtual uint16 *get_next_buffer (void) = 0;
        virtual uint16 *get_current_buffer (void) = 0;
        virtual void push_buffer (uint16 *src) = 0;
        virtual void reconfigure (int width, int height) = 0;

        /* Namespaced sizing constants */
        static const int image_width = 1024;
        static const int image_height = 478;
        static const int image_bpp = 2;
        static const int scaled_max_width = 1024;
        static const int scaled_max_height = 956;

        static const int image_size = image_width * image_height * image_bpp;
        static const int image_padded_size = (image_width + 8) *
                                             (image_height + 8) *
                                             image_bpp;
        static const int scaled_size = scaled_max_width *
                                       scaled_max_height *
                                       image_bpp;
        static const int scaled_padded_size = (scaled_max_width + 8) *
                                              (scaled_max_height + 8) *
                                              image_bpp;

    protected:
        Snes9xWindow *window;
        Snes9xConfig *config;
        GtkWidget    *drawing_area;
        void         *padded_buffer[4];
        void         *buffer[4];
};

#endif /* __GTK_DISPLAY_DRIVER_H*/
