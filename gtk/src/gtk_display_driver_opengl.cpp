#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <dlfcn.h>
#include <sys/time.h>

#include "gtk_display.h"
#include "gtk_display_driver_opengl.h"

static const char *glGenBuffersNames[] = { "glGenBuffers",
                                           "glGenBuffersARB",
                                           "glGenBuffersEXT",
                                           NULL };
static const char *glDeleteBuffersNames[] = { "glDeleteBuffers",
                                              "glDeleteBuffersARB",
                                              "glDeleteBuffersEXT",
                                              NULL };
static const char *glBindBufferNames[] = { "glBindBuffer",
                                           "glBindBufferARB",
                                           "glBindBufferEXT",
                                           NULL };
static const char *glBufferDataNames[] = { "glBufferData",
                                           "glBufferDataARB",
                                           "glBufferDataEXT",
                                           NULL };
static const char *glBufferSubDataNames[] = { "glBufferSubData",
                                              "glBufferSubDataARB",
                                              "glBufferSubDataEXT",
                                              NULL };
static const char *glMapBufferNames[] = { "glMapBuffer",
                                          "glMapBufferARB",
                                          "glMapBufferEXT",
                                          NULL };
static const char *glUnmapBufferNames[] = { "glUnmapBuffer",
                                            "glUnmapBufferARB",
                                            "glUnmapBufferEXT",
                                            NULL };

gl_proc
get_null_address_proc (const GLubyte *name)
{
    return NULL;
}

gl_proc
S9xOpenGLDisplayDriver::get_aliased_extension (const char **name)
{
    gl_proc ext_proc = NULL;

    for (int i = 0; name[i]; i++)
    {
        ext_proc = glGetProcAddress ((GLubyte *) name[i]);

        if (ext_proc)
            break;
    }

    return ext_proc;
}

S9xOpenGLDisplayDriver::S9xOpenGLDisplayDriver (Snes9xWindow *window,
                                                Snes9xConfig *config)
{
    void *dl_handle = NULL;

    this->window = window;
    this->config = config;
    this->drawing_area = GTK_WIDGET (window->drawing_area);

    dl_handle = dlopen (NULL, RTLD_LAZY);

    if (dl_handle)
    {
        dlerror ();

#ifdef __GNUC__
__extension__
#endif
        getProcAddressProc functor = reinterpret_cast<getProcAddressProc> (dlsym (dl_handle, "glXGetProcAddress"));
        glGetProcAddress = functor;

        if (dlerror () != NULL)
        {
#ifdef __GNUC__
__extension__
#endif
            getProcAddressProc functor = reinterpret_cast<getProcAddressProc> (dlsym (dl_handle, "glXGetProcAddressARB"));
            glGetProcAddress = functor;

            if (dlerror () != NULL)
                glGetProcAddress = get_null_address_proc;
        }

        /* ok to close the handle, since didn't really open anything */
        dlclose (dl_handle);
    }
    else
    {
        glGetProcAddress = get_null_address_proc;
    }

    return;
}

