#pragma once

#include "X11/Xlib.h"
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
    bool init(Display *dpy, Window xid, int preferred_device = 0);
    bool create_swapchain();
    bool recreate_swapchain();
    void wait_idle();
    vk::CommandBuffer begin_cmd_buffer();
    void end_cmd_buffer();

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

    Display *xlib_display;
    Window xlib_window;

    vk::UniqueInstance instance;
    vk::PhysicalDevice physical_device;
    vk::PhysicalDeviceProperties physical_device_props;
    vk::UniqueSurfaceKHR surface;

    uint32_t graphics_queue_family_index;

    vk::CommandBuffer one_time_use_cmd;
};

} // namespace Vulkan