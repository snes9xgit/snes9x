#include "DisplayPanel.hpp"
#include "EmuMainWindow.hpp"
#include "EmuConfig.hpp"
#include <QFileDialog>

#include "snes9x.h"

DisplayPanel::DisplayPanel(EmuApplication *app_)
    : app(app_)
{
    setupUi(this);

    connect(comboBox_driver, &QComboBox::activated, [&](int index) {
        if (driver_list.empty() || index < 0 || index >= driver_list.size())
            return;

        auto display_driver = driver_list[index].second;

        if (display_driver != app->config->display_driver)
        {
            app->config->display_driver = display_driver;
            app->window->recreateCanvas();
            populateDevices();
        }
    });

    connect(comboBox_device, &QComboBox::activated, [&](int index) {
        if (app->config->display_device_index != index)
        {
            app->config->display_device_index = index;
            app->window->recreateCanvas();
        }
    });

    connect(checkBox_use_shader, &QCheckBox::clicked, [&](bool checked) {
        app->config->use_shader = checked;
        app->window->shaderChanged();
    });

    connect(pushButton_browse_shader, &QPushButton::clicked, [&] {
        selectShaderDialog();
    });

    connect(checkBox_vsync, &QCheckBox::clicked, [&](bool checked) {
        app->config->enable_vsync = checked;
    });

    connect(checkBox_reduce_input_lag, &QCheckBox::clicked, [&](bool checked) {
        app->config->reduce_input_lag = checked;
    });

    connect(checkBox_bilinear_filter, &QCheckBox::clicked, [&](bool checked) {
        app->config->bilinear_filter = checked;
    });

    connect(checkBox_adjust_for_vrr, &QCheckBox::clicked, [&](bool checked) {
        app->config->adjust_for_vrr = checked;
    });

    //

    connect(checkBox_maintain_aspect_ratio, &QCheckBox::clicked, [&](bool checked) {
        app->config->maintain_aspect_ratio = checked;
    });

    connect(checkBox_integer_scaling, &QCheckBox::clicked, [&](bool checked) {
        app->config->use_integer_scaling = checked;
    });

    connect(checkBox_overscan, &QCheckBox::clicked, [&](bool checked) {
        app->config->show_overscan = checked;
        app->updateSettings();
    });

    connect(comboBox_aspect_ratio, &QComboBox::activated, [&](int index) {
        auto &num = app->config->aspect_ratio_numerator;
        auto &den = app->config->aspect_ratio_denominator;
        if (index == 0) { num = 4,  den = 3;  }
        if (index == 1) { num = 64, den = 49; }
        if (index == 2) { num = 8,  den = 7;  }
    });

    connect(comboBox_high_resolution_mode, &QComboBox::currentIndexChanged, [&](int index) {
        app->config->high_resolution_effect = index;
        app->updateSettings();
    });

    connect(comboBox_messages, &QComboBox::currentIndexChanged, [&](int index) {
        bool recreate = (app->config->display_messages == EmuConfig::eOnscreen || index == EmuConfig::eOnscreen);

        app->config->display_messages = index;
        app->updateSettings();
        if (recreate)
            app->window->recreateUIAssets();
    });

    connect(spinBox_osd_size, &QSpinBox::valueChanged, [&](int value) {
        bool recreate = (app->config->osd_size != value && app->config->display_messages == EmuConfig::eOnscreen);
        app->config->osd_size = value;
        if (recreate)
            app->window->recreateUIAssets();
    });

    // Game Boy frame blending (Super Game Boy). Mirrors the win32 "Game Boy Image"
    // box: the Auto checkbox picks the blend per game from a built-in table, while
    // the two dropdowns set it manually when Auto is off.
    connect(checkBox_gb_frame_blend_auto, &QCheckBox::clicked, [&](bool checked) {
        app->config->gb_frame_blend_auto = checked;
        app->updateSettings();
        if (checked)
        {
            // updateSettings just re-picked the blend for the loaded game via the
            // table (S9xSGBApplyAutoBlend on the emu thread) and wrote it back into
            // the single stored value; read it out to show in the disabled dropdowns.
            app->emu_thread->runOnThread([&] {
                app->config->gb_frame_blend = Settings.GBFrameBlend;
                app->config->gb_frame_blend_layer = Settings.GBFrameBlendLayer;
            }, true);
            comboBox_gb_frame_blend->setCurrentIndex(app->config->gb_frame_blend);
            comboBox_gb_frame_blend_layer->setCurrentIndex(app->config->gb_frame_blend_layer);
        }
        updateGBBlendEnabledState();
    });

    connect(comboBox_gb_frame_blend, &QComboBox::currentIndexChanged, [&](int index) {
        app->config->gb_frame_blend = index;
        app->updateSettings();
        updateGBBlendEnabledState();
    });

    connect(comboBox_gb_frame_blend_layer, &QComboBox::currentIndexChanged, [&](int index) {
        app->config->gb_frame_blend_layer = index;
        app->updateSettings();
    });

    groupBox_software_filters->hide();
}

