#include "EmuCanvasQt.hpp"
#include "EmuConfig.hpp"

#include <QGuiApplication>
#include <QtEvents>
#include <QThread>
#include <qguiapplication.h>

EmuCanvasQt::EmuCanvasQt(EmuConfig *config, QWidget *parent, QWidget *main_window)
    : EmuCanvas(config, parent, main_window)
{
    setMinimumSize(256, 224);
}

EmuCanvasQt::~EmuCanvasQt()
{
    deinit();
}

void EmuCanvasQt::deinit()
{
}

void EmuCanvasQt::draw()
{
    qimage_mutex.lock();
    qimage = QImage((const uchar *)output_data.buffer, output_data.width, output_data.height, output_data.bytes_per_line, output_data.format);
    qimage_mutex.unlock();
    throttle();
    update();
}

void EmuCanvasQt::paintEvent(QPaintEvent *event)
{
    // TODO: If emu not running
    if (!output_data.ready)
    {
        QPainter paint(this);
        paint.fillRect(QRect(0, 0, width(), height()), QBrush(QColor(0, 0, 0)));
        return;
    }

    QPainter paint(this);
    paint.setRenderHint(QPainter::SmoothPixmapTransform, config->bilinear_filter);
    QRect dest = { 0, 0, width(), height() };
    if (config->maintain_aspect_ratio)
    {
        paint.fillRect(QRect(0, 0, width(), height()), QBrush(QColor(0, 0, 0)));
        dest = applyAspect(dest);
    }

    qimage_mutex.lock();
    paint.drawImage(dest, qimage, QRect(0, 0, output_data.width, output_data.height));
    qimage_mutex.unlock();
}

void EmuCanvasQt::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}