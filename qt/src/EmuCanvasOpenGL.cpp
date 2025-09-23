#include "EmuCanvasOpenGL.hpp"
#include <QtGui/QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <QMessageBox>
#include "common/video/opengl/opengl_context.hpp"

#ifndef _WIN32
#include "common/video/opengl/glx_context.hpp"
#include "common/video/opengl/wayland_egl_context.hpp"
using namespace QNativeInterface;
#include <X11/Xlib.h>
#else
#include "common/video/opengl/wgl_context.hpp"
#endif
#include "common/video/opengl/shaders/glsl.h"
#include "EmuMainWindow.hpp"
#include "snes9x_imgui.h"
#include "imgui_impl_opengl3.h"
#include <clocale>

static const char *stock_vertex_shader_140 = R"(
#version 140

in vec2 in_position;
in vec2 in_texcoord;
out vec2 texcoord;

void main()
{
    gl_Position = vec4(in_position, 0.0, 1.0);
    texcoord = in_texcoord;
}
)";

static const char *stock_fragment_shader_140 = R"(
#version 140

uniform sampler2D texmap;
out vec4 fragcolor;
in vec2 texcoord;

void main()
{
    fragcolor = texture(texmap, texcoord);
}
)";

EmuCanvasOpenGL::EmuCanvasOpenGL(EmuConfig *config, QWidget *main_window)
    : EmuCanvas(config, main_window)
{
    setMinimumSize(256 / devicePixelRatioF(), 224 / devicePixelRatioF());
    setUpdatesEnabled(false);
    setAutoFillBackground(false);

    if (QGuiApplication::platformName() == "wayland")
    {
        main_window->createWinId();
        return;
    }

    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_OpaquePaintEvent);

    createWinId();
}

void EmuCanvasOpenGL::createStockShaders()
{
    stock_program = glCreateProgram();

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader, 1, &stock_vertex_shader_140, nullptr);
    glShaderSource(fragment_shader, 1, &stock_fragment_shader_140, nullptr);

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
}

void EmuCanvasOpenGL::stockShaderDraw()
{
    auto viewport = applyAspect(QRect(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio()));
    glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());

    GLint texture_uniform = glGetUniformLocation(stock_program, "texmap");
    glUniform1i(texture_uniform, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static QRect g_viewport;
static void S9xViewportCallback(int src_width, int src_height,
                                int viewport_x, int viewport_y,
                                int viewport_width, int viewport_height,
                                int *out_x, int *out_y,
                                int *out_width, int *out_height)
{
    *out_x = g_viewport.x();
    *out_y = g_viewport.y();
    *out_width = g_viewport.width();
    *out_height = g_viewport.height();
}

void EmuCanvasOpenGL::customShaderDraw()
{
    auto viewport = applyAspect(QRect(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio()));
    glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
    g_viewport = viewport;

    shader->render(texture, output_data.width, output_data.height, viewport.x(), viewport.y(), viewport.width(), viewport.height(), S9xViewportCallback);
}

bool EmuCanvasOpenGL::createContext()
{
    if (context)
        return true;

    auto platform = QGuiApplication::platformName();
    auto pni = QGuiApplication::platformNativeInterface();
    QGuiApplication::sync();
#ifndef _WIN32
    if (platform == "wayland")
    {
        auto display = (wl_display *)pni->nativeResourceForWindow("display", main_window->windowHandle());
        auto surface = (wl_surface *)pni->nativeResourceForWindow("surface", main_window->windowHandle());
        auto wayland_egl_context = new WaylandEGLContext();
        int s = devicePixelRatio();

        if (!wayland_egl_context->attach(display, surface, { x() - main_window->x(), y() - main_window->y(), width(), height(), s }))
        {
            printf("Couldn't attach context to wayland surface.\n");
            context.reset();
            return false;
        }

        context.reset(wayland_egl_context);
    }
    else if (platform == "xcb")
    {
        auto display = (Display *)pni->nativeResourceForWindow("display", windowHandle());
        auto xid = (Window)winId();

        auto glx_context = new GTKGLXContext();
        if (!glx_context->attach(display, xid))
        {
            printf("Couldn't attach to X11 window.\n");
            context.reset();
            return false;
        }

        context.reset(glx_context);
    }
#else
    auto hwnd = winId();
    auto wgl_context = new WGLContext();
    if (!wgl_context->attach((HWND)hwnd))
    {
        printf("Couldn't attach to context\n");
        context.reset();
        return false;
    }
    context.reset(wgl_context);
#endif

    if (!context->create_context())
    {
        printf("Couldn't create OpenGL context.\n");
    }

    context->make_current();
    gladLoaderLoadGL();

    if (config->display_messages == EmuConfig::eOnscreen)
    {
        recreateUIAssets();
    }

    loadShaders();

    glGenTextures(1, &texture);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &stock_coord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, stock_coord_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, coords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    context->swap_interval(config->enable_vsync ? 1 : 0);
    QGuiApplication::sync();
    paintEvent(nullptr);

    return true;
}

void EmuCanvasOpenGL::loadShaders()
{
    auto endswith = [&](const std::string &ext) ->bool {
        return config->shader.rfind(ext) == config->shader.length() - ext.length();
    };
    using_shader = true;
    if (!config->use_shader ||
        !(endswith(".glslp") || endswith(".slangp")))
        using_shader = false;

    if (!using_shader)
    {
        createStockShaders();
    }
    else
    {
        auto previous_locale = setlocale(LC_NUMERIC, "C");
        shader = std::make_unique<GLSLShader>();
        if (!shader->load_shader(config->shader.c_str()))
        {
            shader.reset();
            using_shader = false;
            createStockShaders();
        }
        setlocale(LC_NUMERIC, previous_locale);
    }
}

void EmuCanvasOpenGL::uploadTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLuint filter = config->bilinear_filter ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, output_data.bytes_per_line / 2);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, output_data.width, output_data.height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, output_data.buffer);
}

