#include "SoundPanel.hpp"
#include "EmuApplication.hpp"
#include "EmuConfig.hpp"
#include "snes9x.h"

static constexpr int playback_rates[] = { 96000, 48000, 44100 };

SoundPanel::SoundPanel(EmuApplication *app_)
    : app(app_)
{
    setupUi(this);

    connect(comboBox_driver, &QComboBox::activated, [&](int index){
        if (app->config->sound_driver != driver_list[index])
        {
            app->config->sound_driver = driver_list[index];
            app->restartAudio();
        }
    });

    connect(comboBox_playback_rate, &QComboBox::activated, [&](int index)
    {
        if (index < 3)
        {
            if (playback_rates[index] != app->config->playback_rate)
            {
                app->config->playback_rate = playback_rates[index];
                app->restartAudio();
                app->updateSettings();
            }
        }
    });

    connect(spinBox_buffer_size, &QSpinBox::valueChanged, [&](int value) {
        if (app->config->audio_buffer_size_ms != value)
        {
            app->config->audio_buffer_size_ms = value;
            app->restartAudio();
        }
    });

    connect(checkBox_adjust_input_rate, &QCheckBox::clicked, [&](bool checked) {
        if (!checked && checked != app->config->adjust_input_rate_automatically)
            app->config->input_rate = 32040;
        app->config->adjust_input_rate_automatically = checked;
        app->updateSettings();
        updateInputRate();

        if (checked)
        {
           horizontalSlider_input_rate->setValue(app->config->input_rate);
        }
    });

    connect(horizontalSlider_input_rate, &QSlider::valueChanged, [&](int value) {
        app->config->input_rate = value;
        app->updateSettings();
        updateInputRate();
    });

    connect(checkBox_dynamic_rate_control, &QCheckBox::clicked, [&](bool checked) {
        app->config->dynamic_rate_control = checked;
        app->updateSettings();
    });

    connect(doubleSpinBox_dynamic_rate_limit, &QDoubleSpinBox::valueChanged, [&](double value) {
        app->config->dynamic_rate_limit = value;
        app->updateSettings();
    });

    connect(checkBox_mute_sound, &QCheckBox::toggled, [&](bool checked) {
        app->config->mute_audio = checked;
    });

    connect(checkBox_mute_during_alt_speed, &QCheckBox::toggled, [&](bool checked) {
        app->config->mute_audio_during_alternate_speed = checked;
    });

    // --- Volume + SGB Mix sliders --------------------------------------------
    //
    // Two-way binding between each QSlider and its QSpinBox is handled by a
    // helper lambda. The Regular slider also drags the active per-source
    // sliders by the same delta (clamped 0..100): both SPC and GB in SGB BIOS
    // mode, only GB in BIOS-less .gb/.gbc (since SPC isn't engaged), nothing
    // when no GB ROM is loaded.

    auto pairSync = [&](QSlider *slider, QSpinBox *spin) {
        connect(slider, &QSlider::valueChanged, [&, spin](int value) {
            if (suppress_volume_sync) return;
            suppress_volume_sync = true;
            spin->setValue(value);
            suppress_volume_sync = false;
        });
        connect(spin, &QSpinBox::valueChanged, [&, slider](int value) {
            if (suppress_volume_sync) return;
            suppress_volume_sync = true;
            slider->setValue(value);
            suppress_volume_sync = false;
        });
    };
    pairSync(slider_volume_regular,      spinBox_volume_regular);
    pairSync(slider_volume_fast_forward, spinBox_volume_fast_forward);
    pairSync(slider_volume_spc,          spinBox_volume_spc);
    pairSync(slider_volume_gb,           spinBox_volume_gb);

    auto dragLinked = [&](int new_regular) {
        int delta = new_regular - prev_regular_volume;
        if (delta == 0)
            return;

        auto shiftSlider = [&](QSlider *slider) {
            int cur = slider->value();
            int next = cur + delta;
            if (next < 0)   next = 0;
            if (next > 100) next = 100;
            slider->setValue(next);
        };

        if (Settings.SGB_BIOSModeActive)
        {
            shiftSlider(slider_volume_spc);
            shiftSlider(slider_volume_gb);
        }
        else if (Settings.SuperGameBoy)
        {
            shiftSlider(slider_volume_gb);
        }
        prev_regular_volume = new_regular;
    };

    connect(slider_volume_regular, &QSlider::valueChanged, [&, dragLinked](int value) {
        app->config->master_volume_regular = value;
        dragLinked(value);
    });

    connect(slider_volume_fast_forward, &QSlider::valueChanged, [&](int value) {
        app->config->master_volume_fast_forward = value;
    });

    connect(slider_volume_spc, &QSlider::valueChanged, [&](int value) {
        // Only persist in BIOS mode — BIOS-less mode would otherwise clobber
        // the user's previous BIOS-mode preference with a synced value.
        if (Settings.SGB_BIOSModeActive)
            app->config->sgb_mix_volume_spc = value;
        app->updateSettings();
    });

    connect(slider_volume_gb, &QSlider::valueChanged, [&](int value) {
        if (Settings.SGB_BIOSModeActive)
            app->config->sgb_mix_volume_gb = value;
        app->updateSettings();
    });
}

