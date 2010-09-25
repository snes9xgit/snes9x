#ifndef __GTK_BINDING_H
#define __GTK_BINDING_H

#include <gdk/gdk.h>

#define AXIS_POS                    1
#define AXIS_NEG                    0
#define JOY_AXIS(axis, button)      (512 + ((axis) * 2 + (button)))
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
        Binding (void);
        void to_string (char *str);
        unsigned int hex (void);   
        unsigned int base_hex (void);
        bool matches (Binding &binding);
        void clear (void);
        bool is_joy (void);
        bool is_key (void);
        bool is_mouse (void);
        bool is_positive (void);
        bool is_negative (void);
        unsigned int get_key (void);
        unsigned int get_device (void);
        unsigned int get_threshold (void);
        unsigned int get_axis (void);
        GdkModifierType get_gdk_modifiers (void);
        
    private:
        unsigned int value;
};
    
#endif /* __GTK_BINDING_H_ */
