#pragma once
#include <QWindow>

#include "EmuCanvas.hpp"
#include "ShaderParametersDialog.hpp"
#include "../../vulkan/vulkan_simple_output.hpp"
#include "../../vulkan/vulkan_shader_chain.hpp"

#ifndef _WIN32
#include "common/video/wayland_surface.hpp"
#endif

class EmuCanvasVulkan : public EmuCanvas
{
  public:
    EmuCanvasVulkan(EmuConfig *config, QWidget *parent, QWidget *main_window);
    ~EmuCanvasVulkan();

    void createContext();
    void deinit() override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    QPaintEngine *paintEngine() const override { return nullptr; }

    std::vector<std::string> getDeviceList() override;
    void shaderChanged() override;
    void showParametersDialog() override;
    void saveParameters(std::string filename) override;

    void draw() override;

    bool initImGui();
    vk::UniqueRenderPass imgui_render_pass;
    vk::UniqueDescriptorPool imgui_descriptor_pool;

    std::unique_ptr<Vulkan::Context> context;
    std::unique_ptr<Vulkan::SimpleOutput> simple_output;
    std::unique_ptr<Vulkan::ShaderChain> shader_chain;

  private:
    void tryLoadShader();
    std::string current_shader;
    QWindow *window = nullptr;
    std::unique_ptr<ShaderParametersDialog> shader_parameters_dialog = nullptr;
    QString platform;

#ifndef _WIN32
    std::unique_ptr<WaylandSurface> wayland_surface;
#endif
};