void EmuCanvasOpenGL::draw()
{
    if (!isVisible() || !context)
        return;

    context->make_current();
    gladLoaderLoadGL();

    uploadTexture();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, stock_coord_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void *)32);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!using_shader)
        stockShaderDraw();
    else
        customShaderDraw();

    if (S9xImGuiRunning())
    {
        ImGui_ImplOpenGL3_NewFrame();
        if (context->width <= 0)
            context->width = width() * devicePixelRatioF();
        if (context->height <= 0)
            context->height = height() * devicePixelRatioF();
        if (S9xImGuiDraw(context->width, context->height))
        {
            auto *draw_data = ImGui::GetDrawData();
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        }
    }

    throttle();

    context->swap_buffers();

    if (config->reduce_input_lag)
        glFinish();
}

void EmuCanvasOpenGL::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!context)
        return;

    int s = devicePixelRatio();
    auto platform = QGuiApplication::platformName();
#ifndef _WIN32
    if (QGuiApplication::platformName() == "wayland")
        ((WaylandEGLContext *)context.get())->resize({ x() - main_window->x(), y() - main_window->y(), width(), height(), s });
    else if (platform == "xcb")
        ((GTKGLXContext *)context.get())->resize();
#else
    ((WGLContext *)context.get())->resize();
#endif
}

void EmuCanvasOpenGL::paintEvent(QPaintEvent *event)
{
    // TODO: If emu not running
    if (!context || !isVisible())
        return;

    if (output_data.ready)
    {
        if (!dynamic_cast<EmuMainWindow *>(main_window)->isActivelyDrawing())
            draw();
        return;
    }

    context->resize();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    context->swap_buffers();
}

void EmuCanvasOpenGL::deinit()
{
    shader_parameters_dialog.reset();
    if (ImGui::GetCurrentContext())
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
    }

    context.reset();
}

void EmuCanvasOpenGL::shaderChanged()
{
    shader_parameters_dialog.reset();

    if (shader)
        shader.reset();
    else
        glDeleteProgram(stock_program);

    loadShaders();
    paintEvent(nullptr);
}

void EmuCanvasOpenGL::showParametersDialog()
{
    if (!using_shader)
    {
        QMessageBox::warning(this, tr("OpenGL Driver"), tr("The driver isn't using a specialized shader preset right now."));
        return;
    }

    if (shader && shader->param.empty())
    {
        QMessageBox::information(this, tr("OpenGL Driver"), tr("This shader preset doesn't offer any configurable parameters."));
        return;
    }

    auto parameters = reinterpret_cast<std::vector<EmuCanvas::Parameter> *>(&shader->param);

    if (!shader_parameters_dialog)
        shader_parameters_dialog =
            std::make_unique<ShaderParametersDialog>(this, parameters);

    shader_parameters_dialog->show();
}

void EmuCanvasOpenGL::saveParameters(std::string filename)
{
    if (shader)
        shader->save(filename.c_str());
}

void EmuCanvasOpenGL::recreateUIAssets()
{
    if (S9xImGuiRunning())
    {
        S9xImGuiDeinit();
    }

    if (config->display_messages != EmuConfig::eOnscreen)
        return;

    auto defaults = S9xImGuiGetDefaults();
    defaults.font_size = config->osd_size;
    defaults.spacing = defaults.font_size / 2.4;
    S9xImGuiInit(&defaults);
    ImGui_ImplOpenGL3_Init();
}