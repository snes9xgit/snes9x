#pragma once
#include "EmuCanvas.hpp"

#include <QPainter>
#include <mutex>

class EmuCanvasQt : public EmuCanvas
{
  public:
    EmuCanvasQt(EmuApplication &app, QWidget *parent);

    void deinit() override;
    void draw() override;

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    std::mutex qimage_mutex;
    QImage qimage;
};