#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "gtk_display.h"
#include "gtk_display_driver_xv.h"

static int
get_inv_shift (uint32 mask, int bpp)
{
    int i;

    /* Find mask */
    for (i = 0; (i < bpp) && !(mask & (1 << i)); i++) {};

    /* Find start of mask */
    for (; (i < bpp) && (mask & (1 << i)); i++) {};

    return (bpp - i);
}

S9xXVDisplayDriver::S9xXVDisplayDriver (Snes9xWindow *window,
                                          Snes9xConfig *config)
{
    this->window = window;
    this->config = config;
    this->drawing_area = GTK_WIDGET (window->drawing_area);
    display =
        gdk_x11_display_get_xdisplay (gtk_widget_get_display (drawing_area));
    last_known_width = last_known_height = -1;

    return;
}

void
S9xXVDisplayDriver::resize_window (int width, int height)
{
    g_object_unref (gdk_window);
    XDestroyWindow (display, xwindow);
    create_window (width, height);

    return;
}

void
S9xXVDisplayDriver::create_window (int width, int height)
{
    XSetWindowAttributes window_attr;

    window_attr.colormap = xcolormap;
    window_attr.border_pixel = 0;
    window_attr.event_mask = StructureNotifyMask | ExposureMask;
    window_attr.background_pixmap = None;

    xwindow = XCreateWindow (display,
                             GDK_COMPAT_WINDOW_XID (gtk_widget_get_window (drawing_area)),
                             0,
                             0,
                             width,
                             height,
                             0,
                             vi->depth,
                             InputOutput,
                             vi->visual,
                             CWColormap | CWBorderPixel | CWBackPixmap | CWEventMask,
                             &window_attr);
    XSync (display, False);

    output_window_width = width;
    output_window_height = height;

    XMapWindow (display, xwindow);
    XSync (display, False);

#if USE_GTK3
    gdk_window = gdk_x11_window_foreign_new_for_display (gtk_widget_get_display (drawing_area), xwindow);
#else
    gdk_window = gdk_window_foreign_new (xwindow);
#endif
    XSync (display, False);

    gdk_window_set_user_data (gdk_window, drawing_area);
}

void
S9xXVDisplayDriver::update (int width, int height)
{
    int   current_width, current_height, final_pitch;
    uint8 *final_buffer;
    int   dst_x, dst_y, dst_width, dst_height;
    GtkAllocation allocation;

    gtk_widget_get_allocation (drawing_area, &allocation);
#if GTK_CHECK_VERSION(3,10,0)
    int gdk_scale_factor = gdk_window_get_scale_factor (gdk_window);

    allocation.width *= gdk_scale_factor;
    allocation.height *= gdk_scale_factor;

#endif

    current_width = allocation.width;
    current_height = allocation.height;

    if (output_window_width  != current_width ||
        output_window_height != current_height)
    {
        resize_window (current_width, current_height);
    }

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

    update_image_size (width, height);

    if (format == FOURCC_YUY2)
    {
        S9xConvertYUV (final_buffer,
                       (uint8 *) xv_image->data,
                       final_pitch,
                       2 * xv_image->width,
                       width + (width < xv_image->width ? (width % 2) + 4 : 0),
                       height + (height < xv_image->height ? 4 : 0));
    }
    else
    {
        S9xConvertMask (final_buffer,
                        (uint8 *) xv_image->data,
                        final_pitch,
                        bytes_per_pixel * xv_image->width,
                        width + (width < xv_image->width ? (width % 2) + 4 : 0),
                        height + (height < xv_image->height ? 4 : 0),
                        rshift,
                        gshift,
                        bshift,
                        bpp);
    }

    dst_x = width; dst_y = height;
    dst_width = current_width; dst_height = current_height;
    S9xApplyAspect (dst_x, dst_y, dst_width, dst_height);

    if (last_known_width != dst_width || last_known_height != dst_height)
    {
        last_known_width = dst_width;
        last_known_height = dst_height;
        clear ();
    }

    XvShmPutImage (display,
                   xv_portid,
                   xwindow,
                   XDefaultGC (display, XDefaultScreen (display)),
                   xv_image,
                   0,
                   0,
                   width,
                   height,
                   dst_x,
                   dst_y,
                   dst_width,
                   dst_height,
                   False);

    top_level->set_mouseable_area (dst_x, dst_y, dst_width, dst_height);

    XSync (display, False);

    return;
}

