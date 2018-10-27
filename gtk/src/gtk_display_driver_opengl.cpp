#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include "gtk_display.h"
#include "gtk_display_driver_opengl.h"
#include "gtk_shader_parameters.h"
#include "shaders/shader_helpers.h"

static void S9xViewportCallback (int src_width, int src_height,
                                 int viewport_x, int viewport_y,
                                 int viewport_width, int viewport_height,
                                 int *out_x, int *out_y,
                                 int *out_width, int *out_height)
{

    S9xApplyAspect (src_width, src_height, viewport_width, viewport_height);
    *out_x = src_width + viewport_x;
    *out_y = src_height + viewport_y;
    *out_width = viewport_width;
    *out_height = viewport_height;
}

S9xOpenGLDisplayDriver::S9xOpenGLDisplayDriver (Snes9xWindow *window,
                                                Snes9xConfig *config)
{
    this->window = window;
    this->config = config;
    this->drawing_area = GTK_WIDGET (window->drawing_area);
}

void S9xOpenGLDisplayDriver::update (int width, int height, int yoffset)
{
    uint8 *final_buffer = NULL;
    int   final_pitch;
    void  *pboMemory = NULL;
    int   x, y, w, h;

    GtkAllocation allocation;
    gtk_widget_get_allocation (drawing_area, &allocation);

    if (output_window_width  != allocation.width ||
        output_window_height != allocation.height)
    {
        resize ();
    }

#if GTK_CHECK_VERSION(3,10,0)
    int gdk_scale_factor = gdk_window_get_scale_factor (gdk_window);

    allocation.width *= gdk_scale_factor;
    allocation.height *= gdk_scale_factor;
#endif

    glBindTexture (GL_TEXTURE_2D, texmap);
    GLint filter = Settings.BilinearFilter ? GL_LINEAR : GL_NEAREST;
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    GLint clamp = (using_glsl_shaders || !npot) ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE;
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);

    glClear (GL_COLOR_BUFFER_BIT);
    glEnable (GL_TEXTURE_2D);

    if (config->scale_method > 0)
    {
        uint8 *src_buffer = (uint8 *) padded_buffer[0];
        int   src_pitch = image_width * image_bpp;
        uint8 *dst_buffer;
        int   dst_pitch;

        src_buffer += (src_pitch * yoffset);

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
        final_buffer += (final_pitch * yoffset);
    }

    x = width; y = height;
    w = allocation.width; h = allocation.height;
    S9xApplyAspect (x, y, w, h);

    glViewport (x, allocation.height - y - h, w, h);
    window->set_mouseable_area (x, y, w, h);

    update_texture_size (width, height);

    if (using_pbos)
    {
        if (config->pbo_format == 16)
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
            glTexSubImage2D (GL_TEXTURE_2D,
                             0,
                             0,
                             0,
                             width,
                             height,
                             GL_RGB,
                             GL_UNSIGNED_SHORT_5_6_5,
                             BUFFER_OFFSET (0));

            glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
        }
        else /* 32-bit color */
        {
            glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo);
            glBufferData (GL_PIXEL_UNPACK_BUFFER,
                          width * height * 4,
                          NULL,
                          GL_STREAM_DRAW);
            pboMemory = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

            /* Pixel swizzling in software */
            S9xSetEndianess (ENDIAN_NORMAL);
            S9xConvert (final_buffer,
                        pboMemory,
                        final_pitch,
                        width * 4,
                        width,
                        height,
                        32);

            glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);

            glPixelStorei (GL_UNPACK_ROW_LENGTH, width);
            glTexSubImage2D (GL_TEXTURE_2D,
                             0,
                             0,
                             0,
                             width,
                             height,
                             GL_BGRA,
                             GL_UNSIGNED_BYTE,
                             BUFFER_OFFSET (0));

            glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
        }
    }
    else
    {
        glPixelStorei (GL_UNPACK_ROW_LENGTH, final_pitch / image_bpp);
        glTexSubImage2D (GL_TEXTURE_2D,
                         0,
                         0,
                         0,
                         width,
                         height,
                         GL_RGB,
                         GL_UNSIGNED_SHORT_5_6_5,
                         final_buffer);
    }

    texcoords[1] = (float) (height) / texture_height;
    texcoords[2] = (float) (width) / texture_width;
    texcoords[3] = texcoords[1];
    texcoords[4] = texcoords[2];

    if (using_glsl_shaders)
    {
        glsl_shader->render (texmap, width, height, x, allocation.height - y - h, w, h, S9xViewportCallback);
        swap_buffers ();
        return;
    }

    glDrawArrays (GL_QUADS, 0, 4);

    swap_buffers ();
}

