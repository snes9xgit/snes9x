#pragma once
#include "ui_ShortcutsPanel.h"
#include "EmuApplication.hpp"
#include "BindingPanel.hpp"
#include <QMenu>

class ShortcutsPanel :
    public Ui::ShortcutsPanel,
    public BindingPanel
{
  public:
    ShortcutsPanel(EmuApplication *app);
    ~ShortcutsPanel();

    void setDefaultKeys(int slot);
    QMenu reset_to_default_menu;
};