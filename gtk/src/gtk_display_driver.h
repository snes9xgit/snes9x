/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "gtk_s9x.h"

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
    virtual int get_width() = 0;
    virtual int get_height() = 0;
    virtual void shrink() {};
    virtual void regrow() {};

  protected:
    Snes9xWindow *window = nullptr;
    Snes9xConfig *config = nullptr;
    Gtk::DrawingArea *drawing_area = nullptr;
};