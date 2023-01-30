#include "CVulkan.h"

CVulkan::CVulkan()
{
}

CVulkan::~CVulkan()
{
}

bool CVulkan::Initialize(HWND hWnd)
{
    this->hWnd = hWnd;

    RECT window_rect;
    GetWindowRect(hWnd, &window_rect);
    current_width = window_rect.right - window_rect.left;
    current_height = window_rect.bottom - window_rect.top;

    context = std::make_unique<Vulkan::Context>();


    return true;
}

void CVulkan::DeInitialize()
{
}

void CVulkan::Render(SSurface Src)
{
}

bool CVulkan::ChangeRenderSize(unsigned int newWidth, unsigned int newHeight)
{
    return false;
}

bool CVulkan::ApplyDisplayChanges(void)
{
    return false;
}

bool CVulkan::SetFullscreen(bool fullscreen)
{
    return false;
}

void CVulkan::SetSnes9xColorFormat()
{
}

void CVulkan::EnumModes(std::vector<dMode>* modeVector)
{
}
