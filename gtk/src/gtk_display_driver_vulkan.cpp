/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_compat.h"
#include "gtk_display.h"
#include "gtk_display_driver_vulkan.h"
#include "gtk_shader_parameters.h"
#include "snes9x.h"
#include "gfx.h"
#include "fmt/format.h"

#include "snes9x_imgui.h"
#include "external/imgui/imgui_impl_vulkan.h"

#ifdef GDK_WINDOWING_WAYLAND
static WaylandSurface::Metrics get_metrics(Gtk::DrawingArea &w)
{
    int x, y, width, height;
    w.get_window()->get_geometry(x, y, width, height);
    return { x, y, width, height, w.get_window()->get_scale_factor() };
}
#endif

S9xVulkanDisplayDriver::S9xVulkanDisplayDriver(Snes9xWindow *_window, Snes9xConfig *_config)
{
    window = _window;
    config = _config;
    drawing_area = window->drawing_area;
    gdk_window = nullptr;
    gdk_display = nullptr;
    context.reset();
}

S9xVulkanDisplayDriver::~S9xVulkanDisplayDriver()
{
}

bool S9xVulkanDisplayDriver::init_imgui()
{
    auto defaults = S9xImGuiGetDefaults();
    defaults.font_size = gui_config->osd_size;
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
    imgui_descriptor_pool = device.createDescriptorPoolUnique(descriptor_pool_create_info).value;

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = context->instance.get();
    init_info.PhysicalDevice = context->physical_device;
    init_info.Device = context->device;;
    init_info.QueueFamily = context->graphics_queue_family_index;
    init_info.Queue = context->queue;
    init_info.DescriptorPool = static_cast<VkDescriptorPool>(imgui_descriptor_pool.get());
    init_info.Subpass = 0;
    init_info.MinImageCount = context->swapchain->get_num_frames();
    init_info.ImageCount = context->swapchain->get_num_frames();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, static_cast<VkRenderPass>(context->swapchain->get_render_pass()));

    auto cmd = context->begin_cmd_buffer();
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    context->end_cmd_buffer();
    context->wait_idle();

    return true;
}

void S9xVulkanDisplayDriver::refresh()
{
    if (!context)
        return;

    context->swapchain->set_vsync(gui_config->sync_to_vblank);
    int new_width, new_height;

#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW(drawing_area->get_window()->gobj()))
    {
        std::tie(new_width, new_height) = wayland_surface->get_size_for_metrics(get_metrics(*drawing_area));
        context->swapchain->set_desired_size(new_width, new_height);
    }
    else
#endif
    {
        new_width = drawing_area->get_width() * drawing_area->get_scale_factor();
        new_height = drawing_area->get_height() * drawing_area->get_scale_factor();
    }

    if (new_width != current_width || new_height != current_height)
    {
        context->recreate_swapchain();
        context->wait_idle();
        current_width = new_width;
        current_height = new_height;

#ifdef GDK_WINDOWING_WAYLAND
        if (GDK_IS_WAYLAND_WINDOW(drawing_area->get_window()->gobj()))
            wayland_surface->resize(get_metrics(*drawing_area));
#endif
    }
}

