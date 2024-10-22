#pragma once
#include "vulkan_context.hpp"

namespace Vulkan
{

struct PipelineImage
{
    PipelineImage() = default;
    void init(vk::Device device, vk::CommandPool command, vk::Queue queue, vma::Allocator allocator);
    void init(Vulkan::Context *context);
    ~PipelineImage();

    void create(int width, int height, vk::Format fmt, vk::RenderPass renderpass, bool mipmap = false);
    void destroy();
    void barrier(vk::CommandBuffer cmd);
    void generate_mipmaps(vk::CommandBuffer cmd);
    void clear(vk::CommandBuffer cmd);

    vk::ImageView image_view;
    vk::ImageView mipless_view;
    vk::Image image;
    vk::Format format;
    vk::UniqueFramebuffer framebuffer;
    vma::Allocation image_allocation;
    vk::ImageLayout current_layout;
    int image_width;
    int image_height;
    bool mipmap;

    vk::Device device;
    vk::Queue queue;
    vk::CommandPool command_pool;
    vma::Allocator allocator;
};

} // namespace Vulkan