void *S9xOpenGLDisplayDriver::get_parameters ()
{
    if (using_glsl_shaders && glsl_shader)
    {
        return (void *) &glsl_shader->param;
    }

    return NULL;
}

void S9xOpenGLDisplayDriver::save (const char *filename)
{
    if (using_glsl_shaders && glsl_shader)
    {
        glsl_shader->save(filename);
    }
}

void S9xOpenGLDisplayDriver::clear_buffers ()
{
    memset (buffer[0], 0, image_padded_size);
    memset (buffer[1], 0, scaled_padded_size);

    glPixelStorei (GL_UNPACK_ROW_LENGTH, scaled_max_width);
    glTexSubImage2D (GL_TEXTURE_2D,
                     0,
                     0,
                     0,
                     scaled_max_width,
                     scaled_max_height,
                     GL_RGB,
                     GL_UNSIGNED_SHORT_5_6_5,
                     buffer[1]);
}

void S9xOpenGLDisplayDriver::update_texture_size (int width, int height)
{
    if (width != texture_width || height != texture_height)
    {
        if (npot)
        {
            glBindTexture (GL_TEXTURE_2D, texmap);

            if (using_pbos && config->pbo_format == 32)
            {
                glTexImage2D (GL_TEXTURE_2D,
                              0,
                              4,
                              width,
                              height,
                              0,
                              GL_BGRA,
                              GL_UNSIGNED_BYTE,
                              NULL);
            }
            else
            {
                glTexImage2D (GL_TEXTURE_2D,
                              0,
                              GL_RGB565,
                              width,
                              height,
                              0,
                              GL_RGB,
                              GL_UNSIGNED_SHORT_5_6_5,
                              NULL);
            }

            texture_width = width;
            texture_height = height;
        }
    }
}

int S9xOpenGLDisplayDriver::load_shaders (const char *shader_file)
{
    int length = strlen (shader_file);

    if ((length > 6 && !strcasecmp(shader_file + length - 6, ".glslp")) ||
        (length > 5 && !strcasecmp(shader_file + length - 5, ".glsl")))
    {
        glsl_shader = new GLSLShader;
        if (glsl_shader->load_shader ((char *) shader_file))
        {
            using_glsl_shaders = true;
            npot = true;

            if (glsl_shader->param.size () > 0)
                window->enable_widget ("shader_parameters_item", TRUE);

            return 1;
        }

        delete glsl_shader;
    }

    return 0;
}

int S9xOpenGLDisplayDriver::opengl_defaults ()
{
    npot = false;
    using_pbos = false;

    if (config->use_pbos)
    {
        using_pbos = true;
    }

    using_glsl_shaders = false;
    glsl_shader = NULL;

    if (config->use_shaders)
    {
        if (!load_shaders (config->fragment_shader))
        {
            config->use_shaders = false;
        }
    }

    texture_width = 1024;
    texture_height = 1024;

    if (config->npot_textures)
    {
        npot = true;
    }

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);

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

        glBindTexture (GL_TEXTURE_2D, texmap);
        glTexImage2D (GL_TEXTURE_2D,
                      0,
                      config->pbo_format == 16 ? GL_RGB565 : 4,
                      texture_width,
                      texture_height,
                      0,
                      config->pbo_format == 16 ? GL_RGB : GL_BGRA,
                      config->pbo_format == 16 ? GL_UNSIGNED_SHORT_5_6_5 : GL_UNSIGNED_BYTE,
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

        glBindTexture (GL_TEXTURE_2D, texmap);
        glTexImage2D (GL_TEXTURE_2D,
                      0,
                      GL_RGB565,
                      texture_width,
                      texture_height,
                      0,
                      GL_RGB,
                      GL_UNSIGNED_SHORT_5_6_5,
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

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0.0, 1.0, 0.0, 1.0, -1, 1);

    return 1;
}

