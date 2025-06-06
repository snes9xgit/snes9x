#include "EmulationPanel.hpp"
#include "EmuApplication.hpp"
#include "EmuConfig.hpp"

EmulationPanel::EmulationPanel(EmuApplication *app_)
    : app(app_)
{
    setupUi(this);

    auto connect_checkbox = [&](QCheckBox *box, bool *config) {
        connect(box, &QCheckBox::clicked, [&, box, config](bool is_checked) {
            *config = is_checked;
            app->updateSettings();
        });
    };
    auto connect_spin = [&](QSpinBox *box, int *config) {
        connect(box, &QSpinBox::valueChanged, [&, box, config](int value) {
            *config = value;
            app->updateSettings();
        });
    };
    auto connect_combo = [&](QComboBox *box, int *config) {
        connect(box, &QComboBox::activated, [&, box, config](int index) {
            *config = index;
            app->updateSettings();
        });
    };
    connect_combo(comboBox_speed_control_method, &app->config->speed_sync_method);
    connect(doubleSpinBox_frame_rate, &QDoubleSpinBox::valueChanged, [&](double value) {
        app->config->fixed_frame_rate = value;
    });

    connect_spin(spinBox_rewind_buffer_size, &app->config->rewind_buffer_size);
    connect_spin(spinBox_rewind_frames, &app->config->rewind_frame_interval);
    connect_spin(spinBox_fast_forward_skip_frames, &app->config->fast_forward_skip_frames);

    connect_checkbox(checkBox_allow_invalid_vram_access, &app->config->allow_invalid_vram_access);
    connect_checkbox(checkBox_allow_opposing_dpad_directions, &app->config->allow_opposing_dpad_directions);
    connect_combo(comboBox_overclock, &app->config->overclock);
    connect_checkbox(checkBox_remove_sprite_limit, &app->config->remove_sprite_limit);
    connect_checkbox(checkBox_use_shadow_echo_buffer, &app->config->enable_shadow_buffer);
    connect_spin(spinBox_superfx_clock_speed, &app->config->superfx_clock_multiplier);
    connect_combo(comboBox_sound_filter, &app->config->sound_filter);
}

void EmulationPanel::showEvent(QShowEvent *event)
{
    auto &config = app->config;
    comboBox_speed_control_method->setCurrentIndex(config->speed_sync_method);
    doubleSpinBox_frame_rate->setValue(config->fixed_frame_rate);
    spinBox_fast_forward_skip_frames->setValue(config->fast_forward_skip_frames);

    spinBox_rewind_buffer_size->setValue(config->rewind_buffer_size);
    spinBox_rewind_frames->setValue(config->rewind_frame_interval);

    checkBox_allow_invalid_vram_access->setChecked(config->allow_invalid_vram_access);
    checkBox_allow_opposing_dpad_directions->setChecked(config->allow_opposing_dpad_directions);
    comboBox_overclock->setCurrentIndex(config->overclock);
    checkBox_remove_sprite_limit->setChecked(config->remove_sprite_limit);
    checkBox_use_shadow_echo_buffer->setChecked(config->enable_shadow_buffer);
    spinBox_superfx_clock_speed->setValue(config->superfx_clock_multiplier);
    comboBox_sound_filter->setCurrentIndex(config->sound_filter);

    QWidget::showEvent(event);
}

