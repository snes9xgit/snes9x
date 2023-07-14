#ifndef __EMU_CANVAS_OPENGL_HPP
#define __EMU_CANVAS_OPENGL_HPP
#include <QWindow>

#include "EmuCanvas.hpp"
#include "ShaderParametersDialog.hpp"

class OpenGLContext;
class GLSLShader;

class EmuCanvasOpenGL : public EmuCanvas
{
  public:
    EmuCanvasOpenGL(EmuConfig *config, QWidget *parent, QWidget *main_window);
    ~EmuCanvasOpenGL();

    void createContext() override;
    void deinit() override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    QPaintEngine * paintEngine() const override { return nullptr; }
    void draw() override;
    void shaderChanged() override;
    void showParametersDialog() override;
    void saveParameters(std::string filename) override;


  private:
    void resizeTexture(int width, int height);
    void createStockShaders();
    void stockShaderDraw();
    void customShaderDraw();
    void uploadTexture();
    void loadShaders();

    unsigned int stock_program;
    unsigned int texture;
    unsigned stock_coord_buffer;
    std::unique_ptr<OpenGLContext> context;
    bool using_shader;
    std::unique_ptr<GLSLShader> shader;
    std::unique_ptr<ShaderParametersDialog> shader_parameters_dialog;

    // The first 8 values are vertices for a triangle strip, the second are texture
    // coordinates for a stock NPOT texture.
    const float coords[16] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
                                0.0f,  1.0f, 1.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, };
};

#endif