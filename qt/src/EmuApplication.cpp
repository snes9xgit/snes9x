#include "EmuApplication.hpp"
#include "common/audio/s9x_sound_driver_sdl.hpp"
#include "common/audio/s9x_sound_driver_portaudio.hpp"
#include "common/audio/s9x_sound_driver_cubeb.hpp"
#include <QTimer>
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

EmuApplication::EmuApplication()
{
    core = Snes9xController::get();
}

EmuApplication::~EmuApplication()
{
    core->deinit();
}

void EmuApplication::restartAudio()
{
    sound_driver.reset();
    core->sound_output_function = nullptr;

    if (config->sound_driver == "portaudio")
        sound_driver = std::make_unique<S9xPortAudioSoundDriver>();
    else if (config->sound_driver == "cubeb")
        sound_driver = std::make_unique<S9xCubebSoundDriver>();
    else
    {
        config->sound_driver = "sdl";
        sound_driver = std::make_unique<S9xSDLSoundDriver>();
    }

    sound_driver->init();
    if (sound_driver->open_device(config->playback_rate, config->audio_buffer_size_ms))
        sound_driver->start();
    else
    {
        printf("Couldn't initialize sound driver: %s\n", config->sound_driver.c_str());
        sound_driver.reset();
    }

    if (sound_driver)
        core->sound_output_function = [&](int16_t *data, int samples) {
            writeSamples(data, samples);
        };
}

void EmuApplication::writeSamples(int16_t *data, int samples)
{
    if (config->speed_sync_method == EmuConfig::eSoundSync && !core->isAbnormalSpeed())
    {
        int iterations = 0;
        while (sound_driver->space_free() < samples && iterations < 100)
        {
            iterations++;
            std::this_thread::sleep_for(50us);
        }
    }

    sound_driver->write_samples(data, samples);
    auto buffer_level = sound_driver->buffer_level();
    core->updateSoundBufferLevel(buffer_level.first, buffer_level.second);
}

void EmuApplication::startGame()
{
    if (!sound_driver)
        restartAudio();

    core->screen_output_function = [&](uint16_t *data, int width, int height, int stride_bytes, double frame_rate) {
        if (window->canvas)
            window->canvas->output((uint8_t *)data, width, height, QImage::Format_RGB16, stride_bytes, frame_rate);
    };

    core->updateSettings(config.get());
    updateBindings();

    startIdleLoop();
}

bool EmuApplication::isPaused()
{
    return (pause_count != 0);
}

void EmuApplication::pause()
{
    pause_count++;
    if (pause_count > 0)
    {
        core->setPaused(true);
        if (sound_driver)
            sound_driver->stop();
    }
}

void EmuApplication::stopIdleLoop()
{
    idle_loop->stop();
    pause_count = 0;
}

void EmuApplication::unpause()
{
    pause_count--;
    if (pause_count < 0)
        pause_count = 0;
    if (pause_count > 0)
        return;

    core->setPaused(false);
    if (core->active && sound_driver)
        sound_driver->start();
}

void EmuApplication::startIdleLoop()
{
    if (!idle_loop)
    {
        idle_loop = std::make_unique<QTimer>();
        idle_loop->setTimerType(Qt::TimerType::PreciseTimer);
        idle_loop->setInterval(0);
        idle_loop->setSingleShot(false);
        idle_loop->callOnTimeout([&]{ idleLoop(); });
        pause_count = 0;
    }

    idle_loop->start();
}

void EmuApplication::idleLoop()
{
    if (core->active && pause_count == 0)
    {            
        idle_loop->setInterval(0);
        pollJoysticks();
        bool muted = config->mute_audio || (config->mute_audio_during_alternate_speed && core->isAbnormalSpeed());
        core->mute(muted);
        core->mainLoop();
    }
    else
    {
        pollJoysticks();
        idle_loop->setInterval(32);
    }
}

bool EmuApplication::openFile(std::string filename)
{
    auto result = core->openFile(filename);

    return result;
}

void EmuApplication::reportBinding(EmuBinding b, bool active)
{
    if (binding_callback && active)
    {
        binding_callback(b);
        return;
    }

    auto it = bindings.find(b.hash());
    if (it == bindings.end())
        return;

    if (it->second.second == UI)
    {
        handleBinding(it->second.first, active);
        return;
    }

    core->reportBinding(b, active);
}

