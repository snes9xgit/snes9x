#pragma once
#include "ui_ControllerPanel.h"
#include "BindingPanel.hpp"
#include "EmuApplication.hpp"
#include <QMenu>

class ControllerPanel :
    public Ui::ControllerPanel,
    public BindingPanel
{
  public:
    ControllerPanel(EmuApplication *app);
    ~ControllerPanel();
    void showEvent(QShowEvent *event) override;
    void clearAllControllers();
    void clearCurrentController();
    void autoPopulateWithKeyboard(int slot);
    void autoPopulateWithJoystick(int joystick_id, int slot);
    void swapControllers(int first, int second);
    void recreateAutoAssignMenu();

    QMenu edit_menu;
    QMenu auto_assign_menu;
};