int S9xVulkanDisplayDriver::init()
{
    current_width = drawing_area->get_width() * drawing_area->get_scale_factor();
    current_height = drawing_area->get_height() * drawing_area->get_scale_factor();

    context = std::make_unique<Vulkan::Context>();

#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW(drawing_area->get_window()->gobj()))
    {
        wayland_surface = std::make_unique<WaylandSurface>();
        wl_surface *surface = gdk_wayland_window_get_wl_surface(drawing_area->get_window()->gobj());
        wl_display *display = gdk_wayland_display_get_wl_display(drawing_area->get_display()->gobj());

        if (!wayland_surface->attach(display, surface, get_metrics(*drawing_area)))
            goto abort;
        if (!context->init_wayland())
            goto abort;
        if (!context->create_wayland_surface(wayland_surface->display, wayland_surface->child))
            goto abort;
        context->swapchain->set_desired_size(current_width, current_height);
        if (!context->create_swapchain())
            goto abort;
    }
#endif
    if (GDK_IS_X11_WINDOW(drawing_area->get_window()->gobj()))
    {
        display = gdk_x11_display_get_xdisplay(drawing_area->get_display()->gobj());
        xid = gdk_x11_window_get_xid(drawing_area->get_window()->gobj());

        if (!context->init_Xlib())
            goto abort;
        if (!context->create_Xlib_surface(display, xid))
            goto abort;
        if (!context->create_swapchain())
            goto abort;
    }

    device = context->device;
    init_imgui();

    if (!gui_config->shader_filename.empty() && gui_config->use_shaders)
    {
        shaderchain = std::make_unique<Vulkan::ShaderChain>(context.get());
        setlocale(LC_NUMERIC, "C");
        if (!shaderchain->load_shader_preset(gui_config->shader_filename))
        {
            fmt::print("Couldn't load shader preset file\n");
            shaderchain = nullptr;
            setlocale(LC_NUMERIC, "");
        }
        else
        {
            window->enable_widget("shader_parameters_item", true);
            setlocale(LC_NUMERIC, "");

            return 0;
        }
    }

    simple_output = std::make_unique<Vulkan::SimpleOutput>(context.get(), vk::Format::eR5G6B5UnormPack16);

    return 0;

abort:
    context.reset();
    return -1;
}

void S9xVulkanDisplayDriver::deinit()
{
    if (!context)
        return;

    if (shaderchain)
        gtk_shader_parameters_dialog_close();

    context->wait_idle();

    if (ImGui::GetCurrentContext())
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();
    }
}

void S9xVulkanDisplayDriver::update(uint16_t *buffer, int width, int height, int stride_in_pixels)
{
    if (!context)
        return;

    if (S9xImGuiDraw(current_width, current_height))
    {
        ImDrawData *draw_data = ImGui::GetDrawData();

        context->swapchain->on_render_pass_end([&, draw_data] {
            ImGui_ImplVulkan_RenderDrawData(draw_data, context->swapchain->get_cmd());
        });
    }

    auto viewport = S9xApplyAspect(width, height, current_width, current_height);
    bool retval = false;

    if (shaderchain)
    {
        retval = shaderchain->do_frame_without_swap((uint8_t *)buffer, width, height, stride_in_pixels << 1, vk::Format::eR5G6B5UnormPack16, viewport.x, viewport.y, viewport.w, viewport.h);
    }
    else if (simple_output)
    {
        simple_output->set_filter(Settings.BilinearFilter);
        retval = simple_output->do_frame_without_swap((uint8_t *)buffer, width, height, stride_in_pixels << 1, viewport.x, viewport.y, viewport.w, viewport.h);
    }

    if (retval)
    {
        if (Settings.SkipFrames == THROTTLE_TIMER || Settings.SkipFrames == THROTTLE_TIMER_FRAMESKIP)
        {
            throttle.set_frame_rate(Settings.PAL ? PAL_PROGRESSIVE_FRAME_RATE : NTSC_PROGRESSIVE_FRAME_RATE);
            throttle.wait_for_frame_and_rebase_time();
        }

        context->swapchain->set_vsync(gui_config->sync_to_vblank);
        context->swapchain->swap();

        if (gui_config->reduce_input_lag)
            context->wait_idle();
    }
}

int S9xVulkanDisplayDriver::query_availability()
{
    return 0;
}

void *S9xVulkanDisplayDriver::get_parameters()
{
    if (shaderchain)
        return &shaderchain->preset->parameters;
    return nullptr;
}

void S9xVulkanDisplayDriver::save(const char *filename)
{
    setlocale(LC_NUMERIC, "C");
    if (shaderchain)
        shaderchain->preset->save_to_file(filename);
    setlocale(LC_NUMERIC, "");
}

bool S9xVulkanDisplayDriver::is_ready()
{
    return true;
}

void S9xVulkanDisplayDriver::shrink()
{
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW(drawing_area->get_window()->gobj()))
        wayland_surface->shrink();
#endif
}

void S9xVulkanDisplayDriver::regrow()
{
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW(drawing_area->get_window()->gobj()))
        wayland_surface->regrow();
#endif
}