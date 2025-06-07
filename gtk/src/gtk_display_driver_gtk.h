/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "gtk_display_driver.h"

class S9xGTKDisplayDriver : public S9xDisplayDriver
{
  public:
    S9xGTKDisplayDriver(Snes9xWindow *window, Snes9xConfig *config);
    void refresh() override;
    int init() override;
    void deinit() override;
    void update(uint16_t *buffer, int width, int height, int stride_in_pixels) override;
    void *get_parameters() override { return nullptr; }
    void save(const std::string &filename) override {}
    bool is_ready() override { return true; }
    int get_width() override { return last_known_width; }
    int get_height() override { return last_known_height; }

  private:
    void clear();
    void output(void *src,
                int src_pitch,
                int x,
                int y,
                int width,
                int height,
                int dst_width,
                int dst_height);

    int last_known_width;
    int last_known_height;
};