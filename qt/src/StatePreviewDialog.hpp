/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QToolButton>
#include <vector>

#include "EmuApplication.hpp"

/* win32-style save/load-with-preview picker: a grid of the selected bank's
 * slots showing each snapshot's screenshot, plus a bank selector. */
class StatePreviewDialog : public QDialog
{
    Q_OBJECT

  public:
    StatePreviewDialog(EmuApplication *app, QWidget *parent, bool is_save);

    /* Flat state index (bank * save_slots_per_bank + slot) that was picked,
     * or -1 when the dialog was cancelled. */
    int selection() const { return picked; }

  private:
    void refresh();

    EmuApplication *app;
    QComboBox *bank_combo;
    std::vector<QToolButton *> slot_buttons;
    int picked = -1;
};