void
S9xOpenGLDisplayDriver::update (int width, int height)
{
    GLint filter;
    uint8 *final_buffer = NULL;
    int   final_pitch;
    int   c_width, c_height;
    void  *pboMemory = NULL;

    c_width = drawing_area->allocation.width;
    c_height = drawing_area->allocation.height;

    if (width <= 0)
        return;

    gl_lock ();

    /* This avoids messing with the texture parameters every time */
    if (config->bilinear_filter != filtering)
    {
        filter = config->bilinear_filter ? GL_LINEAR : GL_NEAREST;
        glTexParameteri (tex_target, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri (tex_target, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri (tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        filtering = config->bilinear_filter;
    }

    glClear (GL_COLOR_BUFFER_BIT);
    glEnable (tex_target);

    if (config->scale_method > 0)
    {
        uint8 *src_buffer = (uint8 *) padded_buffer[0];
        int   src_pitch = image_width * image_bpp;
        uint8 *dst_buffer;
        int   dst_pitch;

        dst_buffer = (uint8 *) padded_buffer[1];
        dst_pitch = scaled_max_width * image_bpp;
        final_buffer = (uint8 *) padded_buffer[1];
        final_pitch = scaled_max_width * image_bpp;

        S9xFilter (src_buffer,
                   src_pitch,
                   dst_buffer,
                   dst_pitch,
                   width,
                   height);
    }
    else
    {
        final_buffer = (uint8 *) padded_buffer[0];
        final_pitch = image_width * image_bpp;
    }

    double screen_aspect = (double) c_width / (double) c_height;
    double snes_aspect = S9xGetAspect ();
    double granularity = 1.0 / (double) MAX (c_width, c_height);

    if (!config->scale_to_fit)
    {
        glViewport ((c_width - width) / 2, (c_height - height) / 2,
                    width, height);

        window->set_mouseable_area ((c_width - width) / 2,
                                       (c_height - height) / 2,
                                       width,
                                       height);
    }

    else if (config->maintain_aspect_ratio &&
            !(screen_aspect <= snes_aspect * (1.0 + granularity) &&
              screen_aspect >= snes_aspect * (1.0 - granularity)))
    {
        if (screen_aspect > snes_aspect)
        {
            glViewport ((c_width - (int)(c_height * snes_aspect)) / 2, 0,
                        (int)(c_height * snes_aspect), c_height);

            window->set_mouseable_area ((c_width -
                    (int) (c_height * snes_aspect)) / 2,
                    0,
                    (int) (c_height * snes_aspect),
                    c_height);
        }

        else
        {
            glViewport (0, (c_height - (int) (c_width / snes_aspect)) / 2,
                        c_width, (int) (c_width / snes_aspect));
            window->set_mouseable_area (0,
                                           (c_height -
                                             (int) (c_width / snes_aspect)) / 2,
                                           c_width,
                                           (int) (c_width / snes_aspect));
        }
    }

    else
    {
        glViewport (0, 0, c_width, c_height);
        window->set_mouseable_area (0, 0, c_width, c_height);
    }

    if (width > 0 && height > 0)
    {
        update_texture_size (width, height);

        if (using_pbos)
        {
            if (config->pbo_format == PBO_FMT_16)
            {

                glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo);
                glBufferData (GL_PIXEL_UNPACK_BUFFER,
                              width * height * 2,
                              NULL,
                              GL_STREAM_DRAW);
                pboMemory = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

                for (int y = 0; y < height; y++)
                {
                    memcpy ((uint8 *) pboMemory + (width * y * 2),
                            final_buffer + (y * final_pitch),
                            width * image_bpp);
                }

                glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);

                glPixelStorei (GL_UNPACK_ROW_LENGTH, width);
                glTexSubImage2D (tex_target,
                                 0,
                                 0,
                                 0,
                                 width,
                                 height,
                                 GL_BGRA,
                                 GL_UNSIGNED_SHORT_1_5_5_5_REV,
                                 BUFFER_OFFSET (0));

                glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else if (config->pbo_format == PBO_FMT_24)
            {
                /* Complement width to next multiple of 4 to force line size to
                 * be a multiple of 4 bytes. Otherwise, packing fails. */
                int width_mul_4 = width + ((4 - (width % 4)) % 4);

                glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo);
                glBufferData (GL_PIXEL_UNPACK_BUFFER,
                              width_mul_4 * height * 3,
                              NULL,
                              GL_STREAM_DRAW);
                pboMemory = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

                /* Pixel swizzling in software */
                S9xSetEndianess (ENDIAN_MSB);
                S9xConvert (final_buffer,
                            pboMemory,
                            final_pitch,
                            width_mul_4 * 3,
                            width,
                            height,
                            24);

                glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);

                glPixelStorei (GL_UNPACK_ROW_LENGTH, width_mul_4);
                glTexSubImage2D (tex_target,
                                 0,
                                 0,
                                 0,
                                 width,
                                 height,
                                 GL_RGB,
                                 GL_UNSIGNED_BYTE,
                                 BUFFER_OFFSET (0));

                glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else /* PBO_FMT_32 */
            {
                glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo);
                glBufferData (GL_PIXEL_UNPACK_BUFFER,
                              width * height * 4,
                              NULL,
                              GL_STREAM_DRAW);
                pboMemory = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

                /* Pixel swizzling in software */
#ifdef __BIG_ENDIAN__
                S9xSetEndianess (ENDIAN_MSB);
#else
                S9xSetEndianess (ENDIAN_LSB);
#endif
                S9xConvert (final_buffer,
                            pboMemory,
                            final_pitch,
                            width * 4,
                            width,
                            height,
                            32);

                glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);

                glPixelStorei (GL_UNPACK_ROW_LENGTH, width);
                glTexSubImage2D (tex_target,
                                 0,
                                 0,
                                 0,
                                 width,
                                 height,
                                 GL_BGRA,
                                 PBO_BGRA_NATIVE_ORDER,
                                 BUFFER_OFFSET (0));

                glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
            }
        }
        else
        {
            glPixelStorei (GL_UNPACK_ROW_LENGTH, final_pitch / image_bpp);
            glTexSubImage2D (tex_target,
                             0,
                             0,
                             0,
                             width,
                             height,
                             GL_BGRA,
                             GL_UNSIGNED_SHORT_1_5_5_5_REV,
                             final_buffer);
        }

        if (tex_target == GL_TEXTURE_2D)
        {
            texcoords[1] = (float) (height) / texture_height;
            texcoords[2] = (float) (width) / texture_width;
            texcoords[3] = texcoords[1];
            texcoords[4] = texcoords[2];
        }
        else if (tex_target == GL_TEXTURE_RECTANGLE)
        {
            texcoords[1] = (float) (height);
            texcoords[2] = (float) (width);
            texcoords[3] = texcoords[1];
            texcoords[4] = texcoords[2];
        }

        glDrawArrays (GL_QUADS, 0, 4);
    }

    gl_unlock ();
    gl_swap ();

    return;
}

