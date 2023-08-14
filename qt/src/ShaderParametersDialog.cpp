#include "ShaderParametersDialog.hpp"
#include <QLayout>
#include <QTableWidget>
#include <QLabel>
#include <QSizePolicy>
#include <QPushButton>
#include <QSpacerItem>
#include <QScrollArea>
#include <QFileDialog>
#include <QResizeEvent>

static bool is_simple(const EmuCanvas::Parameter &p)
{
    return (p.min == 0.0 && p.max == 1.0 && p.step == 1.0);
}

static bool is_pointless(const EmuCanvas::Parameter &p)
{
    return (p.min == p.max);
}

ShaderParametersDialog::ShaderParametersDialog(EmuCanvas *parent_, std::vector<EmuCanvas::Parameter> *parameters_)
    : QDialog(parent_), canvas(parent_), config(parent_->config), parameters(parameters_)
{
    setWindowTitle(tr("Shader Parameters"));
    setMinimumSize(600, 200);
    auto layout = new QVBoxLayout(this);

    auto scroll_area = new QScrollArea(this);
    scroll_area->setFrameShape(QFrame::Shape::StyledPanel);
    scroll_area->setWidgetResizable(true);
    auto scroll_area_widget_contents = new QWidget();
    scroll_area_widget_contents->setGeometry(0, 0, 400, 300);

    auto grid = new QGridLayout(scroll_area_widget_contents);
    scroll_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto buttonbox = new QHBoxLayout();

    for (int i = 0; i < parameters->size(); i++)
    {
        auto &p = (*parameters)[i];
        QSlider *slider = nullptr;
        QDoubleSpinBox *spinbox = nullptr;
        QCheckBox *checkbox = nullptr;

        auto label = new QLabel(p.name.c_str(), scroll_area_widget_contents);
        grid->addWidget(label, i, 0, 1, 1);

        if (is_pointless(p))
        {
            widgets.push_back({});
            continue;
        }
        if (is_simple(p))
        {
            checkbox = new QCheckBox(scroll_area_widget_contents);
            checkbox->setChecked(p.val == 1.0);
            grid->addWidget(checkbox, i, 1, 1, 2);
            QObject::connect(checkbox, &QCheckBox::clicked, [&, i](bool checked) {
                (*parameters)[i].val = checked ? 1.0 : 0.0;
            });
            widgets.push_back({ slider, spinbox, checkbox });
            continue;
        }

        slider = new QSlider(scroll_area_widget_contents);
        grid->addWidget(slider, i, 1, 1, 1);
        slider->setOrientation(Qt::Horizontal);
        slider->setTickInterval(1);
        slider->setRange(0, (p.max - p.min) / p.step);
        slider->setValue(p.val / p.step);

        spinbox = new QDoubleSpinBox(scroll_area_widget_contents);
        grid->addWidget(spinbox, i, 2, 1, 1);
        spinbox->setDecimals(p.significant_digits > 5 ? 5 : p.significant_digits);
        spinbox->setRange(p.min, p.max);
        spinbox->setSingleStep(p.step);
        spinbox->setValue(p.val);

        QObject::connect(slider, &QSlider::valueChanged, [&, i, slider, spinbox](int value) {
            auto &p = (*parameters)[i];
            double new_value = value * p.step + p.min;
            spinbox->blockSignals(true);
            spinbox->setValue(new_value);
            spinbox->blockSignals(false);
            p.val = new_value;
        });

        QObject::connect(spinbox, &QDoubleSpinBox::valueChanged, [&, i, slider, spinbox](double value) {
            auto &p = (*parameters)[i];
            int steps = round((value - p.min) / p.step);
            p.val = steps * p.step + p.min;

            slider->blockSignals(true);
            slider->setValue(steps);
            slider->blockSignals(false);

            spinbox->blockSignals(true);
            spinbox->setValue(p.val);
            spinbox->blockSignals(false);
        });

        widgets.push_back({ slider, spinbox, checkbox });
    }

    auto reset = new QPushButton(tr("&Reset"), this);
    QObject::connect(reset, &QPushButton::clicked, [&] {
        *parameters = saved_parameters;
        refreshWidgets();
    });

    buttonbox->addWidget(reset);

    buttonbox->addStretch(1);

    auto saveas = new QPushButton(tr("Save &As"), this);
    buttonbox->addWidget(saveas);
    connect(saveas, &QPushButton::clicked, [&] {
        saveAs();
    });

    auto closebutton = new QPushButton(tr("&Save"), this);
    connect(closebutton, &QPushButton::clicked, [&] {
        save();
        close();
    });

    buttonbox->addWidget(closebutton);
    scroll_area->setWidget(scroll_area_widget_contents);
    layout->addWidget(scroll_area);
    layout->addLayout(buttonbox, 0);

    if (config->shader_parameters_dialog_width != 0)
        resize(config->shader_parameters_dialog_width, config->shader_parameters_dialog_height);
}

void ShaderParametersDialog::save()
{
    if (std::equal(parameters->begin(), parameters->end(), saved_parameters.begin()))
        return;

    QString shadername(config->shader.c_str());
    std::string extension;
    if (shadername.endsWith("slangp", Qt::CaseInsensitive))
        extension = ".slangp";
    else if (shadername.endsWith("glslp", Qt::CaseInsensitive))
        extension = ".glslp";

    saved_parameters = *parameters;

    QDir dir(config->findConfigDir().c_str());
    auto filename = dir.absoluteFilePath(QString::fromStdString("customized_shader" + extension));
    canvas->saveParameters(filename.toStdString());
    config->shader = QDir::toNativeSeparators(filename).toStdString();
}

void ShaderParametersDialog::saveAs()
{
    auto folder = config->last_shader_folder;
    auto filename = QFileDialog::getSaveFileName(this, tr("Save Shader Preset As"), folder.c_str());
    canvas->saveParameters(filename.toStdString());
    config->shader = QDir::toNativeSeparators(filename).toStdString();
}

void ShaderParametersDialog::refreshWidgets()
{
    for (size_t i = 0; i < widgets.size(); i++)
    {
        auto &p = (*parameters)[i];

        if (is_pointless(p))
            continue;

        auto [slider, spinbox, checkbox] = widgets[i];

        if (is_simple(p))
        {
            checkbox->setChecked(p.val == 1.0 ? true : false);
            continue;
        }

        spinbox->blockSignals(true);
        spinbox->setValue(p.val);
        spinbox->blockSignals(false);

        slider->blockSignals(true);
        slider->setValue((p.val - p.min) / p.step);
        slider->blockSignals(false);
    }
}

void ShaderParametersDialog::showEvent(QShowEvent *event)
{
    refreshWidgets();
    saved_parameters = *parameters;
}

void ShaderParametersDialog::closeEvent(QCloseEvent *event)
{
    *parameters = saved_parameters;
}

void ShaderParametersDialog::resizeEvent(QResizeEvent *event)
{
    config->shader_parameters_dialog_width = event->size().width();
    config->shader_parameters_dialog_height = event->size().height();
}

ShaderParametersDialog::~ShaderParametersDialog()
{
}