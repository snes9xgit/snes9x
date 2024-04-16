#include "EmuApplication.hpp"
#include "EmuMainWindow.hpp"
#include "SDLInputManager.hpp"
#include "Snes9xController.hpp"
#include "common/audio/s9x_sound_driver_sdl.hpp"
#include "common/audio/s9x_sound_driver_cubeb.hpp"
#ifdef USE_PULSEAUDIO
#include "common/audio/s9x_sound_driver_pulse.hpp"
#endif

#include <QTimer>
#include <QScreen>
#include <QThread>
#include <QStyleHints>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

#undef SOUND_BUFFER_WINDOW

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
    suspendThread();
    sound_driver.reset();
    core->sound_output_function = nullptr;

#ifdef USE_PULSEAUDIO
    if (config->sound_driver == "pulseaudio")
        sound_driver = std::make_unique<S9xPulseSoundDriver>();
#endif
    if (config->sound_driver == "cubeb")
        sound_driver = std::make_unique<S9xCubebSoundDriver>();

    if (!sound_driver)
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

    unsuspendThread();
}

#ifdef SOUND_BUFFER_WINDOW
#include <QProgressDialog>
static void trackBufferLevel(int percent, QWidget *parent)
{
    static uint64_t total = 0;
    static uint64_t ticks = 0;
    static std::chrono::steady_clock::time_point then;

    static QProgressDialog *dialog = nullptr;

    if (!dialog)
    {
        dialog = new QProgressDialog(parent);
        dialog->setRange(0, 100);
    }

    ticks++;
    total += percent;

    dialog->setValue(percent);
    auto now = std::chrono::steady_clock::now();
    if (ticks > 0 && now - then >= std::chrono::seconds(1))
    {
        dialog->setLabelText(QString("%1").arg(total / ticks));
        then = now;
        total = 0;
        ticks = 0;
    }

    dialog->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    if (!dialog->isVisible())
    {
        dialog->setDisabled(true);
        dialog->show();
        dialog->setModal(false);
    }
}
#endif

void EmuApplication::writeSamples(int16_t *data, int samples)
{
    if (config->speed_sync_method == EmuConfig::eSoundSync && !core->isAbnormalSpeed())
    {
        int iterations = 0;
        while (sound_driver->space_free() < samples && iterations < 500)
        {
            iterations++;
            QThread::usleep(50);
        }
    }
    auto buffer_level = sound_driver->buffer_level();
    core->updateSoundBufferLevel(buffer_level.first, buffer_level.second);

    if (!sound_driver->write_samples(data, samples))
    {
        core->clearSoundBuffer();
    }

#ifdef SOUND_BUFFER_WINDOW
    int percent = (buffer_level.second - buffer_level.first) * 100 / buffer_level.second;
    trackBufferLevel(percent, window.get());
#endif
}

void EmuApplication::startGame()
{
    suspendThread();
    if (!sound_driver)
        restartAudio();

    core->screen_output_function = [&](uint16_t *data, int width, int height, int stride_bytes, double frame_rate) {
        if (window->canvas)
        {
            window->output((uint8_t *)data, width, height, QImage::Format_RGB16, stride_bytes, frame_rate);
            // QMetaObject::invokeMethod(window.get(), "output", Qt::ConnectionType::DirectConnection,
            //     Q_ARG(uint8_t *, (uint8_t *)data),
            //     Q_ARG(int, width),
            //     Q_ARG(int, height),
            //     Q_ARG(QImage::Format, QImage::Format_RGB16),
            //     Q_ARG(int, stride_bytes),
            //     Q_ARG(double, frame_rate));
        }
    };

    updateSettings();
    updateBindings();

    emu_thread->setMainLoop([&] {
        mainLoop();
    });

    unsuspendThread();
    unpause();
}

bool EmuApplication::isPaused()
{
    return (pause_count != 0);
}

void EmuApplication::suspendThread()
{
    suspend_count++;

    if (!emu_thread)
        return;

    if (suspend_count > 0)
    {
        emu_thread->runOnThread([&] { emu_thread->setStatusBits(EmuThread::eSuspended); }, true);
    }
}

