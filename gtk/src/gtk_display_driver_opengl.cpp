/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <dlfcn.h>
#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include "gtk_display.h"
#include "gtk_display_driver_opengl.h"
#include "gtk_shader_parameters.h"

#include "snes9x_imgui.h"
#include "imgui_impl_opengl3.h"

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

#ifdef GDK_WINDOWING_WAYLAND
static WaylandSurface::Metrics get_metrics(Gtk::DrawingArea &w)
{
    int x, y, width, height;
    w.get_window()->get_geometry(x, y, width, height);
    return { x, y, width, height, w.get_window()->get_scale_factor() };
}
#endif

static GLfloat coords[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
                             0.0f,  1.0f, 1.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, };

static void S9xViewportCallback(int src_width, int src_height,
                                int viewport_x, int viewport_y,
                                int viewport_width, int viewport_height,
                                int *out_x, int *out_y,
                                int *out_width, int *out_height)
{
    S9xRect dst = S9xApplyAspect(src_width, src_height, viewport_width, viewport_height);
    *out_x = dst.x + viewport_x;
    *out_y = dst.y + viewport_y;
    *out_width = dst.w;
    *out_height = dst.h;
}

S9xOpenGLDisplayDriver::S9xOpenGLDisplayDriver(Snes9xWindow *window, Snes9xConfig *config)
{
    this->window = window;
    this->config = config;
    this->drawing_area = window->drawing_area;
}

void S9xOpenGLDisplayDriver::update(uint16_t *buffer, int width, int height, int stride_in_pixels)
{
    Gtk::Allocation allocation = drawing_area->get_allocation();
    if (output_window_width != allocation.get_width() ||
        output_window_height != allocation.get_height())
    {
        resize();
    }

    if (!legacy)
        glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texmap);
    GLint filter = Settings.BilinearFilter ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    GLint clamp = (using_glsl_shaders) ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);

    glClear(GL_COLOR_BUFFER_BIT);

    S9xRect content = S9xApplyAspect(width, height, context->width, context->height);
    glViewport(content.x, context->height - content.y - content.h, content.w, content.h);
    window->set_mouseable_area(content.x, content.y, content.w, content.h);

    update_texture_size(width, height);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride_in_pixels);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer);

    if (using_glsl_shaders)
    {
        glsl_shader->render(texmap, width, height, content.x, context->height - content.y - content.h, content.w, content.h, S9xViewportCallback);
    }
    else
    {
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    if (S9xImGuiRunning())
    {
        ImGui_ImplOpenGL3_NewFrame();
        if (S9xImGuiDraw(context->width, context->height))
        {

            auto *draw_data = ImGui::GetDrawData();
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        }
    }

    swap_buffers();
}

void *S9xOpenGLDisplayDriver::get_parameters()
{
    if (using_glsl_shaders && glsl_shader)
    {
        return (void *)&glsl_shader->param;
    }

    return NULL;
}

void S9xOpenGLDisplayDriver::save(const char *filename)
{
    if (using_glsl_shaders && glsl_shader)
    {
        glsl_shader->save(filename);
    }
}

void S9xOpenGLDisplayDriver::update_texture_size(int width, int height)
{
    if (width != texture_width || height != texture_height)
    {
        glBindTexture(GL_TEXTURE_2D, texmap);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB565,
                     width,
                     height,
                     0,
                     GL_RGB,
                     GL_UNSIGNED_SHORT_5_6_5,
                     NULL);

        texture_width = width;
        texture_height = height;

        if (!legacy)
        {
            glBindBuffer(GL_ARRAY_BUFFER, coord_buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, coords, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glVertexPointer(2, GL_FLOAT, 0, coords);
            glTexCoordPointer(2, GL_FLOAT, 0, &coords[8]);
        }
    }
}

bool S9xOpenGLDisplayDriver::load_shaders(const char *shader_file)
{
    setlocale(LC_NUMERIC, "C");
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

            if (glsl_shader->param.size() > 0)
                window->enable_widget("shader_parameters_item", true);

            setlocale(LC_NUMERIC, "");
            return true;
        }

        delete glsl_shader;
    }

    setlocale(LC_NUMERIC, "");
    return false;
}

bool S9xOpenGLDisplayDriver::opengl_defaults()
{
    using_glsl_shaders = false;
    glsl_shader = NULL;

    if (config->use_shaders)
    {
        if (legacy || !load_shaders(config->shader_filename.c_str()))
        {
            config->use_shaders = false;
        }
    }

    texture_width = 256;
    texture_height = 224;

    if (legacy)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glVertexPointer(2, GL_FLOAT, 0, coords);
        glTexCoordPointer(2, GL_FLOAT, 0, &coords[8]);
    }
    else
    {
        stock_program = glCreateProgram();

        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        if (version < 30)
        {
            glShaderSource(vertex_shader, 1, &stock_vertex_shader_110, NULL);
            glShaderSource(fragment_shader, 1, &stock_fragment_shader_110, NULL);
        }
        else
        {
            glShaderSource(vertex_shader, 1, &stock_vertex_shader_140, NULL);
            glShaderSource(fragment_shader, 1, &stock_fragment_shader_140, NULL);
        }

        glCompileShader(vertex_shader);
        glAttachShader(stock_program, vertex_shader);
        glCompileShader(fragment_shader);
        glAttachShader(stock_program, fragment_shader);

        glBindAttribLocation(stock_program, 0, "in_position");
        glBindAttribLocation(stock_program, 1, "in_texcoord");

        glLinkProgram(stock_program);
        glUseProgram(stock_program);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        GLint texture_uniform = glGetUniformLocation(stock_program, "texmap");
        glUniform1i(texture_uniform, 0);

        if (core)
        {
            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
        }

        glGenBuffers(1, &coord_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, coord_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, coords, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(32));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glGenTextures(1, &texmap);

    glBindTexture(GL_TEXTURE_2D, texmap);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB565,
                 texture_width,
                 texture_height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_SHORT_5_6_5,
                 NULL);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    return true;
}

