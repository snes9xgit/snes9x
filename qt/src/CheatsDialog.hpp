#pragma once
#include "ui_CheatsDialog.h"

class EmuApplication;

class CheatsDialog : public QDialog, public Ui_Dialog
{
  public:
    CheatsDialog(QWidget *parent, EmuApplication *app);
    void addCode();
    void removeCode();
    void updateCurrent();
    void disableAll();
    void removeAll();
    void searchDatabase();
    void refreshList();
    void showEvent(QShowEvent *) override;
    EmuApplication *app;
    int row_removed = 0;
    bool ignore_movement = false;
    void resizeEvent(QResizeEvent *event) override;
};

