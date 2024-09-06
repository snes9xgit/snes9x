#include <cassert>

#include "vulkan_pipeline_image.hpp"
#include "slang_helpers.hpp"

namespace Vulkan
{

PipelineImage::PipelineImage()
{
    image_width = 0;
    image_height = 0;
    device = nullptr;
    command_pool = nullptr;
    allocator = nullptr;
    queue = nullptr;
    image = nullptr;
    current_layout = vk::ImageLayout::eUndefined;
}

PipelineImage::~PipelineImage()
{
    destroy();
}

void PipelineImage::init(vk::Device device_, vk::CommandPool command_, vk::Queue queue_, vma::Allocator allocator_)
{
    device = device_;
    command_pool = command_;
    allocator = allocator_;
    queue = queue_;
}

void PipelineImage::init(Context *context)
{
    device = context->device;
    command_pool = context->command_pool.get();
    allocator = context->allocator;
    queue = context->queue;
}

void PipelineImage::destroy()
{
    if (!device || !allocator)
        return;

    if (image_width != 0 || image_height != 0)
    {
        framebuffer.reset();
        device.destroyImageView(image_view);
        device.destroyImageView(mipless_view);
        allocator.destroyImage(image, image_allocation);
        image_width = image_height = 0;
        image_view = nullptr;
        image = nullptr;
        image_allocation = nullptr;
        current_layout = vk::ImageLayout::eUndefined;
    }
}

void PipelineImage::generate_mipmaps(vk::CommandBuffer cmd)
{
    if (!mipmap)
        return;

    auto srr = [](unsigned int i) { return vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, i, 1, 0, 1); };
    auto srl = [](unsigned int i) { return vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1); };
    auto image_memory_barrier = vk::ImageMemoryBarrier{}.setImage(image);

    auto mipmap_levels = mipmap ? mipmap_levels_for_size(image_width, image_height) : 1;

    // Transition base layer to readable format.
    image_memory_barrier
        .setImage(image)
        .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
        .setSubresourceRange(srr(0));

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllGraphics,
                        vk::PipelineStageFlagBits::eTransfer,
                        {}, {}, {}, image_memory_barrier);

    int base_width = image_width;
    int base_height = image_height;
    int base_level = 0;
    for (; base_level + 1 < mipmap_levels; base_level++)
    {
        // Transition base layer to readable format.
        if (base_level > 0)
        {
            image_memory_barrier
                .setImage(image)
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                .setSubresourceRange(srr(base_level));

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                vk::PipelineStageFlagBits::eTransfer,
                                {}, {}, {}, image_memory_barrier);
        }

        // Transition mipmap layer to writable
        image_memory_barrier
            .setImage(image)
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
            .setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setSubresourceRange(srr(base_level + 1));

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                            vk::PipelineStageFlagBits::eTransfer,
                            {}, {}, {}, image_memory_barrier);

        // Blit base layer to mipmap layer
        int mipmap_width = base_width >> 1;
        int mipmap_height = base_height >> 1;
        if (mipmap_width < 1)
            mipmap_width = 1;
        if (mipmap_height < 1)
            mipmap_height = 1;

        auto blit = vk::ImageBlit{}
            .setSrcOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(base_width, base_height, 1) })
            .setDstOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(mipmap_width, mipmap_height, 1) })
            .setSrcSubresource(srl(base_level))
            .setDstSubresource(srl(base_level + 1));

        base_width = mipmap_width;
        base_height = mipmap_height;

        cmd.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

        // Transition base layer to shader readable
        image_memory_barrier
            .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
            .setSubresourceRange(srr(base_level));

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                            vk::PipelineStageFlagBits::eFragmentShader,
                            {}, {}, {}, image_memory_barrier);
    }

    // Transition final layer to shader readable
    image_memory_barrier
        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
        .setSubresourceRange(srr(base_level));

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                        vk::PipelineStageFlagBits::eFragmentShader,
                        {}, {}, {}, image_memory_barrier);
}

void PipelineImage::barrier(vk::CommandBuffer cmd)
{
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllGraphics,
                        vk::PipelineStageFlagBits::eFragmentShader,
                        {}, {}, {}, {});
}

void PipelineImage::clear(vk::CommandBuffer cmd)
{
    vk::ImageSubresourceRange subresource_range(vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, 1);

    auto image_memory_barrier = vk::ImageMemoryBarrier{}
        .setImage(image)
        .setSubresourceRange(subresource_range)
        .setOldLayout(vk::ImageLayout::eUndefined)
        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eTransferRead);

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                        vk::PipelineStageFlagBits::eTransfer,
                        {}, {}, {},
                        image_memory_barrier);

    vk::ClearColorValue color{};

    color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });

    cmd.clearColorImage(image,
                        vk::ImageLayout::eTransferDstOptimal,
                        color,
                        subresource_range);

    image_memory_barrier
        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                        vk::PipelineStageFlagBits::eFragmentShader,
                        {}, {}, {},
                        image_memory_barrier);

    current_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

void PipelineImage::create(int width, int height, vk::Format fmt, vk::RenderPass renderpass, bool mipmap)
{
    assert(width + height);
    assert(device && allocator);
    this->mipmap = mipmap;
    int mipmap_levels = mipmap ? mipmap_levels_for_size(width, height): 1;

    format = fmt;

    auto allocation_create_info = vma::AllocationCreateInfo{}
        .setUsage(vma::MemoryUsage::eAuto);

    auto image_create_info = vk::ImageCreateInfo{}
        .setUsage(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
        .setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D(width, height, 1))
        .setMipLevels(mipmap_levels)
        .setArrayLayers(1)
        .setFormat(format)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(vk::SharingMode::eExclusive);

    std::tie(image, image_allocation) = allocator.createImage(image_create_info, allocation_create_info).value;

    auto subresource_range = vk::ImageSubresourceRange{}
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseArrayLayer(0)
        .setBaseMipLevel(0)
        .setLayerCount(1)
        .setLevelCount(mipmap_levels);

    auto image_view_create_info = vk::ImageViewCreateInfo{}
        .setImage(image)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(format)
        .setComponents(vk::ComponentMapping())
        .setSubresourceRange(subresource_range);

    image_view = device.createImageView(image_view_create_info).value;

    image_view_create_info.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    mipless_view = device.createImageView(image_view_create_info).value;

    image_width = width;
    image_height = height;

    auto framebuffer_create_info = vk::FramebufferCreateInfo{}
        .setAttachments(mipless_view)
        .setWidth(width)
        .setHeight(height)
        .setRenderPass(renderpass)
        .setLayers(1);

    framebuffer = device.createFramebufferUnique(framebuffer_create_info).value;
}

} // namespace Vulkan