void S9xOpenGLDisplayDriver::refresh()
{
    resize();
}

void S9xOpenGLDisplayDriver::resize()
{
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW(gdk_window))
    {
        ((WaylandEGLContext *)context)->resize(get_metrics(*drawing_area));
    }
#endif
#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_WINDOW(gdk_window))
    {
        context->resize();
    }
#endif


    context->swap_interval(config->sync_to_vblank);
    Gtk::Allocation allocation = drawing_area->get_allocation();
    output_window_width = allocation.get_width();
    output_window_height = allocation.get_height();
}

bool S9xOpenGLDisplayDriver::create_context()
{
    gdk_window = drawing_area->get_window()->gobj();
    GdkDisplay *gdk_display = drawing_area->get_display()->gobj();

#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW(gdk_window))
    {
        wl_surface *surface = gdk_wayland_window_get_wl_surface(drawing_area->get_window()->gobj());
        wl_display *display = gdk_wayland_display_get_wl_display(drawing_area->get_display()->gobj());
        if (!wl.attach(display, surface, get_metrics(*drawing_area)))
            return false;
        context = &wl;
    }
#endif
#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_WINDOW(gdk_window))
    {
        if (!glx.attach(gdk_x11_display_get_xdisplay(gdk_display), gdk_x11_window_get_xid(gdk_window)))
            return false;
        context = &glx;
    }
#endif

    if (!context->create_context())
        return false;

    context->make_current();
    gladLoaderLoadGL();

    legacy = false;

    auto version_string = (const char *)glGetString(GL_VERSION);
    int major_version = 1;
    int minor_version = 0;
    std::sscanf(version_string, "%d.%d", &major_version, &minor_version);
    version = major_version * 10 + minor_version;

    if (version < 20)
    {
        printf("OpenGL version is only %d.%d. Recommended version is 2.0.\n",
               version / 10,
               version % 10);
        legacy = true;
    }

    int profile_mask = 0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile_mask);
    if (profile_mask & GL_CONTEXT_CORE_PROFILE_BIT)
        core = true;
    else
        core = false;

    return true;
}

int S9xOpenGLDisplayDriver::init()
{
    initialized = false;

    if (!create_context())
    {
        return -1;
    }

    if (!opengl_defaults())
    {
        return -1;
    }

    context->swap_interval(config->sync_to_vblank);

    if (version >= 33)
    {
        auto defaults = S9xImGuiGetDefaults();
        defaults.font_size = gui_config->osd_size;
        defaults.spacing = defaults.font_size / 2.4;
        S9xImGuiInit(&defaults);
        ImGui_ImplOpenGL3_Init();
    }

    initialized = true;

    return 0;
}

void S9xOpenGLDisplayDriver::swap_buffers()
{
    if (Settings.SkipFrames == THROTTLE_TIMER_FRAMESKIP || Settings.SkipFrames == THROTTLE_TIMER)
    {
        throttle.set_frame_rate(Settings.PAL ? PAL_PROGRESSIVE_FRAME_RATE : NTSC_PROGRESSIVE_FRAME_RATE);
        throttle.wait_for_frame_and_rebase_time();
    }

    context->swap_buffers();

    if (config->reduce_input_lag)
    {
        usleep(0);
        glFinish();
    }
}

void S9xOpenGLDisplayDriver::deinit()
{
    if (!initialized)
        return;

    if (using_glsl_shaders)
    {
        window->enable_widget("shader_parameters_item", false);
        gtk_shader_parameters_dialog_close();
        glsl_shader->destroy();
        delete glsl_shader;
    }

    glDeleteTextures(1, &texmap);

    if (S9xImGuiRunning())
    {
        ImGui_ImplOpenGL3_Shutdown();
        S9xImGuiDeinit();
    }

    gladLoaderUnloadGL();
}

int S9xOpenGLDisplayDriver::query_availability()
{
    GdkDisplay *gdk_display = gdk_display_get_default();

#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_DISPLAY(gdk_display))
    {
        return 1;
    }
#endif

#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_DISPLAY(gdk_display))
    {
            return 1;
    }
#endif

    return 0;
}

bool S9xOpenGLDisplayDriver::is_ready()
{
    if (context->ready())
    {
        return true;
    }

    return false;
}

void S9xOpenGLDisplayDriver::shrink()
{
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW(gdk_window))
    {
        ((WaylandEGLContext *)context)->shrink();
    }
#endif
}

void S9xOpenGLDisplayDriver::regrow()
{
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW(gdk_window))
    {
        ((WaylandEGLContext *)context)->regrow();
    }
#endif
}