void
S9xXVDisplayDriver::update_image_size (int width, int height)
{
    if (desired_width != width || desired_height != height)
    {
        XShmDetach (display, &shm);
        XSync (display, 0);

        shmctl (shm.shmid, IPC_RMID, 0);
        shmdt (shm.shmaddr);

        xv_image = XvShmCreateImage (display,
                                     xv_portid,
                                     format,
                                     0,
                                     width,
                                     height,
                                     &shm);

        shm.shmid = shmget (IPC_PRIVATE, xv_image->data_size, IPC_CREAT | 0777);
        for (int tries = 0; tries <= 10; tries++)
        {
            shm.shmaddr = (char *) shmat (shm.shmid, 0, 0);

            if (shm.shmaddr == (void *) -1 && tries >= 10)
            {
                /* Can't recover, send exit. */
                fprintf (stderr, "Couldn't reallocate shared memory.\n");
                S9xExit ();
            }
            else if (shm.shmaddr != (void *) -1)
            {
                break;
            }
        }

        shm.readOnly = FALSE;

        xv_image->data = shm.shmaddr;

        XShmAttach (display, &shm);

        desired_width = width;
        desired_height = height;
    }

    return;
}

int
S9xXVDisplayDriver::init (void)
{
    int                 padding;
    int                 depth = 0, num_formats, num_attrs, highest_formats = 0;
    XvImageFormatValues *formats = NULL;
    XvAdaptorInfo       *adaptors;
    XvAttribute         *port_attr;
    VisualID            visualid = None;
    unsigned int        num_adaptors;
    GdkScreen           *screen;
    GdkWindow           *root;

    buffer[0] = malloc (image_padded_size);
    buffer[1] = malloc (scaled_padded_size);

    padding = (image_padded_size - image_size) / 2;
    padded_buffer[0] = (void *) (((uint8 *) buffer[0]) + padding);

    padding = (scaled_padded_size - scaled_size) / 2;
    padded_buffer[1] = (void *) (((uint8 *) buffer[1]) + padding);

    memset (buffer[0], 0, image_padded_size);
    memset (buffer[1], 0, scaled_padded_size);

    /* Setup XV */
    gtk_widget_realize (drawing_area);

    display = gdk_x11_display_get_xdisplay (gtk_widget_get_display (drawing_area));
    screen = gtk_widget_get_screen (drawing_area);
    root = gdk_screen_get_root_window (screen);

    xv_portid = -1;
    XvQueryAdaptors (display,
                     GDK_COMPAT_WINDOW_XID (root),
                     &num_adaptors,
                     &adaptors);


    for (int i = 0; i < (int) num_adaptors; i++)
    {
        if (adaptors[i].type & XvInputMask &&
            adaptors[i].type & XvImageMask)
        {
            formats = XvListImageFormats (display,
                                          adaptors[i].base_id,
                                          &num_formats);

            if (num_formats > highest_formats)
            {
                xv_portid = adaptors[i].base_id;
                highest_formats = num_formats;
                visualid = adaptors[i].formats->visual_id;
            }

            free (formats);
        }
    }

    XvFreeAdaptorInfo (adaptors);

    if (xv_portid < 0)
    {
        fprintf (stderr, "Could not open Xv output port.\n");
        return -1;
    }

    /* Set XV_AUTOPAINT_COLORKEY _only_ if available */
    port_attr = XvQueryPortAttributes (display, xv_portid, &num_attrs);

    for (int i = 0; i < num_attrs; i++)
    {
        if (!strcmp (port_attr[i].name, "XV_AUTOPAINT_COLORKEY"))
        {
            Atom colorkey;

            colorkey = XInternAtom (display, "XV_AUTOPAINT_COLORKEY", True);
            if (colorkey != None)
                XvSetPortAttribute (display, xv_portid, colorkey, 1);
        }
    }

    /* Try to find an RGB format */
    format = FOURCC_YUY2;
    bpp = 100;

    formats = XvListImageFormats (display,
                                  xv_portid,
                                  &num_formats);

    for (int i = 0; i < num_formats; i++)
    {
        if (formats[i].id == 0x3 || formats[i].type == XvRGB)
        {
            if (formats[i].bits_per_pixel < bpp)
            {
                format = formats[i].id;
                bpp = formats[i].bits_per_pixel;
                bytes_per_pixel = (bpp == 15) ? 2 : bpp >> 3;
                depth = formats[i].depth;

                this->rshift = get_inv_shift (formats[i].red_mask, bpp);
                this->gshift = get_inv_shift (formats[i].green_mask, bpp);
                this->bshift = get_inv_shift (formats[i].blue_mask, bpp);

                /* Check for red-blue inversion on SiliconMotion drivers */
                if (formats[i].red_mask  == 0x001f &&
                    formats[i].blue_mask == 0x7c00)
                {
                    int copy = this->rshift;
                    this->rshift = this->bshift;
                    this->bshift = copy;
                }

                /* on big-endian Xv still seems to like LSB order */
                if (config->force_inverted_byte_order)
                    S9xSetEndianess (ENDIAN_MSB);
                else
                    S9xSetEndianess (ENDIAN_LSB);
            }
        }
    }

    if (format == FOURCC_YUY2)
    {
        for (int i = 0; i < num_formats; i++)
        {
            if (formats[i].id == FOURCC_YUY2)
            {
                depth = formats[i].depth;

                if (formats[i].byte_order == LSBFirst)
                {
                    if (config->force_inverted_byte_order)
                        S9xSetEndianess (ENDIAN_MSB);
                    else
                        S9xSetEndianess (ENDIAN_LSB);
                }
                else
                {
                    if (config->force_inverted_byte_order)
                        S9xSetEndianess (ENDIAN_LSB);
                    else
                        S9xSetEndianess (ENDIAN_MSB);
                }

                break;
            }
        }
    }

    free (formats);

    /* Build a table for yuv conversion */
    if (format == FOURCC_YUY2)
    {
        for (unsigned int color = 0; color < 65536; color++)
        {
            int r, g, b;
            int y, u, v;

            r = (color & 0x7c00) >> 7;
            g = (color & 0x03e0) >> 2;
            b = (color & 0x001F) << 3;

            y = (int) ((0.257  * ((double) r)) + (0.504  * ((double) g)) + (0.098  * ((double) b)) + 16.0);
            u = (int) ((-0.148 * ((double) r)) + (-0.291 * ((double) g)) + (0.439  * ((double) b)) + 128.0);
            v = (int) ((0.439  * ((double) r)) + (-0.368 * ((double) g)) + (-0.071 * ((double) b)) + 128.0);

            y_table[color] = CLAMP (y, 0, 255);
            u_table[color] = CLAMP (u, 0, 255);
            v_table[color] = CLAMP (v, 0, 255);
        }

        S9xRegisterYUVTables (y_table, u_table, v_table);
    }

    /* Create a sub-window */
    XVisualInfo vi_template;
    int vi_num_items;

    vi_template.visualid = visualid;
    vi_template.depth = depth;
    vi_template.visual = NULL;
    vi = XGetVisualInfo (display, VisualIDMask | VisualDepthMask, &vi_template, &vi_num_items);

    if (!vi)
    {
        vi_template.depth = 0;
        vi = XGetVisualInfo (display, VisualIDMask, &vi_template, &vi_num_items);

        if (!vi)
        {
            fprintf (stderr, "Couldn't map visual.\n");
            return -1;
        }
    }

    xcolormap = XCreateColormap (display,
                                GDK_COMPAT_WINDOW_XID (gtk_widget_get_window (drawing_area)),
                                vi->visual,
                                AllocNone);

    create_window (1, 1);
    gdk_window_hide (gdk_window);

    /* Allocate a shared memory image. */
    xv_image = XvShmCreateImage (display,
                                 xv_portid,
                                 format,
                                 0,
                                 scaled_max_width,
                                 scaled_max_width,
                                 &shm);

    shm.shmid = shmget (IPC_PRIVATE, xv_image->data_size, IPC_CREAT | 0777);
    shm.shmaddr = (char *) shmat (shm.shmid, 0, 0);
    if (shm.shmaddr == (void *) -1)
    {
        fprintf (stderr, "Could not attach shared memory segment.\n");
        return -1;
    }

    shm.readOnly = FALSE;

    xv_image->data = shm.shmaddr;

    XShmAttach (display, &shm);

    desired_width = scaled_max_width;
    desired_height = scaled_max_width;

    clear_buffers ();

    /* Give Snes9x core a pointer to draw on */
    GFX.Screen = (uint16 *) padded_buffer[0];
    GFX.Pitch = image_width * image_bpp;

    return 0;
}

