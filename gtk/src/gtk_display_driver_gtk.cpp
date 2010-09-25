#include <gtk/gtk.h>
#include "gtk_display.h"
#include "gtk_display_driver_gtk.h"


S9xGTKDisplayDriver::S9xGTKDisplayDriver (Snes9xWindow *window,
                                          Snes9xConfig *config)
{
    this->window = window;
    this->config = config;
    this->drawing_area = GTK_WIDGET (window->drawing_area);

    return;
}

void
S9xGTKDisplayDriver::update (int width, int height)
{
    int   c_width, c_height, final_pitch;
    uint8 *final_buffer;

    c_width = drawing_area->allocation.width;
    c_height = drawing_area->allocation.height;

    if (width == SIZE_FLAG_DIRTY)
    {
        this->clear ();
        return;
    }

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

    if (!config->scale_to_fit &&
            (width > gdk_buffer_width || height > gdk_buffer_height))
    {
        this->clear ();

        return;
    }

    if (config->scale_to_fit)
    {
        double screen_aspect = (double) c_width / (double) c_height;
        double snes_aspect = S9xGetAspect ();
        double granularity = 1.0 / (double) MAX (c_width, c_height);

        if (config->maintain_aspect_ratio &&
            !(screen_aspect <= snes_aspect * (1.0 + granularity) &&
              screen_aspect >= snes_aspect * (1.0 - granularity)))
        {
            if (screen_aspect > snes_aspect)
            {
                output (final_buffer,
                        final_pitch,
                        (c_width - (int) (c_height * snes_aspect)) / 2,
                        0,
                        width,
                        height,
                        (int) (c_height * snes_aspect),
                        c_height);
            }
            else
            {
                output (final_buffer,
                        final_pitch,
                        0,
                        (c_height - c_width / snes_aspect) / 2,
                        width,
                        height,
                        c_width,
                        (c_width / snes_aspect));
            }
        }
        else
        {
            output (final_buffer,
                    final_pitch,
                    0,
                    0,
                    width,
                    height,
                    c_width,
                    c_height);
        }
    }
    else
    {
        output (final_buffer,
                final_pitch,
                (c_width - width) / 2,
                (c_height - height) / 2,
                width,
                height,
                width,
                height);
    }

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
    GdkGC *gc = drawing_area->style->bg_gc[GTK_WIDGET_STATE (drawing_area)];

    if (width != dst_width || height != dst_height)
    {
        S9xConvertScale (src,
                padded_buffer[2],
                src_pitch,
                gdk_buffer_width * 3,
                width,
                height,
                dst_width, dst_height,
                24);
    }
    else
    {
        S9xConvert (src,
                padded_buffer[2],
                src_pitch,
                gdk_buffer_width * 3,
                width,
                height,
                24);
    }

    gdk_draw_rgb_image (drawing_area->window,
                        gc,
                        x, y,
                        dst_width, dst_height,
                        GDK_RGB_DITHER_NORMAL,
                        (guchar *) padded_buffer[2],
                        gdk_buffer_width * 3);

    window->set_mouseable_area (x, y, width, height);

    return;
}

int
S9xGTKDisplayDriver::init (void)
{
    int padding;

    buffer[0] = malloc (image_padded_size);
    buffer[1] = malloc (scaled_padded_size);

    padding = (image_padded_size - image_size) / 2;
    padded_buffer[0] = (void *) (((uint8 *) buffer[0]) + padding);

    padding = (scaled_padded_size - scaled_size) / 2;
    padded_buffer[1] = (void *) (((uint8 *) buffer[1]) + padding);

    gdk_buffer_width = drawing_area->allocation.width;
    gdk_buffer_height = drawing_area->allocation.height;

    padded_buffer[2] = malloc (gdk_buffer_width * gdk_buffer_height * 3);
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
    free (padded_buffer[2]);

    return;
}

void
S9xGTKDisplayDriver::clear (void)
{
    int      w, h;
    int      c_width = drawing_area->allocation.width;
    int      c_height = drawing_area->allocation.height;
    GdkColor black = { 0, 0, 0, 0 };
    GdkGC    *gc = NULL;

    gc = drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)];
    gdk_gc_set_rgb_fg_color (gc, &black);

    if (window->last_width <= 0 || window->last_height <= 0)
    {
        gdk_draw_rectangle (drawing_area->window,
                            gc,
                            TRUE,
                            0, 0,
                            c_width, c_height);
        return;
    }

    /* Get width of modified display */
    w = window->last_width;
    h = window->last_height;
    get_filter_scale (w, h);

    if (config->scale_to_fit)
    {
        double screen_aspect = (double) c_width / (double) c_height;
        double snes_aspect = S9xGetAspect ();
        double granularity = 1.0 / (double) MAX (c_width, c_height);

        if (config->maintain_aspect_ratio &&
            !(screen_aspect <= snes_aspect * (1.0 + granularity) &&
              screen_aspect >= snes_aspect * (1.0 - granularity)))
        {
            int    bar_size;
            if (screen_aspect > snes_aspect)
            {
                /* Black bars on left and right */
                w = (int) (c_height * snes_aspect);
                bar_size = (c_width - w) / 2;

                gdk_draw_rectangle (drawing_area->window,
                                    gc,
                                    TRUE,
                                    0, 0,
                                    bar_size, c_height);
                gdk_draw_rectangle (drawing_area->window,
                                    gc,
                                    TRUE,
                                    bar_size + w, 0,
                                    c_width - bar_size - w,
                                    c_height);
            }
            else
            {
                /* Black bars on top and bottom */
                h = (int) (c_width / snes_aspect);
                bar_size = (c_height - h) / 2;
                gdk_draw_rectangle (drawing_area->window,
                                    gc,
                                    TRUE,
                                    0, 0,
                                    c_width, bar_size);
                gdk_draw_rectangle (drawing_area->window,
                                    gc,
                                    TRUE,
                                    0, bar_size + h,
                                    c_width,
                                    c_height - bar_size - h);
            }
        }
        else
            return;
    }
    else
    {
        /* Black bars on top, bottom, left, and right :-) */
        int bar_width, bar_height;

        bar_height = (c_height - h) / 2;
        bar_width = (c_width - w) / 2;

        gdk_draw_rectangle (drawing_area->window,
                            gc,
                            TRUE,
                            0, 0,
                            c_width, bar_height);
        gdk_draw_rectangle (drawing_area->window,
                            gc,
                            TRUE,
                            0,
                            bar_height + h,
                            c_width,
                            c_height - (bar_height + h));
        gdk_draw_rectangle (drawing_area->window,
                            gc,
                            TRUE,
                            0, bar_height,
                            bar_width, h);
        gdk_draw_rectangle (drawing_area->window,
                            gc,
                            TRUE,
                            bar_width + w, bar_height,
                            c_width - (bar_width + w),
                            h);
    }

    return;
}

void
S9xGTKDisplayDriver::refresh (int width, int height)
{
    int c_width, c_height;

    c_width = drawing_area->allocation.width;
    c_height = drawing_area->allocation.height;

    if (c_width != gdk_buffer_width || c_height != gdk_buffer_height)
    {
        free (padded_buffer[2]);

        gdk_buffer_width = c_width;
        gdk_buffer_height = c_height;

        padded_buffer[2] = malloc (gdk_buffer_width * gdk_buffer_height * 3);
    }

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
