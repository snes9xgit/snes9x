/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_CONTROL_H
#define __GTK_CONTROL_H

#include <queue>
#include <array>

#include "gtk_binding.h"
#include "SDL.h"
// SDL.h may include altivec.h which redefines vector and bool
#undef vector
#undef bool

const int NUM_JOYPADS = 10;

enum {
    JOY_MODE_GLOBAL     = 0,
    JOY_MODE_INDIVIDUAL = 1,
    JOY_MODE_CALIBRATE  = 2
};

enum {
    JOY_RELEASED = 0,
    JOY_PRESSED  = 1
};

enum {
    PORT_COMMAND_FULLSCREEN = 1,
    PORT_COMMAND_SAVE_SPC   = 2,
    PORT_OPEN_ROM           = 3,
    PORT_PAUSE              = 4,
    PORT_SEEK_TO_FRAME      = 5,
    PORT_QUIT               = 6,
    PORT_SWAP_CONTROLLERS   = 7,
    PORT_REWIND             = 8,
    PORT_QUICKLOAD0         = 9,
    PORT_QUICKLOAD1         = 10,
    PORT_QUICKLOAD2         = 11,
    PORT_QUICKLOAD3         = 12,
    PORT_QUICKLOAD4         = 13,
    PORT_QUICKLOAD5         = 14,
    PORT_QUICKLOAD6         = 15,
    PORT_QUICKLOAD7         = 16,
    PORT_QUICKLOAD8         = 17,
    PORT_QUICKLOAD9         = 18,
    PORT_SAVESLOT           = 19,
    PORT_LOADSLOT           = 20,
    PORT_INCREMENTSAVESLOT  = 21,
    PORT_DECREMENTLOADSLOT  = 22,
    PORT_INCREMENTSLOT      = 23,
    PORT_DECREMENTSLOT      = 24,
    PORT_GRABMOUSE          = 25
};

typedef struct BindingLink
{
    const char *button_name;
    const char *snes9x_name;

} BindingLink;

extern const BindingLink b_links[];
extern const int b_breaks[];
const int NUM_JOYPAD_LINKS = 24;
const int NUM_EMU_LINKS = 62;

typedef struct JoypadBinding
{
    std::array<Binding, NUM_JOYPAD_LINKS> data;
} JoypadBinding;

bool S9xGrabJoysticks();
void S9xReleaseJoysticks();

typedef struct JoyEvent
{
    unsigned int parameter;
    unsigned int state;

} JoyEvent;

typedef struct Calibration
{
    int min;
    int max;
    int center;
} Calibration;

class JoyDevice
{
  public:
    JoyDevice();
    ~JoyDevice();
    int get_event(JoyEvent *event);
    void flush();
    void handle_event(SDL_Event *event);
    void register_centers();
    bool set_sdl_joystick_num(unsigned int device_num);

    SDL_Joystick *filedes;
    std::queue<JoyEvent> queue;
    int mode;
    int joynum;
    std::vector<Calibration> calibration;
    std::vector<int> axis;
    std::vector<int> hat;
    bool enabled;

  private:
    void poll_events();
    void add_event(unsigned int parameter, unsigned int state);
};

void S9xDeinitInputDevices();
Binding S9xGetBindingByName(const char *name);
bool S9xIsMousePluggedIn();

#endif /* __GTK_CONTROL_H*/
