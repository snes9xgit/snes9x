/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_compat.h"
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "gtk_display.h"
#include "gtk_display_driver_xv.h"

static int get_inv_shift(uint32 mask, int bpp)
{
    int i;

    /* Find mask */
    for (i = 0; (i < bpp) && !(mask & (1 << i)); i++)
    {
    };

    /* Find start of mask */
    for (; (i < bpp) && (mask & (1 << i)); i++)
    {
    };

    return (bpp - i);
}

S9xXVDisplayDriver::S9xXVDisplayDriver(Snes9xWindow *window, Snes9xConfig *config)
{
    this->window = window;
    this->config = config;
    this->drawing_area = window->drawing_area;
    display = gdk_x11_display_get_xdisplay(drawing_area->get_display()->gobj());
    last_known_width = -1;
    last_known_height = -1;
}

void S9xXVDisplayDriver::resize_window(int width, int height)
{
    gdk_window_destroy(gdk_window);
    create_window(width, height);
}

void S9xXVDisplayDriver::create_window(int width, int height)
{
    GdkWindowAttr window_attr;
    memset(&window_attr, 0, sizeof(GdkWindowAttr));
    window_attr.event_mask = GDK_EXPOSURE_MASK | GDK_STRUCTURE_MASK;
    window_attr.width = width;
    window_attr.height = height;
    window_attr.x = 0;
    window_attr.y = 0;
    window_attr.wclass = GDK_INPUT_OUTPUT;
    window_attr.window_type = GDK_WINDOW_CHILD;
    window_attr.visual = gdk_x11_screen_lookup_visual(drawing_area->get_screen()->gobj(), vi->visualid);

    gdk_window = gdk_window_new(drawing_area->get_window()->gobj(),
                                &window_attr,
                                GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL);

    gdk_window_set_user_data(gdk_window, (gpointer)drawing_area->gobj());

    gdk_window_show(gdk_window);
    xwindow = gdk_x11_window_get_xid(gdk_window);

    output_window_width = width;
    output_window_height = height;
}

void S9xXVDisplayDriver::update(uint16_t *buffer, int width, int height, int stride_in_pixels)
{
    int current_width, current_height;

    auto allocation = drawing_area->get_allocation();

    if (output_window_width != allocation.get_width() ||
        output_window_height != allocation.get_height())
    {
        resize_window(allocation.get_width(), allocation.get_height());
    }

    int scale_factor = drawing_area->get_scale_factor();

    allocation.set_width(allocation.get_width() * scale_factor);
    allocation.set_height(allocation.get_height() * scale_factor);

    current_width = allocation.get_width();
    current_height = allocation.get_height();

    update_image_size(width, height);

    if (format == FOURCC_YUY2)
    {
        S9xConvertYUV(buffer,
                      (uint8 *)xv_image->data,
                      stride_in_pixels * 2,
                      2 * xv_image->width,
                      width + (width < xv_image->width ? (width % 2) + 4 : 0),
                      height + (height < xv_image->height ? 4 : 0));
    }
    else
    {
        S9xConvertMask(buffer,
                       (uint8 *)xv_image->data,
                       stride_in_pixels * 2,
                       bytes_per_pixel * xv_image->width,
                       width + (width < xv_image->width ? (width % 2) + 4 : 0),
                       height + (height < xv_image->height ? 4 : 0),
                       rshift,
                       gshift,
                       bshift,
                       bpp);
    }

    S9xRect dst = S9xApplyAspect(width, height, current_width, current_height);

    if (last_known_width != dst.w || last_known_height != dst.h)
    {
        last_known_width = dst.w;
        last_known_height = dst.h;
        clear();
    }

    XvShmPutImage(display,
                  xv_portid,
                  xwindow,
                  XDefaultGC(display, XDefaultScreen(display)),
                  xv_image,
                  0,
                  0,
                  width,
                  height,
                  dst.x,
                  dst.y,
                  dst.w,
                  dst.h,
                  False);

    top_level->set_mouseable_area(dst.x, dst.y, dst.w, dst.h);

    XSync(display, False);
}