void
S9xOpenGLDisplayDriver::clear_buffers (void)
{
    memset (buffer[0], 0, image_padded_size);
    memset (buffer[1], 0, scaled_padded_size);

    gl_lock ();

    glPixelStorei (GL_UNPACK_ROW_LENGTH, scaled_max_width);
    glTexSubImage2D (tex_target,
                     0,
                     0,
                     0,
                     scaled_max_width,
                     scaled_max_height,
                     GL_BGRA,
                     GL_UNSIGNED_SHORT_1_5_5_5_REV,
                     buffer[1]);
    gl_unlock ();

    return;
}

void
S9xOpenGLDisplayDriver::update_texture_size (int width, int height)
{
    if (width != texture_width || height != texture_height)
    {
        if (dyn_resizing)
        {
            glBindTexture (tex_target, texmap);

            if (using_pbos)
            {
                glTexImage2D (tex_target,
                              0,
                              config->pbo_format == PBO_FMT_16 ? GL_RGB5_A1 : 4,
                              width,
                              height,
                              0,
                              GL_RGB,
                              GL_UNSIGNED_BYTE,
                              NULL);
            }
            else
            {
                glTexImage2D (tex_target,
                              0,
                              GL_RGB5_A1,
                              width,
                              height,
                              0,
                              GL_BGRA,
                              GL_UNSIGNED_SHORT_1_5_5_5_REV,
                              NULL);
            }

            texture_width = width;
            texture_height = height;
        }
    }

    return;
}

int
S9xOpenGLDisplayDriver::load_pixel_buffer_functions (void)
{
    const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

    if (strstr (extensions, "pixel_buffer_object"))
    {
        glGenBuffers =
            (glGenBuffersProc)
            get_aliased_extension (glGenBuffersNames);

        glDeleteBuffers =
            (glDeleteBuffersProc)
            get_aliased_extension (glDeleteBuffersNames);

        glBindBuffer =
            (glBindBufferProc)
            get_aliased_extension (glBindBufferNames);

        glBufferData =
            (glBufferDataProc)
            get_aliased_extension (glBufferDataNames);

        glBufferSubData =
            (glBufferSubDataProc)
            get_aliased_extension (glBufferSubDataNames);

        glMapBuffer =
            (glMapBufferProc)
            get_aliased_extension (glMapBufferNames);

        glUnmapBuffer =
            (glUnmapBufferProc)
            get_aliased_extension (glUnmapBufferNames);

        if (!glGenBuffers    ||
            !glBindBuffer    ||
            !glBufferData    ||
            !glBufferSubData ||
            !glMapBuffer     ||
            !glUnmapBuffer   ||
            !glDeleteBuffers)
        {
            return 0;
        }
    }

    else
    {
        return 0;
    }

    return 1;
}

