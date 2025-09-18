#include "EmuApplication.hpp"
#include "EmuConfig.hpp"
#include "EmuMainWindow.hpp"
#include "SDLInputManager.hpp"

#include <clocale>
#include <qnamespace.h>
#include <QStyle>
#include <QStyleHints>

#ifndef _WIN32
#include <csignal>
#endif

#ifdef _WIN32
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char *lpCmdLine, int nShowCmd)
{
    char **argv = nullptr;
    int argc = 0;
    setlocale(LC_ALL, ".utf8");
#else
int main(int argc, char *argv[])
{
#endif
    EmuApplication emu;
    emu.qtapp = std::make_unique<QApplication>(argc, argv);

    QGuiApplication::setDesktopFileName("snes9x-qt");

    if (QApplication::platformName() == "windows")
    {
        if (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
        {
            QApplication::setStyle("fusion");

            const QColor darkGray(53, 53, 53);
            const QColor gray(128, 128, 128);
            const QColor black(25, 25, 25);
            const QColor blue(198, 238, 255);
            const QColor blue2(0, 88, 208);

            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, darkGray);
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Base, black);
            darkPalette.setColor(QPalette::AlternateBase, darkGray);
            darkPalette.setColor(QPalette::ToolTipBase, blue2);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Button, darkGray);
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::Link, blue);
            darkPalette.setColor(QPalette::Highlight, blue2);
            darkPalette.setColor(QPalette::HighlightedText, Qt::white);
            darkPalette.setColor(QPalette::PlaceholderText, QColor(Qt::white).darker());

            darkPalette.setColor(QPalette::Active, QPalette::Button, darkGray);
            darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
            darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
            darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
            darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);
            QApplication::setPalette(darkPalette);
        }
        else
        {
            QApplication::setStyle("windowsvista");
        }
    }

#ifndef _WIN32
    auto quit_handler = [](int) { QApplication::quit(); };
    for (auto s : { SIGQUIT, SIGINT, SIGTERM, SIGHUP })
        signal(s, quit_handler);
#endif

    emu.startThread();

    emu.config = std::make_unique<EmuConfig>();
    emu.config->setDefaults();
    emu.config->loadFile(EmuConfig::findConfigFile());

    emu.input_manager = std::make_unique<SDLInputManager>();
    emu.window = std::make_unique<EmuMainWindow>(&emu);
    emu.window->show();

    emu.updateBindings();
    emu.startInputTimer();
    emu.qtapp->exec();

    emu.stopThread();
    emu.config->saveFile(EmuConfig::findConfigFile());

    return 0;
}
