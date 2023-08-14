#include "ui_CheatsDialog.h"

class EmuApplication;

class CheatsDialog : public QDialog, public Ui_Dialog
{
  public:
    CheatsDialog(QWidget *parent, EmuApplication *app);

    EmuApplication *app;
};

