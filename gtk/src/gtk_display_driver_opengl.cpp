#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <fcntl.h>
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
    uint8 *final_buffer = NULL;
    int   final_pitch;
    void  *pboMemory = NULL;
    int   x, y, w, h;

    GtkAllocation allocation;
    gtk_widget_get_allocation (drawing_area, &allocation);

#if GTK_CHECK_VERSION(3,10,0)
    int gdk_scale_factor = gdk_window_get_scale_factor (gdk_window);

    allocation.width *= gdk_scale_factor;
    allocation.height *= gdk_scale_factor;

#endif

    if (output_window_width  != allocation.width ||
        output_window_height != allocation.height)
    {
        resize_window (allocation.width, allocation.height);
    }

    GLint filter = config->bilinear_filter ? GL_LINEAR : GL_NEAREST;
    glTexParameteri (tex_target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri (tex_target, GL_TEXTURE_MIN_FILTER, filter);
    GLint clamp = (using_shaders || !dyn_resizing) ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE;
    glTexParameteri (tex_target, GL_TEXTURE_WRAP_S, clamp);
    glTexParameteri (tex_target, GL_TEXTURE_WRAP_T, clamp);

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

    if (using_shaders)
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

void
S9xOpenGLDisplayDriver::clear_buffers (void)
{
    memset (buffer[0], 0, image_padded_size);
    memset (buffer[1], 0, scaled_padded_size);

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
                              PBO_GET_FORMAT (config->pbo_format),
                              PBO_GET_PACKING (config->pbo_format),
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

    if (!extensions)
        return 0;

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

        if (glGenBuffers    &&
            glBindBuffer    &&
            glBufferData    &&
            glBufferSubData &&
            glMapBuffer     &&
            glUnmapBuffer   &&
            glDeleteBuffers)
        {
            return 1;
        }
    }

    return 0;
}

int
S9xOpenGLDisplayDriver::load_shader_functions (void)
{
    const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

    if (!extensions)
        return 0;

    if (strstr (extensions, "fragment_program"))
    {
        glCreateProgram = (glCreateProgramProc) glGetProcAddress ((GLubyte *) "glCreateProgram");
        glCreateShader = (glCreateShaderProc) glGetProcAddress ((GLubyte *) "glCreateShader");
        glCompileShader = (glCompileShaderProc) glGetProcAddress ((GLubyte *) "glCompileShader");
        glDeleteShader = (glDeleteShaderProc) glGetProcAddress ((GLubyte *) "glDeleteShader");
        glDeleteProgram = (glDeleteProgramProc) glGetProcAddress ((GLubyte *) "glDeleteProgram");
        glAttachShader = (glAttachShaderProc) glGetProcAddress ((GLubyte *) "glAttachShader");
        glDetachShader = (glDetachShaderProc) glGetProcAddress ((GLubyte *) "glDetachShader");
        glLinkProgram = (glLinkProgramProc) glGetProcAddress ((GLubyte *) "glLinkProgram");
        glUseProgram = (glUseProgramProc) glGetProcAddress ((GLubyte *) "glUseProgram");
        glShaderSource = (glShaderSourceProc) glGetProcAddress ((GLubyte *) "glShaderSource");
        glGetUniformLocation = (glGetUniformLocationProc) glGetProcAddress ((GLubyte *) "glGetUniformLocation");
        glUniform2fv = (glUniform2fvProc) glGetProcAddress ((GLubyte *) "glUniform2fv");

        if (glCreateProgram      &&
            glCreateShader       &&
            glCompileShader      &&
            glDeleteShader       &&
            glDeleteProgram      &&
            glAttachShader       &&
            glDetachShader       &&
            glLinkProgram        &&
            glUseProgram         &&
            glShaderSource       &&
            glGetUniformLocation &&
            glUniform2fv)
        {
            return 1;
        }
    }

    return 0;
}

