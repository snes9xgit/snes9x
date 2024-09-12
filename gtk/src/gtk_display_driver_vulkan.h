/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "gtk_s9x.h"
#include "gtk_display_driver.h"
#include "common/video/vulkan/vulkan_context.hpp"
#include "common/video/vulkan/vulkan_shader_chain.hpp"
#include "common/video/vulkan/vulkan_simple_output.hpp"
#include "common/video/std_chrono_throttle.hpp"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "common/video/wayland/wayland_surface.hpp"
#endif

class S9xVulkanDisplayDriver : public S9xDisplayDriver
{
  public:
    S9xVulkanDisplayDriver(Snes9xWindow *window, Snes9xConfig *config);
    ~S9xVulkanDisplayDriver();
    void refresh() override;
    int init() override;
    void deinit() override;
    void update(uint16_t *buffer, int width, int height, int stride_in_pixels) override;
    void *get_parameters() override;
    void save(const char *filename) override;
    bool is_ready() override;
    bool can_throttle() override { return true; }
    int get_width() final override { return current_width; }
    int get_height() final override { return current_height; }
    void shrink() override;
    void regrow() override;

    static int query_availability();

  private:
    bool init_imgui();

    std::unique_ptr<Vulkan::Context> context;
    vk::Device device;
    vk::UniqueDescriptorPool imgui_descriptor_pool;

    GdkDisplay *gdk_display;
    GdkWindow *gdk_window;
    Display *display;
    Window xid;
    int current_width;
    int current_height;
    Throttle throttle;

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    std::unique_ptr<WaylandSurface> wayland_surface;
#endif

    std::unique_ptr<Vulkan::ShaderChain> shaderchain;
    std::unique_ptr<Vulkan::SimpleOutput> simple_output;
};
