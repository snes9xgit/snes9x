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
    void resizeEvent(QResizeEvent *event) override;
};

