#ifndef __EMU_CANVAS_QT_HPP
#define __EMU_CANVAS_QT_HPP

#include "EmuCanvas.hpp"

#include <QPainter>

class EmuCanvasQt : public EmuCanvas
{
  public:
    EmuCanvasQt(EmuConfig *config, QWidget *parent, QWidget *main_window);
    ~EmuCanvasQt();

    virtual void deinit() override;
    virtual void draw() override;

    void paintEvent(QPaintEvent *event) override;
};

#endif