void S9xXVDisplayDriver::update_image_size(int width, int height)
{
    if (xv_image_width != width || xv_image_height != height)
    {
        XShmDetach(display, &shm);
        XSync(display, 0);

        shmctl(shm.shmid, IPC_RMID, 0);
        shmdt(shm.shmaddr);

        xv_image = XvShmCreateImage(display,
                                    xv_portid,
                                    format,
                                    0,
                                    width,
                                    height,
                                    &shm);

        shm.shmid = shmget(IPC_PRIVATE, xv_image->data_size, IPC_CREAT | 0777);
        for (int tries = 0; tries <= 10; tries++)
        {
            shm.shmaddr = (char *)shmat(shm.shmid, 0, 0);

            if (shm.shmaddr == (void *)-1 && tries >= 10)
            {
                /* Can't recover, send exit. */
                fprintf(stderr, "Couldn't reallocate shared memory.\n");
                exit(1);
            }
            else if (shm.shmaddr != (void *)-1)
            {
                break;
            }
        }

        shm.readOnly = false;

        xv_image->data = shm.shmaddr;

        XShmAttach(display, &shm);

        xv_image_width = width;
        xv_image_height = height;
    }
}

int S9xXVDisplayDriver::init()
{
    int depth = 0, num_formats, num_attrs, highest_formats = 0;
    XvImageFormatValues *formats = NULL;
    XvAdaptorInfo *adaptors;
    XvAttribute *port_attr;
    VisualID visualid = None;
    unsigned int num_adaptors = 0;
    GdkScreen *screen;
    GdkWindow *root;

    /* Setup XV */
    gtk_widget_realize(GTK_WIDGET(drawing_area->gobj()));

    display = gdk_x11_display_get_xdisplay(drawing_area->get_display()->gobj());
    screen = drawing_area->get_screen()->gobj();
    root = gdk_screen_get_root_window(screen);

    xv_portid = -1;
    if ((XvQueryAdaptors(display,
                         gdk_x11_window_get_xid(root),
                         &num_adaptors,
                         &adaptors)) != Success)
    {
        fprintf(stderr, "No Xv compatible adaptors.\n");
    }

    for (int i = 0; i < (int)num_adaptors; i++)
    {
        if (adaptors[i].type & XvInputMask &&
            adaptors[i].type & XvImageMask)
        {
            formats = XvListImageFormats(display,
                                         adaptors[i].base_id,
                                         &num_formats);

            if (num_formats > highest_formats)
            {
                xv_portid = adaptors[i].base_id;
                highest_formats = num_formats;
                visualid = adaptors[i].formats->visual_id;
            }

            free(formats);
        }
    }

    XvFreeAdaptorInfo(adaptors);

    if (xv_portid < 0)
    {
        fprintf(stderr, "Could not open Xv output port.\n");
        return -1;
    }

    /* Set XV_AUTOPAINT_COLORKEY _only_ if available */
    port_attr = XvQueryPortAttributes(display, xv_portid, &num_attrs);

    for (int i = 0; i < num_attrs; i++)
    {
        if (!strcmp(port_attr[i].name, "XV_AUTOPAINT_COLORKEY"))
        {
            Atom colorkey;

            colorkey = XInternAtom(display, "XV_AUTOPAINT_COLORKEY", True);
            if (colorkey != None)
                XvSetPortAttribute(display, xv_portid, colorkey, 1);
        }
    }

    /* Try to find an RGB format */
    format = -1;
    bpp = 100;

    formats = XvListImageFormats(display,
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

                this->rshift = get_inv_shift(formats[i].red_mask, bpp);
                this->gshift = get_inv_shift(formats[i].green_mask, bpp);
                this->bshift = get_inv_shift(formats[i].blue_mask, bpp);

                /* Check for red-blue inversion on SiliconMotion drivers */
                if (formats[i].red_mask == 0x001f &&
                    formats[i].blue_mask == 0x7c00)
                {
                    int copy = this->rshift;
                    this->rshift = this->bshift;
                    this->bshift = copy;
                }
            }
        }
    }

    if (format == -1)
    {
        for (int i = 0; i < num_formats; i++)
        {
            if (formats[i].id == FOURCC_YUY2)
            {
                format = formats[i].id;
                depth = formats[i].depth;

                break;
            }
        }
    }

    free(formats);

    if (format == -1)
    {
        fprintf(stderr, "No compatible formats found for Xv.\n");
        return -1;
    }

    /* Build a table for yuv conversion */
    if (format == FOURCC_YUY2)
    {
        for (unsigned int color = 0; color < 65536; color++)
        {
            int r, g, b;
            int y, u, v;

            r = ((color & 0xf800) >> 8) | ((color >> 13) & 0x7);
            g = ((color & 0x07e0) >> 3) | ((color >> 9) & 0x3);
            b = ((color & 0x001F) << 3) | ((color >> 3) & 0x7);

            y = (int)((0.257 * ((double)r)) + (0.504 * ((double)g)) + (0.098 * ((double)b)) + 16.0);
            u = (int)((-0.148 * ((double)r)) + (-0.291 * ((double)g)) + (0.439 * ((double)b)) + 128.0);
            v = (int)((0.439 * ((double)r)) + (-0.368 * ((double)g)) + (-0.071 * ((double)b)) + 128.0);

            y_table[color] = CLAMP(y, 0, 255);
            u_table[color] = CLAMP(u, 0, 255);
            v_table[color] = CLAMP(v, 0, 255);
        }

        S9xRegisterYUVTables(y_table, u_table, v_table);
    }

    /* Create a sub-window */
    XVisualInfo vi_template;
    int vi_num_items;

    vi_template.visualid = visualid;
    vi_template.depth = depth;
    vi_template.visual = NULL;
    vi = XGetVisualInfo(display, VisualIDMask | VisualDepthMask, &vi_template, &vi_num_items);

    if (!vi)
    {
        vi_template.depth = 0;
        vi = XGetVisualInfo(display, VisualIDMask, &vi_template, &vi_num_items);

        if (!vi)
        {
            fprintf(stderr, "Couldn't map visual.\n");
            return -1;
        }
    }

    xcolormap = XCreateColormap(display,
                                gdk_x11_window_get_xid(drawing_area->get_window()->gobj()),
                                vi->visual,
                                AllocNone);

    create_window(1, 1);
    gdk_window_hide(gdk_window);

    /* Allocate a shared memory image. */
    xv_image = XvShmCreateImage(display,
                                xv_portid,
                                format,
                                0,
                                512,
                                512,
                                &shm);

    shm.shmid = shmget(IPC_PRIVATE, xv_image->data_size, IPC_CREAT | 0777);
    shm.shmaddr = (char *)shmat(shm.shmid, 0, 0);
    if (shm.shmaddr == (void *)-1)
    {
        fprintf(stderr, "Could not attach shared memory segment.\n");
        return -1;
    }

    shm.readOnly = false;

    xv_image->data = shm.shmaddr;

    XShmAttach(display, &shm);

    xv_image_width = 512;
    xv_image_height = 512;

    return 0;
}

