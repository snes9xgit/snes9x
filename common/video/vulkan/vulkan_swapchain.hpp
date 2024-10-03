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
    bool create();
    bool uncreate();
    bool recreate();
    bool create_resources();
    bool check_and_resize(int width = -1, int height = -1);
    Swapchain &set_desired_size(int width, int height) { desired_width = width; desired_height = height; return *this; }
    void unset_desired_size() { desired_width = -1; desired_height = -1; }
    Swapchain &set_desired_latency(int latency) { desired_latency = latency; return *this; }
    void unset_desired_latency() { desired_latency = -1; }

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
    int desired_width = -1;
    int desired_height = -1;
    int desired_latency = -1;
    uint64_t presentation_id = 0;
    bool vsync = true;
    bool supports_immediate = false;
    bool supports_mailbox = false;
    bool supports_relaxed = false;
    std::vector<Frame> frames;
    std::vector<ImageData> image_data;

    Vulkan::Context &context;
    vk::Device device;
    vk::CommandPool command_pool;
    vk::PhysicalDevice physical_device;
    vk::Queue queue;
};

} // namespace Vulkan