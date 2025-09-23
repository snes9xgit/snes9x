/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "gtk_s9x.h"
#include "snes9x.h"
#include "common/video/std_chrono_throttle.hpp"

class S9xDisplayDriver
{
  public:
    virtual ~S9xDisplayDriver() = default;
    virtual void refresh() = 0;
    virtual int init() = 0;
    virtual void deinit() = 0;
    virtual void update(uint16_t *buffer, int width, int height, int stride_in_pixels) = 0;
    virtual void *get_parameters() = 0;
    virtual void save(const std::string &filename) = 0;
    virtual bool is_ready() = 0;
    virtual bool can_throttle() { return false; };
    virtual double get_late_frames()
    {
        if (!can_throttle() || Settings.SkipFrames != THROTTLE_TIMER_FRAMESKIP)
            return 0.0;
        throttle.set_frame_rate(Settings.PAL ? PAL_PROGRESSIVE_FRAME_RATE : NTSC_PROGRESSIVE_FRAME_RATE);
        return throttle.get_late_frames();
    };
    virtual int get_width() = 0;
    virtual int get_height() = 0;
    virtual void shrink() {};
    virtual void regrow() {};
    Throttle throttle;

  protected:
    Snes9xWindow *window = nullptr;
    Snes9xConfig *config = nullptr;
    Gtk::DrawingArea *drawing_area = nullptr;
};