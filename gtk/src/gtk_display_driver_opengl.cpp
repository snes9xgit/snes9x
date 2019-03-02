/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_2_3_compat.h"
#include <dlfcn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include "gtk_display.h"
#include "gtk_display_driver_opengl.h"
#include "gtk_shader_parameters.h"
#include "shaders/shader_helpers.h"

static const GLchar *stock_vertex_shader_110 =
"#version 110\n"

"attribute vec2 in_position;\n"
"attribute vec2 in_texcoord;\n"
"varying vec2 texcoord;\n"

"void main()\n"
"{\n"
"    gl_Position = vec4 (in_position, 0.0, 1.0);\n"
"    texcoord = in_texcoord;\n"
"}\n";

static const GLchar *stock_vertex_shader_140 =
"#version 140\n"

"in vec2 in_position;\n"
"in vec2 in_texcoord;\n"
"out vec2 texcoord;\n"

"void main()\n"
"{\n"
"    gl_Position = vec4 (in_position, 0.0, 1.0);\n"
"    texcoord = in_texcoord;\n"
"}\n";

static const GLchar *stock_fragment_shader_110 =
"#version 110\n"

"uniform sampler2D texmap;\n"
"varying vec2 texcoord;\n"

"void main()\n"
"{\n"
"    gl_FragColor = texture2D(texmap, texcoord);\n"
"}\n";

static const GLchar *stock_fragment_shader_140 =
"#version 140\n"

"uniform sampler2D texmap;\n"
"out vec4 fragcolor;\n"
"in vec2 texcoord;\n"

"void main()\n"
"{\n"
"    fragcolor = texture(texmap, texcoord);\n"
"}\n";


