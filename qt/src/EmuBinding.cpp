#include "EmuBinding.hpp"
#include "SDL_joystick.h"
#include <QString>
#include <QKeySequence>
#include <sstream>

// Hash format:
//
// Bit 31-30: Joystick or Keyboard bit
//
// Keyboard:
//   Bit 30: Alt
//   Bit 29: Ctrl
//   Bit 28: Super
//   Bit 27: Shift
//   Bits 15-0: keycode
//
// Joystick:
//   Bits 29-28: Type:
//                  00 Button
//                  01 Axis
//                  10 Hat
//   Bit 27: If axis or hat, positive or negative
//   Bits 26-19: Which button/hat/axis
//   Bits 15-8: Hat direction
//   Bits 7-0: Device identifier
uint32_t EmuBinding::hash() const
{
    uint32_t hash = 0;

    hash |= type << 30;
    if (type == Keyboard)
    {
        hash |= alt << 29;
        hash |= ctrl << 28;
        hash |= super << 27;
        hash |= shift << 26;
        hash |= keycode & 0xfffffff;
    }
    else
    {
        hash |= (input_type & 0x3) << 28;
        hash |= (threshold < 0 ? 1 : 0) << 27;
        hash |= (button & 0xff) << 19;
        hash |= (input_type == Hat) ? direction << 8 : 0;
        hash |= (guid & 0xff);
    }

    return hash;
}

bool EmuBinding::operator==(const EmuBinding &other)
{
    return other.hash() == hash();
}

EmuBinding EmuBinding::joystick_axis(int device, int axis, int threshold)
{
    EmuBinding binding;
    binding.type = Joystick;
    binding.input_type = Axis;
    binding.guid = device;
    binding.axis = axis;
    binding.threshold = threshold;
    return binding;
}

EmuBinding EmuBinding::joystick_hat(int device, int hat, uint8_t direction)
{
    EmuBinding binding{};
    binding.type = Joystick;
    binding.input_type = Hat;
    binding.guid = device;
    binding.hat = hat;
    binding.direction = direction;
    return binding;
}

EmuBinding EmuBinding::joystick_button(int device, int button)
{
    EmuBinding binding{};
    binding.type = Joystick;
    binding.input_type = Button;
    binding.guid = device;
    binding.button = button;
    return binding;
}

EmuBinding EmuBinding::keyboard(int keycode, bool shift, bool alt, bool ctrl, bool super)
{
    EmuBinding binding{};
    binding.type = Keyboard;
    binding.alt = alt;
    binding.ctrl = ctrl;
    binding.shift = shift;
    binding.super = super;
    binding.keycode = keycode;
    return binding;
}

EmuBinding EmuBinding::from_config_string(std::string string)
{
    for (auto &c : string)
        if (c >= 'A' && c <= 'Z')
            c += 32;

    if (string.compare(0, 9, "keyboard ") == 0)
    {
        EmuBinding b{};
        b.type = Keyboard;

        QString qstr(string.substr(9).c_str());
        auto seq = QKeySequence::fromString(qstr);
        if (seq.count())
        {
            b.keycode = seq[0].key();
            b.alt = seq[0].keyboardModifiers().testAnyFlag(Qt::AltModifier);
            b.ctrl = seq[0].keyboardModifiers().testAnyFlag(Qt::ControlModifier);
            b.super = seq[0].keyboardModifiers().testAnyFlag(Qt::MetaModifier);
            b.shift = seq[0].keyboardModifiers().testAnyFlag(Qt::ShiftModifier);
        }

        return b;
    }
    else if (string.compare(0, 8, "joystick") == 0)
    {
        auto substr = string.substr(8);
        unsigned int axis;
        unsigned int button;
        unsigned int percent;
        unsigned int device;
        char direction_string[6]{};
        char posneg;

        if (sscanf(substr.c_str(), "%u axis %u %c %u", &device, &axis, &posneg, &percent) == 4)
        {
            int sign = posneg == '-' ? -1 : 1;
            return joystick_axis(device, axis, sign * percent);
        }
        else if (sscanf(substr.c_str(), "%u button %u", &device, &button) == 2)
        {
            return joystick_button(device, button);
        }
        else if (sscanf(substr.c_str(), "%u hat %u %5s", &device, &axis, direction_string))
        {
            uint8_t direction;
            if (!strcmp(direction_string, "up"))
                direction = SDL_HAT_UP;
            else if (!strcmp(direction_string, "down"))
                direction = SDL_HAT_DOWN;
            else if (!strcmp(direction_string, "left"))
                direction = SDL_HAT_LEFT;
            else if (!strcmp(direction_string, "right"))
                direction = SDL_HAT_RIGHT;

            return joystick_hat(device, axis, direction);
        }
    }

    return {};
}

std::string EmuBinding::to_config_string()
{
    return to_string(true);
}

std::string EmuBinding::to_string(bool config)
{
    std::string rep;
    if (type == Keyboard)
    {
        if (config)
            rep += "Keyboard ";

        if (ctrl)
            rep += "Ctrl+";
        if (alt)
            rep += "Alt+";
        if (shift)
            rep += "Shift+";
        if (super)
            rep += "Super+";

        QKeySequence seq(keycode);
        rep += seq.toString().toStdString();
    }
    else if (type == Joystick)
    {
        if (config)
            rep += "joystick " + std::to_string(guid) + " ";
        else
            rep += "J" + std::to_string(guid) + " ";

        if (input_type == Button)
        {
            rep += "Button ";
            rep += std::to_string(button);
        }
        if (input_type == Axis)
        {
            rep += "Axis ";
            rep += std::to_string(axis) + " ";
            rep += std::to_string(threshold) + "%";
        }
        if (input_type == Hat)
        {
            rep += "Hat ";
            rep += std::to_string(hat) + " ";
            if (direction == SDL_HAT_UP)
                rep += "Up";
            else if (direction == SDL_HAT_DOWN)
                rep += "Down";
            else if (direction == SDL_HAT_LEFT)
                rep += "Left";
            else if (direction == SDL_HAT_RIGHT)
                rep += "Right";
        }
    }
    else
    {
        rep = "None";
    }

    return rep;
}
