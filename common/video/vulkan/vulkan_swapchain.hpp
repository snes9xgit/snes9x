#pragma once

#include "vulkan_hpp_wrapper.hpp"
#include <functional>

namespace Vulkan
{

class Context;

class Swapchain
{
  public:
    Swapchain(Context &);
    ~Swapchain();
    bool create(unsigned int num_frames, int width = -1, int height = -1);
    bool recreate(int width = -1, int height = -1);
    bool create_resources();
    bool check_and_resize(int width = -1, int height = -1);
    bool begin_frame();
    void begin_render_pass();
    void end_render_pass();
    bool end_frame();
    void end_frame_without_swap();
    bool swap();
    void set_vsync(bool on);
    void on_render_pass_end(std::function<void()> function);
    int get_num_frames() { return num_swapchain_images; }
    vk::PresentModeKHR get_present_mode();

    vk::Image get_image();
    vk::Framebuffer get_framebuffer();
    vk::CommandBuffer &get_cmd();
    vk::Extent2D get_extents();
    vk::RenderPass &get_render_pass();

  private:
    std::function<void()> end_render_pass_function;
    void create_render_pass();

    struct Frame
    {
        vk::UniqueFence fence;
        vk::UniqueSemaphore acquire;
        vk::UniqueSemaphore complete;
        vk::UniqueCommandBuffer command_buffer;
    };

    struct ImageData
    {
        vk::Image image;
        vk::UniqueImageView image_view;
        vk::UniqueFramebuffer framebuffer;
    };

    vk::UniqueSwapchainKHR swapchain_object;
    vk::Extent2D extents;

    vk::UniqueRenderPass render_pass;

    unsigned int current_frame = 0;
    unsigned int current_swapchain_image = 0;
    unsigned int num_swapchain_images = 0;
    uint64_t presentation_id = 0;
    bool vsync = true;
    bool supports_immediate = false;
    bool supports_mailbox = false;
    bool supports_relaxed = false;
    std::vector<Frame> frames;
    std::vector<ImageData> image_data;

    Vulkan::Context &context;
    vk::Device device;
    vk::SurfaceKHR surface;
    vk::CommandPool command_pool;
    vk::PhysicalDevice physical_device;
    vk::Queue queue;
};

} // namespace Vulkan