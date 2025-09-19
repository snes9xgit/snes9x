#include "EmuApplication.hpp"
#include "EmuMainWindow.hpp"
#include "SDLInputManager.hpp"
#include "Snes9xController.hpp"
#include "common/audio/s9x_sound_driver_sdl3.hpp"
#include "common/audio/s9x_sound_driver_cubeb.hpp"
#ifdef USE_PULSEAUDIO
#include "common/audio/s9x_sound_driver_pulse.hpp"
#endif

#include <QTimer>
#include <QScreen>
#include <QThread>
#include <QStyleHints>
#include <thread>

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
        sound_driver = std::make_unique<S9xSDL3SoundDriver>();
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
#include <chrono>
using namespace std::chrono_literals;

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

bool EmuApplication::openFile(const std::string &filename)
{
    window->gameChanging();
    updateSettings();
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

    config->additional_controllers.clear();
    if (config->automap_gamepads)
    {
        for (auto &[joystick_id, device] : input_manager->devices)
        {
            if (!device.is_gamepad)
                continue;

            const SDL_GamepadButton list[] = { SDL_GAMEPAD_BUTTON_DPAD_UP,
                                               SDL_GAMEPAD_BUTTON_DPAD_DOWN,
                                               SDL_GAMEPAD_BUTTON_DPAD_LEFT,
                                               SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
                                               // B, A and X, Y are inverted on XInput vs SNES
                                               SDL_GAMEPAD_BUTTON_EAST,
                                               SDL_GAMEPAD_BUTTON_SOUTH,
                                               SDL_GAMEPAD_BUTTON_NORTH,
                                               SDL_GAMEPAD_BUTTON_WEST,
                                               SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
                                               SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
                                               SDL_GAMEPAD_BUTTON_START,
                                               SDL_GAMEPAD_BUTTON_BACK
            };

            int num_bindings;
            auto sdl_bindings = SDL_GetGamepadBindings(device.gamepad, &num_bindings);
            auto get_binding_for_button = [&](SDL_GamepadButton button) -> SDL_GamepadBinding
            {
                for (int i = 0; i < num_bindings; i++)
                    if (sdl_bindings[i]->output_type == SDL_GAMEPAD_BINDTYPE_BUTTON &&
                        sdl_bindings[i]->output.button == button)
                        return *sdl_bindings[i];
                return SDL_GamepadBinding{};
            };

            EmuConfig::controller_t controller{};

            for (int i = 0; i < std::size(list); i++)
            {
                auto sdl_binding = get_binding_for_button(list[i]);

                if (SDL_GAMEPAD_BINDTYPE_BUTTON == sdl_binding.input_type)
                    controller.buttons[i] = EmuBinding::joystick_button(device.index, sdl_binding.input.button);
                else if (SDL_GAMEPAD_BINDTYPE_HAT == sdl_binding.input_type)
                    controller.buttons[i] = EmuBinding::joystick_hat(device.index, sdl_binding.input.hat.hat, sdl_binding.input.hat.hat_mask);
                else if (SDL_GAMEPAD_BINDTYPE_AXIS == sdl_binding.input_type)
                    controller.buttons[i] = EmuBinding::joystick_axis(device.index, sdl_binding.input.axis.axis, sdl_binding.input.axis.axis);

                if (controller.buttons[i].type != EmuBinding::None)
                {
                    bindings.insert({ controller.buttons[i].hash(), { "Snes9x", Core } });
                }
            }

            // Check axes for sticks, using slots 12-15 in controller
            for (int i = 0; i < num_bindings; i++)
            {
                auto b = sdl_bindings[i];

                if (b->output_type == SDL_GAMEPAD_BINDTYPE_AXIS && b->input.axis.axis == SDL_GAMEPAD_AXIS_LEFTY)
                {
                    controller.buttons[12] = EmuBinding::joystick_axis(device.index, b->input.axis.axis, -1);
                    controller.buttons[13] = EmuBinding::joystick_axis(device.index, b->input.axis.axis, 1);
                    for (int i = 12; i <= 13; i++)
                        bindings.insert({ controller.buttons[i].hash(), { "Snes9x", Core } });
                }
                if (b->output_type == SDL_GAMEPAD_BINDTYPE_AXIS && b->input.axis.axis == SDL_GAMEPAD_AXIS_LEFTX)
                {
                    controller.buttons[14] = EmuBinding::joystick_axis(device.index, b->input.axis.axis, -1);
                    controller.buttons[15] = EmuBinding::joystick_axis(device.index, b->input.axis.axis, 1);
                    for (int i = 14; i <= 15; i++)
                        bindings.insert({ controller.buttons[i].hash(), { "Snes9x", Core } });
                }
            }

            printf("Automapping XInput Gamepad: %s\n", SDL_GetGamepadName(device.gamepad));

            config->additional_controllers.push_back(controller);

            SDL_free(sdl_bindings);
        }
    }

    suspendThread();
    core->updateBindings(config.get());
    unsuspendThread();
}

