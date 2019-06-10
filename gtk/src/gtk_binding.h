/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_BINDING_H
#define __GTK_BINDING_H

#include "gtk_2_3_compat.h"

#define AXIS_POS                    1
#define AXIS_NEG                    0
#define JOY_AXIS(axis, direction)   (512 + ((axis) * 2 + (direction)))
#define JOY_AXIS_UNMASK(bin)        ((((bin) & 0x0000ffff) - 512) / 2)
#define JOY_DIRECTION_UNMASK(bin)   ((((bin) & 0x0000ffff) - 512) % 2)
#define BINDING_KEY                 0x10000000
#define BINDING_JOY                 0x20000000
#define BINDING_CTRL                0x01000000
#define BINDING_SHIFT               0x02000000
#define BINDING_ALT                 0x04000000
#define BINDING_JOYDEV_MASK         0x0f000000
#define BINDING_THRESHOLD_MASK      0x00ff0000
#define BINDING_KEY_MASK            0x0000ffff
#define THRESHOLD_MASK(threshold)   ((threshold) << 16)
#define THRESHOLD_UNMASK(bin)       (((bin) & 0x00ff0000) >> 16)
#define JOY_DEVICE_MASK(devnum)     ((devnum) << 24)
#define JOY_DEVICE_UNMASK(bin)      (((bin) & 0x0f000000) >> 24)
#define BINDING_MOUSE_POINTER       0x41000000
#define BINDING_MOUSE_BUTTON0       0x42000000
#define BINDING_MOUSE_BUTTON1       0x42000001
#define BINDING_MOUSE_BUTTON2       0x42000002

class Binding 
{
    public:
        Binding (unsigned int key, bool ctrl, bool shift, bool alt);
        Binding (unsigned int device, unsigned int button, unsigned int threshold);
        Binding (const Binding &binding);
        Binding (GdkEventKey *event);
        Binding (unsigned int);
        Binding ();
        Binding (const char *str);
        void to_string (char *str, bool translate = true);
        unsigned int hex ();
        unsigned int base_hex ();
        bool matches (Binding &binding);
        void clear ();
        bool is_joy ();
        bool is_key ();
        bool is_mouse ();
        bool is_positive ();
        bool is_negative ();
        unsigned int get_key ();
        unsigned int get_device ();
        unsigned int get_threshold ();
        unsigned int get_axis ();
        GdkModifierType get_gdk_modifiers ();
        
    private:
        unsigned int value;
};
    
#endif /* __GTK_BINDING_H_ */