void EmuApplication::updateBindings()
{
    bindings.clear();
    for (auto i = 0; i < EmuConfig::num_shortcuts; i++)
    {
        auto name = EmuConfig::getShortcutNames()[i];

        for (auto b = 0; b < EmuConfig::allowed_bindings; b++)
        {
            auto &binding = config->binding.shortcuts[i * EmuConfig::allowed_bindings + b];

            if (binding.type != EmuBinding::None)
            {
                auto handler = core->acceptsCommand(name) ? Core : UI;
                bindings.insert({ binding.hash(), { name, handler } });
            }
        }
    }

    for (int i = 0; i < EmuConfig::num_controller_bindings; i++)
    {
        for (int c = 0; c < 5; c++)
        {
            for (int b = 0; b < EmuConfig::allowed_bindings; b++)
            {
                auto binding = config->binding.controller[c].buttons[i * EmuConfig::allowed_bindings + b];
                if (binding.hash() != 0)
                    bindings.insert({ binding.hash(), { "Snes9x", Core } });
            }
        }
    }

    core->updateBindings(config.get());
}

void EmuApplication::handleBinding(std::string name, bool pressed)
{
    if (core->active)
    {
        if (name == "Rewind")
        {
            core->rewinding = pressed;
        }
        else if (pressed) // Only activate with core active and on button down
        {
            if (name == "PauseContinue")
            {
                window->pauseContinue();
            }
            
            else if (name == "IncreaseSlot")
            {
                save_slot++;
                if (save_slot > 999)
                    save_slot = 0;
                core->setMessage("Current slot: " + std::to_string(save_slot));
            }
            else if (name == "DecreaseSlot")
            {
                save_slot--;
                if (save_slot < 0)
                    save_slot = 999;
                core->setMessage("Current slot: " + std::to_string(save_slot));
            }
            else if (name == "SaveState")
            {
                saveState(save_slot);
            }
            else if (name == "LoadState")
            {
                loadState(save_slot);
            }
        }
    }

    if (name == "ToggleFullscreen" && !pressed)
    {
        window->toggleFullscreen();
    }
    else if (name == "OpenROM" && pressed)
    {
        window->openFile();
    }
}

bool EmuApplication::isBound(EmuBinding b)
{
    if (bindings.find(b.hash()) != bindings.end())
        return true;
    return false;
}

void EmuApplication::updateSettings()
{
    core->updateSettings(config.get());
}

void EmuApplication::pollJoysticks()
{
    while (1)
    {
        auto event = input_manager->ProcessEvent();
        if (!event)
            return;

        switch (event->type)
        {
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
            if (joypads_changed_callback)
                joypads_changed_callback();
            break;
        case SDL_JOYAXISMOTION: {
            auto axis_event = input_manager->DiscretizeJoyAxisEvent(event.value());
            if (axis_event)
            {
                auto binding = EmuBinding::joystick_axis(
                    axis_event->joystick_num,
                    axis_event->axis,
                    axis_event->direction);

                reportBinding(binding, axis_event->pressed);
            }
            break;
        }
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            reportBinding(EmuBinding::joystick_button(
                              input_manager->devices[event->jbutton.which].index,
                              event->jbutton.button), event->jbutton.state == 1);
            break;
        case SDL_JOYHATMOTION:
            auto hat_event = input_manager->DiscretizeHatEvent(event.value());
            if (hat_event)
            {
                reportBinding(EmuBinding::joystick_hat(hat_event->joystick_num,
                                                       hat_event->hat,
                                                       hat_event->direction),
                              hat_event->pressed);
            }

            break;
        }
    }
}

void EmuApplication::loadState(int slot)
{
    core->loadState(slot);
}

void EmuApplication::loadState(std::string filename)
{
    core->loadState(filename);
}

void EmuApplication::saveState(int slot)
{
    core->saveState(slot);
}

void EmuApplication::saveState(std::string filename)
{
    core->saveState(filename);
}

void EmuApplication::reset()
{
    core->softReset();
}

void EmuApplication::powerCycle()
{
    core->reset();
}