void
S9xOpenGLDisplayDriver::opengl_defaults (void)
{
    XVisualInfo *vi;
    int         glx_attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };
    Display     *display = GDK_DISPLAY ();

    vi = glXChooseVisual (display, DefaultScreen (display), glx_attribs);

    glx_context = glXCreateContext (display,
                                    vi,
                                    0,
                                    1);
    glXMakeCurrent (display, GDK_WINDOW_XWINDOW (drawing_area->window), glx_context);

    gl_lock ();

    const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

    using_pbos = 0;
    if (config->use_pbos)
    {
        if (!load_pixel_buffer_functions ())
        {
            fprintf (stderr, _("pixel_buffer_object extension not supported.\n"));

            config->use_pbos = 0;
        }
        else
        {
            using_pbos = 1;
        }
    }

    tex_target = GL_TEXTURE_2D;
    texture_width = 1024;
    texture_height = 1024;
    dyn_resizing = FALSE;

    if (config->npot_textures)
    {
        if (strstr (extensions, "_texture_rectangle"))
        {
            tex_target = GL_TEXTURE_RECTANGLE;
            texture_width = scaled_max_width;
            texture_height = scaled_max_height;
            dyn_resizing = TRUE;
        }
        else if (strstr (extensions, "GL_ARB_texture_non_power_of_two"))
        {
            dyn_resizing = TRUE;
        }
    }

    glEnable (GL_VERTEX_ARRAY);
    glEnable (GL_TEXTURE_COORD_ARRAY);

    vertices[0] = 0.0f;
    vertices[1] = 0.0f;
    vertices[2] = 1.0f;
    vertices[3] = 0.0f;
    vertices[4] = 1.0f;
    vertices[5] = 1.0f;
    vertices[6] = 0.0f;
    vertices[7] = 1.0f;

    glVertexPointer (2, GL_FLOAT, 0, vertices);

    texcoords[0] = 0.0f;
    texcoords[5] = 0.0f;
    texcoords[6] = 0.0f;
    texcoords[7] = 0.0f;

    glTexCoordPointer (2, GL_FLOAT, 0, texcoords);

    if (config->use_pbos)
    {
        glGenBuffers (1, &pbo);
        glGenTextures (1, &texmap);

        glBindTexture (tex_target, texmap);
        glTexImage2D (tex_target,
                      0,
                      config->pbo_format == PBO_FMT_16 ? GL_RGB5_A1 : 4,
                      texture_width,
                      texture_height,
                      0,
                      GL_RGB,
                      GL_UNSIGNED_BYTE,
                      NULL);

        glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo);
        glBufferData (GL_PIXEL_UNPACK_BUFFER,
                      texture_width * texture_height * 3,
                      NULL,
                      GL_STREAM_DRAW);

        glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
    }
    else
    {
        glGenTextures (1, &texmap);

        glBindTexture (tex_target, texmap);
        glTexImage2D (tex_target,
                      0,
                      GL_RGB5_A1,
                      texture_width,
                      texture_height,
                      0,
                      GL_BGRA,
                      GL_UNSIGNED_SHORT_1_5_5_5_REV,
                      NULL);
    }

    glEnable (GL_DITHER);

    glDisable (GL_POLYGON_SMOOTH);
    glShadeModel (GL_FLAT);
    glPolygonMode (GL_FRONT, GL_FILL);

    glEnable (GL_CULL_FACE);
    glCullFace (GL_BACK);

    glClearColor (0.0, 0.0, 0.0, 0.0);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glDisable (GL_BLEND);
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_TEXTURE_RECTANGLE);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0.0, 1.0, 0.0, 1.0, -1, 1);

    gl_unlock ();

    return;
}

