#include "SoundPanel.hpp"
#include <QScreen>

static const int playback_rates[] = { 96000, 48000, 44100 };

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
        app->config->adjust_input_rate_automatically = checked;
        if (checked)
        {
            int calculated = screen()->refreshRate() / 60.09881 * 32040;
            horizontalSlider_input_rate->setValue(calculated);
        }

        horizontalSlider_input_rate->setDisabled(checked);
        app->updateSettings();
    });

    connect(horizontalSlider_input_rate, &QSlider::valueChanged, [&](int value) {
        app->config->input_rate = value;
        setInputRateText(value);
        app->updateSettings();
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
}

SoundPanel::~SoundPanel()
{
}

void SoundPanel::setInputRateText(int value)
{
    double hz = value / 32040.0 * 60.09881;
    label_input_rate->setText(QString("%1\n%2 Hz").arg(value).arg(hz, 6, 'g', 6));
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
    setInputRateText(config->input_rate);
    checkBox_dynamic_rate_control->setChecked(config->dynamic_rate_control);
    doubleSpinBox_dynamic_rate_limit->setValue(config->dynamic_rate_limit);

    checkBox_mute_sound->setChecked(config->mute_audio);
    checkBox_mute_during_alt_speed->setChecked(config->mute_audio_during_alternate_speed);
}

