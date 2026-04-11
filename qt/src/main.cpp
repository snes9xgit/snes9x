#include "EmuApplication.hpp"
#include "EmuConfig.hpp"
#include "EmuMainWindow.hpp"
#include "SDLInputManager.hpp"
#include "display.h"

#ifdef RETROACHIEVEMENTS_SUPPORT
#include "RAIntegrationQt.hpp"
#include "retroachievements.h"
#endif

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
    int argc = 0;
    LPWSTR *argvw = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Convert wide args to UTF-8
    char **argv = new char *[argc];
    for (int i = 0; i < argc; i++)
    {
        int size = WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, nullptr, 0, nullptr, nullptr);
        argv[i] = new char[size];
        WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, argv[i], size, nullptr, nullptr);
    }
    LocalFree(argvw);

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        if (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
        {
#else
        if (false) {
#endif
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

    char *rom_filename = S9xParseArgs(argv, argc);
    if (rom_filename)
        emu.window->openFile(rom_filename);

#ifdef _WIN32
    for (int i = 0; i < argc; i++)
        delete[] argv[i];
    delete[] argv;
#endif

#ifdef RETROACHIEVEMENTS_SUPPORT
    if (emu.config->ra_enabled)
    {
        RA_Qt_RegisterCallbacks(&emu);
        RA_Init();
        RA_SetEnabled(true);
        RA_SetHardcoreEnabled(emu.config->ra_hardcore_mode);
        RA_AttemptLogin(emu.config->ra_username.c_str(), emu.config->ra_api_token.c_str());
    }
#endif

    emu.qtapp->exec();

    emu.stopThread();
    emu.config->saveFile(EmuConfig::findConfigFile());

    return 0;
}
