#include "vk2d.hpp"

#include <glslang/Include/BaseTypes.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>
#include <vulkan/vulkan_core.h>

static const char *vertex_shader = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
)";

static const char *fragment_shader = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)";

bool vk2d::dispatcher_initialized = false;

vk2d::vk2d()
{
    instance = nullptr;
    surface = nullptr;

    if (!dispatcher_initialized)
    {
        vk::DynamicLoader *dl = new vk::DynamicLoader;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl->getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
        dispatcher_initialized = true;
    }
}

vk2d::~vk2d()
{
    deinit();
}

bool vk2d::init_device()
{
    if (!instance || !surface)
        return false;

    choose_physical_device();
    create_device();
    create_sync_objects();
    create_swapchain();
    create_render_pass();
    create_pipeline();
    create_framebuffers();
    create_command_buffers();

    return true;
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool vk2d::init_xlib_instance()
{
    if (instance)
        return true;

    std::vector<const char *> extensions = { VK_KHR_XLIB_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME };
    vk::ApplicationInfo ai({}, {}, {}, {}, VK_API_VERSION_1_0);
    vk::InstanceCreateInfo ci({}, &ai, {}, extensions);

    auto rv = vk::createInstance(ci);
    if (rv.result != vk::Result::eSuccess)
        return false;

    instance = rv.value;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
    return true;
}

void vk2d::attach(Display *dpy, Window xid)
{
    if (surface)
    {
        instance.destroySurfaceKHR(surface);
        surface = nullptr;
    }

    vk::XlibSurfaceCreateInfoKHR sci({}, dpy, xid);
    auto rv = instance.createXlibSurfaceKHR(sci);
    VK_CHECK(rv.result);
    surface = rv.value;
}
#endif // VK_USE_PLATFORM_XLIB_KHR

bool vk2d::create_instance()
{
    std::vector<const char *> extensions = { VK_KHR_XLIB_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME };

    vk::ApplicationInfo ai({}, {}, {}, {}, VK_API_VERSION_1_1);
    vk::InstanceCreateInfo ci({}, &ai, {}, {}, extensions.size(), extensions.data());

    auto rv = vk::createInstance(ci);
    VK_CHECK(rv.result);
    instance = rv.value;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
    return true;
}

void vk2d::deinit()
{
    destroy_swapchain();

    frame_queue.clear();

    if (command_pool)
        device.destroyCommandPool(command_pool);

    if (device)
        device.destroy();

    if (surface)
        instance.destroySurfaceKHR(surface);

    if (instance)
        instance.destroy();
}

void vk2d::destroy_swapchain()
{
    if (device)
    {
        VK_CHECK(device.waitIdle());
    }
    swapchain.framebuffers.clear();
    swapchain.views.clear();

    device.freeCommandBuffers(command_pool, swapchain.command_buffers);

    if (graphics_pipeline)
        device.destroyPipeline(graphics_pipeline);

    if (render_pass)
        device.destroyRenderPass(render_pass);

    if (pipeline_layout)
        device.destroyPipelineLayout(pipeline_layout);

    if (swapchain.obj)
    {
        device.destroySwapchainKHR(swapchain.obj);
        swapchain.obj = nullptr;
    }
}

void vk2d::recreate_swapchain()
{
    frame_queue_index = 0;
    destroy_swapchain();
    create_swapchain();
    create_render_pass();
    create_pipeline();
    create_framebuffers();
    create_command_buffers();
}

void vk2d::choose_physical_device()
{
    if (!surface)
        assert(0);

    auto devices = instance.enumeratePhysicalDevices();
    VK_CHECK(devices.result);
    std::vector<vk::PhysicalDevice> candidates;
    for (auto &d : devices.value)
    {
        auto extension_properties = d.enumerateDeviceExtensionProperties();
        VK_CHECK(extension_properties.result);
        bool presentable = false;
        for (auto &e : extension_properties.value)
        {
            std::string name = e.extensionName;
            if (name == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
            {
                presentable = true;
            }
        }

        if (!presentable)
            continue;

        auto queue_families = d.getQueueFamilyProperties();

        for (size_t q = 0; q < queue_families.size(); q++)
        {
            if (queue_families[q].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                graphics_queue_index = q;
                presentable = true;
                break;
            }

            presentable = false;
        }

        presentable = presentable && d.getSurfaceSupportKHR(graphics_queue_index, surface).value;

        if (presentable)
        {
            printf("Using %s\n", (char *)d.getProperties().deviceName);
            physical_device = d;
            return;
        }
    }

    physical_device = nullptr;
    graphics_queue_index = -1;
}

void vk2d::wait_idle()
{
    if (device)
    {
        auto result = device.waitIdle();
        VK_CHECK(result);
    }
}

void vk2d::draw()
{
    auto &frame = frame_queue[frame_queue_index];
    uint32_t next;

    VK_CHECK(device.waitForFences(1, &frame.fence.get(), true, UINT64_MAX));
    vk::ResultValue resval = device.acquireNextImageKHR(swapchain.obj, UINT64_MAX, frame.image_ready.get(), nullptr);

    if (resval.result != vk::Result::eSuccess)
    {
        if (resval.result == vk::Result::eErrorOutOfDateKHR)
        {
            printf("Recreating swapchain\n");
            recreate_swapchain();
            return;
        }

        VK_CHECK(resval.result);
        exit(1);
    }
    next = resval.value;

    if (swapchain.frame_fence[next] > -1)
    {
        VK_CHECK(device.waitForFences(1, &frame_queue[next].fence.get(), true, UINT64_MAX));
    }
    swapchain.frame_fence[next] = frame_queue_index;

    VK_CHECK(device.resetFences(1, &frame.fence.get()));

    vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submit_info(frame.image_ready.get(),
                               flags,
                               swapchain.command_buffers[next],
                               frame.render_finished.get());

    VK_CHECK(queue.submit(submit_info, frame.fence.get()));

    vk::PresentInfoKHR present_info(frame.render_finished.get(), swapchain.obj, next, {});

    VK_CHECK(queue.presentKHR(present_info));

    frame_queue_index = (frame_queue_index + 1) % frame_queue_size;
}

void vk2d::create_device()
{
    float queue_priority = 1.0f;
    std::vector<const char *> extension_names = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    vk::DeviceQueueCreateInfo dqci({}, graphics_queue_index, 1, &queue_priority);

    vk::DeviceCreateInfo dci;
    dci.setPEnabledExtensionNames(extension_names);

    std::vector<vk::DeviceQueueCreateInfo> pqci = {dqci};
    dci.setQueueCreateInfos(pqci);
    device = physical_device.createDevice(dci).value;

    queue = device.getQueue(graphics_queue_index, 0);

    vk::CommandPoolCreateInfo command_pool_info({}, graphics_queue_index);
    command_pool = device.createCommandPool(command_pool_info).value;
}

void vk2d::create_swapchain()
{
    if (!device || !surface)
        assert(0);

    vk::SurfaceCapabilitiesKHR surface_caps = physical_device.getSurfaceCapabilitiesKHR(surface).value;
    swapchain.size = surface_caps.minImageCount;

    vk::SwapchainCreateInfoKHR sci;
    sci
        .setSurface(surface)
        .setMinImageCount(swapchain.size)
        .setPresentMode(vk::PresentModeKHR::eFifo)
        .setImageFormat(vk::Format::eB8G8R8A8Unorm)
        .setImageExtent(surface_caps.currentExtent)
        .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
        .setImageArrayLayers(1)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setClipped(true);

    if (swapchain.obj)
        sci.setOldSwapchain(swapchain.obj);

    swapchain.obj = device.createSwapchainKHR(sci).value;
    swapchain.extents = surface_caps.currentExtent;
    swapchain.images = device.getSwapchainImagesKHR(swapchain.obj).value;

    swapchain.views.resize(swapchain.size);
    for (size_t i = 0; i < swapchain.size; i++)
    {
        vk::ImageViewCreateInfo image_view_create_info;
        image_view_create_info
            .setImage(swapchain.images[i])
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(vk::Format::eB8G8R8A8Unorm)
            .setComponents(vk::ComponentMapping())
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        swapchain.views[i] = device.createImageViewUnique(image_view_create_info).value;
    }

    swapchain.frame_fence.resize(swapchain.size);
    for (auto &f : swapchain.frame_fence)
        f = -1;

}

void vk2d::create_sync_objects()
{
    frame_queue.resize(frame_queue_size);
    for (size_t i = 0; i < frame_queue_size; i++)
    {
        vk::SemaphoreCreateInfo semaphore_create_info;
        frame_queue[i].image_ready = device.createSemaphoreUnique(semaphore_create_info).value;
        frame_queue[i].render_finished = device.createSemaphoreUnique(semaphore_create_info).value;

        vk::FenceCreateInfo fence_create_info(vk::FenceCreateFlagBits::eSignaled);
        frame_queue[i].fence = device.createFenceUnique(fence_create_info).value;
    }
    frame_queue_index = 0;
}

namespace glslang 
{
extern const TBuiltInResource DefaultTBuiltInResource;
}

void vk2d::create_shader_modules()
{
    glslang::InitializeProcess();
    EShMessages message_flags = (EShMessages)(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

    glslang::TShader vertex(EShLangVertex);
    glslang::TShader fragment(EShLangFragment);

    vertex.setStrings(&vertex_shader, 1);
    fragment.setStrings(&fragment_shader, 1);

    vertex.parse(&glslang::DefaultTBuiltInResource, 450, true, message_flags);
    fragment.parse(&glslang::DefaultTBuiltInResource, 450, true, message_flags);

    glslang::TProgram vertex_program;
    glslang::TProgram fragment_program;

    vertex_program.addShader(&vertex);
    fragment_program.addShader(&fragment);

    vertex_program.link(message_flags);
    fragment_program.link(message_flags);

    auto log = [](const char *msg)
    {
        if (msg != nullptr && msg[0] != '\0')
            puts(msg);
    };
    log(vertex_program.getInfoLog());
    log(vertex_program.getInfoDebugLog());
    log(fragment_program.getInfoLog());
    log(fragment_program.getInfoDebugLog());

    std::vector<uint32_t> vertex_spirv;
    std::vector<uint32_t> fragment_spirv;

    glslang::GlslangToSpv(*vertex_program.getIntermediate(EShLangVertex), vertex_spirv);
    glslang::GlslangToSpv(*fragment_program.getIntermediate(EShLangFragment), fragment_spirv);

    vk::ShaderModuleCreateInfo smci;
    smci.setCode(vertex_spirv);
    vertex_module = device.createShaderModule(smci).value;
    smci.setCode(fragment_spirv);
    fragment_module = device.createShaderModule(smci).value;
}

void vk2d::create_pipeline()
{
    create_shader_modules();

    if (!vertex_module || !fragment_module)
        assert(0);

    vk::PipelineShaderStageCreateInfo vertex_ci;
    vertex_ci
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(vertex_module)
        .setPName("main");

    vk::PipelineShaderStageCreateInfo fragment_ci;
    fragment_ci
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(fragment_module)
        .setPName("main");

    std::vector<vk::PipelineShaderStageCreateInfo> stages = { vertex_ci, fragment_ci };

    vk::PipelineVertexInputStateCreateInfo vertex_input_info;
    vertex_input_info
        .setVertexBindingDescriptionCount(0)
        .setVertexAttributeDescriptionCount(0);

    // Add Vertex attributes here

    vk::PipelineInputAssemblyStateCreateInfo pipeline_input_assembly_info;
    pipeline_input_assembly_info
        .setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(false);

    std::vector<vk::Viewport> viewports(1);
    viewports[0]
        .setX(0.0f)
        .setY(0.0f)
        .setWidth(swapchain.extents.width)
        .setHeight(swapchain.extents.height)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    std::vector<vk::Rect2D> scissors(1);
    scissors[0].extent = swapchain.extents;
    scissors[0].offset = vk::Offset2D(0, 0);

    vk::PipelineViewportStateCreateInfo pipeline_viewport_info;
    pipeline_viewport_info
        .setViewports(viewports)
        .setScissors(scissors);

    vk::PipelineRasterizationStateCreateInfo rasterizer_info;
    rasterizer_info
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setLineWidth(1.0f)
        .setDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setDepthBiasEnable(false)
        .setRasterizerDiscardEnable(false);

    vk::PipelineMultisampleStateCreateInfo multisample_info;
    multisample_info
        .setSampleShadingEnable(false)
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
    blend_state_info
        .setLogicOpEnable(false)
        .setAttachmentCount(1)
        .setPAttachments(&blend_attachment_info);

    vk::PipelineDynamicStateCreateInfo dynamic_state_info;
    dynamic_state_info.setDynamicStateCount(0);

    vk::PipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info
        .setSetLayoutCount(0)
        .setPushConstantRangeCount(0);

    pipeline_layout = device.createPipelineLayout(pipeline_layout_info).value;

    vk::GraphicsPipelineCreateInfo pipeline_create_info;
    pipeline_create_info
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
        .setLayout(pipeline_layout)
        .setRenderPass(render_pass)
        .setSubpass(0);

    vk::ResultValue<vk::Pipeline> result = device.createGraphicsPipeline(nullptr, pipeline_create_info);
    graphics_pipeline = result.value;

    device.destroyShaderModule(vertex_module);
    device.destroyShaderModule(fragment_module);
}

void vk2d::create_render_pass()
{
    vk::AttachmentDescription attachment_description(
        {}, 
        vk::Format::eB8G8R8A8Unorm, 
        vk::SampleCountFlagBits::e1, 
        vk::AttachmentLoadOp::eClear, 
        vk::AttachmentStoreOp::eStore, 
        vk::AttachmentLoadOp::eLoad, 
        vk::AttachmentStoreOp::eStore, 
        vk::ImageLayout::eUndefined, 
        vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference attachment_reference(0, vk::ImageLayout::eColorAttachmentOptimal);
    attachment_reference
        .setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDependency subpass_dependency;
    subpass_dependency
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits(0))
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::SubpassDescription subpass_description;
    subpass_description
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachments(attachment_reference);

    vk::RenderPassCreateInfo render_pass_info(
        {},
        attachment_description,
        subpass_description,
        subpass_dependency);
    render_pass = device.createRenderPass(render_pass_info).value;
}

void vk2d::create_framebuffers()
{
    swapchain.framebuffers.resize(swapchain.images.size());

    for (size_t i = 0; i < swapchain.images.size(); i++)
    {
        vk::FramebufferCreateInfo fci;
        std::vector<vk::ImageView> attachments = { swapchain.views[i].get() };
        fci
            .setAttachments(attachments)
            .setLayers(1)
            .setRenderPass(render_pass)
            .setWidth(swapchain.extents.width)
            .setHeight(swapchain.extents.height);

        swapchain.framebuffers[i] = device.createFramebufferUnique(fci).value;
    }

}

void vk2d::create_command_buffers()
{
    vk::CommandBufferAllocateInfo allocate_info;
    allocate_info.setCommandBufferCount(swapchain.images.size());
    allocate_info.setCommandPool(command_pool);
    swapchain.command_buffers = device.allocateCommandBuffers(allocate_info).value;

    for (size_t i = 0; i < swapchain.command_buffers.size(); i++)
    {
        auto &cb = swapchain.command_buffers[i];

        VK_CHECK(cb.begin(vk::CommandBufferBeginInfo{}));

        vk::ClearColorValue color;
        color.setFloat32({ 0.0f, 1.0f, 1.0f, 0.0f});
        vk::ClearValue clear_value(color);
        vk::RenderPassBeginInfo rpbi;
        rpbi.setRenderPass(render_pass);
        rpbi.setFramebuffer(swapchain.framebuffers[i].get());
        rpbi.setPClearValues(&clear_value);
        rpbi.setClearValueCount(1);
        rpbi.renderArea.setExtent(swapchain.extents);
        rpbi.renderArea.setOffset({0, 0});
        cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);
        cb.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
        cb.draw(3, 1, 0, 0);
        cb.endRenderPass();
        VK_CHECK(cb.end());
    }
}