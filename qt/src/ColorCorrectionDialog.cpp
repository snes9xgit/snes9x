/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "ColorCorrectionDialog.hpp"
#include "EmuConfig.hpp"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

static QString adjustmentText(int value)
{
    return value > 0 ? QStringLiteral("+%1").arg(value) : QString::number(value);
}

ColorCorrectionDialog::ColorCorrectionDialog(EmuApplication *app, QWidget *parent)
    : QDialog(parent), app(app)
{
    setWindowTitle(tr("Color Correction"));

    auto layout = new QVBoxLayout(this);

    color_correction_checkbox = new QCheckBox(tr("Enable color correction (accurate SNES colors)"));
    layout->addWidget(color_correction_checkbox);

    auto group = new QGroupBox(tr("Adjustments"));
    auto grid = new QGridLayout(group);

    adjustments_checkbox = new QCheckBox(tr("Apply adjustments"));
    grid->addWidget(adjustments_checkbox, 0, 0, 1, 3);

    const std::pair<QString, QSlider **> rows[] = {
        { tr("Gamma:"), &gamma_slider },
        { tr("Contrast:"), &contrast_slider },
        { tr("Saturation:"), &saturation_slider },
    };

    int row = 1;
    for (auto &[name, slider_ptr] : rows)
    {
        auto label = new QLabel(name);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto slider = new QSlider(Qt::Horizontal);
        slider->setRange(-100, 100);
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setTickInterval(25);
        slider->setMinimumWidth(300);
        *slider_ptr = slider;

        auto value_label = new QLabel(adjustmentText(0));
        value_label->setAlignment(Qt::AlignCenter);
        value_label->setMinimumWidth(value_label->fontMetrics().horizontalAdvance("+100 "));
        connect(slider, &QSlider::valueChanged, value_label, [value_label](int value) {
            value_label->setText(adjustmentText(value));
        });

        grid->addWidget(label, row, 0);
        grid->addWidget(slider, row, 1);
        grid->addWidget(value_label, row, 2);
        row++;
    }

    // Sliders mean nothing until "Apply adjustments" is on, as on win32.
    auto enable_sliders = [this](bool enabled) {
        gamma_slider->setEnabled(enabled);
        contrast_slider->setEnabled(enabled);
        saturation_slider->setEnabled(enabled);
    };
    connect(adjustments_checkbox, &QCheckBox::toggled, this, enable_sliders);

    layout->addWidget(group);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::RestoreDefaults |
                                        QDialogButtonBox::Ok |
                                        QDialogButtonBox::Cancel);
    connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
            this, &ColorCorrectionDialog::setDefaults);
    connect(buttons, &QDialogButtonBox::accepted, this, &ColorCorrectionDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ColorCorrectionDialog::reject);
    layout->addWidget(buttons);

    auto config = app->config.get();
    color_correction_checkbox->setChecked(config->color_correction);
    adjustments_checkbox->setChecked(config->color_adjustments_enabled);
    gamma_slider->setValue(config->color_gamma);
    contrast_slider->setValue(config->color_contrast);
    saturation_slider->setValue(config->color_saturation);
    enable_sliders(config->color_adjustments_enabled);
}

void ColorCorrectionDialog::setDefaults()
{
    color_correction_checkbox->setChecked(false);
    adjustments_checkbox->setChecked(false);
    gamma_slider->setValue(0);
    contrast_slider->setValue(0);
    saturation_slider->setValue(0);
}

void ColorCorrectionDialog::accept()
{
    auto config = app->config.get();
    config->color_correction = color_correction_checkbox->isChecked();
    config->color_adjustments_enabled = adjustments_checkbox->isChecked();
    config->color_gamma = gamma_slider->value();
    config->color_contrast = contrast_slider->value();
    config->color_saturation = saturation_slider->value();
    app->updateSettings();
    QDialog::accept();
}
