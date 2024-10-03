#include "CVulkan.h"
#include "snes9x.h"
#include <win32_display.h>
#include <gfx.h>

#include "../filter/hq2x.h"
#include "../filter/2xsai.h"

#include "snes9x_imgui.h"
#include "imgui_impl_vulkan.h"

bool CVulkan::InitImGui()
{
    auto defaults = S9xImGuiGetDefaults();
    defaults.font_size = GUI.OSDSize;
    defaults.spacing = (int)(defaults.font_size / 2.4);
    S9xImGuiInit(&defaults);

    ImGui_ImplVulkan_LoadFunctions([](const char* function, void* instance) {
        return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(*((VkInstance*)instance), function);
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
    imgui_descriptor_pool = context->device.createDescriptorPoolUnique(descriptor_pool_create_info).value;

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = context->instance.get();
    init_info.PhysicalDevice = context->physical_device;
    init_info.Device = context->device;;
    init_info.QueueFamily = context->graphics_queue_family_index;
    init_info.Queue = context->queue;
    init_info.DescriptorPool = (VkDescriptorPool)imgui_descriptor_pool.get();
    init_info.Subpass = 0;
    init_info.MinImageCount = context->swapchain->get_num_frames();
    init_info.ImageCount = context->swapchain->get_num_frames();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, (VkRenderPass)context->swapchain->get_render_pass());

    auto cmd = context->begin_cmd_buffer();
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    context->end_cmd_buffer();
    context->wait_idle();

    return true;
}

bool CVulkan::Initialize(HWND hWnd)
{
    this->hWnd = hWnd;
    RECT window_rect;
    GetWindowRect(hWnd, &window_rect);
    current_width = window_rect.right - window_rect.left;
    current_height = window_rect.bottom - window_rect.top;

    try {
        context = std::make_unique<Vulkan::Context>();
        if (!context->init_win32())
            return false;
        if (!context->create_win32_surface(0, hWnd))
            return false;
        if (!context->create_swapchain())
            return false;
    }
    catch (std::exception& e)
    {
        return false;
    }

    context->swapchain->set_vsync(GUI.Vsync);

    if (!Settings.AutoDisplayMessages)
    {
        Settings.DisplayIndicators = true;
        InitImGui();
    }

    if (GUI.shaderEnabled && GUI.OGLshaderFileName && GUI.OGLshaderFileName[0])
    {
        shaderchain = std::make_unique<Vulkan::ShaderChain>(context.get());
        std::string shaderstring = std::string(_tToChar(GUI.OGLshaderFileName));
        if (!shaderchain->load_shader_preset(shaderstring))
        {
            shaderchain.reset();
        }
        else
        {
            current_shadername = shaderstring;
            return true;
        }
    }

    simple_output = std::make_unique<Vulkan::SimpleOutput>(context.get(), vk::Format::eR5G6B5UnormPack16);

    return true;
}

void CVulkan::DeInitialize()
{
    current_shadername = "";

    if (context)
    {
        context->wait_idle();
        if (ImGui::GetCurrentContext())
        {
            ImGui_ImplVulkan_Shutdown();
            imgui_descriptor_pool.reset();
            S9xImGuiDeinit();
        }
    }
    shaderchain.reset();
    simple_output.reset();
    filtered_image.clear();
    context.reset();
}

void CVulkan::Render(SSurface Src)
{
    if (!context)
        return;

    SSurface Dst{};

    RECT dstRect = GetFilterOutputSize(Src);
    Dst.Width = dstRect.right - dstRect.left;
    Dst.Height = dstRect.bottom - dstRect.top;
    Dst.Pitch = Dst.Width << 1;
    size_t requiredSize = Dst.Width * Dst.Height;
    if (filtered_image.size() < requiredSize)
        filtered_image.resize(requiredSize * 2);
    Dst.Surface = (unsigned char *)filtered_image.data();

    RenderMethod(Src, Dst, &dstRect);

    RECT windowSize, displayRect;
    GetClientRect(hWnd, &windowSize);
    //Get maximum rect respecting AR setting
    displayRect = CalculateDisplayRect(Dst.Width, Dst.Height, windowSize.right, windowSize.bottom);

    bool result;

    if (S9xImGuiDraw(windowSize.right, windowSize.bottom))
    {
        ImDrawData* draw_data = ImGui::GetDrawData();

        context->swapchain->on_render_pass_end([&, draw_data] {
            ImGui_ImplVulkan_RenderDrawData(draw_data, context->swapchain->get_cmd());
            });
    }

    if (shaderchain)
    {
        result = shaderchain->do_frame_without_swap(Dst.Surface, Dst.Width, Dst.Height, Dst.Pitch, vk::Format::eR5G6B5UnormPack16, displayRect.left, displayRect.top, displayRect.right - displayRect.left, displayRect.bottom - displayRect.top);
    }
    else if (simple_output)
    {
        simple_output->set_filter(Settings.BilinearFilter);
        result = simple_output->do_frame_without_swap(Dst.Surface, Dst.Width, Dst.Height, Dst.Pitch, displayRect.left, displayRect.top, displayRect.right - displayRect.left, displayRect.bottom - displayRect.top);
    }

    WinThrottleFramerate();

    if (result)
    {
        context->swapchain->swap();

        if (GUI.ReduceInputLag)
            context->wait_idle();
    }
}

bool CVulkan::ChangeRenderSize(unsigned int newWidth, unsigned int newHeight)
{
    if (!context)
        return false;

    if (newWidth != current_width || newHeight != current_height)
    {
        context->recreate_swapchain();
        context->wait_idle();
        current_width = newWidth;
        current_height = newHeight;
    }

    return true;
}

bool CVulkan::ApplyDisplayChanges(void)
{
    if ((!GUI.shaderEnabled && shaderchain) || (GUI.shaderEnabled && !shaderchain))
    {
        DeInitialize();
        Initialize(hWnd);
        return true;
    }

    std::string shadername = std::string(_tToChar(GUI.OGLshaderFileName));
    if (GUI.shaderEnabled && shaderchain && (shadername != current_shadername))
    {
        shaderchain.reset();
        shaderchain = std::make_unique<Vulkan::ShaderChain>(context.get());
        if (!shaderchain->load_shader_preset(shadername))
        {
            DeInitialize();
            GUI.shaderEnabled = false;
            Initialize(hWnd);
            return false;
        }
        current_shadername = shadername;
    }

    context->swapchain->set_vsync(GUI.Vsync);

    return true;
}

bool CVulkan::SetFullscreen(bool fullscreen)
{
    return false;
}

void CVulkan::SetSnes9xColorFormat()
{
    GUI.ScreenDepth = 16;
    GUI.BlueShift = 0;
    GUI.GreenShift = 6;
    GUI.RedShift = 11;
    S9xBlit2xSaIFilterInit();
    S9xBlitHQ2xFilterInit();
    GUI.NeedDepthConvert = FALSE;
    GUI.DepthConverted = TRUE;
}

void CVulkan::EnumModes(std::vector<dMode>* modeVector)
{
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(dd);
    DWORD dev = 0;
    int iMode = 0;
    dMode mode;

    while (EnumDisplayDevices(0, dev, &dd, 0))
    {
        if (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
        {
            DEVMODE dm;
            memset(&dm, 0, sizeof(dm));
            dm.dmSize = sizeof(dm);
            iMode = 0;
            while (EnumDisplaySettings(dd.DeviceName, iMode, &dm)) {
                if (dm.dmBitsPerPel >= 32) {
                    mode.width = dm.dmPelsWidth;
                    mode.height = dm.dmPelsHeight;
                    mode.rate = dm.dmDisplayFrequency;
                    mode.depth = dm.dmBitsPerPel;
                    modeVector->push_back(mode);
                }
                iMode++;
            }
        }
        dev++;
    }
}

std::vector<ShaderParam> *CVulkan::GetShaderParameters()
{
    if (shaderchain)
    {
        // SlangShader::Parameter currently equal ShaderParam, so no conversion is neccessary
        return (std::vector<ShaderParam>*)&shaderchain->preset->parameters;
    }
    else
        return nullptr;
}

std::function<void(const char *)> CVulkan::GetShaderParametersSaveFunction()
{
    return [&](const char *filename) {
        if (shaderchain)
            shaderchain->preset->save_to_file(filename);
    };
}
