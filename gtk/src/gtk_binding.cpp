/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gtk_s9x.h"
#include "gtk_binding.h"
#include "fmt/format.h"

Binding::Binding()
{
    value = 0;
}

Binding::Binding(GdkEventKey *event)
{
    GdkKeymapKey* keys;
    guint* keyvals;
    int n_entries;

    gdk_keymap_get_entries_for_keycode(
        gdk_keymap_get_for_display(top_level->window->get_display()->gobj()),
        event->hardware_keycode,
        &keys,
        &keyvals,
        &n_entries
    );
    event->keyval = keyvals[0];

    g_free(keys);
    g_free(keyvals);

    value = BINDING_KEY | (event->keyval & BINDING_KEY_MASK);

    /* Strip modifiers from modifiers */
    if (event->keyval == GDK_Control_L ||
        event->keyval == GDK_Control_R ||
        event->keyval == GDK_Shift_L ||
        event->keyval == GDK_Shift_R ||
        event->keyval == GDK_Alt_L ||
        event->keyval == GDK_Alt_R)
    {
        return;
    }

    if (event->state & GDK_SHIFT_MASK)
        value |= BINDING_SHIFT;

    if (event->state & GDK_CONTROL_MASK)
        value |= BINDING_CTRL;

    if (event->state & GDK_MOD1_MASK)
        value |= BINDING_ALT;
}

Binding::Binding(unsigned int key, bool ctrl, bool shift, bool alt)
{
    value = BINDING_KEY;

    value |= key & BINDING_KEY_MASK;
    if (ctrl)
        value |= BINDING_CTRL;
    if (shift)
        value |= BINDING_SHIFT;
    if (alt)
        value |= BINDING_ALT;
}

Binding::Binding(unsigned int device, unsigned int button, unsigned int threshold)
{
    value = BINDING_JOY;
    value |= JOY_DEVICE_MASK(device + 1);
    value |= BINDING_KEY_MASK & button;
    if (button >= 512)
        value |= THRESHOLD_MASK(threshold);
}

Binding::Binding(unsigned int val)
{
    value = val;
}

Binding::Binding(const Binding &binding)
{
    this->value = binding.value;
}

Binding &Binding::operator=(const Binding &binding)
{
    this->value = binding.value;
    return *this;
}

bool Binding::operator==(const Binding &binding)
{
    if ((value & ~BINDING_THRESHOLD_MASK) == (binding.value & ~BINDING_THRESHOLD_MASK))
        return true;

    return false;
}

void Binding::clear()
{
    value = 0;
}

unsigned int Binding::hex()
{
    return value;
}

unsigned int Binding::base_hex()
{
    return (value & ~BINDING_THRESHOLD_MASK);
}

bool Binding::is_joy()
{
    return (value & BINDING_JOY);
}

bool Binding::is_key()
{
    return (value & BINDING_KEY);
}

unsigned int Binding::get_key()
{
    return (value & BINDING_KEY_MASK);
}

unsigned int Binding::get_device()
{
    return JOY_DEVICE_UNMASK(value);
}

unsigned int Binding::get_threshold()
{
    return THRESHOLD_UNMASK(value);
}

unsigned int Binding::get_axis()
{
    return JOY_AXIS_UNMASK(value);
}

Gdk::ModifierType Binding::get_gdk_modifiers()
{
    return (Gdk::ModifierType)(((BINDING_CTRL & value)  ? Gdk::CONTROL_MASK : 0) |
                               ((BINDING_ALT & value)   ? Gdk::MOD1_MASK    : 0) |
                               ((BINDING_SHIFT & value) ? Gdk::SHIFT_MASK   : 0));
}

bool Binding::is_positive()
{
    return JOY_DIRECTION_UNMASK(value) == AXIS_POS;
}

bool Binding::is_negative()
{
    return JOY_DIRECTION_UNMASK(value) == AXIS_NEG;
}

Binding::Binding(const char *raw_string)
{
    value = 0;

    if (!raw_string)
        return;

    char substr[80];
    if (sscanf(raw_string, "Keyboard %79s", substr) == 1)
    {
        bool ctrl = false;
        bool shift = false;
        bool alt = false;
        bool direct = false;
        unsigned int keyval = 0;
        char *key;

        if (!strchr(substr, '+'))
            direct = true;

        key = strtok(substr, "+");
        while (key)
        {
            if (strstr(key, "Alt") && !direct)
                alt = true;
            else if (strstr(key, "Ctrl") && !direct)
                ctrl = true;
            else if (strstr(key, "Shift") && !direct)
                shift = true;
            else
            {
                keyval = gdk_keyval_from_name(key);
            }

            key = strtok(NULL, "+");
        }

        if (keyval != GDK_KEY_VoidSymbol)
            value = Binding(keyval, ctrl, shift, alt).value;
        else
            value = 0;
    }
    else if (!strncmp(raw_string, "Joystick", 8))
    {
        unsigned int axis;
        unsigned int button;
        unsigned int percent;
        unsigned int device;
        char posneg;
        const char *substr = &raw_string[8];

        if (sscanf(substr, "%u Axis %u %c %u", &device, &axis, &posneg, &percent) == 4)
        {
            value = Binding(device - 1, JOY_AXIS(axis, posneg == '+' ? AXIS_POS : AXIS_NEG), percent).value;
        }
        else if (sscanf(substr, "%u Button %u", &device, &button) == 2)
        {
            value = Binding(device - 1, button, 0).value;
        }
    }
}

std::string Binding::as_string()
{
    return to_string(false);
}

std::string Binding::to_string(bool translate)
{
    std::string str;

#undef maybegettext
#define maybegettext(String) translate ? gettext(String) : (String)

    if (is_key())
    {
        unsigned int keyval = gdk_keyval_to_lower(get_key());
        char *keyval_name = gdk_keyval_name(keyval);

        if (keyval_name == nullptr)
            str = maybegettext("Unknown");
        else
            str = keyval_name;

        if (translate)
            for (char &c : str)
                if (c == '_')
                    c = ' ';

        str = fmt::format(maybegettext("Keyboard {}{}{}{}"),
                          (value & BINDING_SHIFT) ? "Shift+" : "",
                          (value & BINDING_CTRL) ? "Ctrl+" : "",
                          (value & BINDING_ALT) ? "Alt+" : "",
                          str);
    }

    else if (is_joy())
    {
        if ((get_key()) >= 512)
            str = fmt::format(maybegettext("Axis {} {} {}%"),
                              get_axis(),
                              is_positive() ? "+" : "-",
                              get_threshold());
        else
            str = fmt::format(maybegettext("Button {}"), get_key());

        str = fmt::format(maybegettext("Joystick {} {}"), get_device(), str);
    }

    else
        str = maybegettext("Unset");

    return str;
}
