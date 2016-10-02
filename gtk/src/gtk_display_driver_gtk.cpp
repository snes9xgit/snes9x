#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>
#include "gtk_display.h"
#include "gtk_display_driver_gtk.h"


S9xGTKDisplayDriver::S9xGTKDisplayDriver (Snes9xWindow *window,
                                          Snes9xConfig *config)
{
    this->window = window;
    this->config = config;
    this->drawing_area = GTK_WIDGET (window->drawing_area);
    this->pixbuf = NULL;

    return;
}

void
S9xGTKDisplayDriver::update (int width, int height)
{
    int           x, y, w, h;
    int           c_width, c_height, final_pitch;
    uint8         *final_buffer;
    GtkAllocation allocation;

    gtk_widget_get_allocation (drawing_area, &allocation);
    c_width = allocation.width;
    c_height = allocation.height;

    if (width <= 0)
        return;

    if (config->scale_method > 0)
    {
        uint8 *src_buffer = (uint8 *) padded_buffer[0];
        uint8 *dst_buffer = (uint8 *) padded_buffer[1];
        int   src_pitch = image_width * image_bpp;
        int   dst_pitch = scaled_max_width * image_bpp;

        S9xFilter (src_buffer,
                   src_pitch,
                   dst_buffer,
                   dst_pitch,
                   width,
                   height);

        final_buffer = (uint8 *) padded_buffer[1];
        final_pitch = dst_pitch;
    }
    else
    {
        final_buffer = (uint8 *) padded_buffer[0];
        final_pitch = image_width * image_bpp;
    }

    x = width; y = height; w = c_width; h = c_height;
    S9xApplyAspect (x, y, w, h);

    output (final_buffer, final_pitch, x, y, width, height, w, h);

    return;
}

void
S9xGTKDisplayDriver::output (void *src,
                             int  src_pitch,
                             int  x,
                             int  y,
                             int  width,
                             int  height,
                             int  dst_width,
                             int  dst_height)
{
    if (width != gdk_buffer_width || height != gdk_buffer_height)
    {
        gdk_buffer_width = width;
        gdk_buffer_height = height;

        g_object_unref (pixbuf);

        padded_buffer[2] = realloc (padded_buffer[2],
                                    gdk_buffer_width * gdk_buffer_height * 3);
        pixbuf = gdk_pixbuf_new_from_data ((guchar *) padded_buffer[2],
                                           GDK_COLORSPACE_RGB,
                                           FALSE,
                                           8,
                                           gdk_buffer_width,
                                           gdk_buffer_height,
                                           gdk_buffer_width * 3,
                                           NULL,
                                           NULL);
    }

    if (last_known_width != dst_width || last_known_height != dst_height)
    {
        clear ();

        last_known_width = dst_width;
        last_known_height = dst_height;
    }

    S9xConvert (src,
                padded_buffer[2],
                src_pitch,
                gdk_buffer_width * 3,
                width,
                height,
                24);

    cairo_t *cr = gdk_cairo_create (gtk_widget_get_window (drawing_area));

    gdk_cairo_set_source_pixbuf (cr, pixbuf, x, y);

    if (width != dst_width || height != dst_height)
    {
        cairo_matrix_t matrix;
        cairo_pattern_t *pattern = cairo_get_source (cr);;

        cairo_matrix_init_identity (&matrix);
        cairo_matrix_scale (&matrix,
                            (double) width / (double) dst_width,
                            (double) height / (double) dst_height);
        cairo_matrix_translate (&matrix, -x, -y);
        cairo_pattern_set_matrix (pattern, &matrix);
        cairo_pattern_set_filter (pattern,
                                  config->bilinear_filter
                                       ? CAIRO_FILTER_BILINEAR
                                       : CAIRO_FILTER_NEAREST);
    }

    cairo_rectangle (cr, x, y, dst_width, dst_height);
    cairo_fill (cr);

    cairo_destroy (cr);

    window->set_mouseable_area (x, y, width, height);

    return;
}

