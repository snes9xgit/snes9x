#pragma once
#include "ui_EmulationPanel.h"
#include "EmuApplication.hpp"

class EmulationPanel :
    public Ui::EmulationPanel,
    public QWidget
{
  public:
    EmulationPanel(EmuApplication *app);
    ~EmulationPanel();
    void showEvent(QShowEvent *event) override;

    EmuApplication *app;
};