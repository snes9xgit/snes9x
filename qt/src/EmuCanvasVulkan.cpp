#include <QtGui/QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <QTimer>
#include <QtEvents>
#include <QMessageBox>
#include "EmuCanvasVulkan.hpp"
#include "src/ShaderParametersDialog.hpp"
#include "EmuMainWindow.hpp"

#include "snes9x_imgui.h"
#include "imgui_impl_vulkan.h"

using namespace QNativeInterface;

EmuCanvasVulkan::EmuCanvasVulkan(EmuConfig *config, QWidget *parent, QWidget *main_window)
    : EmuCanvas(config, parent, main_window)
{
    setMinimumSize(256, 224);
    setUpdatesEnabled(false);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    createWinId();
    window = windowHandle();
}

EmuCanvasVulkan::~EmuCanvasVulkan()
{
    deinit();
}

bool EmuCanvasVulkan::initImGui()
{
    auto defaults = S9xImGuiGetDefaults();
    defaults.font_size = config->osd_size;
    defaults.spacing = defaults.font_size / 2.4;
    S9xImGuiInit(&defaults);

    ImGui_ImplVulkan_LoadFunctions([](const char *function, void *instance) {
        return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(*((VkInstance *)instance), function);
    }, &context->instance.get());

    vk::DescriptorPoolSize pool_sizes[] =
    {
        { vk::DescriptorType::eCombinedImageSampler, 1000 },
        { vk::DescriptorType::eUniformBuffer, 1000 }
    };
    auto descriptor_pool_create_info = vk::DescriptorPoolCreateInfo{}
        .setPoolSizes(pool_sizes)
        .setMaxSets(1000)
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    imgui_descriptor_pool = context->device.createDescriptorPoolUnique(descriptor_pool_create_info);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = context->instance.get();
    init_info.PhysicalDevice = context->physical_device;
    init_info.Device = context->device;;
    init_info.QueueFamily = context->graphics_queue_family_index;
    init_info.Queue = context->queue;
    init_info.DescriptorPool = imgui_descriptor_pool.get();
    init_info.Subpass = 0;
    init_info.MinImageCount = context->swapchain->get_num_frames();
    init_info.ImageCount = context->swapchain->get_num_frames();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, context->swapchain->get_render_pass());

    auto cmd = context->begin_cmd_buffer();
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    context->end_cmd_buffer();
    context->wait_idle();

    return true;
}

void EmuCanvasVulkan::createContext()
{
    if (simple_output)
        return;

    platform = QGuiApplication::platformName();
    auto pni = QGuiApplication::platformNativeInterface();
    setVisible(true);
    QGuiApplication::sync();

    context = std::make_unique<Vulkan::Context>();

#ifdef _WIN32
    auto hwnd = (HWND)winId();
    context->init_win32(nullptr, hwnd, config->display_device_index);
#else
    if (platform == "wayland")
    {
        wayland_surface = std::make_unique<WaylandSurface>();
        if (!main_window->isVisible())
        {
            main_window->setVisible(true);
            QGuiApplication::processEvents();
        }
        auto display = (wl_display *)pni->nativeResourceForWindow("display", window);
        auto surface = (wl_surface *)pni->nativeResourceForWindow("surface", main_window->windowHandle());
        wayland_surface->attach(display, surface, { parent->x(), parent->y(), width(), height(), static_cast<int>(devicePixelRatio()) });
        auto [scaled_width, scaled_height] = wayland_surface->get_size();
        context->init_wayland(display, wayland_surface->child, scaled_width, scaled_height, config->display_device_index);
    }
    else if (platform == "xcb")
    {
        auto display = (Display *)pni->nativeResourceForWindow("display", window);
        auto xid = (Window)winId();

        context->init_Xlib(display, xid, config->display_device_index);
    }
#endif

    if (config->display_messages == EmuConfig::eOnscreen)
        initImGui();

    tryLoadShader();

    QGuiApplication::sync();
    paintEvent(nullptr);
}

void EmuCanvasVulkan::tryLoadShader()
{
    simple_output.reset();
    shader_chain.reset();
    shader_parameters_dialog.reset();

    if (config->use_shader && !config->shader.empty())
    {
        shader_chain = std::make_unique<Vulkan::ShaderChain>(context.get());
        setlocale(LC_NUMERIC, "C");
        current_shader = config->shader;
        if (!shader_chain->load_shader_preset(config->shader))
        {
            printf("Couldn't load shader preset: %s\n", config->shader.c_str());
            shader_chain.reset();
        }
        setlocale(LC_NUMERIC, "");
    }

    if (!shader_chain)
        simple_output = std::make_unique<Vulkan::SimpleOutput>(context.get(), vk::Format::eR5G6B5UnormPack16);
}

