#include "ShortcutsPanel.hpp"
#include "EmuConfig.hpp"

ShortcutsPanel::ShortcutsPanel(EmuApplication *app_)
    : BindingPanel(app_)
{
    setupUi(this);
    setTableWidget(tableWidget_shortcuts,
                   app->config->binding.shortcuts,
                   EmuConfig::allowed_bindings,
                   EmuConfig::num_shortcuts);

    toolButton_reset_to_default->setPopupMode(QToolButton::InstantPopup);

    for (auto slot = 0; slot < EmuConfig::allowed_bindings; slot++)
    {
        auto action = reset_to_default_menu.addAction(tr("Slot %1").arg(slot));
        connect(action, &QAction::triggered, [&, slot](bool checked) {
            setDefaultKeys(slot);
        });
    }
    toolButton_reset_to_default->setMenu(&reset_to_default_menu);

    connect(pushButton_clear_all, &QAbstractButton::clicked, [&](bool) {
        for (auto &b : app->config->binding.shortcuts)
            b = {};
        fillTable();
    });

    fillTable();
}

void ShortcutsPanel::setDefaultKeys(int slot)
{
    for (int i = 0; i < EmuConfig::num_shortcuts; i++)
    {
        std::string str = EmuConfig::getDefaultShortcutKeys()[i];
        if (!str.empty())
            app->config->binding.shortcuts[i * 4 + slot] = EmuBinding::from_config_string(str);
    }

    fillTable();
}

