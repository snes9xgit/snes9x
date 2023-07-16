#include "DisplayPanel.hpp"
#include <QFileDialog>

DisplayPanel::DisplayPanel(EmuApplication *app_)
    : app(app_)
{
    setupUi(this);

    QObject::connect(comboBox_driver, &QComboBox::activated, [&](int index) {
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

    QObject::connect(comboBox_device, &QComboBox::activated, [&](int index) {
        if (app->config->display_device_index != index)
        {
            app->config->display_device_index = index;
            app->window->recreateCanvas();
        }
    });

    QObject::connect(checkBox_use_shader, &QCheckBox::clicked, [&](bool checked) {
        app->config->use_shader = checked;
        app->window->canvas->shaderChanged();
    });

    QObject::connect(pushButton_browse_shader, &QPushButton::clicked, [&] {
        selectShaderDialog();
    });

    QObject::connect(checkBox_vsync, &QCheckBox::clicked, [&](bool checked) {
        app->config->enable_vsync = checked;
    });

    QObject::connect(checkBox_reduce_input_lag, &QCheckBox::clicked, [&](bool checked) {
        app->config->reduce_input_lag = checked;
    });

    QObject::connect(checkBox_bilinear_filter, &QCheckBox::clicked, [&](bool checked) {
        app->config->bilinear_filter = checked;
    });

    QObject::connect(checkBox_adjust_for_vrr, &QCheckBox::clicked, [&](bool checked) {
        app->config->adjust_for_vrr = checked;
    });

    //

    QObject::connect(checkBox_maintain_aspect_ratio, &QCheckBox::clicked, [&](bool checked) {
        app->config->maintain_aspect_ratio = checked;
    });

    QObject::connect(checkBox_integer_scaling, &QCheckBox::clicked, [&](bool checked) {
        app->config->use_integer_scaling = checked;
    });

    QObject::connect(checkBox_overscan, &QCheckBox::clicked, [&](bool checked) {
        app->config->show_overscan = checked;
        app->updateSettings();
    });

    QObject::connect(comboBox_aspect_ratio, &QComboBox::activated, [&](int index) {
        auto &num = app->config->aspect_ratio_numerator;
        auto &den = app->config->aspect_ratio_denominator;
        if (index == 0) { num = 4,  den = 3;  }
        if (index == 1) { num = 64, den = 49; }
        if (index == 2) { num = 8,  den = 7;  }
    });

    QObject::connect(comboBox_high_resolution_mode, &QComboBox::currentIndexChanged, [&](int index) {
        app->config->high_resolution_effect = index;
        app->updateSettings();
    });

    QObject::connect(comboBox_messages, &QComboBox::currentIndexChanged, [&](int index) {
        bool recreate = (app->config->display_messages == EmuConfig::eOnscreen || index == EmuConfig::eOnscreen);

        app->config->display_messages = index;
        app->updateSettings();
        if (recreate)
            app->window->recreateUIAssets();
    });

    QObject::connect(spinBox_osd_size, &QSpinBox::valueChanged, [&](int value) {
        bool recreate = (app->config->osd_size != value && app->config->display_messages == EmuConfig::eOnscreen);
        app->config->osd_size = value;
        if (recreate)
            app->window->recreateUIAssets();
    });
}

DisplayPanel::~DisplayPanel()
{
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
    app->window->canvas->shaderChanged();
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
    comboBox_driver->addItem("Vulkan");

    driver_list.clear();
    driver_list.push_back({ driver_list.size(), "qt" });
    driver_list.push_back({ driver_list.size(), "opengl" });
    driver_list.push_back({ driver_list.size(), "vulkan" });

    for (auto &i : driver_list)
        if (config->display_driver == i.second)
        {
            comboBox_driver->setCurrentIndex(i.first);
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

    QWidget::showEvent(event);
}


