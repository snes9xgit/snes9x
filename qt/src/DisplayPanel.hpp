#pragma once
#include "ui_DisplayPanel.h"
#include "EmuApplication.hpp"

class DisplayPanel :
    public Ui::DisplayPanel,
    public QWidget
{
  public:
    DisplayPanel(EmuApplication *app);
    ~DisplayPanel();
    void showEvent(QShowEvent *event) override;
    void populateDevices();
    void selectShaderDialog();

    std::vector<std::pair<int, std::string>> driver_list;
    bool updating = true;

    EmuApplication *app;
};