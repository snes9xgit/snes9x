#ifndef __EMU_INPUT_PANEL_HPP
#define __EMU_INPUT_PANEL_HPP

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "EmuBinding.hpp"

class EmuInputBinder : public QWidget
{
  public:
    EmuInputBinder(const QString &text, std::vector<EmuBinding> *bindings, int min_label_width = 0);
    ~EmuInputBinder();
    void updateDisplay();
    std::vector<EmuBinding> *bindings;
    void connectAddButton(std::function<void(EmuInputBinder *)>);
    void untoggleAddButton();
    void reset(const QString &text, std::vector<EmuBinding> *bindings);

    void keyPressEvent(QKeyEvent *event) override;
    
  private:
    QLabel *label;
    QHBoxLayout *layout;
    QPushButton *buttons[3];
    QPushButton *add_button;
    std::function<void(EmuInputBinder *)> add_button_func;
    QIcon remove_icon;
    QIcon add_icon;
};

class EmuInputPanel : QWidget
{
};

#endif