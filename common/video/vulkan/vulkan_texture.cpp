#include <cassert>

#include "vulkan_texture.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "slang_helpers.hpp"

namespace Vulkan
{

Texture::Texture()
{
    image_width = 0;
    image_height = 0;
    buffer_size = 0;
    device = nullptr;
    command_pool = nullptr;
    allocator = nullptr;
    queue = nullptr;
    buffer = nullptr;
    image = nullptr;
    sampler = nullptr;
}

Texture::~Texture()
{
    destroy();
}

void Texture::destroy()
{
    if (!device || !allocator)
        return;

    if (sampler)
    {
        device.destroySampler(sampler);
        sampler = nullptr;
    }

    if (image_width != 0 || image_height != 0)
    {
        device.destroyImageView(image_view);
        allocator.destroyImage(image, image_allocation);
        image_width = image_height = 0;
        image_view = nullptr;
        image = nullptr;
        image_allocation = nullptr;
    }

    if (buffer_size != 0)
    {
        allocator.destroyBuffer(buffer, buffer_allocation);
        buffer_size = 0;
        buffer = nullptr;
        buffer_allocation = nullptr;
    }
}

void Texture::init(vk::Device device_, vk::CommandPool command_, vk::Queue queue_, vma::Allocator allocator_)
{
    device = device_;
    command_pool = command_;
    allocator = allocator_;
    queue = queue_;
}

void Texture::init(Context *context)
{
    device = context->device;
    command_pool = context->command_pool.get();
    allocator = context->allocator;
    queue = context->queue;
}

void Texture::from_buffer(vk::CommandBuffer cmd,
                          uint8_t *buffer,
                          int width,
                          int height,
                          int byte_stride)
{
    if (image_width != width || image_height != height)
    {
        destroy();
        create(width, height, format, wrap_mode, linear, mipmap);
    }

    int pixel_size = 4;
    if (format == vk::Format::eR5G6B5UnormPack16)
        pixel_size = 2;

    if (byte_stride == 0)
    {
        byte_stride = pixel_size * width;
    }

    auto map = allocator.mapMemory(buffer_allocation).value;
    for (int y = 0; y < height; y++)
    {
        auto src = buffer + byte_stride * y;
        auto dst = (uint8_t *)map + width * pixel_size * y;
        memcpy(dst, src, width * pixel_size);
    }
    allocator.unmapMemory(buffer_allocation);
    allocator.flushAllocation(buffer_allocation, 0, width * height * pixel_size);

    auto srr = [](unsigned int i) { return vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, i, 1, 0, 1); };
    auto srl = [](unsigned int i) { return vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1); };

    auto barrier = vk::ImageMemoryBarrier{}
        .setImage(image)
        .setOldLayout(vk::ImageLayout::eUndefined)
        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setSubresourceRange(srr(0));

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                        vk::PipelineStageFlagBits::eTransfer,
                        {}, {}, {}, barrier);

    auto buffer_image_copy = vk::BufferImageCopy{}
        .setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(height)
        .setImageExtent(vk::Extent3D(width, height, 1))
        .setImageOffset(vk::Offset3D(0, 0, 0))
        .setImageSubresource(srl(0));
    cmd.copyBufferToImage(this->buffer, image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copy);

    auto mipmap_levels = mipmap ? mipmap_levels_for_size(image_width, image_height) : 1;

    int base_width = image_width;
    int base_height = image_height;
    int base_level = 0;
    for (; base_level + 1 < mipmap_levels; base_level++)
    {
        // Transition base layer to readable format.
        barrier
            .setImage(image)
            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
            .setSubresourceRange(srr(base_level));

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                            vk::PipelineStageFlagBits::eTransfer,
                            {}, {}, {}, barrier);

        // Transition mipmap layer to writable
        barrier
            .setImage(image)
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
            .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
            .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setSubresourceRange(srr(base_level + 1));

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                            vk::PipelineStageFlagBits::eTransfer,
                            {}, {}, {}, barrier);

        // Blit base layer to mipmap layer
        int mipmap_width = base_width >> 1;
        int mipmap_height = base_height >> 1;
        if (mipmap_width < 1)
            mipmap_width = 1;
        if (mipmap_height < 1)
            mipmap_height = 1;

        auto blit = vk::ImageBlit{}
            .setSrcOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(base_width, base_height, 1) })
            .setDstOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(mipmap_width, mipmap_height, 1)})
            .setSrcSubresource(srl(base_level))
            .setDstSubresource(srl(base_level + 1));

        base_width = mipmap_width;
        base_height = mipmap_height;

        cmd.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

        // Transition base layer to shader readable
        barrier
            .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
            .setSubresourceRange(srr(base_level));

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                            vk::PipelineStageFlagBits::eFragmentShader,
                            {}, {}, {}, barrier);
    }

    // Transition final layer to shader readable
    barrier
        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
        .setSubresourceRange(srr(base_level));

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                        vk::PipelineStageFlagBits::eFragmentShader,
                        {}, {}, {}, barrier);
}

