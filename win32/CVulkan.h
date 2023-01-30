#pragma once

#include <windows.h>
#include "IS9xDisplayOutput.h"
#include "../../../vulkan/vulkan_context.hpp"

class CVulkan : public IS9xDisplayOutput
{
  private:
    HWND hWnd;
    std::unique_ptr<Vulkan::Context> context;
    Vulkan::Swapchain* swapchain;
    vk::Device device;

    int current_width;
    int current_height;

  public:
    CVulkan();
    ~CVulkan();
    bool Initialize(HWND hWnd);
    void DeInitialize();
    void Render(SSurface Src);
    bool ChangeRenderSize(unsigned int newWidth, unsigned int newHeight);
    bool ApplyDisplayChanges(void);
    bool SetFullscreen(bool fullscreen);
    void SetSnes9xColorFormat();
    void EnumModes(std::vector<dMode>* modeVector);
};

