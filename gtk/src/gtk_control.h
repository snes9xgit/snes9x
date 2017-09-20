#ifndef __GTK_CONTROL_H
#define __GTK_CONTROL_H

#include <queue>

#include "gtk_binding.h"
#ifdef USE_JOYSTICK
#include "SDL.h"
#endif

#ifndef NUM_JOYPADS
#define NUM_JOYPADS 10
#endif

#define JOY_MODE_GLOBAL     0
#define JOY_MODE_INDIVIDUAL 1
#define JOY_MODE_CALIBRATE  2

#define JOY_RELEASED                0
#define JOY_PRESSED                 1

#define PORT_COMMAND_FULLSCREEN 1
#define PORT_COMMAND_SAVE_SPC   2
#define PORT_OPEN_ROM           3
#define PORT_PAUSE              4
#define PORT_SEEK_TO_FRAME      5
#define PORT_QUIT               6
#define PORT_SWAP_CONTROLLERS   7
#define PORT_REWIND             8

typedef struct BindingLink
{
    const char *button_name;
    const char *snes9x_name;

} BindingLink;

extern const BindingLink b_links[];
extern const int b_breaks[];
#define NUM_JOYPAD_LINKS 24
#define NUM_EMU_LINKS    55

typedef struct JoypadBinding
{
    Binding data[NUM_JOYPAD_LINKS]; /* Avoid packing issues */
} JoypadBinding;

#ifdef USE_JOYSTICK

bool S9xGrabJoysticks (void);
void S9xReleaseJoysticks (void);

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
        JoyDevice (unsigned int device_num);
        ~JoyDevice (void);
        int get_event (JoyEvent *event);
        void flush (void);
        void handle_event (SDL_Event *event);
        void register_centers (void);

        SDL_Joystick         *filedes;
        Calibration          *calibration;
        std::queue<JoyEvent> queue;
        int                  mode;
        int                  joynum;
        int                  num_axes;
        int                  num_hats;
        int                  *axis;
        int                  *hat;
        bool                 enabled;


    private:
        void poll_events (void);
        void add_event (unsigned int parameter, unsigned int state);

};
#endif

void S9xDeinitInputDevices (void);
Binding S9xGetBindingByName (const char *name);
bool S9xIsMousePluggedIn (void);

#endif /* __GTK_CONTROL_H*/
