#pragma once
#include "vulkan_context.hpp"
#include "slang_preset.hpp"
#include "vulkan_slang_pipeline.hpp"
#include "vulkan_texture.hpp"
#include <string>
#include <deque>

namespace Vulkan
{

class ShaderChain
{
  public:
    const int queue_size = 3;

    ShaderChain(Context *context_);
    ~ShaderChain();
    bool load_shader_preset(const std::string &filename);
    void update_and_propagate_sizes(int original_width_, int original_height_, int viewport_width_, int viewport_height_);
    bool load_lookup_textures();
    bool do_frame(uint8_t *data, int width, int height, int stride, vk::Format format, int viewport_x, int viewport_y, int viewport_width, int viewport_height);
    bool do_frame_without_swap(uint8_t *data, int width, int height, int stride, vk::Format format, int viewport_x, int viewport_y, int viewport_width, int viewport_height);
    void upload_original(uint8_t *data, int width, int height, int stride, vk::Format format);
    void upload_original(vk::CommandBuffer cmd, uint8_t *data, int width, int height, int stride, vk::Format format);
    void construct_buffer_objects();
    void update_framebuffers(vk::CommandBuffer cmd, int frame_num);
    void update_descriptor_set(vk::CommandBuffer cmd, int pipe_num, int swapchain_index);

    std::unique_ptr<SlangPreset> preset;
    Context *context;
    std::vector<std::unique_ptr<SlangPipeline>> pipelines;
    size_t original_history_size;
    uint32_t frame_count;
    int original_width;
    int original_height;
    int viewport_width;
    int viewport_height;
    vk::UniqueDescriptorPool descriptor_pool;
    std::vector<std::unique_ptr<Texture> > lookup_textures;
    std::deque<std::unique_ptr<Texture> > original;
    vk::Buffer vertex_buffer;
    vma::Allocation vertex_buffer_allocation;
    int current_frame_index;
    int last_frame_index;
};

} // namespace Vulkan