void S9xOpenGLDisplayDriver::refresh (int width, int height)
{
}

void S9xOpenGLDisplayDriver::resize ()
{
    context->resize ();
    context->swap_interval (config->sync_to_vblank);
    output_window_width = context->width;
    output_window_height = context->height;
}

int S9xOpenGLDisplayDriver::create_context ()
{
    gdk_window = gtk_widget_get_window (drawing_area);

#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW (gdk_window))
    {
        context = &wl;
    }
#endif
#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_WINDOW (gdk_window))
    {
        context = &glx;
    }
#endif

    if (!context->attach (drawing_area))
        return 0;

    if (!context->create_context ())
        return 0;

    output_window_width = context->width;
    output_window_height = context->height;

    context->make_current ();

    if (epoxy_gl_version () < 20)
    {
        printf ("OpenGL version is only %d. Need 20.\n", epoxy_gl_version ());
        return 0;
    }

    return 1;
}

int S9xOpenGLDisplayDriver::init ()
{
    initialized = false;

    if (!create_context ())
    {
        return -1;
    }

    if (!opengl_defaults ())
    {
        return -1;
    }

    buffer[0] = malloc (image_padded_size);
    buffer[1] = malloc (scaled_padded_size);

    clear_buffers ();

    padded_buffer[0] = (void *) (((uint8 *) buffer[0]) + image_padded_offset);
    padded_buffer[1] = (void *) (((uint8 *) buffer[1]) + scaled_padded_offset);

    GFX.Screen = (uint16 *) padded_buffer[0];
    GFX.Pitch = image_width * image_bpp;

    context->swap_interval (config->sync_to_vblank);

    initialized = true;

    return 0;
}

uint16 *S9xOpenGLDisplayDriver::get_next_buffer ()
{
    return (uint16 *) padded_buffer[0];
}

void S9xOpenGLDisplayDriver::push_buffer (uint16 *src)
{
    memmove (padded_buffer[0], src, image_size);
}

uint16 *S9xOpenGLDisplayDriver::get_current_buffer ()
{
    return (uint16 *) padded_buffer[0];
}

void S9xOpenGLDisplayDriver::swap_buffers ()
{
    context->swap_buffers ();

    if (config->sync_every_frame)
    {
        usleep (0);
        glFinish ();
    }
}

void S9xOpenGLDisplayDriver::deinit ()
{
    if (!initialized)
        return;

    if (using_glsl_shaders)
    {
        window->enable_widget ("shader_parameters_item", FALSE);
        gtk_shader_parameters_dialog_close ();
        glsl_shader->destroy();
        delete glsl_shader;
    }

    GFX.Screen = NULL;

    padded_buffer[0] = NULL;
    padded_buffer[1] = NULL;

    free (buffer[0]);
    free (buffer[1]);

    if (using_pbos)
    {
        glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
        glDeleteBuffers (1, &pbo);
    }

    glDeleteTextures (1, &texmap);
}

int S9xOpenGLDisplayDriver::query_availability ()
{
    GdkDisplay *gdk_display = gdk_display_get_default ();

#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_DISPLAY (gdk_display))
    {
        return 1;
    }
#endif

#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_DISPLAY (gdk_display))
    {
        Display *dpy = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

        if (epoxy_has_glx (dpy))
            return 1;
    }
#endif

    if (gui_config->hw_accel == HWA_OPENGL)
        gui_config->hw_accel = HWA_NONE;

    return 0;
}
