#include <cstring>
#include <vector>
#include <string>
#include <optional>
#include "vulkan_context.hpp"

namespace Vulkan
{

static std::unique_ptr<vk::DynamicLoader> dl;

Context::Context()
{
}

Context::~Context()
{
    if (!device)
        return;

    wait_idle();
    swapchain.reset();
    command_pool.reset();
    descriptor_pool.reset();
    allocator.destroy();
    surface.reset();
    wait_idle();
    device.destroy();
}

static bool load_loader()
{
    if (dl)
        return true;

    dl = std::make_unique<vk::DynamicLoader>();
    if (!dl->success())
    {
        dl.reset();
        return false;
    }

    auto vkGetInstanceProcAddr =
        dl->getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    return true;
}

static vk::UniqueInstance create_instance_preamble(const char *wsi_extension)
{
    load_loader();
    if (!dl || !dl->success())
        return {};

    std::vector<const char *> extensions = {
        wsi_extension,
        VK_KHR_SURFACE_EXTENSION_NAME
    };
    vk::ApplicationInfo application_info({}, {}, {}, {}, VK_API_VERSION_1_1);
    vk::InstanceCreateInfo instance_create_info({}, &application_info, {}, extensions);

    auto [result, instance] = vk::createInstanceUnique(instance_create_info);

    if (result != vk::Result::eSuccess)
    {
        instance.reset();
        return {};
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

    return std::move(instance);
}

std::vector<std::string> Vulkan::Context::get_device_list()
{
    std::vector<std::string> device_names;
    auto instance = create_instance_preamble(VK_KHR_SURFACE_EXTENSION_NAME);
    if (!instance)
        return {};

    auto [result, device_list] = instance->enumeratePhysicalDevices();
    for (auto &d : device_list)
    {
        auto props = d.getProperties();
        std::string device_name((const char *)props.deviceName);

        device_name += " (" + vk::to_string(props.deviceType) + ")";
        device_names.push_back(device_name);
    }

    return device_names;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool Context::init_win32()
{
    instance = create_instance_preamble(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    if (!instance)
        return false;

    return init();
}

bool Context::create_win32_surface(HINSTANCE hinstance, HWND hwnd)
{
    auto win32_surface_create_info = vk::Win32SurfaceCreateInfoKHR{}
        .setHinstance(hinstance)
        .setHwnd(hwnd);
    auto retval = instance->createWin32SurfaceKHRUnique(win32_surface_create_info);
    if (retval.result != vk::Result::eSuccess)
        return false;
    surface = std::move(retval.value);
    return true;
}
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool Context::init_Xlib()
{
    instance = create_instance_preamble(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    if (!instance)
        return false;

    return init();
}

bool Context::create_Xlib_surface(Display *dpy, Window xid)
{
    auto retval = instance->createXlibSurfaceKHRUnique({ {}, dpy, xid });
    if (retval.result != vk::Result::eSuccess)
        return false;
    surface = std::move(retval.value);

    return true;
}
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool Context::init_wayland()
{
    instance = create_instance_preamble(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    if (!instance)
        return false;

    return init();
}

bool Context::create_wayland_surface(wl_display *dpy, wl_surface *parent)
{
    auto wayland_surface_create_info = vk::WaylandSurfaceCreateInfoKHR{}
        .setSurface(parent)
        .setDisplay(dpy);

    auto [result, new_surface] = instance->createWaylandSurfaceKHRUnique(wayland_surface_create_info);
    if (result != vk::Result::eSuccess)
        return false;
    surface = std::move(new_surface);

    return true;
}
#endif

bool Context::destroy_surface()
{
    wait_idle();
    if (swapchain)
        swapchain->uncreate();

    surface.reset();

    return true;
}

bool Context::init()
{
    init_device();
    init_vma();
    init_command_pool();
    init_descriptor_pool();

    swapchain = std::make_unique<Swapchain>(*this);
    wait_idle();
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

    auto retval = device.createDescriptorPoolUnique(descriptor_pool_create_info);
    descriptor_pool = std::move(retval.value);

    return true;
}

bool Context::init_command_pool()
{
    vk::CommandPoolCreateInfo cpci({}, graphics_queue_family_index);
    cpci.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    auto retval = device.createCommandPoolUnique(cpci);
    command_pool = std::move(retval.value);

    return true;
}

static bool find_extension(std::vector<vk::ExtensionProperties> &props, const char *extension_string)
{
    return std::find_if(props.begin(),
                        props.end(),
                        [&](vk::ExtensionProperties &ext) {
                            return (std::string(ext.extensionName.data()) == extension_string);
                        }) != props.end();
};

static std::optional<uint32_t> find_graphics_queue(vk::PhysicalDevice &device)
{
    auto queue_props = device.getQueueFamilyProperties();
    for (size_t i = 0; i < queue_props.size(); i++)
    {
        if (queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            return i;
        }
    }

    return std::nullopt;
}

static bool check_extensions(std::vector<const char *> &required_extensions, vk::PhysicalDevice &device)
{
    auto props = device.enumerateDeviceExtensionProperties().value;
    for (const auto &extension : required_extensions)
    {
        if (!find_extension(props, extension))
            return false;
    }
    return true;
};

bool Context::init_device()
{
    std::vector<const char *> required_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    auto device_list = instance->enumeratePhysicalDevices().value;
    bool device_chosen = false;
    physical_device = vk::PhysicalDevice();

    if (preferred_device > -1 &&
        (size_t)preferred_device < device_list.size() &&
        check_extensions(required_extensions, device_list[preferred_device]))
    {
        physical_device = device_list[preferred_device];
        device_chosen = true;
    }

    if (!device_chosen)
    {
        for (auto &device : device_list)
        {
            if (check_extensions(required_extensions, device))
            {
                physical_device = device;
                device_chosen = true;
                break;
            }
        }
    }

    if (!device_chosen)
        return false;

    if (auto index = find_graphics_queue(physical_device))
        graphics_queue_family_index = *index;
    else
        return false;

    std::vector<float> priorities = { 1.0f };
    vk::DeviceQueueCreateInfo dqci({}, graphics_queue_family_index, priorities);
    vk::DeviceCreateInfo dci({}, dqci, {}, required_extensions);

    device = physical_device.createDevice(dci).value;
    queue = device.getQueue(graphics_queue_family_index, 0);

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
    allocator = vma::createAllocator(allocator_create_info).value;

    return true;
}

bool Context::create_swapchain()
{
    wait_idle();
    return swapchain->create();
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
    auto command_buffer = device.allocateCommandBuffers(command_buffer_allocate_info).value;
    one_time_use_cmd = command_buffer[0];
    one_time_use_cmd.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    return one_time_use_cmd;
}

void Context::hard_barrier(vk::CommandBuffer cmd)
{
    vk::MemoryBarrier barrier(vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
                              vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite);
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                        vk::PipelineStageFlagBits::eAllCommands,
                        {}, barrier, {}, {});
}

void Context::end_cmd_buffer()
{
    one_time_use_cmd.end();
    vk::SubmitInfo submit_info{};
    submit_info.setCommandBuffers(one_time_use_cmd);
    queue.submit(submit_info);
    queue.waitIdle();
    device.freeCommandBuffers(command_pool.get(), one_time_use_cmd);
    one_time_use_cmd = nullptr;
}

} // namespace Vulkan