void Texture::from_buffer(uint8_t *buffer, int width, int height, int byte_stride)
{
    vk::CommandBufferAllocateInfo cbai(command_pool, vk::CommandBufferLevel::ePrimary, 1);
    auto command_buffer_vector = device.allocateCommandBuffersUnique(cbai).value;
    auto &cmd = command_buffer_vector[0];
    cmd->begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    from_buffer(cmd.get(), buffer, width, height, byte_stride);
    cmd->end();
    vk::SubmitInfo si{};
    si.setCommandBuffers(cmd.get());
    queue.submit(si);
    queue.waitIdle();
}

void Texture::create(int width, int height, vk::Format fmt, vk::SamplerAddressMode wrap_mode, bool linear, bool mipmap)
{
    assert(image_width + image_height + buffer_size == 0);

    this->mipmap = mipmap;
    this->wrap_mode = wrap_mode;
    this->linear = linear;
    int mipmap_levels = mipmap ? mipmap_levels_for_size(width, height) : 1;
    format = fmt;

    auto aci = vma::AllocationCreateInfo{}
        .setUsage(vma::MemoryUsage::eAuto);

    auto ici = vk::ImageCreateInfo{}
        .setUsage(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
        .setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D(width, height, 1))
        .setMipLevels(mipmap_levels)
        .setArrayLayers(1)
        .setFormat(format)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(vk::SharingMode::eExclusive);

    std::tie(image, image_allocation) = allocator.createImage(ici, aci).value;

    buffer_size = width * height * 4;
    if (format == vk::Format::eR5G6B5UnormPack16)
        buffer_size = width * height * 2;
    auto bci = vk::BufferCreateInfo{}
        .setSize(buffer_size)
        .setUsage(vk::BufferUsageFlagBits::eTransferSrc);

    aci.setRequiredFlags(vk::MemoryPropertyFlagBits::eHostVisible)
        .setFlags(vma::AllocationCreateFlagBits::eHostAccessSequentialWrite)
        .setUsage(vma::MemoryUsage::eAutoPreferHost);

    std::tie(buffer, buffer_allocation) = allocator.createBuffer(bci, aci).value;

    auto isrr = vk::ImageSubresourceRange{}
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseArrayLayer(0)
        .setBaseMipLevel(0)
        .setLayerCount(1)
        .setLevelCount(mipmap_levels);
    auto ivci = vk::ImageViewCreateInfo{}
        .setImage(image)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(format)
        .setComponents(vk::ComponentMapping())
        .setSubresourceRange(isrr);

    image_view = device.createImageView(ivci).value;

    image_width = width;
    image_height = height;

    auto sampler_create_info = vk::SamplerCreateInfo{}
        .setMagFilter(vk::Filter::eNearest)
        .setMinFilter(vk::Filter::eNearest)
        .setAddressModeU(wrap_mode)
        .setAddressModeV(wrap_mode)
        .setAddressModeW(wrap_mode)
        .setBorderColor(vk::BorderColor::eFloatOpaqueBlack);

    if (linear)
        sampler_create_info
            .setMagFilter(vk::Filter::eLinear)
            .setMinFilter(vk::Filter::eLinear);

    if (mipmap)
        sampler_create_info
            .setMinLod(0.0f)
            .setMaxLod(10000.0f)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear);

    sampler = device.createSampler(sampler_create_info).value;
}

void Texture::discard_staging_buffer()
{
    if (buffer_size != 0)
    {
        allocator.destroyBuffer(buffer, buffer_allocation);
        buffer_size = 0;
    }
}

} // namespace Vulkan
