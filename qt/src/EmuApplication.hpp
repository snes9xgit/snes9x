#pragma once
#include <QApplication>
#include <QTimer>
#include <QThread>

class EmuMainWindow;
class EmuConfig;
class EmuBinding;
class SDLInputManager;
class Snes9xController;
class S9xSoundDriver;

struct EmuThread : public QThread
{
Q_OBJECT
  public:
    EmuThread(QThread *main_thread);
    QThread *main_thread;
    void run() override;
    void waitForStatusBit(int);
    void waitForStatusBitCleared(int);
    void setStatusBits(int);
    void unsetStatusBits(int);
    void pause();
    void unpause();
    void setMainLoop(const std::function<void()> &loop);

    std::function<void()> main_loop = nullptr;

    enum StatusBits
    {
        eDead       = 0,
        ePaused     = 1,
        eSuspended  = 2,
        eRunning    = 4,
        eQuit       = 8
    };

    int status = eDead;

  public slots:
    void runOnThread(const std::function<void()> &func, bool blocking = false);
};

struct EmuApplication
{
    std::unique_ptr<QApplication> qtapp;
    std::unique_ptr<EmuConfig> config;
    std::unique_ptr<SDLInputManager> input_manager;
    std::unique_ptr<EmuMainWindow> window;
    std::unique_ptr<S9xSoundDriver> sound_driver;
    std::unique_ptr<EmuThread> emu_thread;
    Snes9xController *core;

    EmuApplication();
    ~EmuApplication();
    bool openFile(const std::string &filename);
    void handleBinding(std::string name, bool pressed);
    void updateSettings();
    void updateBindings();
    bool isBound(EmuBinding b);
    void reportBinding(EmuBinding b, bool active);
    void startInputTimer();
    void pollJoysticks();
    void reportPointer(int x, int y);
    void reportMouseButton(int button, bool pressed);
    void restartAudio();
    void writeSamples(int16_t *data, int samples);
    void mainLoop();
    void pause();
    void reset();
    void powerCycle();
    void suspendThread();
    void unsuspendThread();
    bool isPaused();
    void unpause();
    void loadState(int slot);
    void loadState(const std::string& filename);
    void saveState(int slot);
    void saveState(const std::string& filename);
    std::string getStateFolder();
    void loadUndoState();
    void startGame();
    void startThread();
    void stopThread();
    bool isCoreActive();
    QString iconPrefix();
    std::string getContentFolder();

    std::vector<std::tuple<bool, std::string, std::string>> getCheatList();
    void disableAllCheats();
    void enableCheat(int index);
    void disableCheat(int index);
    bool addCheat(const std::string &description, const std::string &code);
    void deleteCheat(int index);
    void deleteAllCheats();
    int tryImportCheats(const std::string &filename);
    std::string validateCheat(const std::string &code);
    int modifyCheat(int index, const std::string &name,
                    const std::string &code);

    enum Handler
    {
        Core = 0,
        UI   = 1
    };
    std::map<uint32_t, std::pair<std::string, Handler>> bindings;
    std::unique_ptr<QTimer> poll_input_timer;
    std::function<void(EmuBinding)> binding_callback = nullptr;
    std::function<void()> joypads_changed_callback = nullptr;
    int save_slot = 0;
    int pause_count = 0;
    int suspend_count = 0;
};