void SoundPanel::updateInputRate()
{
    constexpr double ir_ratio = 60.098813 / 32040.0;

    app->updateSettings();
    if (app->config->adjust_input_rate_automatically)
        horizontalSlider_input_rate->setEnabled(false);
    else
        horizontalSlider_input_rate->setEnabled(true);

    double hz = app->config->input_rate * ir_ratio;

    label_input_rate->setText(QString("%1%3\n%2 Hz")
        .arg(app->config->input_rate)
        .arg(hz, 6, 'g', 6)
        .arg(app->config->input_rate == 32040.0 ? " (Default)" : ""));
}

void SoundPanel::updateSGBVolumeEnableState()
{
    // SGB-mix gains only fire inside S9xMixSpcOverGB, which short-circuits
    // outside SGB BIOS mode. So enable SPC+GB only when BIOS mode is active.
    // In BIOS-less GB mode the GB slider mirrors Regular (disabled, since the
    // master is what actually scales BIOS-less GB output).
    bool bios_mode = Settings.SGB_BIOSModeActive;
    bool bios_less_gb = Settings.SuperGameBoy && !Settings.SGB_BIOSModeActive;

    slider_volume_spc->setEnabled(bios_mode);
    spinBox_volume_spc->setEnabled(bios_mode);
    label_volume_spc->setEnabled(bios_mode);

    slider_volume_gb->setEnabled(bios_mode);
    spinBox_volume_gb->setEnabled(bios_mode);
    label_volume_gb->setEnabled(bios_mode);

    if (bios_less_gb)
    {
        suppress_volume_sync = true;
        slider_volume_gb->setValue(app->config->master_volume_regular);
        spinBox_volume_gb->setValue(app->config->master_volume_regular);
        suppress_volume_sync = false;
    }
}

void SoundPanel::showEvent(QShowEvent *event)
{
    auto &config = app->config;

    comboBox_driver->clear();
    driver_list.clear();

    comboBox_driver->addItem("Cubeb");
    driver_list.push_back("cubeb");
    comboBox_driver->addItem("SDL");
    driver_list.push_back("sdl");
#ifdef USE_PULSEAUDIO
    comboBox_driver->addItem("PulseAudio");
    driver_list.push_back("pulseaudio");
#endif

    for (int i = 0; i < driver_list.size(); i++)
    {
        if (driver_list[i] == config->sound_driver)
        {
            comboBox_driver->setCurrentIndex(i);
            break;
        }
    }

    comboBox_device->clear();
    comboBox_device->addItem("Default");

    comboBox_playback_rate->clear();
    comboBox_playback_rate->addItem("96000Hz");
    comboBox_playback_rate->addItem("48000Hz");
    comboBox_playback_rate->addItem("44100Hz");
    int pbr_index = 1;
    if (config->playback_rate == 96000)
        pbr_index = 0;
    else if (config->playback_rate == 44100)
        pbr_index = 2;

    comboBox_playback_rate->setCurrentIndex(pbr_index);
    spinBox_buffer_size->setValue(config->audio_buffer_size_ms);

    checkBox_adjust_input_rate->setChecked(config->adjust_input_rate_automatically);
    updateInputRate();
    horizontalSlider_input_rate->setValue(config->input_rate);
    checkBox_dynamic_rate_control->setChecked(config->dynamic_rate_control);
    doubleSpinBox_dynamic_rate_limit->setValue(config->dynamic_rate_limit);

    checkBox_mute_sound->setChecked(config->mute_audio);
    checkBox_mute_during_alt_speed->setChecked(config->mute_audio_during_alternate_speed);

    suppress_volume_sync = true;
    slider_volume_regular->setValue(config->master_volume_regular);
    spinBox_volume_regular->setValue(config->master_volume_regular);
    slider_volume_fast_forward->setValue(config->master_volume_fast_forward);
    spinBox_volume_fast_forward->setValue(config->master_volume_fast_forward);
    slider_volume_spc->setValue(config->sgb_mix_volume_spc);
    spinBox_volume_spc->setValue(config->sgb_mix_volume_spc);
    slider_volume_gb->setValue(config->sgb_mix_volume_gb);
    spinBox_volume_gb->setValue(config->sgb_mix_volume_gb);
    suppress_volume_sync = false;
    prev_regular_volume = config->master_volume_regular;

    updateSGBVolumeEnableState();
}
