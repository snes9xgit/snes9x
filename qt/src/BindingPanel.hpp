#pragma once

#include <QWidget>
#include <QTableWidget>
#include "EmuBinding.hpp"

class EmuApplication;

class BindingPanel : public QWidget
{
  public:
    BindingPanel(EmuApplication *app);
    ~BindingPanel();
    void setTableWidget(QTableWidget *bindingTableWidget, EmuBinding *binding, int width, int height);
    void cellActivated(int row, int column);
    void handleKeyPressEvent(QKeyEvent *event);
    void updateCellFromBinding(int row, int column);
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void fillTable();
    void checkJoypadInput();
    void finalizeCurrentBinding(const EmuBinding &b);
    void setRedirectInput(bool redirect);
    void onJoypadsChanged(const std::function<void()> &func);

    bool awaiting_binding;
    bool accept_return;
    int table_width;
    int table_height;
    int cell_row;
    int cell_column;
    QIcon keyboard_icon;
    QIcon joypad_icon;
    std::unique_ptr<QTimer> timer;
    EmuApplication *app;
    QTableWidget *binding_table_widget;
    EmuBinding *binding;
    std::function<void()> joypads_changed;
};