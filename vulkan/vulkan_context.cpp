#include <exception>
#include <cstring>
#include <tuple>
#include "vulkan_context.hpp"
#include "vk_mem_alloc.hpp"
#include "slang_shader.hpp"
#include "vulkan/vulkan.hpp"

namespace Vulkan
{

Context::Context()
{
    auto dl = new vk::DynamicLoader;
    auto vkGetInstanceProcAddr =
        dl->getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}

Context::~Context()
{
    if (!device)
        return;
    device.waitIdle();

    swapchain = nullptr;

    command_pool.reset();
    descriptor_pool.reset();

    allocator.destroy();

    surface.reset();
    device.waitIdle();

    device.destroy();
}

bool Context::init(Display *dpy, Window xid, int preferred_device)
{
    if (instance)
        return false;

    xlib_display = dpy;
    xlib_window = xid;

    std::vector<const char *> extensions = { VK_KHR_XLIB_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME };
    vk::ApplicationInfo application_info({}, {}, {}, {}, VK_API_VERSION_1_0);
    vk::InstanceCreateInfo instance_create_info({}, &application_info, {}, extensions);

    instance = vk::createInstanceUnique(instance_create_info);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());
    surface = instance->createXlibSurfaceKHRUnique({ {}, xlib_display, xlib_window });

    init_device(preferred_device);
    init_vma();
    init_command_pool();
    init_descriptor_pool();

    create_swapchain();

    device.waitIdle();

    return true;
}

bool Context::init_descriptor_pool()
{
    auto descriptor_pool_size = vk::DescriptorPoolSize{}
        .setDescriptorCount(9)
        .setType(vk::DescriptorType::eCombinedImageSampler);
    auto descriptor_pool_create_info = vk::DescriptorPoolCreateInfo{}
        .setPoolSizes(descriptor_pool_size)
        .setMaxSets(20)
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    descriptor_pool = device.createDescriptorPoolUnique(descriptor_pool_create_info);

    return true;
}

bool Context::init_command_pool()
{
    vk::CommandPoolCreateInfo cpci({}, graphics_queue_family_index);
    cpci.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    command_pool = device.createCommandPoolUnique(cpci);

    return true;
}

bool Context::init_device(int preferred_device)
{
    auto device_list = instance->enumeratePhysicalDevices();

    auto find_device = [&]() -> vk::PhysicalDevice {
        for (auto &d : device_list)
        {
            auto ep = d.enumerateDeviceExtensionProperties();
            auto exists = std::find_if(ep.begin(), ep.end(), [](vk::ExtensionProperties &ext) {
                return (std::string(ext.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            });

            if (exists != ep.end())
                return d;
        }
        return device_list[0];
    };

    if (preferred_device > 0)
        physical_device = device_list[preferred_device];
    else
        physical_device = find_device();

    physical_device.getProperties(&physical_device_props);

    graphics_queue_family_index = UINT32_MAX;
    auto queue_props = physical_device.getQueueFamilyProperties();
    for (size_t i = 0; i < queue_props.size(); i++)
    {
        if (queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics)
            graphics_queue_family_index = i;
    }

    if (graphics_queue_family_index == UINT32_MAX)
        return false;

    std::vector<const char *> extension_names = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    std::vector<float> priorities { 1.0f };
    vk::DeviceQueueCreateInfo dqci({}, graphics_queue_family_index, priorities);
    vk::DeviceCreateInfo dci({}, dqci, {}, extension_names, {});
    device = physical_device.createDevice(dci);
    queue = device.getQueue(graphics_queue_family_index, 0);

    auto surface_formats = physical_device.getSurfaceFormatsKHR(surface.get());
    auto format = std::find_if(surface_formats.begin(), surface_formats.end(), [](vk::SurfaceFormatKHR &f) {
        return (f.format == vk::Format::eB8G8R8A8Unorm);
    });

    if (format == surface_formats.end())
        return false;

    return true;
}

bool Context::init_vma()
{
    auto vulkan_functions = vma::VulkanFunctions{}
        .setVkGetInstanceProcAddr(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr)
        .setVkGetDeviceProcAddr(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr);

    auto allocator_create_info = vma::AllocatorCreateInfo{}
        .setDevice(device)
        .setInstance(instance.get())
        .setPhysicalDevice(physical_device)
        .setPVulkanFunctions(&vulkan_functions);
    allocator = vma::createAllocator(allocator_create_info);

    return true;
}

bool Context::create_swapchain()
{
    wait_idle();
    swapchain = std::make_unique<Swapchain>(device, physical_device, queue, surface.get(), command_pool.get());
    return swapchain->create(3);
}

bool Context::recreate_swapchain()
{
    return swapchain->recreate();
}

void Context::wait_idle()
{
    if (device)
        device.waitIdle();
}

vk::CommandBuffer Context::begin_cmd_buffer()
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info(command_pool.get(), vk::CommandBufferLevel::ePrimary, 1);
    auto command_buffer = device.allocateCommandBuffers(command_buffer_allocate_info);
    one_time_use_cmd = command_buffer[0];
    one_time_use_cmd.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    return one_time_use_cmd;
}

void Context::end_cmd_buffer()
{
    one_time_use_cmd.end();
    vk::SubmitInfo submit_info{};
    submit_info.setCommandBuffers(one_time_use_cmd);
    queue.submit(vk::SubmitInfo());
    queue.waitIdle();
    device.freeCommandBuffers(command_pool.get(), one_time_use_cmd);
    one_time_use_cmd = nullptr;
}

} // namespace Vulkan