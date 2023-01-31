#pragma once

#include "vk_mem_alloc.hpp"
#include "vulkan_context.hpp"

namespace Vulkan
{

struct Texture
{
    Texture();
    void init(vk::Device device, vk::CommandPool command, vk::Queue queue, vma::Allocator allocator);
    void init(Context *context);
    ~Texture();

    void create(int width, int height, vk::Format fmt, vk::SamplerAddressMode wrap_mode, bool linear, bool mipmap);
    void destroy();
    void from_buffer(vk::CommandBuffer cmd, uint8_t *buffer, int width, int height, int byte_stride = 0);
    void from_buffer(uint8_t *buffer, int width, int height, int byte_stride = 0);
    void discard_staging_buffer();

    vk::Sampler sampler;
    vk::ImageView image_view;
    vk::Image image;
    vk::Format format;
    vk::SamplerAddressMode wrap_mode;
    vma::Allocation image_allocation;
    int image_width;
    int image_height;
    bool mipmap;
    bool linear;

    vk::Buffer buffer;
    vma::Allocation buffer_allocation;
    size_t buffer_size;

    vk::Device device;
    vk::Queue queue;
    vk::CommandPool command_pool;
    vma::Allocator allocator;
};

} // namespace Vulkan