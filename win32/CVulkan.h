#pragma once

#include "IS9xDisplayOutput.h"
#include "../vulkan/vulkan_context.hpp"
#include "../vulkan/vulkan_texture.hpp"
#include "../vulkan/vulkan_shader_chain.hpp"
#include <functional>

class CVulkan : public IS9xDisplayOutput
{
  private:
    void create_pipeline();

    HWND hWnd;
    std::unique_ptr<Vulkan::Context> context;
    Vulkan::Swapchain* swapchain;
    vk::Device device;
    vk::UniqueDescriptorSetLayout descriptor_set_layout;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniquePipeline pipeline;
    vk::Sampler linear_sampler;
    vk::Sampler nearest_sampler;
    std::vector<Vulkan::Texture> textures;
    std::vector<vk::UniqueDescriptorSet> descriptors;
    std::vector<uint16_t> filtered_image;
    std::unique_ptr<Vulkan::ShaderChain> shaderchain;
    std::string current_shadername;

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