int
S9xOpenGLDisplayDriver::load_shaders (const char *shader_file)
{
    xmlDoc *xml_doc = NULL;
    xmlNodePtr node = NULL;
    char *fragment = NULL, *vertex = NULL;

    if (!load_shader_functions ())
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

    using_shaders = 0;
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

    tex_target = GL_TEXTURE_2D;
    texture_width = 1024;
    texture_height = 1024;
    dyn_resizing = FALSE;

    const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

    if (extensions && config->npot_textures)
    {
        if (!using_shaders && strstr (extensions, "_texture_rectangle"))
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
    g_object_unref (gdk_window);
    XDestroyWindow (display, xwindow);
    create_window (width, height);
    glXMakeCurrent (display, xwindow, glx_context);

    return;
}

void
S9xOpenGLDisplayDriver::create_window (int width, int height)
{
    XSetWindowAttributes window_attr;

    window_attr.colormap = xcolormap;
    window_attr.border_pixel = 0;
    window_attr.event_mask = StructureNotifyMask | ExposureMask;
    window_attr.do_not_propagate_mask = 0;
    window_attr.save_under = False;
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
                             CWColormap | CWBorderPixel | CWBackPixmap | CWEventMask | CWSaveUnder | CWDontPropagate,
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

int
S9xOpenGLDisplayDriver::init_glx (void)
{
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
        g_object_unref (gdk_window);
        XDestroyWindow (display, xwindow);

        fprintf (stderr, _("Couldn't create an OpenGL context.\n"));
        return 0;
    }

    if (!glXMakeCurrent (display, xwindow, glx_context))
    {
        XFreeColormap (display, xcolormap);
        g_object_unref (gdk_window);
        XDestroyWindow (display, xwindow);
        glXDestroyContext (display, glx_context);

        fprintf (stderr, "glXMakeCurrent failed.\n");
        return 0;
    }

    return 1;
}

int
S9xOpenGLDisplayDriver::init (void)
{
    int padding;
    initialized = 0;

    if (!init_glx ())
    {
        return -1;
    }

    if (!opengl_defaults ())
    {
        return -1;
    }

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

    swap_control (config->sync_to_vblank);

    initialized = 1;

    return 0;
}

void
S9xOpenGLDisplayDriver::swap_control (int enable)
{
    static glSwapIntervalProc     glSwapInterval = NULL;
    static glXSwapIntervalEXTProc glXSwapIntervalEXT = NULL;
    static int                    queried = FALSE;
    const char                   *ext_str;

    enable = enable ? 1 : 0;

    if (!queried)
    {
        ext_str = glXQueryExtensionsString (display, DefaultScreen (display));

        /* We try to set this with both extensions since some cards pretend
         * to support both, but ignore one. */

        if (strstr (ext_str, "GLX_MESA_swap_control"))
        {
            glSwapInterval = (glSwapIntervalProc)
                    glGetProcAddress ((GLubyte *) "glXSwapIntervalMESA");
        }

        if (strstr (ext_str, "GLX_SGI_swap_control"))
        {
            glSwapInterval = (glSwapIntervalProc)
                    glGetProcAddress ((GLubyte *) "glXSwapIntervalSGI");
        }

        if (strstr (ext_str, "GLX_EXT_swap_control"))
        {
            glXSwapIntervalEXT = (glXSwapIntervalEXTProc)
                    glGetProcAddress ((GLubyte *) "glXSwapIntervalEXT");
        }

        queried = TRUE;
    }

    if (glXSwapIntervalEXT)
    {
        if (glSwapInterval)
            glSwapInterval (0);

        glXSwapIntervalEXT (display, xwindow, enable);
    }

    else if (glSwapInterval)
    {
        glSwapInterval (enable);
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
S9xOpenGLDisplayDriver::gl_swap (void)
{
    glXSwapBuffers (display, xwindow);

    if (config->sync_every_frame)
    {
        usleep (0);
        glXWaitX ();
        glFinish ();
    }

    return;
}

void
S9xOpenGLDisplayDriver::deinit (void)
{
    if (!initialized)
        return;

    if (using_shaders)
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
    glXDestroyContext (display, glx_context);

    XFree (vi);
    XFreeColormap (display, xcolormap);

    g_object_unref (gdk_window);
    XDestroyWindow (display, xwindow);

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
    Display *display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    if (glXQueryExtension (display, &errorBase, &eventBase) == False)
    {
        if (gui_config->hw_accel == HWA_OPENGL)
            gui_config->hw_accel = HWA_NONE;
        return 0;
    }

    return 1;
}