void S9xXVDisplayDriver::deinit()
{
    gdk_window_destroy(gdk_window);

    XShmDetach(display, &shm);
    XSync(display, 0);

    XFreeColormap(display, xcolormap);
    XFree(vi);

    shmctl(shm.shmid, IPC_RMID, 0);
    shmdt(shm.shmaddr);
}

void S9xXVDisplayDriver::clear()
{
    GC xgc = XDefaultGC(display, XDefaultScreen(display));

    int width = drawing_area->get_width() * drawing_area->get_scale_factor();
    int height = drawing_area->get_height() * drawing_area->get_scale_factor();

    if (window->last_width <= 0 || window->last_height <= 0)
    {
        XDrawRectangle(display, xwindow, xgc, 0, 0, width, height);
        return;
    }

    /* Get width of modified display */
    S9xRect dst;
    dst.w = window->last_width;
    dst.h = window->last_height;
    apply_filter_scale(dst.w, dst.h);
    dst = S9xApplyAspect(dst.w, dst.h, width, height);

    if (dst.x > 0)
    {
        XFillRectangle(display, xwindow, xgc, 0, dst.y, dst.x, dst.h);
    }
    if (dst.x + dst.w < width)
    {
        XFillRectangle(display, xwindow, xgc, dst.x + dst.w, dst.y, width - (dst.x + dst.w), dst.h);
    }
    if (dst.y > 0)
    {
        XFillRectangle(display, xwindow, xgc, 0, 0, width, dst.y);
    }
    if (dst.y + dst.h < height)
    {
        XFillRectangle(display, xwindow, xgc, 0, dst.y + dst.h, width, height - (dst.y + dst.h));
    }

    XSync(display, False);
}

void S9xXVDisplayDriver::refresh()
{
    clear();
}

int S9xXVDisplayDriver::query_availability()
{
    unsigned int p_version,
        p_release,
        p_request_base,
        p_event_base,
        p_error_base;
    Display *display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

    if (!display)
        return 0;

    /* Test if XV and SHM are feasible */
    if (!XShmQueryExtension(display))
    {
        return 0;
    }

    if (XvQueryExtension(display,
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
