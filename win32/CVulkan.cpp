#include "CVulkan.h"
#include "snes9x.h"
#include <win32_display.h>
#include <gfx.h>

#include "../filter/hq2x.h"
#include "../filter/2xsai.h"

bool CVulkan::Initialize(HWND hWnd)
{
    this->hWnd = hWnd;
    RECT window_rect;
    GetWindowRect(hWnd, &window_rect);
    current_width = window_rect.right - window_rect.left;
    current_height = window_rect.bottom - window_rect.top;

    context = std::make_unique<Vulkan::Context>();
    if (!context->init_win32(0, hWnd))
        return false;

    if (GUI.shaderEnabled && GUI.OGLshaderFileName && GUI.OGLshaderFileName[0])
    {
        shaderchain = std::make_unique<Vulkan::ShaderChain>(context.get());
        std::string shaderstring = _tToChar(GUI.OGLshaderFileName);
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
        context->wait_idle();
    shaderchain.reset();
    simple_output.reset();
    filtered_image.clear();
    context.reset();
}

void CVulkan::Render(SSurface Src)
{
    if (!context)
        return;

    if (GUI.ReduceInputLag)
        context->wait_idle();

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

    if (!Settings.AutoDisplayMessages) {
        WinSetCustomDisplaySurface((void*)Dst.Surface, Dst.Pitch / 2, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top, GetFilterScale(GUI.Scale));
        S9xDisplayMessages((uint16*)Dst.Surface, Dst.Pitch / 2, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top, GetFilterScale(GUI.Scale));
    }

    RECT windowSize, displayRect;
    GetClientRect(hWnd, &windowSize);
    //Get maximum rect respecting AR setting
    displayRect = CalculateDisplayRect(Dst.Width, Dst.Height, windowSize.right, windowSize.bottom);

    if (shaderchain)
    {
        shaderchain->do_frame(Dst.Surface, Dst.Width, Dst.Height, Dst.Pitch, vk::Format::eR5G6B5UnormPack16, displayRect.left, displayRect.top, displayRect.right - displayRect.left, displayRect.bottom - displayRect.top);
        return;
    }

    simple_output->set_filter(Settings.BilinearFilter);
    simple_output->do_frame(Dst.Surface, Dst.Width, Dst.Height, Dst.Pitch, displayRect.left, displayRect.top, displayRect.right - displayRect.left, displayRect.bottom - displayRect.top);
}

bool CVulkan::ChangeRenderSize(unsigned int newWidth, unsigned int newHeight)
{
    if (!context)
        return false;

    bool vsync_changed = context->swapchain->set_vsync(GUI.Vsync);

    if (newWidth != current_width || newHeight != current_height || vsync_changed)
    {
        context->recreate_swapchain(newWidth, newHeight);
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

    if (context->swapchain->set_vsync(GUI.Vsync))
    {
        context->recreate_swapchain();
    }

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

std::vector<SlangShader::Parameter> *CVulkan::GetShaderParameters()
{
    if (shaderchain)
    {
        return &shaderchain->preset->parameters;
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