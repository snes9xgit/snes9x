#ifndef __EMU_CANVAS_QT_HPP
#define __EMU_CANVAS_QT_HPP

#include "EmuCanvas.hpp"

#include <QPainter>
#include <mutex>

class EmuCanvasQt : public EmuCanvas
{
  public:
    EmuCanvasQt(EmuConfig *config, QWidget *main_window);
    ~EmuCanvasQt();

    virtual void deinit() override;
    virtual void draw() override;

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    std::mutex qimage_mutex;
    QImage qimage;
};

#endif