void EmuApplication::unsuspendThread()
{
    suspend_count--;
    assert(suspend_count >= 0);

    if (!emu_thread)
        return;

    if (suspend_count == 0)
    {
        emu_thread->runOnThread([&] { emu_thread->unsetStatusBits(EmuThread::eSuspended); }, true);
    }
}

void EmuApplication::pause()
{
    pause_count++;
    if (pause_count > 0)
    {
        if (emu_thread)
            emu_thread->pause();
        core->setPaused(true);
        if (sound_driver)
            sound_driver->stop();
    }
}

void EmuApplication::stopThread()
{
    if (!emu_thread)
        return;

    emu_thread->setStatusBits(EmuThread::eQuit);
    while (!emu_thread->isFinished())
    {
        std::this_thread::yield();
    }

    emu_thread = nullptr;
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
    if (emu_thread)
        emu_thread->unpause();
}

void EmuApplication::startThread()
{
    if (!emu_thread)
    {
        emu_thread = std::make_unique<EmuThread>(QThread::currentThread());
        emu_thread->start();
        emu_thread->waitForStatusBit(EmuThread::ePaused);
        emu_thread->moveToThread(emu_thread.get());
    }
}

bool EmuApplication::openFile(std::string filename)
{
    window->gameChanging();
    suspendThread();
    auto result = core->openFile(filename);
    unsuspendThread();

    return result;
}

