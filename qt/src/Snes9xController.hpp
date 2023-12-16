#ifndef __SNES9X_CONTROLLER_HPP
#define __SNES9X_CONTROLLER_HPP
#include <functional>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

#include "EmuConfig.hpp"

class Snes9xController
{
  public:
    static Snes9xController *get();

    void init();
    void deinit();
    void mainLoop();
    bool openFile(std::string filename);
    bool loadState(std::string filename);
    bool loadState(int slot);
    void loadUndoState();
    bool saveState(std::string filename);
    bool saveState(int slot);
    void increaseSaveSlot();
    void decreaseSaveSlot();
    void updateSettings(const EmuConfig * const config);
    void updateBindings(const EmuConfig * const config);
    void reportBinding(EmuBinding b, bool active);
    void updateSoundBufferLevel(int, int);
    bool acceptsCommand(const char *command);
    bool isAbnormalSpeed();
    void mute(bool muted);
    void reset();
    void softReset();
    void setPaused(bool paused);
    void setMessage(std::string message);
    void clearSoundBuffer();
    std::vector<std::tuple<bool, std::string, std::string>> getCheatList();
    void disableAllCheats();
    void enableCheat(int index);
    void disableCheat(int index);
    bool addCheat(std::string description, std::string code);
    void deleteCheat(int index);
    void deleteAllCheats();
    int tryImportCheats(std::string filename);
    std::string validateCheat(std::string code);
    int modifyCheat(int index, std::string name, std::string code);

    std::string getStateFolder();
    std::string config_folder;
    std::string sram_folder;
    std::string state_folder;
    std::string cheat_folder;
    std::string patch_folder;
    std::string export_folder;
    int high_resolution_effect;
    int rewind_buffer_size;
    int rewind_frame_interval;
    bool rewinding = false;

    std::function<void(uint16_t *, int, int, int, double)> screen_output_function = nullptr;
    std::function<void(int16_t *, int)> sound_output_function = nullptr;

    bool active = false;

  protected:
    Snes9xController();
    ~Snes9xController();

  private:
    void SamplesAvailable();

};

#endif