int
S9xGTKDisplayDriver::init (void)
{
    int padding;
    GtkAllocation allocation;

    buffer[0] = malloc (image_padded_size);
    buffer[1] = malloc (scaled_padded_size);

    padding = (image_padded_size - image_size) / 2;
    padded_buffer[0] = (void *) (((uint8 *) buffer[0]) + padding);

    padding = (scaled_padded_size - scaled_size) / 2;
    padded_buffer[1] = (void *) (((uint8 *) buffer[1]) + padding);

    gtk_widget_get_allocation (drawing_area, &allocation);
    gdk_buffer_width = allocation.width;
    gdk_buffer_height = allocation.height;

    padded_buffer[2] = malloc (gdk_buffer_width * gdk_buffer_height * 3);
    pixbuf = gdk_pixbuf_new_from_data ((guchar *) padded_buffer[2],
                                       GDK_COLORSPACE_RGB,
                                       FALSE,
                                       8,
                                       gdk_buffer_width,
                                       gdk_buffer_height,
                                       gdk_buffer_width * 3,
                                       NULL,
                                       NULL);

    S9xSetEndianess (ENDIAN_MSB);

    memset (buffer[0], 0, image_padded_size);
    memset (buffer[1], 0, scaled_padded_size);

    GFX.Screen = (uint16 *) padded_buffer[0];
    GFX.Pitch = image_width * image_bpp;

    return 0;
}

void
S9xGTKDisplayDriver::deinit (void)
{
    padded_buffer[0] = NULL;
    padded_buffer[1] = NULL;

    free (buffer[0]);
    free (buffer[1]);

    g_object_unref (pixbuf);
    free (padded_buffer[2]);

    return;
}

void
S9xGTKDisplayDriver::clear (void)
{
    int  x, y, w, h;
    int  width, height;
    GtkAllocation allocation;

    gtk_widget_get_allocation (drawing_area, &allocation);
    width = allocation.width;
    height = allocation.height;

    cairo_t *cr = gdk_cairo_create (gtk_widget_get_window (drawing_area));

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);

    if (window->last_width <= 0 || window->last_height <= 0)
    {
        cairo_paint (cr);
        cairo_destroy (cr);

        return;
    }

    x = window->last_width;
    y = window->last_height;
    get_filter_scale (x, y);
    w = width;
    h = height;
    S9xApplyAspect (x, y, w, h);

    if (x > 0)
    {
        cairo_rectangle (cr, 0, y, x, h);
    }
    if (x + w < width)
    {
        cairo_rectangle (cr, x + w, y, width - (x + w), h);
    }
    if (y > 0)
    {
        cairo_rectangle (cr, 0, 0, width, y);
    }
    if (y + h < height)
    {
        cairo_rectangle (cr, 0, y + h, width, height - (y + h));
    }

    cairo_fill (cr);
    cairo_destroy (cr);

    return;
}

void
S9xGTKDisplayDriver::refresh (int width, int height)
{
    if (!config->rom_loaded)
        return;

    clear ();

    return;
}

uint16 *
S9xGTKDisplayDriver::get_next_buffer (void)
{
    return (uint16 *) padded_buffer[0];
}

uint16 *
S9xGTKDisplayDriver::get_current_buffer (void)
{
    return (uint16 *) padded_buffer[0];
}

void
S9xGTKDisplayDriver::push_buffer (uint16 *src)
{
    memmove (GFX.Screen, src, image_size);
    update (window->last_width, window->last_height);

    return;
}

void
S9xGTKDisplayDriver::clear_buffers (void)
{
    memset (buffer[0], 0, image_padded_size);
    memset (buffer[1], 0, scaled_padded_size);

    return;
}

void
S9xGTKDisplayDriver::reconfigure (int width, int height)
{
    return;
}
