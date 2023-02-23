#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"
#include "std_chrono_throttle.hpp"

namespace Vulkan
{

class Swapchain
{
  public:
    const int max_latency = 3;

    Swapchain(vk::Device device,
              vk::PhysicalDevice physical_device,
              vk::Queue queue,
              vk::SurfaceKHR surface,
              vk::CommandPool command_pool);
    ~Swapchain();
    bool create(unsigned int num_frames, int width = -1, int height = -1);
    bool recreate(int width = -1, int height = -1);
    bool begin_frame();
    void begin_render_pass();
    void end_render_pass();
    bool wait_on_frame(int frame_num);
    bool end_frame();
    // Returns true if vsync setting was changed, false if it was the same
    bool set_vsync(bool on);
    void set_max_frame_rate(double frame_rate);

    vk::Image get_image();
    vk::Framebuffer get_framebuffer();
    vk::CommandBuffer &get_cmd();
    unsigned int get_current_frame();
    vk::Extent2D get_extents();
    vk::RenderPass &get_render_pass();
    unsigned int get_num_frames();

  private:

    void create_render_pass();

    struct Frame
    {
        vk::UniqueFence fence;
        vk::UniqueSemaphore acquire;
        vk::UniqueSemaphore complete;
        vk::UniqueCommandBuffer command_buffer;
    };

    struct ImageViewFB
    {
        vk::Image image;
        vk::UniqueImageView image_view;
        vk::UniqueFramebuffer framebuffer;
    };

    vk::UniqueSwapchainKHR swapchain_object;
    vk::Extent2D extents;

    vk::UniqueRenderPass render_pass;

    Throttle throttle;
    unsigned int current_frame = 0;
    unsigned int current_swapchain_image = 0;
    unsigned int num_swapchain_images = 0;
    bool vsync = true;
    std::vector<Frame> frames;
    std::vector<ImageViewFB> imageviewfbs;

    vk::Device device;
    vk::SurfaceKHR surface;
    vk::CommandPool command_pool;
    vk::PhysicalDevice physical_device;
    vk::Queue queue;
};

} // namespace Vulkan