void
S9xXVDisplayDriver::deinit (void)
{
    XShmDetach (display, &shm);
    XSync (display, 0);

    XFreeColormap (display, xcolormap);
    XFree (vi);

    g_object_unref (gdk_window);
    XDestroyWindow (display, xwindow);

    free (buffer[0]);
    free (buffer[1]);

    shmctl (shm.shmid, IPC_RMID, 0);
    shmdt (shm.shmaddr);

    padded_buffer[0] = NULL;
    padded_buffer[1] = NULL;

    return;
}

void
S9xXVDisplayDriver::clear (void)
{
    int  x, y, w, h;
    int  width, height;
    GtkAllocation allocation;
    GC   xgc = XDefaultGC (display, XDefaultScreen (display));

    gtk_widget_get_allocation (drawing_area, &allocation);
#if GTK_CHECK_VERSION(3,10,0)
    int gdk_scale_factor = gdk_window_get_scale_factor (gdk_window);

    allocation.width *= gdk_scale_factor;
    allocation.height *= gdk_scale_factor;

#endif
    width = allocation.width;
    height = allocation.height;

    if (window->last_width <= 0 || window->last_height <= 0)
    {
        XDrawRectangle (display, xwindow, xgc, 0, 0, width, height);
        return;
    }

    /* Get width of modified display */
    x = window->last_width;
    y = window->last_height;
    get_filter_scale (x, y);
    w = width;
    h = height;
    S9xApplyAspect (x, y, w, h);

    if (x > 0)
    {
        XFillRectangle (display, xwindow, xgc, 0, y, x, h);
    }
    if (x + w < width)
    {
        XFillRectangle (display, xwindow, xgc, x + w, y, width - (x + w), h);
    }
    if (y > 0)
    {
        XFillRectangle (display, xwindow, xgc, 0, 0, width, y);
    }
    if (y + h < height)
    {
        XFillRectangle (display, xwindow, xgc, 0, y + h, width, height - (y + h));
    }

    XSync (display, False);

    return;
}

