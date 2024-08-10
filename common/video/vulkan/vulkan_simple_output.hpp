#pragma once
#include "vulkan_context.hpp"
#include "vulkan_texture.hpp"

namespace Vulkan
{

class SimpleOutput
{
  public:
    SimpleOutput(Vulkan::Context *context, vk::Format format);
    ~SimpleOutput();
    bool do_frame(uint8_t *buffer, int width, int height, int byte_stride, int viewport_x, int viewport_y, int viewport_width, int viewport_height);
    bool do_frame_without_swap(uint8_t *buffer, int width, int height, int byte_stride, int viewport_x, int viewport_y, int viewport_width, int viewport_height);
    void set_filter(bool on);

  private:
    const int queue_size = 3;
    int current_frame = 0;

    void create_pipeline();
    void create_objects();

    Context *context;
    vk::Device device;
    Vulkan::Swapchain *swapchain;

    vk::UniqueDescriptorSetLayout descriptor_set_layout;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniquePipeline pipeline;
    vk::Sampler linear_sampler;
    vk::Sampler nearest_sampler;
    vk::Format format;
    std::vector<Vulkan::Texture> textures;
    std::vector<vk::UniqueDescriptorSet> descriptors;

    bool filter = true;
};

} // namespace Vulkan