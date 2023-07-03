#include "vulkan_simple_output.hpp"
#include "slang_shader.hpp"

namespace Vulkan
{

static const char *vertex_shader = R"(
#version 450

layout(location = 0) out vec2 texcoord;

vec2 positions[3] = vec2[](vec2(-1.0, -3.0), vec2(3.0, 1.0), vec2(-1.0, 1.0));
vec2 texcoords[3] = vec2[](vec2(0.0, -1.0), vec2(2.0, 1.0), vec2(0.0, 1.0));

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    texcoord = texcoords[gl_VertexIndex];
}
)";

static const char *fragment_shader = R"(
#version 450

layout(location = 0) in vec2 texcoord;
layout(binding = 0) uniform sampler2D tsampler;

layout(location = 0) out vec4 fragcolor;

void main()
{
    fragcolor = texture(tsampler, texcoord);
}
)";

SimpleOutput::SimpleOutput(Context *context, vk::Format format)
{
    this->context = context;
    device = context->device;
    swapchain = context->swapchain.get();
    this->format = format;
    create_pipeline();
    create_objects();
}

SimpleOutput::~SimpleOutput()
{
    context->wait_idle();
    textures.clear();
    descriptors.clear();
    device.destroySampler(linear_sampler);
    device.destroySampler(nearest_sampler);
}

void SimpleOutput::create_objects()
{
    descriptors.clear();
    for (size_t i = 0; i < queue_size; i++)
    {
        vk::DescriptorSetAllocateInfo dsai{};
        dsai
            .setDescriptorPool(context->descriptor_pool.get())
            .setDescriptorSetCount(1)
            .setSetLayouts(descriptor_set_layout.get());
        auto descriptor = device.allocateDescriptorSetsUnique(dsai);
        descriptors.push_back(std::move(descriptor[0]));
    }

    textures.clear();
    textures.resize(queue_size);
    for (auto &t : textures)
    {
        t.init(context);
        t.create(256, 224, format, vk::SamplerAddressMode::eClampToEdge, filter, false);
    }

    vk::SamplerCreateInfo sci{};
    sci.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
        .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
        .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setAnisotropyEnable(false)
        .setMinFilter(vk::Filter::eLinear)
        .setMagFilter(vk::Filter::eLinear)
        .setUnnormalizedCoordinates(false)
        .setMinLod(1.0f)
        .setMaxLod(1.0f)
        .setMipLodBias(0.0)
        .setCompareEnable(false);
    linear_sampler = device.createSampler(sci);

    sci.setMinFilter(vk::Filter::eNearest)
       .setMagFilter(vk::Filter::eNearest);
    nearest_sampler = device.createSampler(sci);
}

void SimpleOutput::create_pipeline()
{
    auto vertex_spirv = SlangShader::generate_spirv(vertex_shader, "vertex");
    auto fragment_spirv = SlangShader::generate_spirv(fragment_shader, "fragment");

    auto vertex_module = device.createShaderModuleUnique({ {}, vertex_spirv });
    auto fragment_module = device.createShaderModuleUnique({ {}, fragment_spirv });

    vk::PipelineShaderStageCreateInfo vertex_ci;
    vertex_ci.setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(vertex_module.get())
        .setPName("main");

    vk::PipelineShaderStageCreateInfo fragment_ci;
    fragment_ci.setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(fragment_module.get())
        .setPName("main");

    std::vector<vk::PipelineShaderStageCreateInfo> stages = { vertex_ci, fragment_ci };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};

    vk::PipelineInputAssemblyStateCreateInfo pipeline_input_assembly_info{};
    pipeline_input_assembly_info.setTopology(vk::PrimitiveTopology::eTriangleList)
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

    vk::PipelineViewportStateCreateInfo pipeline_viewport_info;
    pipeline_viewport_info.setViewports(viewports)
        .setScissors(scissors);

    vk::PipelineRasterizationStateCreateInfo rasterizer_info;
    rasterizer_info.setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setLineWidth(1.0f)
        .setDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setDepthBiasEnable(false)
        .setRasterizerDiscardEnable(false);

    vk::PipelineMultisampleStateCreateInfo multisample_info;
    multisample_info.setSampleShadingEnable(false)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    vk::PipelineDepthStencilStateCreateInfo depth_stencil_info;
    depth_stencil_info.setDepthTestEnable(false);

    vk::PipelineColorBlendAttachmentState blend_attachment_info;
    blend_attachment_info
        .setColorWriteMask(vk::ColorComponentFlagBits::eB |
                           vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eR |
                           vk::ColorComponentFlagBits::eA)
        .setBlendEnable(true)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eZero);

    vk::PipelineColorBlendStateCreateInfo blend_state_info;
    blend_state_info.setLogicOpEnable(false)
        .setAttachments(blend_attachment_info);

    std::vector<vk::DynamicState> states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic_state_info({}, states);

    vk::DescriptorSetLayoutBinding dslb{};
    dslb.setBinding(0)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    vk::DescriptorSetLayoutCreateInfo dslci{};
    dslci.setBindings(dslb);
    descriptor_set_layout = device.createDescriptorSetLayoutUnique(dslci);

    vk::PipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.setSetLayoutCount(0)
        .setPushConstantRangeCount(0)
        .setSetLayouts(descriptor_set_layout.get());

    pipeline_layout = device.createPipelineLayoutUnique(pipeline_layout_info);

    vk::GraphicsPipelineCreateInfo pipeline_create_info;
    pipeline_create_info.setStageCount(2)
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
        .setRenderPass(swapchain->get_render_pass())
        .setSubpass(0);

    auto [result, pipeline] = device.createGraphicsPipelineUnique(nullptr, pipeline_create_info);
    this->pipeline = std::move(pipeline);
}

void SimpleOutput::set_filter(bool on)
{
    filter = on;
}

bool SimpleOutput::do_frame_without_swap(uint8_t *buffer, int width, int height, int byte_stride, int viewport_x, int viewport_y, int viewport_width, int viewport_height)
{
    if (!context)
        return false;

    if (!swapchain->begin_frame())
        return false;

    auto &tex = textures[current_frame];
    auto &cmd = swapchain->get_cmd();
    auto extents = swapchain->get_extents();
    auto &dstset = descriptors[current_frame].get();

    tex.from_buffer(cmd, (uint8_t *)buffer, width, height, byte_stride);

    swapchain->begin_render_pass();

    vk::DescriptorImageInfo dii{};
    dii.setImageView(tex.image_view)
        .setSampler(filter ? linear_sampler : nearest_sampler)
        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    vk::WriteDescriptorSet wds{};
    wds.setDescriptorCount(1)
        .setDstBinding(0)
        .setDstArrayElement(0)
        .setDstSet(dstset)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo(dii);
    device.updateDescriptorSets(wds, {});

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout.get(), 0, dstset, {});
    cmd.setViewport(0, vk::Viewport(viewport_x, viewport_y, viewport_width, viewport_height, 0.0f, 1.0f));
    cmd.setScissor(0, vk::Rect2D({}, extents));
    cmd.draw(3, 1, 0, 0);

    swapchain->end_render_pass();
    swapchain->end_frame_without_swap();

    current_frame = (current_frame + 1) % queue_size;

    return true;
}

bool SimpleOutput::do_frame(uint8_t *buffer, int width, int height, int byte_stride, int viewport_x, int viewport_y, int viewport_width, int viewport_height)
{
    if (!do_frame_without_swap(buffer, width, height, byte_stride, viewport_x, viewport_y, viewport_width, viewport_height))
        return false;
    swapchain->swap();
    return true;
}

} // namespace Vulkan