#pragma once

#include "IS9xDisplayOutput.h"
#include "../vulkan/vulkan_context.hpp"
#include "../vulkan/vulkan_shader_chain.hpp"
#include "../vulkan/vulkan_simple_output.hpp"
#include <functional>

class CVulkan : public IS9xDisplayOutput
{
  private:
    HWND hWnd;
    std::unique_ptr<Vulkan::Context> context;
    std::vector<uint16_t> filtered_image;
    std::unique_ptr<Vulkan::ShaderChain> shaderchain;
    std::string current_shadername;
    std::unique_ptr<Vulkan::SimpleOutput> simple_output;

    int current_width;
    int current_height;

  public:
    bool Initialize(HWND hWnd);
    void DeInitialize();
    void Render(SSurface Src);
    bool ChangeRenderSize(unsigned int newWidth, unsigned int newHeight);
    bool ApplyDisplayChanges(void);
    bool SetFullscreen(bool fullscreen);
    void SetSnes9xColorFormat();
    void EnumModes(std::vector<dMode>* modeVector);
    std::vector<SlangShader::Parameter> *GetShaderParameters(void);
    std::function<void(const char *)> GetShaderParametersSaveFunction();
};