void EmuCanvasVulkan::shaderChanged()
{
    if (!config->use_shader)
        current_shader.clear();

    if ((!config->use_shader && shader_chain) ||
        (config->use_shader && current_shader != config->shader))
        tryLoadShader();
}


void EmuCanvasVulkan::draw()
{
    if (!context)
        return;
    if (!window->isVisible())
        return;

    if (S9xImGuiDraw(width() * devicePixelRatioF(), height() * devicePixelRatioF()))
    {
        auto draw_data = ImGui::GetDrawData();
        context->swapchain->on_render_pass_end([&, draw_data] {
            ImGui_ImplVulkan_RenderDrawData(draw_data, context->swapchain->get_cmd());
        });
    }

    auto viewport = applyAspect(QRect(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio()));

    bool retval = false;
    if (shader_chain)
    {
        retval = shader_chain->do_frame_without_swap(output_data.buffer, output_data.width, output_data.height, output_data.bytes_per_line, vk::Format::eR5G6B5UnormPack16, viewport.x(), viewport.y(), viewport.width(), viewport.height());
    }
    else if (simple_output)
    {
        simple_output->set_filter(config->bilinear_filter);
        retval = simple_output->do_frame_without_swap(output_data.buffer, output_data.width, output_data.height, output_data.bytes_per_line, viewport.x(), viewport.y(), viewport.width(), viewport.height());
    }

    if (retval)
    {
        throttle();
        context->swapchain->swap();
        if (config->reduce_input_lag)
            context->wait_idle();
    }
}

void EmuCanvasVulkan::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!context)
        return;

    int width = event->size().width();
    int height = event->size().height();

    context->swapchain->set_vsync(config->enable_vsync);

#ifndef _WIN32
    if (platform == "wayland")
    {
        wayland_surface->resize({ parent->x(), parent->y(), width, height, (int)devicePixelRatio() });
        std::tie(width, height) = wayland_surface->get_size();
        // On Wayland, Vulkan WSI provides the buffer for the subsurface,
        // so we have to specify a width and height instead of polling the parent.
        context->swapchain->check_and_resize(width, height);
        return;
    }
#endif

    context->swapchain->check_and_resize(event->size().width() * devicePixelRatio(), event->size().height() * devicePixelRatio());
}

void EmuCanvasVulkan::paintEvent(QPaintEvent *event)
{
    // TODO: If emu not running
    if (!context || !isVisible())
        return;

    auto window = (EmuMainWindow *)main_window;
    if (output_data.ready)
    {
        if (!window->isActivelyDrawing())
        {
            draw();
        }
        return;
    }

    // Clear to black
    uint8_t buffer[] = { 0, 0, 0, 0 };
    if (shader_chain)
        shader_chain->do_frame(buffer, 1, 1, 1, vk::Format::eR5G6B5UnormPack16, 0, 0, width(), height());
    if (simple_output)
        simple_output->do_frame(buffer, 1, 1, 1, 0, 0, width(), height());
}

void EmuCanvasVulkan::deinit()
{
    shader_parameters_dialog.reset();

    if (ImGui::GetCurrentContext())
    {
        context->wait_idle();
        imgui_descriptor_pool.reset();
        imgui_render_pass.reset();
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();
    }

    simple_output.reset();
    shader_chain.reset();
    context.reset();
#ifndef _WIN32
    wayland_surface.reset();
#endif
}

std::vector<std::string> EmuCanvasVulkan::getDeviceList()
{
    return Vulkan::Context::get_device_list();
}

void EmuCanvasVulkan::showParametersDialog()
{
    if (!context)
    {
        QMessageBox::warning(this, tr("Vulkan Driver"), tr("The vulkan display driver hasn't properly loaded."));
        return;
    }

    if (!shader_chain)
    {
        QMessageBox::warning(this, tr("Vulkan Driver"), tr("The driver isn't using a specialized shader preset right now."));
        return;
    }

    if (shader_chain && shader_chain->preset->parameters.empty())
    {
        QMessageBox::information(this, tr("Vulkan Driver"), tr("This shader preset doesn't offer any configurable parameters."));
        return;
    }

    auto parameters = reinterpret_cast<std::vector<EmuCanvas::Parameter> *>(&shader_chain->preset->parameters);

    if (!shader_parameters_dialog)
        shader_parameters_dialog =
            std::make_unique<ShaderParametersDialog>(this, parameters);

    shader_parameters_dialog->show();
}

void EmuCanvasVulkan::saveParameters(std::string filename)
{
    if (shader_chain && shader_chain->preset)
        shader_chain->preset->save_to_file(filename);
}