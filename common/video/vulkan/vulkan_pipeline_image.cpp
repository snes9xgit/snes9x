#include <cassert>

#include "vulkan_pipeline_image.hpp"
#include "slang_helpers.hpp"
#include "vulkan_common.hpp"

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

    auto range = [](unsigned int i) { return vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, i, 1, 0, 1); };
    auto level = [](unsigned int i) { return vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1); };
    auto image_memory_barrier = vk::ImageMemoryBarrier{}.setImage(image);

    auto mipmap_levels = mipmap ? mipmap_levels_for_size(image_width, image_height) : 1;

    image_layout_transition(cmd, image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal);
    // Transition base layer to readable format.
    int base_width = image_width;
    int base_height = image_height;
    int base_level = 0;
    for (; base_level + 1 < mipmap_levels; base_level++)
    {
        // Transition base layer to readable format.
        if (base_level > 0)
        {
            image_layout_transition(cmd,
                                    image,
                                    vk::ImageLayout::eTransferDstOptimal,
                                    vk::ImageLayout::eTransferSrcOptimal,
                                    range(base_level));
        }

        // Transition mipmap layer to writable
        image_layout_transition(cmd,
                                image,
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eTransferDstOptimal,
                                range(base_level + 1));

        // Blit base layer to mipmap layer
        int mipmap_width = std::max(base_width >> 1, 1);
        int mipmap_height = std::max(base_height >> 1, 1);

        auto blit = vk::ImageBlit{}
            .setSrcOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(base_width, base_height, 1) })
            .setDstOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(mipmap_width, mipmap_height, 1) })
            .setSrcSubresource(level(base_level))
            .setDstSubresource(level(base_level + 1));

        base_width = mipmap_width;
        base_height = mipmap_height;

        cmd.blitImage(image,
                      vk::ImageLayout::eTransferSrcOptimal,
                      image,
                      vk::ImageLayout::eTransferDstOptimal,
                      blit,
                      vk::Filter::eLinear);

        image_layout_transition(cmd,
                                image,
                                vk::ImageLayout::eTransferSrcOptimal,
                                vk::ImageLayout::eShaderReadOnlyOptimal,
                                range(base_level));
    }

    image_layout_transition(cmd,
                            image,
                            vk::ImageLayout::eTransferDstOptimal,
                            vk::ImageLayout::eShaderReadOnlyOptimal,
                            range(base_level));
}

void PipelineImage::barrier(vk::CommandBuffer cmd)
{
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe,
                        vk::PipelineStageFlagBits::eFragmentShader,
                        {}, {}, {}, {});
}

void PipelineImage::clear(vk::CommandBuffer cmd)
{
    vk::ImageSubresourceRange subresource_range(vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, 1);

    image_layout_transition(cmd,
                            image,
                            vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eTransferDstOptimal,
                            subresource_range);

    vk::ClearColorValue color{};

    color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });

    cmd.clearColorImage(image,
                        vk::ImageLayout::eTransferDstOptimal,
                        color,
                        subresource_range);

    image_layout_transition(cmd,
                            image,
                            vk::ImageLayout::eTransferDstOptimal,
                            vk::ImageLayout::eShaderReadOnlyOptimal,
                            subresource_range);

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