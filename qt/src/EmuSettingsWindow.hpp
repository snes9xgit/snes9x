#pragma once

#include "ui_EmuSettingsWindow.h"
#include "EmuApplication.hpp"
#include "GeneralPanel.hpp"
#include "DisplayPanel.hpp"
#include "SoundPanel.hpp"
#include "EmulationPanel.hpp"
#include "ControllerPanel.hpp"
#include "FoldersPanel.hpp"
#include "ShortcutsPanel.hpp"

class EmuSettingsWindow
  : public QDialog,
    public Ui::EmuSettingsWindow
{
  public:
    EmuSettingsWindow(QWidget *parent, EmuApplication *app);
    void show(int page);

    EmuApplication *app;
    GeneralPanel *general_panel;
    DisplayPanel *display_panel;
    SoundPanel *sound_panel;
    EmulationPanel *emulation_panel;
    ControllerPanel *controller_panel;
    ShortcutsPanel *shortcuts_panel;
    FoldersPanel *folders_panel;
};