#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#ifdef USE_GTK3
#include <gdk/gdkkeysyms-compat.h>
#endif

#include "gtk_s9x.h"
#include "gtk_binding.h"

Binding::Binding (void)
{
    value = 0;

    return;
}

Binding::Binding (GdkEventKey *event)
{
    /* Make keypress lower-case */
    if (event->keyval >= GDK_A && event->keyval <= GDK_Z)
        value = BINDING_KEY | ((event->keyval - GDK_A + GDK_a) & BINDING_KEY_MASK);
    else
        value = BINDING_KEY | (event->keyval & BINDING_KEY_MASK);

    /* Strip modifiers from modifiers */
    if (event->keyval == GDK_Control_L ||
        event->keyval == GDK_Control_R ||
        event->keyval == GDK_Shift_L   ||
        event->keyval == GDK_Shift_R   ||
        event->keyval == GDK_Alt_L     ||
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

    return;
}

Binding::Binding (unsigned int key, bool ctrl, bool shift, bool alt)
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

Binding::Binding (unsigned int device,
                  unsigned int button,
                  unsigned int threshold)
{
    value = BINDING_JOY;
    value |= JOY_DEVICE_MASK (device + 1);
    value |= BINDING_KEY_MASK & button;
    if (button >= 512)
        value |= THRESHOLD_MASK (threshold);
}

Binding::Binding (unsigned int val)
{
    value = val;

    return;
}

Binding::Binding (const Binding& binding)
{
    this->value = binding.value;
}

bool
Binding::matches (Binding &binding)
{
    if ((value & ~BINDING_THRESHOLD_MASK) ==
        (binding.value & ~BINDING_THRESHOLD_MASK) &&
        is_joy ())
        return true;

    return false;
}

void
Binding::clear (void)
{
    value = 0;

    return;
}

unsigned int
Binding::hex (void)
{
    return value;
}

unsigned int
Binding::base_hex (void)
{
    return (value & ~BINDING_THRESHOLD_MASK);
}

bool
Binding::is_joy (void)
{
    return (value & BINDING_JOY);
}

bool
Binding::is_key (void)
{
    return (value & BINDING_KEY);
}

unsigned int
Binding::get_key (void)
{
    return (value & BINDING_KEY_MASK);
}

unsigned int
Binding::get_device (void)
{
    return JOY_DEVICE_UNMASK (value);
}

unsigned int
Binding::get_threshold (void)
{
    return THRESHOLD_UNMASK (value);
}

unsigned int
Binding::get_axis (void)
{
    return JOY_AXIS_UNMASK (value);
}

GdkModifierType
Binding::get_gdk_modifiers (void)
{
    return (GdkModifierType) (((BINDING_CTRL  & value) ? GDK_CONTROL_MASK : 0) |
                              ((BINDING_ALT   & value) ? GDK_MOD1_MASK    : 0) |
                              ((BINDING_SHIFT & value) ? GDK_SHIFT_MASK   : 0));
}

bool
Binding::is_positive (void)
{
    return JOY_DIRECTION_UNMASK (value) == AXIS_POS;
}

bool
Binding::is_negative (void)
{
    return JOY_DIRECTION_UNMASK (value) == AXIS_NEG;
}

void
Binding::to_string (char *str)
{
    char buf[256];
    char *c;

    str[0] = '\0';

    if (is_key ())
    {
        char *keyval_name = NULL;
        unsigned int keyval = gdk_keyval_to_lower (get_key ());
        keyval_name = gdk_keyval_name (keyval);

        if (keyval_name == NULL)
        {
            sprintf (buf, _("Unknown"));
        }

        else
        {
            memset (buf, 0, 256);
            strncpy (buf,
                     keyval_name,
                     255);
        }

        while ((c = strstr (buf, "_")))
        {
            *c = ' ';
        }

        sprintf (str, _("Keyboard %s%s%s%s"),
                 (value & BINDING_SHIFT) ? "Shift+" : "",
                 (value & BINDING_CTRL)  ? "Ctrl+"  : "",
                 (value & BINDING_ALT)   ? "Alt+"   : "",
                 buf);
    }

    else if (is_joy ())
    {
        if ((get_key ()) >= 512)
            sprintf (buf,
                     _("Axis #%u %s %u%%"),
                     get_axis (),
                     is_positive () ? "+" : "-",
                     get_threshold ());
        else
            sprintf (buf, _("Button %u"), get_key ());

        sprintf (str, _("Joystick %u %s"), get_device (), buf);
    }

    else
    {
        sprintf (str, _("Unset"));
    }

    return;
}