void
S9xXVDisplayDriver::refresh (int width, int height)
{
    clear ();

    return;
}

uint16 *
S9xXVDisplayDriver::get_next_buffer (void)
{
    return (uint16 *) padded_buffer[0];
}

uint16 *
S9xXVDisplayDriver::get_current_buffer (void)
{
    return get_next_buffer ();
}

void
S9xXVDisplayDriver::push_buffer (uint16 *src)
{
    memmove (GFX.Screen, src, image_size);
    update (window->last_width, window->last_height);

    return;
}

void
S9xXVDisplayDriver::clear_buffers (void)
{
    uint32 black;
    uint8  *color;

    memset (buffer[0], 0, image_padded_size);
    memset (buffer[1], 0, scaled_padded_size);

    /* Construct value byte-order independently */
    if (format == FOURCC_YUY2)
    {
        color = (uint8 *) &black;
        *color++ = y_table[0];
        *color++ = u_table[0];
        *color++ = y_table[0];
        *color++ = v_table[0];

        for (int i = 0; i < xv_image->data_size >> 2; i++)
        {
            *(((uint32 *) xv_image->data) + i) = black;
        }
    }
    else
    {
        memset (xv_image->data, 0, xv_image->data_size);
    }

    return;
}

int
S9xXVDisplayDriver::query_availability (void)
{
    unsigned int p_version,
                 p_release,
                 p_request_base,
                 p_event_base,
                 p_error_base;
    Display *display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    /* Test if XV and SHM are feasible */
    if (!XShmQueryExtension (display))
    {
        return 0;
    }

    if (XvQueryExtension (display,
                          &p_version,
                          &p_release,
                          &p_request_base,
                          &p_event_base,
                          &p_error_base) != Success)
    {
        return 0;
    }

    return 1;
}

void
S9xXVDisplayDriver::reconfigure (int width, int height)
{
    return;
}
