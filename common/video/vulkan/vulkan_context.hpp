#pragma once

#ifdef VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
#undef WINVER
#define WINVER 0x599
#endif
#include <cstdint>
#include "vulkan_hpp_wrapper.hpp"
#include "external/VulkanMemoryAllocator-Hpp/include/vk_mem_alloc.hpp"
#include "vulkan_swapchain.hpp"
#include <memory>

namespace Vulkan
{

class Context
{
  public:
    Context();
    ~Context();
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool init_Xlib();
    bool create_Xlib_surface(Display *dpy, Window xid);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool init_wayland();
    bool create_wayland_surface(wl_display *dpy, wl_surface *parent);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool init_win32();
    bool create_win32_surface(HINSTANCE hinstance, HWND hwnd);
#endif
    bool init();
    bool create_swapchain();
    bool recreate_swapchain();
    bool destroy_surface();
    void wait_idle();
    vk::CommandBuffer begin_cmd_buffer();
    void end_cmd_buffer();
    void hard_barrier(vk::CommandBuffer cmd);
    static std::vector<std::string> get_device_list();
    void set_preferred_device(int device) { preferred_device = device; };
    void unset_preferred_device() { preferred_device = -1; };

    vma::Allocator allocator;
    vk::Device device;
    uint32_t graphics_queue_family_index;
    vk::Queue queue;
    vk::UniqueCommandPool command_pool;
    vk::UniqueDescriptorPool descriptor_pool;
    std::unique_ptr<Swapchain> swapchain;
    vk::UniqueInstance instance;
    vk::PhysicalDevice physical_device;
    vk::PhysicalDeviceProperties physical_device_props;
    vk::UniqueSurfaceKHR surface;

  private:
    bool init_vma();
    bool init_device();
    bool init_command_pool();
    bool init_descriptor_pool();
    int preferred_device;

#ifdef VK_USE_PLATFORM_XLIB_KHR
    Display *xlib_display;
    Window xlib_window;
#endif

    vk::CommandBuffer one_time_use_cmd;
};

} // namespace Vulkan