void EmuApplication::mainLoop()
{
    if (!core->active)
    {
        std::this_thread::yield();
        return;
    }

    core->mainLoop();
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

    emu_thread->runOnThread([&, b, active] { core->reportBinding(b, active); });
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

    suspendThread();
    core->updateBindings(config.get());
    unsuspendThread();
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
                emu_thread->runOnThread([&] {
                    core->setMessage("Current slot: " + std::to_string(save_slot));
                });
            }
            else if (name == "DecreaseSlot")
            {
                save_slot--;
                if (save_slot < 0)
                    save_slot = 999;
                emu_thread->runOnThread([&] {
                    core->setMessage("Current slot: " + std::to_string(save_slot));
                });
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
    if (config->adjust_input_rate_automatically && !config->vrr_enabled)
    {
        constexpr double ir_ratio = 60.098813 / 32040.0;

        auto refresh = window->screen()->refreshRate();
        config->input_rate = refresh / ir_ratio;
        if (refresh > 119 && refresh < 121)
            config->input_rate /= 2;
        else if (refresh > 179 && refresh < 181)
            config->input_rate /= 3;
        else if (refresh > 239 && refresh < 241)
            config->input_rate /= 4;
        else
        {
            // We're not even close to a multiple of 60hz, so we're stuttering
            // anyway. Use the true hardware speed.
            config->input_rate = 32040.0;
        }
    }

    emu_thread->runOnThread([&] {
        core->updateSettings(config.get());
    });
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

void EmuApplication::startInputTimer()
{
    poll_input_timer = std::make_unique<QTimer>();
    poll_input_timer->setTimerType(Qt::TimerType::PreciseTimer);
    poll_input_timer->setInterval(4);
    poll_input_timer->setSingleShot(false);
    poll_input_timer->callOnTimeout([&] { pollJoysticks(); });
    poll_input_timer->start();
}

void EmuApplication::loadState(int slot)
{
    emu_thread->runOnThread([&, slot] {
        core->loadState(slot);
    });
}

void EmuApplication::loadState(std::string filename)
{
    emu_thread->runOnThread([&, filename] {
        core->loadState(filename);
    });
}

void EmuApplication::saveState(int slot)
{
    emu_thread->runOnThread([&, slot] {
        core->saveState(slot);
    });
}

void EmuApplication::saveState(std::string filename)
{
    emu_thread->runOnThread([&, filename] {
        core->saveState(filename);
    });
}

void EmuApplication::reset()
{
    emu_thread->runOnThread([&] {
        core->softReset();
    });
}

void EmuApplication::powerCycle()
{
    emu_thread->runOnThread([&] {
        core->reset();
    });
}

void EmuApplication::loadUndoState()
{
    emu_thread->runOnThread([&] {
        core->loadUndoState();
    });
}

std::string EmuApplication::getStateFolder()
{
    return core->getStateFolder();
}

std::vector<std::tuple<bool, std::string, std::string>> EmuApplication::getCheatList()
{
    suspendThread();
    auto cheat_list = core->getCheatList();
    unsuspendThread();

    return std::move(cheat_list);
}

void EmuApplication::disableAllCheats()
{
    emu_thread->runOnThread([&] {
        core->disableAllCheats();
    });
}

void EmuApplication::enableCheat(int index)
{
    emu_thread->runOnThread([&] {
        core->enableCheat(index);
    });
}

void EmuApplication::disableCheat(int index)
{
    emu_thread->runOnThread([&] {
        core->disableCheat(index);
    });
}

bool EmuApplication::addCheat(std::string description, std::string code)
{
    suspendThread();
    auto retval = core->addCheat(description, code);
    unsuspendThread();
    return retval;
}

void EmuApplication::deleteCheat(int index)
{
    emu_thread->runOnThread([&] {
        core->deleteCheat(index);
    });
}

void EmuApplication::deleteAllCheats()
{
    emu_thread->runOnThread([&] {
        core->deleteAllCheats();
    });
}

int EmuApplication::tryImportCheats(std::string filename)
{
    suspendThread();
    auto retval = core->tryImportCheats(filename);
    unsuspendThread();
    return retval;
}

std::string EmuApplication::validateCheat(std::string code)
{
    suspendThread();
    auto retval = core->validateCheat(code);
    unsuspendThread();
    return retval;
}

int EmuApplication::modifyCheat(int index, std::string name, std::string code)
{
    suspendThread();
    auto retval = core->modifyCheat(index, name, code);
    unsuspendThread();
    return retval;
}

bool EmuApplication::isCoreActive()
{
    return core->active;
}

QString EmuApplication::iconPrefix()
{
    const char *whiteicons = ":/icons/whiteicons/";
    const char *blackicons = ":/icons/blackicons/";

    if (QGuiApplication::palette().color(QPalette::WindowText).lightness() >
        QGuiApplication::palette().color(QPalette::Window).lightness())
        return whiteicons;

    return blackicons;
}

void EmuThread::runOnThread(std::function<void()> func, bool blocking)
{
    if (QThread::currentThread() != this)
    {
        QMetaObject::invokeMethod(this,
                                  "runOnThread",
                                  blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                                  Q_ARG(std::function<void()>, func),
                                  Q_ARG(bool, blocking));
        return;
    }

    func();
}

EmuThread::EmuThread(QThread *main_thread_)
    : main_thread(main_thread_), QThread()
{
    qRegisterMetaType<std::function<void()>>("std::function<void()>");
}

void EmuThread::setStatusBits(int new_status)
{
    status |= new_status;
}

void EmuThread::unsetStatusBits(int new_status)
{
    status &= ~new_status;
}

void EmuThread::waitForStatusBit(int new_status)
{
    if (status & new_status)
        return;

    while (1)
    {
        QThread::yieldCurrentThread();
        if (status & new_status)
            break;
    }
}

void EmuThread::waitForStatusBitCleared(int new_status)
{
    if (!(status & new_status))
        return;

    while (1)
    {
        QThread::yieldCurrentThread();
        if (!(status & new_status))
            break;
    }
}

void EmuThread::pause()
{
    runOnThread([&] { setStatusBits(ePaused); }, true);
}

void EmuThread::unpause()
{
    runOnThread([&] { unsetStatusBits(ePaused); }, true);
}

void EmuThread::run()
{
    auto event_loop = new QEventLoop();

    setStatusBits(ePaused);

    while (1)
    {
        event_loop->processEvents();

        if (status & eQuit)
            break;

        if (status & (ePaused | eSuspended))
        {
            QThread::usleep(2000);
            continue;
        }

        if (main_loop)
            main_loop();
    }
}

void EmuThread::setMainLoop(std::function<void ()> loop)
{
    main_loop = loop;
}