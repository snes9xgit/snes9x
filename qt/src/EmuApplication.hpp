#pragma once
#include <QApplication>
#include <QTimer>

#include "EmuMainWindow.hpp"
#include "EmuConfig.hpp"
#include "SDLInputManager.hpp"
#include "Snes9xController.hpp"
#include "common/audio/s9x_sound_driver.hpp"


struct EmuApplication
{
    std::unique_ptr<QApplication> qtapp;
    std::unique_ptr<EmuConfig> config;
    std::unique_ptr<SDLInputManager> input_manager;
    std::unique_ptr<EmuMainWindow> window;
    std::unique_ptr<S9xSoundDriver> sound_driver;
    Snes9xController *core;

    EmuApplication();
    ~EmuApplication();
    bool openFile(std::string filename);
    void handleBinding(std::string name, bool pressed);
    void updateSettings();
    void updateBindings();
    bool isBound(EmuBinding b);
    void reportBinding(EmuBinding b, bool active);
    void pollJoysticks();
    void restartAudio();
    void writeSamples(int16_t *data, int samples);
    void pause();
    void reset();
    void powerCycle();
    bool isPaused();
    void unpause();
    void loadState(int slot);
    void loadState(std::string filename);
    void saveState(int slot);
    void saveState(std::string filename);
    std::string getStateFolder();
    void loadUndoState();
    void startGame();
    void startIdleLoop();
    void stopIdleLoop();
    void idleLoop();
    bool isCoreActive();

    enum Handler
    {
        Core = 0,
        UI   = 1
    };
    std::map<uint32_t, std::pair<std::string, Handler>> bindings;
    std::unique_ptr<QTimer> idle_loop;
    std::function<void(EmuBinding)> binding_callback = nullptr;
    std::function<void()> joypads_changed_callback = nullptr;
    int save_slot = 0;
    int pause_count = 0;
};