#pragma once
#include "vulkan_hpp_wrapper.hpp"
#include "slang_shader.hpp"
#include "vulkan_context.hpp"
#include "vulkan_pipeline_image.hpp"

namespace Vulkan
{

class SlangPipeline
{
  public:
    SlangPipeline() = default;
    void init(Context *context_, SlangShader *shader_);
    ~SlangPipeline();

    bool generate_pipeline(bool lastpass = false);
    bool generate_frame_resources(vk::DescriptorPool pool);
    void update_framebuffer(vk::CommandBuffer, int frame_num, bool mipmap);

    Context *context;
    vk::Device device;
    vk::Format format;
    SlangShader *shader;
    vk::UniqueRenderPass render_pass;
    vk::UniqueDescriptorSetLayout descriptor_set_layout;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniquePipeline pipeline;
    vk::UniqueSemaphore semaphore;
    vk::UniqueSampler sampler;

    struct
    {
      vk::UniqueDescriptorSet descriptor_set;
      PipelineImage image;
      vk::UniqueFence fence;
    } frame[3];

    vk::Buffer uniform_buffer;
    vma::Allocation uniform_buffer_allocation;
    std::vector<uint8_t> push_constants;

    int source_width;
    int source_height;
    int destination_width;
    int destination_height;
};

vk::SamplerAddressMode wrap_mode_from_string(std::string s);

} // namespace Vulkan