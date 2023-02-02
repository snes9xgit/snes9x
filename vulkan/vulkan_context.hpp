#pragma once

#ifdef VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif
#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan_swapchain.hpp"
#include <memory>
#include <optional>

namespace Vulkan
{

class Context
{
  public:
    Context();
    ~Context();
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool init_Xlib(Display *dpy, Window xid, int preferred_device = 0);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool init_wayland(wl_display *dpy, wl_surface *parent, int width, int height, int preferred_device = 0);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool init_win32(HINSTANCE hinstance, HWND hwnd, int preferred_device = 0);
#endif
    bool init(int preferred_device = 0);
    bool create_swapchain(int width = -1, int height = -1);
    bool recreate_swapchain(int width = -1, int height = -1);
    void wait_idle();
    vk::CommandBuffer begin_cmd_buffer();
    void end_cmd_buffer();
    void hard_barrier(vk::CommandBuffer cmd);

    vma::Allocator allocator;
    vk::Device device;
    vk::Queue queue;
    vk::UniqueCommandPool command_pool;
    vk::UniqueDescriptorPool descriptor_pool;
    std::unique_ptr<Swapchain> swapchain;

  private:
    bool init_vma();
    bool init_device(int preferred_device = 0);
    bool init_command_pool();
    bool init_descriptor_pool();
    void create_vertex_buffer();

    vk::UniquePipeline create_generic_pipeline();

#ifdef VK_USE_PLATFORM_XLIB_KHR
    Display *xlib_display;
    Window xlib_window;
#endif

    vk::UniqueInstance instance;
    vk::PhysicalDevice physical_device;
    vk::PhysicalDeviceProperties physical_device_props;
    vk::UniqueSurfaceKHR surface;

    uint32_t graphics_queue_family_index;

    vk::CommandBuffer one_time_use_cmd;
};

} // namespace Vulkan