void EmuApplication::handleBinding(const std::string &name, bool pressed)
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

            else if (name == "IncreaseSlot" || name == "DecreaseSlot")
            {
                if (name == "IncreaseSlot")
                    save_slot++;
                else
                    save_slot--;

                if (save_slot > 999)
                    save_slot = 0;
                if (save_slot < 0)
                    save_slot = 999;

                emu_thread->runOnThread([&, slot = this->save_slot] {
                    std::string status = core->slotUsed(slot) ? " [used]" : " [empty]";
                    core->setMessage("Current slot: " + std::to_string(save_slot) + status);
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
            else if (name == "SwapControllers1and2")
            {
                int num_bindings = EmuConfig::num_controller_bindings * EmuConfig::allowed_bindings;
                EmuBinding temp[num_bindings];
                memcpy(temp, config->binding.controller[0].buttons, sizeof(temp));
                memcpy(config->binding.controller[0].buttons, config->binding.controller[1].buttons, sizeof(temp));
                memcpy(config->binding.controller[1].buttons, temp, sizeof(temp));
                updateBindings();
            }
            else if (name == "GrabMouse")
            {
                if (config->port_configuration == EmuConfig::eMousePlusController ||
                    config->port_configuration == EmuConfig::eSuperScopePlusController)
                    window->toggleMouseGrab();
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
    else if (name == "Quit" && pressed)
    {
        window->close();
    }
}

bool EmuApplication::isBound(const EmuBinding &b)
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
    while (true)
    {
        auto event = input_manager->processEvent();
        if (!event)
            return;

        switch (event->type)
        {
        default:
            break;
        case SDL_EVENT_JOYSTICK_ADDED:
        case SDL_EVENT_JOYSTICK_REMOVED:
            if (joypads_changed_callback)
                joypads_changed_callback();
            if (core)
                updateBindings();
            break;
        case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
            auto axis_events = input_manager->discretizeJoyAxisEvent(event.value());

            for (auto &axis_event : axis_events)
            {
                auto binding = EmuBinding::joystick_axis(
                    axis_event.joystick_num,
                    axis_event.axis,
                    axis_event.direction);

                reportBinding(binding, axis_event.pressed);
            }
            break;
        }
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
            reportBinding(EmuBinding::joystick_button(
                              input_manager->devices[event->jbutton.which].index,
                              event->jbutton.button), event->jbutton.down == 1);
            break;
        case SDL_EVENT_JOYSTICK_HAT_MOTION:
            auto hat_events = input_manager->discretizeHatEvent(event.value());
            for (auto &hat_event : hat_events)
            {
                reportBinding(EmuBinding::joystick_hat(hat_event.joystick_num,
                                                       hat_event.hat,
                                                       hat_event.direction),
                              hat_event.pressed);
            }

            break;
        }
    }
}

void EmuApplication::reportPointer(int x, int y)
{
    emu_thread->runOnThread([&, x, y] {
        core->reportPointer(x, y);
    });
}

void EmuApplication::reportMouseButton(int button, bool pressed)
{
    emu_thread->runOnThread([&, button, pressed] {
        core->reportMouseButton(button, pressed);
    });
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

void EmuApplication::loadState(const std::string& filename)
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

void EmuApplication::saveState(const std::string& filename)
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
    emu_thread->runOnThread([&, index] {
        core->enableCheat(index);
    });
}

void EmuApplication::disableCheat(int index)
{
    emu_thread->runOnThread([&, index] {
        core->disableCheat(index);
    });
}

bool EmuApplication::addCheat(const std::string &description,
                              const std::string &code)
{
    suspendThread();
    auto retval = core->addCheat(description, code);
    unsuspendThread();
    return retval;
}

void EmuApplication::deleteCheat(int index)
{
    emu_thread->runOnThread([&, index] {
        core->deleteCheat(index);
    });
}

void EmuApplication::deleteAllCheats()
{
    emu_thread->runOnThread([&] {
        core->deleteAllCheats();
    });
}

int EmuApplication::tryImportCheats(const std::string &filename)
{
    suspendThread();
    auto retval = core->tryImportCheats(filename);
    unsuspendThread();
    return retval;
}

std::string EmuApplication::validateCheat(const std::string &code)
{
    suspendThread();
    auto retval = core->validateCheat(code);
    unsuspendThread();
    return retval;
}

int EmuApplication::modifyCheat(int index, const std::string &name,
                                const std::string &code)
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

std::string EmuApplication::getContentFolder()
{
    return core->getContentFolder();
}

void EmuThread::runOnThread(const std::function<void()> &func, bool blocking)
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
    : QThread(), main_thread(main_thread_)
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

    while (true)
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

    while (true)
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
    auto event_loop = std::make_unique<QEventLoop>();

    setStatusBits(ePaused);

    while (true)
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

void EmuThread::setMainLoop(const std::function<void()> &loop)
{
    main_loop = loop;
}
