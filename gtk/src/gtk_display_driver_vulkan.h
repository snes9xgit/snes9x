/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "gtk_s9x.h"
#include "gtk_display_driver.h"
#include "../../vulkan/vulkan_context.hpp"
#include "../../vulkan/vulkan_texture.hpp"
#include "../../vulkan/slang_preset.hpp"
#include "../../vulkan/vulkan_shader_chain.hpp"

class S9xVulkanDisplayDriver : public S9xDisplayDriver
{
  public:
    S9xVulkanDisplayDriver(Snes9xWindow *window, Snes9xConfig *config);
    ~S9xVulkanDisplayDriver();
    void refresh(int width, int height) override;
    int init() override;
    void deinit() override;
    void update(uint16_t *buffer, int width, int height, int stride_in_pixels) override;
    void *get_parameters() override;
    void save(const char *filename) override;
    bool is_ready() override;

    static int query_availability();

  private:
    std::unique_ptr<Vulkan::Context> context;
    Vulkan::Swapchain *swapchain;
    vk::Device device;
    
    GdkDisplay *gdk_display;
    GdkWindow *gdk_window;
    Display *display;
    Window xid;
    Colormap colormap;
    int current_width;
    int current_height;

    void create_pipeline();
    vk::UniqueDescriptorSetLayout descriptor_set_layout;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniquePipeline pipeline;
    vk::Sampler linear_sampler;
    vk::Sampler nearest_sampler;

    void draw_buffer(uint8_t *buffer, int width, int height, int byte_stride);
    bool filter = true;
    std::vector<Vulkan::Texture> textures;
    std::vector<vk::UniqueDescriptorSet> descriptors;
    std::unique_ptr<Vulkan::ShaderChain> shaderchain;
};
