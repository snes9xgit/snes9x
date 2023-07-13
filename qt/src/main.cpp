#include "EmuApplication.hpp"

#ifndef _WIN32
#include <csignal>
#endif

int main(int argc, char *argv[])
{
    EmuApplication emu;
    emu.qtapp = std::make_unique<QApplication>(argc, argv);

    QGuiApplication::setDesktopFileName("snes9x-gtk");

#ifndef _WIN32
    auto quit_handler = [](int) { QApplication::quit(); };
    for (auto s : { SIGQUIT, SIGINT, SIGTERM, SIGHUP })
        signal(s, quit_handler);
#endif

    emu.config = std::make_unique<EmuConfig>();
    emu.config->setDefaults();
    emu.config->loadFile(EmuConfig::findConfigFile());

    emu.input_manager = std::make_unique<SDLInputManager>();
    emu.window = std::make_unique<EmuMainWindow>(&emu);
    emu.window->show();

    emu.updateBindings();
    emu.startInputTimer();
    emu.startThread();
    emu.qtapp->exec();

    emu.stopThread();
    emu.config->saveFile(EmuConfig::findConfigFile());

    return 0;
}
