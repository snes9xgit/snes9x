#ifndef __EMU_BINDING_HPP
#define __EMU_BINDING_HPP
#include <cstdint>
#include <string>

struct EmuBinding
{
    uint32_t hash() const;
    std::string to_string(bool config = false);
    static EmuBinding joystick_axis(int device, int axis, int threshold);
    static EmuBinding joystick_hat(int device, int hat, uint8_t direction);
    static EmuBinding joystick_button(int device, int button);
    static EmuBinding keyboard(int keycode, bool shift = false, bool alt = false, bool ctrl = false, bool super = false);
    static EmuBinding from_config_string(std::string str);
    std::string to_config_string();
    bool operator==(const EmuBinding &);

    static const uint32_t MOUSE_POINTER = 0x0f000000;
    static const uint32_t MOUSE_BUTTON1 = 0x0f000001;
    static const uint32_t MOUSE_BUTTON2 = 0x0f000002;
    static const uint32_t MOUSE_BUTTON3 = 0x0f000003;

    enum Type
    {
        None = 0,
        Keyboard = 1,
        Joystick = 2
    };
    Type type;

    enum JoystickInputType
    {
        Button = 0,
        Axis = 1,
        Hat = 2
    };

    union
    {
        struct
        {
            bool alt;
            bool ctrl;
            bool super;
            bool shift;
            int keycode;
        };

        struct
        {
            JoystickInputType input_type;
            int guid;
            union {
                int button;
                int hat;
                int axis;
            };
            int threshold;
            uint8_t direction;
        };
    };
};

#endif