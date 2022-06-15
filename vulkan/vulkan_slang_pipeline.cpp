#include "vulkan_slang_pipeline.hpp"
#include "slang_helpers.hpp"
#include "fmt/format.h"
#include <unordered_map>

namespace Vulkan
{

static VkFormat format_string_to_format(std::string target, VkFormat default_format)
{
    struct
    {
        std::string string;
        VkFormat format;
    } formats[] = {
        { "R8_UNORM", VK_FORMAT_R8_UNORM },
        { "R8_UINT", VK_FORMAT_R8_UINT },
        { "R8_SINT", VK_FORMAT_R8_SINT },
        { "R8G8_UNORM", VK_FORMAT_R8G8_UNORM },
        { "R8G8_UINT", VK_FORMAT_R8G8_UINT },
        { "R8G8_SINT", VK_FORMAT_R8G8_SINT },
        { "R8G8B8A8_UNORM", VK_FORMAT_R8G8B8A8_UNORM },
        { "R8G8B8A8_UINT", VK_FORMAT_R8G8B8A8_UINT },
        { "R8G8B8A8_SINT", VK_FORMAT_R8G8B8A8_SINT },
        { "R8G8B8A8_SRGB", VK_FORMAT_R8G8B8A8_SRGB },

        { "R16_UINT", VK_FORMAT_R16_UINT },
        { "R16_SINT", VK_FORMAT_R16_SINT },
        { "R16_SFLOAT", VK_FORMAT_R16_SFLOAT },
        { "R16G16_UINT", VK_FORMAT_R16G16_UINT },
        { "R16G16_SINT", VK_FORMAT_R16G16_SINT },
        { "R16G16_SFLOAT", VK_FORMAT_R16G16_SFLOAT },
        { "R16G16B16A16_UINT", VK_FORMAT_R16G16B16A16_UINT },
        { "R16G16B16A16_SINT", VK_FORMAT_R16G16B16A16_SINT },
        { "R16G16B16A16_SFLOAT", VK_FORMAT_R16G16B16A16_SFLOAT },

        { "R32_UINT", VK_FORMAT_R32_UINT },
        { "R32_SINT", VK_FORMAT_R32_SINT },
        { "R32_SFLOAT", VK_FORMAT_R32_SFLOAT },
        { "R32G32_UINT", VK_FORMAT_R32G32_UINT },
        { "R32G32_SINT", VK_FORMAT_R32G32_SINT },
        { "R32G32_SFLOAT", VK_FORMAT_R32G32_SFLOAT },
        { "R32G32B32A32_UINT", VK_FORMAT_R32G32B32A32_UINT },
        { "R32G32B32A32_SINT", VK_FORMAT_R32G32B32A32_SINT },
        { "R32G32B32A32_SFLOAT", VK_FORMAT_R32G32B32A32_SFLOAT }
    };

    for (auto &f : formats)
    {
        if (f.string == target)
            return f.format;
    }

    return default_format;
}

vk::SamplerAddressMode wrap_mode_from_string(std::string s)
{
    if (s == "clamp_to_border")
        return vk::SamplerAddressMode::eClampToBorder;
    if (s == "repeat")
        return vk::SamplerAddressMode::eRepeat;
    if (s == "mirrored_repeat")
        return vk::SamplerAddressMode::eMirroredRepeat;

    return vk::SamplerAddressMode::eClampToBorder;
}

SlangPipeline::SlangPipeline()
{
    device = nullptr;
    shader = nullptr;
    uniform_buffer = nullptr;
    uniform_buffer_allocation = nullptr;
    source_width = 0;
    source_height = 0;
    destination_width = 0;
    destination_height = 0;
}

void SlangPipeline::init(Context *context_, SlangShader *shader_)
{
    this->context = context_;
    this->device = context->device;
    this->shader = shader_;
}

SlangPipeline::~SlangPipeline()
{
    device.waitIdle();

    if (uniform_buffer)
    {
        context->allocator.destroyBuffer(uniform_buffer, uniform_buffer_allocation);
    }

    for (auto &f : frame)
    {
        if (f.descriptor_set)
        {
            f.descriptor_set.reset();
            f.image.destroy();
        }
    }
    pipeline.reset();
    pipeline_layout.reset();
    descriptor_set_layout.reset();
    render_pass.reset();
}

bool SlangPipeline::generate_pipeline(bool lastpass)
{
    VkFormat backup_format = VK_FORMAT_R8G8B8A8_UNORM;
    if (shader->srgb_framebuffer)
        backup_format = VK_FORMAT_R8G8B8A8_SRGB;
    if (shader->float_framebuffer)
        backup_format = VK_FORMAT_R32G32B32A32_SFLOAT;
    this->format = vk::Format(format_string_to_format(shader->format, backup_format));

    auto attachment_description = vk::AttachmentDescription{}
        .setFormat(this->format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    auto attachment_reference = vk::AttachmentReference{}
        .setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    std::array<vk::SubpassDependency, 2> subpass_dependency;
    subpass_dependency[0]
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits(0))
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    subpass_dependency[1]
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

    auto subpass_description = vk::SubpassDescription{}
        .setColorAttachments(attachment_reference)
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

    auto render_pass_create_info = vk::RenderPassCreateInfo{}
        .setSubpasses(subpass_description)
        .setDependencies(subpass_dependency)
        .setAttachments(attachment_description);

    render_pass = device.createRenderPassUnique(render_pass_create_info);

    auto vertex_module = device.createShaderModuleUnique({ {}, shader->vertex_shader_spirv });
    auto fragment_module = device.createShaderModuleUnique({ {}, shader->fragment_shader_spirv });

    auto vertex_ci = vk::PipelineShaderStageCreateInfo{}
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(vertex_module.get())
        .setPName("main");

    auto fragment_ci = vk::PipelineShaderStageCreateInfo{}
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(fragment_module.get())
        .setPName("main");

    std::vector<vk::PipelineShaderStageCreateInfo> stages = { vertex_ci, fragment_ci };

    auto vertex_input_binding_description = vk::VertexInputBindingDescription{}
        .setBinding(0)
        .setInputRate(vk::VertexInputRate::eVertex)
        .setStride(sizeof(float) * 6);

    // Position 4x float
    std::array<vk::VertexInputAttributeDescription, 2> vertex_attribute_description{};
    vertex_attribute_description[0]
        .setBinding(0)
        .setFormat(vk::Format::eR32G32B32A32Sfloat)
        .setOffset(0)
        .setLocation(0);

    // texcoord 2x float
    vertex_attribute_description[1]
        .setBinding(0)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setOffset(sizeof(float) * 4)
        .setLocation(1);

    auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo{}
        .setVertexBindingDescriptions(vertex_input_binding_description)
        .setVertexAttributeDescriptions(vertex_attribute_description);

    auto pipeline_input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo{}
        .setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(false);

    std::vector<vk::Viewport> viewports(1);
    viewports[0]
        .setX(0.0f)
        .setY(0.0f)
        .setWidth(256)
        .setHeight(256)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    std::vector<vk::Rect2D> scissors(1);
    scissors[0].extent.width = 256;
    scissors[0].extent.height = 256;
    scissors[0].offset = vk::Offset2D(0, 0);

    auto pipeline_viewport_info = vk::PipelineViewportStateCreateInfo{}
        .setViewports(viewports)
        .setScissors(scissors);

    auto rasterizer_info = vk::PipelineRasterizationStateCreateInfo{}
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setLineWidth(1.0f)
        .setDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setDepthBiasEnable(false)
        .setRasterizerDiscardEnable(false);

    auto multisample_info = vk::PipelineMultisampleStateCreateInfo{}
        .setSampleShadingEnable(false)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    auto depth_stencil_info = vk::PipelineDepthStencilStateCreateInfo{}
        .setDepthTestEnable(false);

    auto blend_attachment_info = vk::PipelineColorBlendAttachmentState{}
        .setColorWriteMask(vk::ColorComponentFlagBits::eB |
                           vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eR |
                           vk::ColorComponentFlagBits::eA)
        .setBlendEnable(false)
        .setColorBlendOp(vk::BlendOp::eAdd);
        // .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        // .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        // .setAlphaBlendOp(vk::BlendOp::eAdd)
        // .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        // .setSrcAlphaBlendFactor(vk::BlendFactor::eZero);

    auto blend_state_info = vk::PipelineColorBlendStateCreateInfo{}
        .setLogicOpEnable(false)
        .setAttachments(blend_attachment_info);

    std::vector<vk::DynamicState> states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic_state_info({}, states);

    std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings;
    if (shader->ubo_size > 0)
    {
        vk::DescriptorSetLayoutBinding binding(
            shader->ubo_binding,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlagBits::eAllGraphics);

        descriptor_set_layout_bindings.push_back(binding);
    }

    if (!shader->samplers.empty())
    {
        for (const auto &s : shader->samplers)
        {
            vk::DescriptorSetLayoutBinding binding(
                s.binding,
                vk::DescriptorType::eCombinedImageSampler,
                1,
                vk::ShaderStageFlagBits::eFragment);

            descriptor_set_layout_bindings.push_back(binding);
        }
    }

    auto dslci = vk::DescriptorSetLayoutCreateInfo{}
        .setBindings(descriptor_set_layout_bindings);
    descriptor_set_layout = device.createDescriptorSetLayoutUnique(dslci);

    vk::PushConstantRange pcr(vk::ShaderStageFlagBits::eAllGraphics, 0, shader->push_constant_block_size);

    auto pipeline_layout_info = vk::PipelineLayoutCreateInfo{}
        .setSetLayoutCount(0)
        .setSetLayouts(descriptor_set_layout.get());

    if (shader->push_constant_block_size > 0)
        pipeline_layout_info.setPushConstantRanges(pcr);

    pipeline_layout = device.createPipelineLayoutUnique(pipeline_layout_info);

    auto pipeline_create_info = vk::GraphicsPipelineCreateInfo{}
        .setStageCount(2)
        .setStages(stages)
        .setPVertexInputState(&vertex_input_info)
        .setPInputAssemblyState(&pipeline_input_assembly_info)
        .setPViewportState(&pipeline_viewport_info)
        .setPRasterizationState(&rasterizer_info)
        .setPMultisampleState(&multisample_info)
        .setPDepthStencilState(&depth_stencil_info)
        .setPColorBlendState(&blend_state_info)
        .setPDynamicState(&dynamic_state_info)
        .setLayout(pipeline_layout.get())
        .setRenderPass(render_pass.get())
        .setSubpass(0);

    if (lastpass)
        pipeline_create_info.setRenderPass(context->swapchain->get_render_pass());

    auto [result, pipeline] = device.createGraphicsPipelineUnique(nullptr, pipeline_create_info);

    if (result != vk::Result::eSuccess)
    {
        fmt::print("Failed to create pipeline for shader: {}\n", shader->filename);
        return false;
    }

    this->pipeline = std::move(pipeline);
    return true;
}

void SlangPipeline::update_framebuffer(vk::CommandBuffer cmd, int num, bool mipmap)
{
   auto &image = frame[num].image;
   if (image.image_width != destination_width || image.image_height != destination_height)
   {
        image.destroy();
        image.create(destination_width, destination_height, format, render_pass.get(), mipmap);
   }
}

bool SlangPipeline::generate_frame_resources(vk::DescriptorPool pool)
{
    for (auto &f : frame)
    {
        if (f.descriptor_set)
        {
            f.descriptor_set.reset();
            f.image.destroy();
        }

        vk::DescriptorSetAllocateInfo descriptor_set_allocate_info(pool, descriptor_set_layout.get());

        auto result = device.allocateDescriptorSetsUnique(descriptor_set_allocate_info);
        f.descriptor_set = std::move(result[0]);
    }

    semaphore = device.createSemaphoreUnique({});

    push_constants.resize(shader->push_constant_block_size);

    if (shader->ubo_size > 0)
    {
        auto buffer_create_info = vk::BufferCreateInfo{}
            .setSize(shader->ubo_size)
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

        auto allocation_create_info = vma::AllocationCreateInfo{}
            .setFlags(vma::AllocationCreateFlagBits::eHostAccessSequentialWrite)
            .setRequiredFlags(vk::MemoryPropertyFlagBits::eHostVisible);

        std::tie(uniform_buffer, uniform_buffer_allocation) = context->allocator.createBuffer(buffer_create_info, allocation_create_info);
    }
    else
    {
        uniform_buffer = nullptr;
        uniform_buffer_allocation = nullptr;
    }

    for (auto &f : frame)
        f.image.init(context);

    auto wrap_mode = wrap_mode_from_string(shader->wrap_mode);
    auto filter = shader->filter_linear ? vk::Filter::eLinear : vk::Filter::eNearest;
    auto sampler_create_info = vk::SamplerCreateInfo{}
        .setAddressModeU(wrap_mode)
        .setAddressModeV(wrap_mode)
        .setAddressModeW(wrap_mode)
        .setMagFilter(filter)
        .setMinFilter(filter)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setMinLod(0.0f)
        .setMaxLod(VK_LOD_CLAMP_NONE)
        .setAnisotropyEnable(false);

    sampler = device.createSamplerUnique(sampler_create_info);

    return true;
}

} // namespace Vulkan