void DisplayPanel::updateGBBlendEnabledState()
{
    // The Game Boy Image options only apply to a Game Boy / GBC / SGB game — grey
    // them out for SNES titles (or when nothing is loaded). With Auto on, the two
    // dropdowns are list-driven so they're greyed too; the layer one also needs
    // blending to be on (mode != Off).
    bool gb_active = (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive);
    bool manual = gb_active && !app->config->gb_frame_blend_auto;
    groupBox_gb_image->setEnabled(gb_active);
    checkBox_gb_frame_blend_auto->setEnabled(gb_active);
    comboBox_gb_frame_blend->setEnabled(manual);
    comboBox_gb_frame_blend_layer->setEnabled(manual && app->config->gb_frame_blend != EmuConfig::eGBBlendOff);
}

void DisplayPanel::selectShaderDialog()
{
    QFileDialog dialog(this, tr("Select a Folder"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Shader Presets (*.slangp *.glslp)"));
    if (!app->config->last_shader_folder.empty())
        dialog.setDirectory(app->config->last_shader_folder.c_str());

    if (!dialog.exec())
        return;

    app->config->shader = dialog.selectedFiles().at(0).toUtf8();
    app->config->last_shader_folder = dialog.directory().absolutePath().toStdString();
    lineEdit_shader->setText(app->config->shader.c_str());
    app->window->shaderChanged();
}

void DisplayPanel::populateDevices()
{
    comboBox_device->clear();
    auto device_list = app->window->getDisplayDeviceList();
    for (auto &d : device_list)
        comboBox_device->addItem(d.c_str());
    comboBox_device->setCurrentIndex(app->config->display_device_index);
}

void DisplayPanel::showEvent(QShowEvent *event)
{
    auto &config = app->config;

    comboBox_driver->clear();
    comboBox_driver->addItem("Qt Software");
    comboBox_driver->addItem("OpenGL");

    driver_list.clear();
    driver_list.emplace_back(driver_list.size(), "qt");
    driver_list.emplace_back(driver_list.size(), "opengl");

#ifndef __APPLE__
    // macOS has no native Vulkan, so the driver is not built there.
    comboBox_driver->addItem("Vulkan");
    driver_list.emplace_back(driver_list.size(), "vulkan");
#endif

    for (auto &[index, driver] : driver_list)
        if (config->display_driver == driver)
        {
            comboBox_driver->setCurrentIndex(index);
            break;
        }

    populateDevices();

    checkBox_use_shader->setChecked(config->use_shader);
    lineEdit_shader->setText(config->shader.c_str());

    checkBox_vsync->setChecked(config->enable_vsync);
    checkBox_reduce_input_lag->setChecked(config->reduce_input_lag);
    checkBox_bilinear_filter->setChecked(config->bilinear_filter);
    checkBox_adjust_for_vrr->setChecked(config->adjust_for_vrr);

    checkBox_maintain_aspect_ratio->setChecked(config->maintain_aspect_ratio);
    checkBox_integer_scaling->setChecked(config->use_integer_scaling);
    checkBox_overscan->setChecked(config->show_overscan);

    if (config->aspect_ratio_numerator == 4)
        comboBox_aspect_ratio->setCurrentIndex(0);
    else if (config->aspect_ratio_numerator == 64)
        comboBox_aspect_ratio->setCurrentIndex(1);
    else if (config->aspect_ratio_numerator == 8)
        comboBox_aspect_ratio->setCurrentIndex(2);

    comboBox_high_resolution_mode->setCurrentIndex(config->high_resolution_effect);

    comboBox_messages->setCurrentIndex(config->display_messages);
    spinBox_osd_size->setValue(config->osd_size);

    checkBox_gb_frame_blend_auto->setChecked(config->gb_frame_blend_auto);
    if (config->gb_frame_blend_auto &&
        (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive))
    {
        // win32 shows the single stored value, which the auto table has already
        // overwritten for the loaded game; mirror that by reading it back from core.
        app->emu_thread->runOnThread([&] {
            config->gb_frame_blend = Settings.GBFrameBlend;
            config->gb_frame_blend_layer = Settings.GBFrameBlendLayer;
        }, true);
    }
    comboBox_gb_frame_blend->setCurrentIndex(config->gb_frame_blend);
    comboBox_gb_frame_blend_layer->setCurrentIndex(config->gb_frame_blend_layer);
    updateGBBlendEnabledState();

    QWidget::showEvent(event);
}


