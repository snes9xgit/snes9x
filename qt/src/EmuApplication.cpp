#include "EmuApplication.hpp"
#include "EmuMainWindow.hpp"
#include "SDLInputManager.hpp"
#include "Snes9xController.hpp"
#include "common/audio/s9x_sound_driver_sdl3.hpp"
#include "common/audio/s9x_sound_driver_cubeb.hpp"
#include "apu/apu.h"
#ifdef USE_PULSEAUDIO
#include "common/audio/s9x_sound_driver_pulse.hpp"
#endif

#include <QTimer>
#include <QScreen>
#include <QThread>
#include <QStyleHints>
#include <chrono>
#include <thread>

#include "snes9x.h"
#include "controls.h"
#ifdef RETROACHIEVEMENTS_SUPPORT
#include "RAIntegrationQt.hpp"
#include "retroachievements.h"
#include "display.h"
#endif
#ifdef KAILLERA_SUPPORT
#include "kaillera_client.h"
#include "kaillera_server.h"
#endif

#undef SOUND_BUFFER_WINDOW

EmuApplication::EmuApplication()
{
    core = Snes9xController::get();
}

EmuApplication::~EmuApplication()
{
#ifdef RETROACHIEVEMENTS_SUPPORT
    RA_Shutdown();
#endif
#ifdef KAILLERA_SUPPORT
    KailleraClientDisconnect();
    KailleraServerStop();
#endif
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

    // Master volume (post-mix) — Regular for normal play, FastForward when
    // turbo/rewind is active. No sound driver here exposes a host volume API,
    // so scale samples in place. The Regular pre-amp gain rides on top, so
    // the result can exceed unity and needs to clamp at int16.
    {
        int vol_pct = core->isAbnormalSpeed()
            ? config->master_volume_fast_forward
            : config->master_volume_regular;
        if (vol_pct < 0)   vol_pct = 0;
        if (vol_pct > 100) vol_pct = 100;
        int gain_db = config->gain_regular;
        if (gain_db < 0) gain_db = 0;
        const int vol_q8 = (vol_pct * S9xGainQ8(gain_db)) / 100;
        if (vol_q8 != 256)
        {
            for (int i = 0; i < samples; ++i)
            {
                int32_t v = ((int32_t)data[i] * vol_q8) >> 8;
                if (v >  32767) v =  32767;
                if (v < -32768) v = -32768;
                data[i] = (int16_t)v;
            }
        }
    }

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
#ifdef RETROACHIEVEMENTS_SUPPORT
    RA_OnCloseROM();
#endif
    auto result = core->openFile(filename);
    unsuspendThread();
#ifdef RETROACHIEVEMENTS_SUPPORT
    if (result)
        RA_OnLoadROM();
#endif
    return result;
}

