#include "EmuCanvas.hpp"
#include <qnamespace.h>
#include <qwidget.h>

EmuCanvas::EmuCanvas(EmuConfig *config, QWidget *parent, QWidget *main_window)
    : QWidget(parent)
{
    setFocus();
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    output_data.buffer = nullptr;
    output_data.ready = false;
    this->config = config;
    this->parent = parent;
    this->main_window = main_window;
}

EmuCanvas::~EmuCanvas()
{
}

void EmuCanvas::output(uint8_t *buffer, int width, int height, QImage::Format format, int bytes_per_line, double frame_rate)
{
    output_data.buffer = buffer;
    output_data.width = width;
    output_data.height = height;
    output_data.format = format;
    output_data.bytes_per_line = bytes_per_line;
    output_data.frame_rate = frame_rate;
    output_data.ready = true;
    draw();
}

void EmuCanvas::throttle()
{
    if (config->speed_sync_method != EmuConfig::eTimer && config->speed_sync_method != EmuConfig::eTimerWithFrameskip)
        return;

    throttle_object.set_frame_rate(config->fixed_frame_rate == 0.0 ? output_data.frame_rate : config->fixed_frame_rate);
    throttle_object.wait_for_frame_and_rebase_time();
}

QRect EmuCanvas::applyAspect(const QRect &viewport)
{
    if (!config->scale_image)
    {
        return QRect((viewport.width() - output_data.width) / 2,
                     (viewport.height() - output_data.height) / 2,
                     output_data.width,
                     output_data.height);
    }
    if (!config->maintain_aspect_ratio)
        return viewport;

    int num = config->aspect_ratio_numerator;
    int den = config->aspect_ratio_denominator;
    if (config->show_overscan)
    {
        num *= 224;
        den *= 239;
    }

    if (config->use_integer_scaling)
    {
        int max_scale = 1;

        for (int i = 2; i < 20; i++)
        {
            int scaled_height = output_data.height * i;
            int scaled_width = scaled_height * num / den;
            if (scaled_width <= viewport.width() && scaled_height <= viewport.height())
                max_scale = i;
            else
                break;
        }

        int new_height = output_data.height * max_scale;
        int new_width = new_height * num / den;
        return QRect((viewport.width() - new_width) / 2,
                     (viewport.height() - new_height) / 2,
                     new_width,
                     new_height);
    }

    double canvas_aspect = (double)viewport.width() / viewport.height();
    double new_aspect = (double)num / den;

    if (canvas_aspect > new_aspect)
    {
        int new_width = viewport.height() * num / den;
        int new_x = (viewport.width() - new_width) / 2;

        return { new_x,
                 viewport.y(),
                 new_width,
                 viewport.height() };
    }

    int new_height = viewport.width() * den / num;
    int new_y = (viewport.height() - new_height) / 2;

    return { viewport.x(),
             new_y,
             viewport.width(),
             new_height };
}