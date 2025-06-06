#pragma once
#include "ui_EmulationPanel.h"

class EmuApplication;

class EmulationPanel :
    public Ui::EmulationPanel,
    public QWidget
{
  public:
    explicit EmulationPanel(EmuApplication *app);
    void showEvent(QShowEvent *event) override;

    EmuApplication *app;
};