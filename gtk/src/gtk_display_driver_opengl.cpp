#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#endif
#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif
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

static bool
on_wayland (void)
{
#ifdef GDK_WINDOWING_WAYLAND
    return (GDK_IS_WAYLAND_DISPLAY (gdk_display_get_default ()));
#endif
    return 0;
}

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

    return;
}

S9xOpenGLDisplayDriver::S9xOpenGLDisplayDriver (Snes9xWindow *window,
                                                Snes9xConfig *config)
{
    this->window = window;
    this->config = config;
    this->drawing_area = GTK_WIDGET (window->drawing_area);

    return;
}

void
S9xOpenGLDisplayDriver::update (int width, int height, int yoffset)
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
        resize_window (allocation.width, allocation.height);
    }

#if GTK_CHECK_VERSION(3,10,0)
    int gdk_scale_factor = gdk_window_get_scale_factor (gdk_window);

    if (!on_wayland ())
    {
        allocation.width *= gdk_scale_factor;
        allocation.height *= gdk_scale_factor;
    }
#endif

    if (using_glsl_shaders)
    {
        glBindTexture (GL_TEXTURE_2D, texmap);
    }

    GLint filter = Settings.BilinearFilter ? GL_LINEAR : GL_NEAREST;
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    GLint clamp = (using_shaders || !dyn_resizing) ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE;
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
        else if (config->pbo_format == PBO_FMT_24)
        {
            glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
            glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo);
            glBufferData (GL_PIXEL_UNPACK_BUFFER,
                          width * height * 3,
                          NULL,
                          GL_STREAM_DRAW);
            pboMemory = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

            /* Pixel swizzling in software */
            S9xSetEndianess (ENDIAN_SWAPPED);
            S9xConvert (final_buffer,
                        pboMemory,
                        final_pitch,
                        width * 3,
                        width,
                        height,
                        24);

            glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);

            glPixelStorei (GL_UNPACK_ROW_LENGTH, width);
            glTexSubImage2D (GL_TEXTURE_2D,
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
                             PBO_BGRA_NATIVE_ORDER,
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

    if (using_shaders && using_glsl_shaders)
    {
        glsl_shader->render (texmap, width, height, x, allocation.height - y - h, w, h, S9xViewportCallback);
        gl_swap ();
        return;
    }
    else if (using_shaders)
    {
        GLint location;
        float inputSize[2];
        float outputSize[2];
        float textureSize[2];

        inputSize[0] = width;
        inputSize[1] = height;
        location = glGetUniformLocation (program, "rubyInputSize");
        glUniform2fv (location, 1, inputSize);

        outputSize[0] = w;
        outputSize[1] = h;
        location = glGetUniformLocation (program, "rubyOutputSize");
        glUniform2fv (location, 1, outputSize);

        textureSize[0] = texture_width;
        textureSize[1] = texture_height;
        location = glGetUniformLocation (program, "rubyTextureSize");
        glUniform2fv (location, 1, textureSize);
    }

    glDrawArrays (GL_QUADS, 0, 4);

    gl_swap ();

    return;
}

void *
S9xOpenGLDisplayDriver::get_parameters(void)
{
    if (using_glsl_shaders && glsl_shader)
    {
        return (void *) &glsl_shader->param;
    }
    return NULL;
}

void
S9xOpenGLDisplayDriver::save (const char *filename)
{
    if (using_glsl_shaders && glsl_shader)
    {
        glsl_shader->save(filename);
    }

    return;
}

void
S9xOpenGLDisplayDriver::clear_buffers (void)
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

    return;
}

