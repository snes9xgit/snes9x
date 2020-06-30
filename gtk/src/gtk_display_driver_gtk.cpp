/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_2_3_compat.h"
#include <cairo.h>
#include "gtk_display.h"
#include "gtk_display_driver_gtk.h"

S9xGTKDisplayDriver::S9xGTKDisplayDriver(Snes9xWindow *window,
                                         Snes9xConfig *config)
{
    this->window = window;
    this->config = config;
    this->drawing_area = GTK_WIDGET(window->drawing_area);
}

void S9xGTKDisplayDriver::update(uint16_t *buffer, int width, int height, int stride_in_pixels)
{
    GtkAllocation allocation;

    if (width <= 0)
        return;

    gtk_widget_get_allocation(drawing_area, &allocation);

    S9xRect dst = S9xApplyAspect(width, height, allocation.width, allocation.height);
    output(buffer, stride_in_pixels * 2, dst.x, dst.y, width, height, dst.w, dst.h);
}

void S9xGTKDisplayDriver::output(void *src,
                                 int src_pitch,
                                 int x,
                                 int y,
                                 int width,
                                 int height,
                                 int dst_width,
                                 int dst_height)
{
    if (last_known_width != dst_width || last_known_height != dst_height)
    {
        clear();

        last_known_width = dst_width;
        last_known_height = dst_height;
    }

    cairo_t *cr = window->get_cairo();

    cairo_surface_t *surface;

    surface = cairo_image_surface_create_for_data((unsigned char *)src, CAIRO_FORMAT_RGB16_565, width, height, src_pitch);

    cairo_set_source_surface(cr, surface, 0, 0);

    if (width != dst_width || height != dst_height)
    {
        cairo_matrix_t matrix;
        cairo_pattern_t *pattern = cairo_get_source(cr);
        ;

        cairo_matrix_init_identity(&matrix);
        cairo_matrix_scale(&matrix,
                           (double)width / (double)dst_width,
                           (double)height / (double)dst_height);
        cairo_matrix_translate(&matrix, -x, -y);
        cairo_pattern_set_matrix(pattern, &matrix);
        cairo_pattern_set_filter(pattern,
                                 Settings.BilinearFilter
                                     ? CAIRO_FILTER_BILINEAR
                                     : CAIRO_FILTER_NEAREST);
    }

    cairo_rectangle(cr, x, y, dst_width, dst_height);
    cairo_fill(cr);

    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);

    window->release_cairo();
    window->set_mouseable_area(x, y, width, height);
}

int S9xGTKDisplayDriver::init()
{
    return 0;
}

void S9xGTKDisplayDriver::deinit()
{
}

void S9xGTKDisplayDriver::clear()
{
    int width, height;
    GtkAllocation allocation;

    gtk_widget_get_allocation(drawing_area, &allocation);
    width = allocation.width;
    height = allocation.height;

    cairo_t *cr = window->get_cairo();

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

    if (window->last_width <= 0 || window->last_height <= 0)
    {
        cairo_paint(cr);
        window->release_cairo();

        return;
    }

    S9xRect dst;
    dst.w = window->last_width;
    dst.h = window->last_height;
    get_filter_scale(dst.w, dst.h);
    dst = S9xApplyAspect(dst.w, dst.h, width, height);

    if (dst.x > 0)
    {
        cairo_rectangle(cr, 0, dst.y, dst.x, dst.h);
    }
    if (dst.x + dst.w < width)
    {
        cairo_rectangle(cr, dst.x + dst.w, dst.y, width - (dst.x + dst.w), dst.h);
    }
    if (dst.y > 0)
    {
        cairo_rectangle(cr, 0, 0, width, dst.y);
    }
    if (dst.y + dst.h < height)
    {
        cairo_rectangle(cr, 0, dst.y + dst.h, width, height - (dst.y + dst.h));
    }

    cairo_fill(cr);

    window->release_cairo();
}

void S9xGTKDisplayDriver::refresh(int width, int height)
{
    clear();
}