static GLfloat coords[]  = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
                              0.0f,  1.0f, 1.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, };

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
    void  *pbo_map = NULL;
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

    if (!legacy)
        glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, texmap);
    GLint filter = Settings.BilinearFilter ? GL_LINEAR : GL_NEAREST;
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    GLint clamp = (using_glsl_shaders || !npot) ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE;
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);

    glClear (GL_COLOR_BUFFER_BIT);

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

    x = width;
    y = height;
    w = allocation.width;
    h = allocation.height;
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

            if (version >= 30)
                pbo_map = glMapBufferRange (
                    GL_PIXEL_UNPACK_BUFFER, 0, width * height * 2,
                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
                        GL_MAP_UNSYNCHRONIZED_BIT);
            else
                pbo_map = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

            for (int y = 0; y < height; y++)
            {
                memcpy ((uint8 *) pbo_map + (width * y * 2),
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

            if (version >= 30)
                pbo_map = glMapBufferRange (
                    GL_PIXEL_UNPACK_BUFFER, 0, width * height * 4,
                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
                        GL_MAP_UNSYNCHRONIZED_BIT);
            else
                pbo_map = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

            /* Pixel swizzling in software */
            S9xSetEndianess (ENDIAN_NORMAL);
            S9xConvert (final_buffer,
                        pbo_map,
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

    if (using_glsl_shaders)
    {
        glsl_shader->render (texmap, width, height, x, allocation.height - y - h, w, h, S9xViewportCallback);
        swap_buffers ();
        return;
    }

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

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
                     MIN(scaled_max_width, texture_width),
                     MIN(scaled_max_height, texture_height),
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
                              GL_RGBA,
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

            coords[9]  = 1.0f;
            coords[10] = 1.0f;
            coords[11] = 1.0f;
            coords[14] = 1.0f;
        }
        else
        {
            coords[9]  = height / 1024.0f;
            coords[10] = width  / 1024.0f;
            coords[11] = height / 1024.0f;
            coords[14] = width  / 1024.0f;
        }

        texture_width  = width;
        texture_height = height;

        if (!legacy)
        {
            glBindBuffer (GL_ARRAY_BUFFER, coord_buffer);
            glBufferData (GL_ARRAY_BUFFER, sizeof (GLfloat) * 16, coords, GL_STATIC_DRAW);
            glBindBuffer (GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glVertexPointer (2, GL_FLOAT, 0, coords);
            glTexCoordPointer (2, GL_FLOAT, 0, &coords[8]);
        }
    }
}

bool S9xOpenGLDisplayDriver::load_shaders(const char *shader_file)
{
    setlocale(LC_ALL, "C");
    std::string filename(shader_file);

    auto endswith = [&](std::string ext) -> bool {
        return filename.rfind(ext) == filename.length() - ext.length();
    };

    if (endswith(".glslp") || endswith(".glsl")
#ifdef USE_SLANG
        || endswith(".slangp") || endswith(".slang")
#endif
    )
    {
        glsl_shader = new GLSLShader;
        if (glsl_shader->load_shader((char *)shader_file))
        {
            using_glsl_shaders = true;
            npot = true;

            if (glsl_shader->param.size() > 0)
                window->enable_widget("shader_parameters_item", true);

            setlocale(LC_ALL, "");
            return true;
        }

        delete glsl_shader;
    }

    setlocale(LC_ALL, "");
    return false;
}

bool S9xOpenGLDisplayDriver::opengl_defaults()
{
    npot = false;
    using_pbos = false;

    if (config->use_pbos)
    {
        if (version >= 15)
            using_pbos = true;
        else
            config->use_pbos = false;
    }

    using_glsl_shaders = false;
    glsl_shader = NULL;

    if (config->use_shaders)
    {
        if (legacy || !load_shaders (config->shader_filename.c_str ()))
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

    if (legacy)
    {
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glEnable (GL_TEXTURE_2D);
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();

        glVertexPointer (2, GL_FLOAT, 0, coords);
        glTexCoordPointer (2, GL_FLOAT, 0, &coords[8]);
    }
    else
    {
        stock_program = glCreateProgram ();

        GLuint vertex_shader = glCreateShader (GL_VERTEX_SHADER);
        GLuint fragment_shader = glCreateShader (GL_FRAGMENT_SHADER);

        if (version < 30)
        {
            glShaderSource (vertex_shader, 1, &stock_vertex_shader_110, NULL);
            glShaderSource (fragment_shader, 1, &stock_fragment_shader_110, NULL);
        }
        else
        {
            glShaderSource (vertex_shader, 1, &stock_vertex_shader_140, NULL);
            glShaderSource (fragment_shader, 1, &stock_fragment_shader_140, NULL);
        }

        glCompileShader (vertex_shader);
        glAttachShader (stock_program, vertex_shader);
        glCompileShader (fragment_shader);
        glAttachShader (stock_program, fragment_shader);

        glBindAttribLocation (stock_program, 0, "in_position");
        glBindAttribLocation (stock_program, 1, "in_texcoord");

        glLinkProgram (stock_program);
        glUseProgram (stock_program);

        glDeleteShader (vertex_shader);
        glDeleteShader (fragment_shader);

        GLint texture_uniform = glGetUniformLocation (stock_program, "texmap");
        glUniform1i (texture_uniform, 0);

        if (core)
        {
            GLuint vao;
            glGenVertexArrays (1, &vao);
            glBindVertexArray (vao);
        }

        glGenBuffers (1, &coord_buffer);
        glBindBuffer (GL_ARRAY_BUFFER, coord_buffer);
        glBufferData (GL_ARRAY_BUFFER, sizeof (GLfloat) * 16, coords, GL_STATIC_DRAW);

        glEnableVertexAttribArray (0);
        glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET (0));
        glEnableVertexAttribArray (1);
        glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET (32));

        glBindBuffer (GL_ARRAY_BUFFER, 0);
    }

    if (config->use_pbos)
    {
        glGenBuffers (1, &pbo);
        glGenTextures (1, &texmap);

        glBindTexture (GL_TEXTURE_2D, texmap);
        glTexImage2D (GL_TEXTURE_2D,
                      0,
                      config->pbo_format == 16 ? GL_RGB565 : GL_RGBA,
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

    glClearColor (0.0, 0.0, 0.0, 0.0);

    return true;
}

void S9xOpenGLDisplayDriver::refresh (int width, int height)
{
    resize ();
}

void S9xOpenGLDisplayDriver::resize ()
{
    context->resize ();
    context->swap_interval (config->sync_to_vblank);
    output_window_width = context->width;
    output_window_height = context->height;
}

bool S9xOpenGLDisplayDriver::create_context()
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
        return false;

    if (!context->create_context ())
        return false;

    output_window_width = context->width;
    output_window_height = context->height;

    context->make_current ();

    legacy = false;
    version = epoxy_gl_version ();
    if (version < 20)
    {
        printf ("OpenGL version is only %d.%d. Recommended version is 2.0.\n",
                version / 10,
                version % 10);
        legacy = true;
    }

    int profile_mask = 0;
    glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &profile_mask);
    if (profile_mask & GL_CONTEXT_CORE_PROFILE_BIT)
        core = true;
    else
        core = false;

    return true;
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

    if (config->use_glfinish && !config->use_sync_control)
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
        window->enable_widget ("shader_parameters_item", false);
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
        Display *dpy = GDK_DISPLAY_XDISPLAY (gdk_display);

        if (glXQueryExtension (dpy, NULL, NULL) == True)
        {
           return 1;
        }
    }
#endif

    if (gui_config->hw_accel == HWA_OPENGL)
        gui_config->hw_accel = HWA_NONE;

    return 0;
}

bool S9xOpenGLDisplayDriver::is_ready ()
{
    if (context->ready())
    {
        return true;
    }

    return false;
}