void
S9xOpenGLDisplayDriver::refresh (int width, int height)
{
    return;
}

int
S9xOpenGLDisplayDriver::init (void)
{
    int padding;

    opengl_defaults ();

    /* Create two system buffers to avoid DMA contention */

    buffer[0] = malloc (image_padded_size);
    buffer[1] = malloc (scaled_padded_size);

    clear_buffers ();

    padding = (image_padded_size - image_size) / 2;
    padded_buffer[0] = (void *) (((uint8 *) buffer[0]) + padding);

    padding = (scaled_padded_size - scaled_size) / 2;
    padded_buffer[1] = (void *) (((uint8 *) buffer[1]) + padding);

    GFX.Screen = (uint16 *) padded_buffer[0];
    GFX.Pitch = image_width * image_bpp;

    filtering = -1;

    this->swap_control (config->sync_to_vblank);

    return 0;
}

void
S9xOpenGLDisplayDriver::swap_control (int enable)
{
    glSwapIntervalProc glSwapInterval = NULL;
    const char         *ext_str;

    ext_str = glXQueryExtensionsString (GDK_DISPLAY (),
                                        GDK_SCREEN_XNUMBER (
                                            gdk_screen_get_default ()));

    /* We try to set this with both extensions since some cards pretend
     * to support both, but ignore one. */

    if (strstr (ext_str, "GLX_MESA_swap_control"))
    {
        glSwapInterval = (glSwapIntervalProc)
            glGetProcAddress ((GLubyte *) "glXSwapIntervalMESA");
        if (glSwapInterval)
            glSwapInterval (enable ? 1 : 0);
    }

    else if (strstr (ext_str, "GLX_SGI_swap_control"))
    {
        glSwapInterval = (glSwapIntervalProc)
            glGetProcAddress ((GLubyte *) "glXSwapIntervalSGI");
        if (glSwapInterval)
        {
            glSwapInterval (enable ? 1 : 0);
        }
    }

    return;
}

uint16 *
S9xOpenGLDisplayDriver::get_next_buffer (void)
{
    return (uint16 *) padded_buffer[0];
}

void
S9xOpenGLDisplayDriver::push_buffer (uint16 *src)
{
    memmove (padded_buffer[0], src, image_size);
    update (window->last_width, window->last_height);

    return;
}

uint16 *
S9xOpenGLDisplayDriver::get_current_buffer (void)
{
    return (uint16 *) padded_buffer[0];
}

void
S9xOpenGLDisplayDriver::gl_lock (void)
{

    return;
}

void
S9xOpenGLDisplayDriver::gl_unlock (void)
{

    return;
}

void
S9xOpenGLDisplayDriver::gl_swap (void)
{
    glXSwapBuffers (GDK_DISPLAY (), GDK_WINDOW_XWINDOW (drawing_area->window));

    return;
}

void
S9xOpenGLDisplayDriver::deinit (void)
{
    glXDestroyContext (GDK_DISPLAY (), glx_context);

    GFX.Screen = NULL;

    padded_buffer[0] = NULL;
    padded_buffer[1] = NULL;

    free (buffer[0]);
    free (buffer[1]);

    gl_lock ();

    if (using_pbos)
    {
        glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
        glDeleteBuffers (1, &pbo);
    }

    glDeleteTextures (1, &texmap);

    gl_unlock ();

    return;
}

void
S9xOpenGLDisplayDriver::reconfigure (int width, int height)
{
    return;
}

int
S9xOpenGLDisplayDriver::query_availability (void)
{
    int errorBase, eventBase;

    if (glXQueryExtension (GDK_DISPLAY (), &errorBase, &eventBase) == False)
    {
        if (gui_config->hw_accel == HWA_OPENGL)
            gui_config->hw_accel = HWA_NONE;
        return 0;
    }

    return 1;

}