void
S9xOpenGLDisplayDriver::update_texture_size (int width, int height)
{
    if (width != texture_width || height != texture_height)
    {
        if (dyn_resizing)
        {
            glBindTexture (GL_TEXTURE_2D, texmap);

            if (using_pbos)
            {
                glTexImage2D (GL_TEXTURE_2D,
                              0,
                              config->pbo_format == PBO_FMT_16 ? GL_RGB565 : 4,
                              width,
                              height,
                              0,
                              PBO_GET_FORMAT (config->pbo_format),
                              PBO_GET_PACKING (config->pbo_format),
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

    return;
}

int
S9xOpenGLDisplayDriver::pbos_available (void)
{

    if (gl_version_at_least (2, 1))
            return 1;

    const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

    if (!extensions)
        return 0;

    if (strstr (extensions, "pixel_buffer_object"))
    {
            return 1;
    }

    return 0;
}

int
S9xOpenGLDisplayDriver::shaders_available (void)
{
    const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

    if (!extensions)
        return 0;

    if (strstr (extensions, "fragment_program") ||
        strstr (extensions, "fragment_shader"))
    {
        return 1;
    }

    return 0;
}

static int npot_available (void)
{
    if (gl_version_at_least (2, 0))
        return true;

    const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

    if (!extensions)
        return 0;

    if (strstr (extensions, "non_power_of_two") ||
        strstr (extensions, "npot"))
    {
        return 1;
    }

    return 0;
}

int
S9xOpenGLDisplayDriver::load_shaders (const char *shader_file)
{
    xmlDoc *xml_doc = NULL;
    xmlNodePtr node = NULL;
    char *fragment = NULL, *vertex = NULL;

    int length = strlen (shader_file);

    if ((length > 6 && !strcasecmp(shader_file + length - 6, ".glslp")) ||
        (length > 5 && !strcasecmp(shader_file + length - 5, ".glsl")))
    {
        if (shaders_available() && npot_available())
        {
            glsl_shader = new GLSLShader;
            if (glsl_shader->load_shader ((char *) shader_file))
            {
                using_glsl_shaders = 1;
                dyn_resizing = TRUE;

                if (glsl_shader->param.size () > 0)
                    window->enable_widget ("shader_parameters_item", TRUE);

                return 1;
            }
            delete glsl_shader;
            return 0;
        }
        else
        {
            printf ("Need shader extensions and non-power-of-two-textures for GLSL.\n");
        }
    }

    if (!shaders_available ())
    {
        fprintf (stderr, _("Cannot load GLSL shader functions.\n"));
        return 0;
    }

    xml_doc = xmlReadFile (shader_file, NULL, 0);

    if (!xml_doc)
    {
        fprintf (stderr, _("Cannot read shader file.\n"));
        return 0;
    }

    node = xmlDocGetRootElement (xml_doc);

    if (xmlStrcasecmp (node->name, BAD_CAST "shader"))
    {
        fprintf (stderr, _("File %s is not a shader file.\n"), shader_file);
        xmlFreeDoc (xml_doc);
        return 0;
    }

    for (xmlNodePtr i = node->children; i; i = i->next)
    {
        if (!xmlStrcasecmp (i->name, BAD_CAST "vertex"))
        {
            if (i->children)
                vertex = (char *) i->children->content;
        }

        if (!xmlStrcasecmp (i->name, BAD_CAST "fragment"))
        {
            if (i->children)
                fragment = (char *) i->children->content;
        }
    }

    if (!vertex && !fragment)
    {
        fprintf (stderr, _("Shader lacks any programs.\n"));
        xmlFreeDoc (xml_doc);
        return 0;
    }

    program = glCreateProgram ();

    if (vertex)
    {
        vertex_shader = glCreateShader (GL_VERTEX_SHADER);
        glShaderSource (vertex_shader, 1, (const GLchar **) &vertex, NULL);
        glCompileShader (vertex_shader);
        glAttachShader (program, vertex_shader);
    }

    if (fragment)
    {
        fragment_shader = glCreateShader (GL_FRAGMENT_SHADER);
        glShaderSource (fragment_shader, 1, (const GLchar **) &fragment, NULL);
        glCompileShader (fragment_shader);
        glAttachShader (program, fragment_shader);
    }

    glLinkProgram (program);
    glUseProgram (program);

    xmlFreeDoc (xml_doc);

    return 1;
}

int
S9xOpenGLDisplayDriver::opengl_defaults (void)
{
    dyn_resizing = FALSE;
    using_pbos = 0;

    if (config->use_pbos)
    {
        if (!pbos_available ())
        {
            fprintf (stderr, _("pixel_buffer_object extension not supported.\n"));

            config->use_pbos = 0;
        }
        else
        {
            using_pbos = 1;
        }
    }

    using_shaders = 0;
    using_glsl_shaders = 0;
    glsl_shader = NULL;

    if (config->use_shaders)
    {
        if (!load_shaders (config->fragment_shader))
        {
            config->use_shaders = 0;
        }
        else
        {
            using_shaders = 1;
        }
    }

    texture_width = 1024;
    texture_height = 1024;

    if (config->npot_textures)
    {
        if (npot_available ())
        {
            dyn_resizing = TRUE;
        }
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
                      config->pbo_format == PBO_FMT_16 ? GL_RGB565 : 4,
                      texture_width,
                      texture_height,
                      0,
                      PBO_GET_FORMAT (config->pbo_format),
                      PBO_GET_PACKING (config->pbo_format),
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

void
S9xOpenGLDisplayDriver::refresh (int width, int height)
{
    return;
}

void
S9xOpenGLDisplayDriver::resize_window (int width, int height)
{

    if (!using_egl)
        gdk_window_destroy (gdk_window);

    create_window (width, height);

    if (using_egl)
        eglMakeCurrent (egl_display, egl_surface, egl_surface, egl_context);
#ifdef GDK_WINDOWING_X11
    else
        glXMakeCurrent (display, xwindow, glx_context);
#endif

    swap_control (config->sync_to_vblank);

    return;
}

#ifdef GDK_WINDOWING_WAYLAND
static void wl_global (void *data,
                       struct wl_registry *wl_registry,
                       uint32_t name,
                       const char *interface,
                       uint32_t version)
{
    struct wl_collection *wl = (struct wl_collection *) data;

    if (!strcmp (interface, "wl_compositor"))
        wl->compositor = (struct wl_compositor *) wl_registry_bind (wl_registry, name, &wl_compositor_interface, 1);
    else if (!strcmp (interface, "wl_subcompositor"))
        wl->subcompositor = (struct wl_subcompositor *) wl_registry_bind (wl_registry, name, &wl_subcompositor_interface, 1);

    return;
}

static void wl_global_remove (void *data,
                              struct wl_registry *wl_registry,
                              uint32_t name)
{
    return;
}

static const struct wl_registry_listener wl_registry_listener = {
    wl_global,
    wl_global_remove
};
#endif

void
S9xOpenGLDisplayDriver::create_window (int width, int height)
{
    if (using_egl)
    {
        egl_surface = NULL;
        gdk_window = gtk_widget_get_window (drawing_area);

#ifdef GDK_WINDOWING_X11
        if (GDK_IS_X11_WINDOW (gdk_window))
        {
            xwindow = GDK_COMPAT_WINDOW_XID (gdk_window);
            egl_surface = eglCreateWindowSurface (egl_display, egl_config, (EGLNativeWindowType) xwindow, NULL);
        }
#endif
#ifdef GDK_WINDOWING_WAYLAND
        if (GDK_IS_WAYLAND_WINDOW (gdk_window))
        {
            int x, y, w, h;
            gdk_window_get_geometry (gdk_window, &x, &y, &w, &h);

            /* Gets whole window surface */
            wl.parent = gdk_wayland_window_get_wl_surface (gdk_window);
            wl.display = gdk_wayland_display_get_wl_display (gdk_window_get_display (gdk_window));
            wl.registry = wl_display_get_registry (wl.display);
            wl.compositor = NULL;
            wl.subcompositor = NULL;
            wl_registry_add_listener (wl.registry, &wl_registry_listener, &wl);
            wl_display_roundtrip (wl.display);

            if (!wl.compositor || !wl.subcompositor)
                return;

            if (!wl.child)
                wl.child = wl_compositor_create_surface (wl.compositor);

            if (!wl.subsurface)
            {
                wl.subsurface = wl_subcompositor_get_subsurface (wl.subcompositor, wl.child, wl.parent);
            }

            wl_subsurface_set_desync (wl.subsurface);
            wl_subsurface_set_position (wl.subsurface, x, y);

            if (wl.egl_window)
                wl_egl_window_resize (wl.egl_window, w, h, 0, 0);
            else
                wl.egl_window = wl_egl_window_create (wl.child, w, h);

            if (!egl_surface)
                egl_surface = eglCreateWindowSurface (egl_display, egl_config, (EGLNativeWindowType) wl.egl_window, NULL);
        }
#endif
    }
#ifdef GDK_WINDOWING_X11
    else
    {
        GdkWindowAttr window_attr;
        memset (&window_attr, 0, sizeof (GdkWindowAttr));
        window_attr.event_mask = GDK_EXPOSURE_MASK | GDK_STRUCTURE_MASK;
        window_attr.width = width;
        window_attr.height = height;
        window_attr.x = 0;
        window_attr.y = 0;
        window_attr.wclass = GDK_INPUT_OUTPUT;
        window_attr.window_type = GDK_WINDOW_CHILD;
        window_attr.visual = gdk_x11_screen_lookup_visual (gtk_widget_get_screen (drawing_area), vi->visualid);

        gdk_window = gdk_window_new (gtk_widget_get_window (drawing_area),
                                     &window_attr,
                                     GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL);
        gdk_window_set_user_data (gdk_window, (gpointer) drawing_area);

        gdk_window_show (gdk_window);
        xwindow = GDK_COMPAT_WINDOW_XID (gdk_window);
    }
#endif

    output_window_width = width;
    output_window_height = height;
}

int
S9xOpenGLDisplayDriver::init_gl (void)
{
    using_egl = 0;
    if (epoxy_has_egl ())
    {
        printf ("Using EGL context\n");

        using_egl = 1;
        EGLint num_configs;
        EGLint const egl_attribs[] = { EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT, EGL_NONE };

        egl_display = NULL;
        GdkDisplay *d = gdk_display_get_default ();

#ifdef GDK_WINDOWING_X11
        if (GDK_IS_X11_DISPLAY (d))
        {
            display = GDK_DISPLAY_XDISPLAY (d);
            egl_display = eglGetDisplay ((EGLNativeDisplayType) display);
        }
#endif
#ifdef GDK_WINDOWING_WAYLAND
        if (GDK_IS_WAYLAND_DISPLAY (d))
        {
            struct wl_display *wl_d = gdk_wayland_display_get_wl_display (d);
            egl_display = eglGetDisplay ((EGLNativeDisplayType) wl_d);
        }
#endif
        if (!egl_display)
            return 0;

        eglInitialize (egl_display, NULL, NULL);
        eglChooseConfig (egl_display, egl_attribs, &egl_config, 1, &num_configs);
        eglBindAPI (EGL_OPENGL_API);

        if (num_configs < 1)
        {
            fprintf (stderr, _("Couldn't find any appropriate EGL configs.\n"));
            return 0;
        }

#ifdef GDK_WINDOWING_WAYLAND
        if (on_wayland ())
            memset (&wl, 0, sizeof (wl));
#endif
        create_window (1, 1);
        if (!egl_surface)
        {
            fprintf (stderr, _("Couldn't create an EGL surface.\n"));
            return 0;
        }

        egl_context = eglCreateContext (egl_display, egl_config, EGL_NO_CONTEXT, NULL);

        if (!egl_context)
        {
            fprintf (stderr, _("Couldn't create an EGL context.\n"));
            eglDestroySurface (egl_display, egl_surface);
            return 0;
        }

        if (!eglMakeCurrent (egl_display, egl_surface, egl_surface, egl_context))
        {
            fprintf (stderr, _("Couldn't make current EGL context.\n"));
            eglDestroyContext (egl_display, egl_context);
            eglDestroySurface (egl_display, egl_surface);
            return 0;
        }
    }

#ifdef GDK_WINDOWING_X11
    else /* GLX */
    {
        printf ("Using GLX context\n");

        int glx_attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };

        display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

        vi = glXChooseVisual (display, DefaultScreen (display), glx_attribs);

        if (!vi)
        {
            fprintf (stderr, _("Couldn't find an adequate OpenGL visual.\n"));
            return 0;
        }

        xcolormap = XCreateColormap (display,
                                     GDK_COMPAT_WINDOW_XID (gtk_widget_get_window (drawing_area)),
                                     vi->visual,
                                     AllocNone);

        create_window (1, 1);
        gdk_window_hide (gdk_window);

        glx_context = glXCreateContext (display, vi, 0, 1);

        if (!glx_context)
        {
            XFreeColormap (display, xcolormap);
            gdk_window_destroy (gdk_window);

            fprintf (stderr, _("Couldn't create an OpenGL context.\n"));
            return 0;
        }

        if (!glXMakeCurrent (display, xwindow, glx_context))
        {
            XFreeColormap (display, xcolormap);
            g_object_unref (gdk_window);
            gdk_window_destroy (gdk_window);

            fprintf (stderr, "glXMakeCurrent failed.\n");
            return 0;
        }
    }
#endif

    return 1;
}

int
S9xOpenGLDisplayDriver::init (void)
{
    initialized = 0;

    if (!init_gl ())
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

    swap_control (config->sync_to_vblank);

    initialized = 1;

    return 0;
}

void
S9xOpenGLDisplayDriver::swap_control (int enable)
{
    enable = enable ? 1 : 0;

    if (using_egl)
    {
        eglSwapInterval (egl_display, enable);
    }
#ifdef GDK_WINDOWING_X11
    else
    {
        const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

        if (strstr (extensions, "EXT_swap_control"))
        {
            glXSwapIntervalEXT (display, xwindow, enable);
        }
        else if (strstr (extensions, "SGI_swap_control"))
        {
            glXSwapIntervalSGI (enable);
        }
    }
#endif

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

    return;
}

uint16 *
S9xOpenGLDisplayDriver::get_current_buffer (void)
{
    return (uint16 *) padded_buffer[0];
}

void
S9xOpenGLDisplayDriver::gl_swap (void)
{
    if (using_egl)
    {
        eglSwapBuffers (egl_display, egl_surface);
#ifdef GDK_WINDOWING_WAYLAND
        if (on_wayland ())
        {
            wl_display_flush (wl.display);
        }
#endif
    }
#ifdef GDK_WINDOWING_X11
    else
        glXSwapBuffers (display, xwindow);
#endif

    if (config->sync_every_frame)
    {
        usleep (0);
        glFinish ();
    }

    return;
}

void
S9xOpenGLDisplayDriver::deinit (void)
{
    if (!initialized)
        return;

    if (using_shaders && using_glsl_shaders)
    {
        window->enable_widget ("shader_parameters_item", FALSE);
        gtk_shader_parameters_dialog_close ();
        glsl_shader->destroy();
        delete glsl_shader;
    }
    else if (using_shaders)
    {
        glUseProgram (0);
        glDetachShader (program, vertex_shader);
        glDetachShader (program, fragment_shader);
        glDeleteShader (vertex_shader);
        glDeleteShader (fragment_shader);
        glDeleteProgram (program);
        using_shaders = 0;
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

    if (using_egl)
    {
        eglDestroyContext (egl_display, egl_context);
        eglDestroySurface (egl_display, egl_surface);
#ifdef GDK_WINDOWING_WAYLAND
        if (on_wayland ())
        {
            if (wl.egl_window)
                wl_egl_window_destroy (wl.egl_window);
            if (wl.subsurface)
                wl_subsurface_destroy (wl.subsurface);
            if (wl.child)
                wl_surface_destroy (wl.child);
        }
#endif
    }
#ifdef GDK_WINDOWING_X11
    else
    {
        glXDestroyContext (display, glx_context);
        gdk_window_destroy (gdk_window);
        XFree (vi);
        XFreeColormap (display, xcolormap);
    }
#endif

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
    if (epoxy_has_egl ())
        return 1;

#ifdef GDK_WINDOWING_X11
    Display *dpy = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    if (epoxy_has_glx (dpy))
        return 1;
#endif

    if (gui_config->hw_accel == HWA_OPENGL)
        gui_config->hw_accel = HWA_NONE;

    return 0;
}
