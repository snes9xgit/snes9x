#pragma once
#include <QDialog>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>

#include "EmuCanvas.hpp"
#include "EmuConfig.hpp"

class ShaderParametersDialog : public QDialog
{
  public:
    ShaderParametersDialog(EmuCanvas *parent_, EmuCanvas::ShaderProperties properties_);

    void refreshWidgets();
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void save();
    void saveAs();

    std::vector<std::tuple<QSlider *, QDoubleSpinBox *, QCheckBox *>> widgets;
    EmuCanvas::ShaderProperties properties;
    std::string saved_name;
    std::vector<EmuCanvas::Parameter> saved_parameters;
    std::vector<EmuCanvas::Parameter> *parameters;

    EmuCanvas *canvas = nullptr;
    EmuConfig *config = nullptr;
};