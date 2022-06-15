#pragma once

#include "vulkan/vulkan.hpp"
#include <fstream>

#ifdef VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include <wayland-client.h>
#endif

#define VK_CHECK(result) vk_check_result_function(result, __FILE__, __LINE__)
inline void vk_check_result_function(vk::Result result, const char *file, int line)
{
    if (result != vk::Result::eSuccess)
    {
        printf("%s:%d  Vulkan error: %s\n", file, line, vk::to_string(result).c_str());
    }
}

class vk2d
{
  public:
    vk2d();
    ~vk2d();

#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool init_xlib_instance();
    void attach(Display *dpy, Window xid);
#endif

    bool init_device();
    void deinit();
    bool create_instance();
    void choose_physical_device();
    void create_device();
    void create_swapchain();
    void create_sync_objects();
    void create_shader_modules();
    void create_pipeline();
    void create_render_pass();
    void create_framebuffers();
    void create_command_buffers();
    void recreate_swapchain();
    void destroy_swapchain();
    void wait_idle();
    void draw();

    vk::Instance instance;
    vk::PhysicalDevice physical_device;
    vk::SurfaceKHR surface;
    vk::Pipeline graphics_pipeline;
    vk::PipelineLayout pipeline_layout;
    vk::RenderPass render_pass;

    vk::ShaderModule vertex_module;
    vk::ShaderModule fragment_module;

    vk::Device device;
    vk::Queue queue;
    vk::CommandPool command_pool;
    size_t graphics_queue_index;

    struct frame_t {
        vk::UniqueSemaphore render_finished;
        vk::UniqueSemaphore image_ready;
        vk::UniqueFence fence;
    };

    static const size_t frame_queue_size = 3;
    size_t frame_queue_index;
    std::vector<frame_t> frame_queue;

    struct {
        vk::SwapchainKHR obj;
        vk::Extent2D extents;
        std::vector<vk::Image> images;
        std::vector<vk::UniqueImageView> views;
        std::vector<vk::UniqueFramebuffer> framebuffers;
        std::vector<vk::CommandBuffer> command_buffers;
        std::vector<int> frame_fence;
        size_t size;
    } swapchain;

    static bool dispatcher_initialized;
};