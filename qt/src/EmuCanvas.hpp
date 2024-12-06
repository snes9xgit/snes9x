#pragma once
#include <QWidget>
#include <QImage>
#include "common/video/std_chrono_throttle.hpp"

class EmuConfig;

class EmuCanvas : public QWidget
{
  public:
    EmuCanvas(EmuConfig *config, QWidget *main_window);
    ~EmuCanvas();

    virtual void deinit() = 0;
    virtual void draw() = 0;
    void paintEvent(QPaintEvent *) override = 0;
    virtual bool createContext() { return false; }
    virtual void recreateUIAssets() {}
    void output(uint8_t *buffer, int width, int height, QImage::Format format, int bytes_per_line, double frame_rate);
    void throttle();
    void resizeEvent(QResizeEvent *event) override = 0;

    virtual std::vector<std::string> getDeviceList()
    {
        return std::vector<std::string>{ "Default" };
    }

    bool ready()
    {
        return output_data.ready;
    }

    QRect applyAspect(const QRect &viewport);

    struct Parameter
    {
        bool operator==(const Parameter &other) const
        {
            if (name == other.name &&
                id   == other.id &&
                min  == other.min &&
                max  == other.max &&
                val  == other.val &&
                step == other.step &&
                significant_digits == other.significant_digits)
                return true;
            return false;
        };

        std::string name;
        std::string id;
        float min;
        float max;
        float val;
        float step;
        int significant_digits;
    };

    virtual void showParametersDialog() {};
    virtual void shaderChanged() {};
    virtual void saveParameters(std::string filename) {};

    struct
    {
        bool ready;
        uint8_t *buffer;
        int width;
        int height;
        QImage::Format format;
        int bytes_per_line;
        double frame_rate;
    } output_data;

    QWidget *main_window{};
    EmuConfig *config{};
    Throttle throttle_object;
};