void EmuApplication::mainLoop()
{
    if (!core->active)
    {
        std::this_thread::yield();
        return;
    }

    if (window->canvas)
        window->canvas->signalInputStage();

    QGuiApplication::processEvents();

    core->mainLoop();
#ifdef RETROACHIEVEMENTS_SUPPORT
    // Suspend achievement processing during netplay so remote players' inputs
    // can't earn unlocks on this account.
    bool ra_netplay = Settings.NetPlay || Settings.NetPlayServer;
#ifdef KAILLERA_SUPPORT
    ra_netplay = ra_netplay || KailleraClientIsPlaying();
#endif
    RA_SetNetplayActive(ra_netplay);
    RA_DoFrame();
#endif
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
                /* The core's QuickSave/QuickLoad commands address absolute
                 * slots; handle them here so they apply to the current bank. */
                bool bank_relative = strncmp(name, "QuickSave", 9) == 0 ||
                                     strncmp(name, "QuickLoad", 9) == 0;
                auto handler = (!bank_relative && core->acceptsCommand(name)) ? Core : UI;
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

            const SDL_GamepadButton list[] = {
                SDL_GAMEPAD_BUTTON_DPAD_UP,
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

            auto sdl_bindings = SDLInputManager::getXInputButtonBindings(device.gamepad);

            EmuConfig::controller_t controller{};

            for (int i = 0; i < std::size(list); i++)
            {
                if (!sdl_bindings.contains({ SDL_GAMEPAD_BINDTYPE_BUTTON, list[i] }))
                    continue;

                auto &sdl_binding = sdl_bindings[{ SDL_GAMEPAD_BINDTYPE_BUTTON, list[i] }];
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
            auto do_axis = [&](int sdl_axis, int negative_slot, int positive_slot)
            {
                std::pair<int, int> axis{ SDL_GAMEPAD_BINDTYPE_AXIS, sdl_axis };
                if (sdl_bindings.contains(axis))
                {
                    auto &b = sdl_bindings[axis];
                    controller.buttons[negative_slot] = EmuBinding::joystick_axis(device.index, b.input.axis.axis, -1);
                    controller.buttons[positive_slot] = EmuBinding::joystick_axis(device.index, b.input.axis.axis, 1);
                    for (int i = negative_slot; i <= positive_slot; i++)
                        bindings.insert({ controller.buttons[i].hash(), { "Snes9x", Core } });
                }
            };
            do_axis(SDL_GAMEPAD_AXIS_LEFTY, 12, 13);
            do_axis(SDL_GAMEPAD_AXIS_LEFTX, 14, 15);

            printf("Automapping XInput Gamepad: %s\n", SDL_GetGamepadName(device.gamepad));

            config->additional_controllers.push_back(controller);
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
#ifdef RETROACHIEVEMENTS_SUPPORT
            if (RA_IsHardcoreModeActive())
            {
                core->rewinding = false;
                if (pressed)
                    S9xSetInfoString("Rewind is not allowed in Hardcore mode");
            }
            else
#endif
            core->rewinding = pressed;
        }
        else if (pressed) // Only activate with core active and on button down
        {
            if (name == "PauseContinue")
            {
                window->pauseContinue();
            }

            else if (name == "IncreaseSlot" || name == "DecreaseSlot" ||
                     name == "IncreaseBank" || name == "DecreaseBank")
            {
                auto &slot = config->current_save_slot;
                auto &bank = config->current_save_bank;

                /* Slots wrap inside the current bank; banks wrap on their own. */
                if (name == "IncreaseSlot")
                    slot++;
                else if (name == "DecreaseSlot")
                    slot--;
                else if (name == "IncreaseBank")
                    bank++;
                else
                    bank--;

                if (slot >= EmuConfig::save_slots_per_bank)
                    slot = 0;
                if (slot < 0)
                    slot = EmuConfig::save_slots_per_bank - 1;
                if (bank >= EmuConfig::num_save_banks)
                    bank = 0;
                if (bank < 0)
                    bank = EmuConfig::num_save_banks - 1;

                emu_thread->runOnThread([&, slot, bank, index = currentSaveSlot()] {
                    std::string status = core->slotUsed(index) ? " [used]" : " [empty]";
                    core->setMessage("Current slot: " + std::to_string(slot) +
                                     ", bank: " + std::to_string(bank) + status);
                });
            }
            else if (name == "SaveState")
            {
                saveState(currentSaveSlot());
            }
            else if (name == "LoadState")
            {
                loadState(currentSaveSlot());
            }
            else if (name.compare(0, 9, "QuickSave") == 0)
            {
                /* The numbered hotkeys address slots inside the current bank. */
                saveState(config->current_save_bank * EmuConfig::save_slots_per_bank +
                          std::stoi(name.substr(9)));
            }
            else if (name.compare(0, 9, "QuickLoad") == 0)
            {
                loadState(config->current_save_bank * EmuConfig::save_slots_per_bank +
                          std::stoi(name.substr(9)));
            }
            else if (name.compare(0, 10, "SelectSlot") == 0)
            {
                /* Select a slot in the current bank without saving or loading. */
                config->current_save_slot = std::stoi(name.substr(10));

                emu_thread->runOnThread([&, index = currentSaveSlot()] {
                    std::string status = core->slotUsed(index) ? " [used]" : " [empty]";
                    core->setMessage("Current slot: " +
                                     std::to_string(config->current_save_slot) +
                                     ", bank: " +
                                     std::to_string(config->current_save_bank) + status);
                });
            }
            else if (name == "SaveStateDialog")
            {
                window->statePreviewDialog(true);
            }
            else if (name == "LoadStateDialog")
            {
                window->statePreviewDialog(false);
            }
            else if (name == "SaveStateFile")
            {
                window->chooseState(true);
            }
            else if (name == "LoadStateFile")
            {
                window->chooseState(false);
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

// LRG rumble dongle -> the SDL device holding SNES Port 1's bindings, as on
// win32. The motor magnitudes are plain uint8 snapshots written by the core on
// the emu thread; a stale read here only delays the motor by one poll.
// SDL effects have a finite duration, so refresh while active; one zero-send
// stops the motors when the game goes quiet.
void EmuApplication::updateRumble()
{
    static uint16_t last_low = 0, last_high = 0;
    static std::chrono::steady_clock::time_point last_send;

    uint8_t l = 0, r = 0;
    if (config->enable_rumble && !isPaused())
        S9xGetRumble(l, r);
    const uint16_t low = l * 0x1111, high = r * 0x1111;

    auto now = std::chrono::steady_clock::now();
    const bool changed = (low != last_low || high != last_high);
    const bool refresh = (low || high) &&
                         (now - last_send >= std::chrono::milliseconds(50));
    if (changed || refresh)
    {
        int device = -1;
        for (auto &b : config->binding.controller[0].buttons)
        {
            if (b.type == EmuBinding::Joystick)
            {
                device = b.guid;
                break;
            }
        }

        for (auto &d : input_manager->devices)
        {
            if (d.second.index != device)
                continue;
            // 120ms outlives the refresh interval; auto-stops if we go quiet.
            if (d.second.gamepad)
                SDL_RumbleGamepad(d.second.gamepad, low, high, 120);
            else if (d.second.joystick)
                SDL_RumbleJoystick(d.second.joystick, low, high, 120);
        }
        last_send = now;
    }

    last_low = low;
    last_high = high;
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
    poll_input_timer->setInterval(1);
    poll_input_timer->setSingleShot(false);
    poll_input_timer->callOnTimeout([&] {
        pollJoysticks();
        updateRumble();
    });
    poll_input_timer->start();
}

/* Flat state index of the currently selected bank/slot pair. */
int EmuApplication::currentSaveSlot()
{
    return config->current_save_bank * EmuConfig::save_slots_per_bank +
           config->current_save_slot;
}

void EmuApplication::loadState(int slot)
{
    emu_thread->runOnThread([&, slot] {
#ifdef RETROACHIEVEMENTS_SUPPORT
        if (!RA_WarnDisableHardcore("Loading save states"))
            return;
#endif
        core->loadState(slot);
    });
}

void EmuApplication::loadState(const std::string& filename)
{
    emu_thread->runOnThread([&, filename] {
#ifdef RETROACHIEVEMENTS_SUPPORT
        if (!RA_WarnDisableHardcore("Loading save states"))
            return;
#endif
        core->loadState(filename);
#ifdef RETROACHIEVEMENTS_SUPPORT
        RA_OnLoadState(filename.c_str());
#endif
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
#ifdef RETROACHIEVEMENTS_SUPPORT
        RA_OnSaveState(filename.c_str());
#endif
    });
}

void EmuApplication::reset()
{
    emu_thread->runOnThread([&] {
        core->softReset();
#ifdef RETROACHIEVEMENTS_SUPPORT
        RA_OnReset();
#endif
    });
}

void EmuApplication::powerCycle()
{
    emu_thread->runOnThread([&] {
        core->reset();
#ifdef RETROACHIEVEMENTS_SUPPORT
        RA_OnReset();
#endif
    });
}

void EmuApplication::loadUndoState()
{
    emu_thread->runOnThread([&] {
        core->loadUndoState();
    });
}

uint8_t EmuApplication::getSoundChannelMask()
{
    return S9xGetSoundChannelMask();
}

void EmuApplication::setSoundChannelMask(uint8_t mask)
{
    emu_thread->runOnThread([mask] {
        S9xSetSoundChannelMask(mask);
    });
}

std::string EmuApplication::getStateFolder()
{
    return core->getStateFolder();
}

std::string EmuApplication::getStateFilename(int slot)
{
    return core->getStateFilename(slot);
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
