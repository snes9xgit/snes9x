/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "SDLInput.h"
#include "wsnes9x.h"
#include <algorithm>
#include <cstring>

// The global Joystick[] array is defined in win32.cpp
extern struct SJoyState Joystick[16];
// JoystickChanged is defined in InputCustom.cpp (binding capture callback)
extern void JoystickChanged(short ID, short Movement);

// SDL devices tracked by instance ID, mapped to Joystick[] slots
static std::map<SDL_JoystickID, SDLDeviceInfo> s_devices;
static bool s_sdl_initialized = false;

// Binding capture previous state (mirrors JoystickF[] for transition detection)
struct BindingAxisState {
    bool negative = false;
    bool positive = false;
};
static struct BindingDeviceState {
    bool button[32] = {};
    BindingAxisState axis[6] = {};
    bool povUp = false, povDown = false, povLeft = false, povRight = false;
    bool povUpLeft = false, povUpRight = false, povDnLeft = false, povDnRight = false;
} s_bindState[16];

static bool IsSlotInUse(int slot)
{
    for (auto &pair : s_devices)
        if (pair.second.slot == slot)
            return true;
    return false;
}

// A slot is reserved if its saved GUID belongs to a different (maybe unplugged) device.
static bool IsSlotReservedForOther(int slot, const char *guidStr)
{
    return slot < 8 && GUI.JoypadGUID[slot][0] != '\0' &&
           _stricmp(GUI.JoypadGUID[slot], guidStr) != 0;
}

static int FindFirstFreeSlot(const char *guidStr)
{
    // prefer unreserved slots so a new device can't hijack a saved controller's
    // slot (and its bindings/hotkeys) just by enumerating first
    for (int i = 0; i < 16; i++)
        if (!IsSlotInUse(i) && !IsSlotReservedForOther(i, guidStr))
            return i;
    for (int i = 0; i < 16; i++)
        if (!IsSlotInUse(i))
            return i;
    return -1;
}

// Find a slot whose saved GUID matches this device's GUID.
// Returns -1 if no match found.
static int FindSlotByGUID(const char *guidStr)
{
    if (!guidStr || guidStr[0] == '\0')
        return -1;

    for (int i = 0; i < 8; i++)
    {
        if (GUI.JoypadGUID[i][0] != '\0' &&
            _stricmp(GUI.JoypadGUID[i], guidStr) == 0 &&
            !IsSlotInUse(i))
        {
            return i;
        }
    }
    return -1;
}

static SDLDeviceInfo *FindDeviceBySlot(int slot)
{
    for (auto &pair : s_devices)
    {
        if (pair.second.slot == slot)
            return &pair.second;
    }
    return nullptr;
}

static void OpenDevice(int device_index)
{
    SDLDeviceInfo dev;

    dev.is_gamepad = SDL_IsGamepad(device_index);

    if (dev.is_gamepad)
    {
        dev.gamepad = SDL_OpenGamepad(device_index);
        if (!dev.gamepad)
            return;
        dev.joystick = SDL_GetGamepadJoystick(dev.gamepad);
    }
    else
    {
        dev.joystick = SDL_OpenJoystick(device_index);
        if (!dev.joystick)
            return;
    }

    dev.instance_id = SDL_GetJoystickID(dev.joystick);
    dev.num_axes = SDL_GetNumJoystickAxes(dev.joystick);
    dev.num_buttons = SDL_GetNumJoystickButtons(dev.joystick);
    dev.num_hats = SDL_GetNumJoystickHats(dev.joystick);

    const char *jname = SDL_GetJoystickName(dev.joystick);
    dev.name = jname ? jname : "Unknown Controller";

    // Store initial axis positions for deadzone centering
    dev.axes.resize(dev.num_axes);
    for (int i = 0; i < dev.num_axes; i++)
    {
        SDL_GetJoystickAxisInitialState(dev.joystick, i, &dev.axes[i].initial);
        dev.axes[i].last = dev.axes[i].initial;
    }

    // Get the device GUID as a hex string for identity matching
    SDL_GUID guid = SDL_GetJoystickGUID(dev.joystick);
    char guidStr[64] = {};
    SDL_GUIDToString(guid, guidStr, sizeof(guidStr));

    // Try to match against saved GUIDs first (stable slot assignment)
    dev.slot = FindSlotByGUID(guidStr);

    // Fall back to first free slot if no GUID match
    if (dev.slot < 0)
        dev.slot = FindFirstFreeSlot(guidStr);

    if (dev.slot < 0)
    {
        if (dev.is_gamepad)
            SDL_CloseGamepad(dev.gamepad);
        else
            SDL_CloseJoystick(dev.joystick);
        return;
    }

    // Save this device's GUID to the slot for future sessions
    if (dev.slot < 8)
        strncpy(GUI.JoypadGUID[dev.slot], guidStr, 63);

    Joystick[dev.slot].Attached = true;
    s_devices[dev.instance_id] = dev;
}

// Helper: check if a WORD binding references a specific joystick slot
static bool IsJoyBinding(WORD key, int joySlot)
{
    return (key & 0x8000) && (((key >> 8) & 0xF) == joySlot);
}

// Remove joystick bindings for a disconnected slot from a joypad's config.
// In multi-bind mode, promotes keyboard extras to primary.
static void CleanupBindingsForSlot(int joySlot)
{
    bool isMulti = GUI.MultiBindingMode;

    for (int pad = 0; pad < 16; pad++)
    {
        // Process each button field with a macro
        #define CLEANUP(field) do { \
            if (IsJoyBinding(Joypad[pad].field, joySlot)) { \
                Joypad[pad].field = 0; \
                if (isMulti) { \
                    /* Promote first keyboard extra to primary */ \
                    for (int _e = 0; _e < MAX_EXTRA_BINDS; _e++) { \
                        if (JoypadExtra[pad].field[_e] != 0 && \
                            JoypadExtra[pad].field[_e] != VK_ESCAPE && \
                            !(JoypadExtra[pad].field[_e] & 0x8000)) { \
                            Joypad[pad].field = JoypadExtra[pad].field[_e]; \
                            /* Shift remaining extras up */ \
                            for (int _j = _e; _j < MAX_EXTRA_BINDS - 1; _j++) \
                                JoypadExtra[pad].field[_j] = JoypadExtra[pad].field[_j+1]; \
                            JoypadExtra[pad].field[MAX_EXTRA_BINDS-1] = 0; \
                            break; \
                        } \
                    } \
                } \
            } \
            /* Also remove joystick extras for this slot */ \
            for (int _e = 0; _e < MAX_EXTRA_BINDS; _e++) { \
                if (IsJoyBinding(JoypadExtra[pad].field[_e], joySlot)) { \
                    JoypadExtra[pad].field[_e] = 0; \
                    /* Compact: shift remaining up */ \
                    for (int _j = _e; _j < MAX_EXTRA_BINDS - 1; _j++) \
                        JoypadExtra[pad].field[_j] = JoypadExtra[pad].field[_j+1]; \
                    JoypadExtra[pad].field[MAX_EXTRA_BINDS-1] = 0; \
                    _e--; /* re-check this slot */ \
                } \
            } \
        } while(0)

        CLEANUP(Up); CLEANUP(Down); CLEANUP(Left); CLEANUP(Right);
        CLEANUP(A); CLEANUP(B); CLEANUP(X); CLEANUP(Y);
        CLEANUP(L); CLEANUP(R); CLEANUP(Start); CLEANUP(Select);
        CLEANUP(Left_Up); CLEANUP(Left_Down);
        CLEANUP(Right_Up); CLEANUP(Right_Down);
        #undef CLEANUP
    }
}

static void CloseDevice(SDL_JoystickID instance_id)
{
    auto it = s_devices.find(instance_id);
    if (it == s_devices.end())
        return;

    auto &dev = it->second;
    if (dev.slot >= 0 && dev.slot < 16)
    {
        // Clean up bindings that reference this joystick slot
        CleanupBindingsForSlot(dev.slot);

        Joystick[dev.slot].Attached = false;
        memset(&Joystick[dev.slot].Left, 0,
               sizeof(SJoyState) - offsetof(SJoyState, Left));
    }

    if (dev.is_gamepad)
        SDL_CloseGamepad(dev.gamepad);
    else
        SDL_CloseJoystick(dev.joystick);

    s_devices.erase(it);
}

// Discretize an SDL axis value to a direction (-1, 0, +1) using threshold
static int AxisDirection(int16_t value, int16_t center, int threshold_percent)
{
    // threshold_percent of the range from center to edge
    if (value > (center + (32767 - center) * threshold_percent / 100))
        return 1;
    if (value < (center - (center + 32768) * threshold_percent / 100))
        return -1;
    return 0;
}

void SDLInput_Init()
{
    if (s_sdl_initialized)
        return;

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    // Some HID devices (e.g. Gamesir G7 Pro "HID-compliant vendor-defined device"
    // under HID\VID_3537) cause SDL_PollEvent to stall 70-100 ms on USB hotplug
    // when DirectInput is active. Let users disable the DI backend as a workaround.
    if (!GUI.UseDirectInput)
        SDL_SetHint(SDL_HINT_JOYSTICK_DIRECTINPUT, "0");
    SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK);
    s_sdl_initialized = true;
}

void SDLInput_OnDeviceChange()
{
    if (!s_sdl_initialized)
        return;

    // Force SDL to run its internal message pump so queued WM_DEVICECHANGE
    // notifications on SDL's hidden message window are processed now rather
    // than on the next timed poll. Then drain the resulting events.
    SDL_PumpEvents();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_JOYSTICK_ADDED)
            OpenDevice(event.jdevice.which);
        else if (event.type == SDL_EVENT_JOYSTICK_REMOVED)
            CloseDevice(event.jdevice.which);
    }
}

void SDLInput_Shutdown()
{
    if (!s_sdl_initialized)
        return;

    // Close all open devices
    for (auto &pair : s_devices)
    {
        auto &dev = pair.second;
        if (dev.is_gamepad)
            SDL_CloseGamepad(dev.gamepad);
        else
            SDL_CloseJoystick(dev.joystick);
    }
    s_devices.clear();

    SDL_QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK);
    s_sdl_initialized = false;
}

void SDLInput_DetectJoypads()
{
    // Process any pending add/remove events first
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_JOYSTICK_ADDED)
            OpenDevice(event.jdevice.which);
        else if (event.type == SDL_EVENT_JOYSTICK_REMOVED)
            CloseDevice(event.jdevice.which);
    }

    // If no devices were found via events (first call), enumerate
    if (s_devices.empty())
    {
        int count = 0;
        SDL_JoystickID *joysticks = SDL_GetJoysticks(&count);
        if (joysticks)
        {
            for (int i = 0; i < count; i++)
                OpenDevice(joysticks[i]);
            SDL_free(joysticks);
        }
    }
}

void SDLInput_Poll()
{
    // Process SDL events for hot-plug
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_JOYSTICK_ADDED:
            OpenDevice(event.jdevice.which);
            break;
        case SDL_EVENT_JOYSTICK_REMOVED:
            CloseDevice(event.jdevice.which);
            break;
        default:
            break;
        }
    }

    // Use the threshold matching existing S9X_JOY_NEUTRAL (60 out of 100)
    const int threshold_percent = S9X_JOY_NEUTRAL;

    // Update Joystick[] state for each attached device
    for (auto &pair : s_devices)
    {
        auto &dev = pair.second;
        int slot = dev.slot;
        if (slot < 0 || slot >= 16)
            continue;

        auto &js = Joystick[slot];

        // Axes: axis 0 = X (Left/Right), axis 1 = Y (Up/Down)
        // axis 2 = Z, axis 3 = R, axis 4 = U, axis 5 = V
        for (int a = 0; a < dev.num_axes && a < 6; a++)
        {
            int16_t val = SDL_GetJoystickAxis(dev.joystick, a);
            int dir = AxisDirection(val, dev.axes[a].initial, threshold_percent);
            dev.axes[a].last = val;

            switch (a)
            {
            case 0: // X axis
                js.Left  = (dir == -1);
                js.Right = (dir ==  1);
                break;
            case 1: // Y axis
                js.Up   = (dir == -1);
                js.Down = (dir ==  1);
                break;
            case 2: // Z axis
                js.ZUp   = (dir == -1);
                js.ZDown = (dir ==  1);
                break;
            case 3: // R axis
                js.RUp   = (dir == -1);
                js.RDown = (dir ==  1);
                break;
            case 4: // U axis
                js.UUp   = (dir == -1);
                js.UDown = (dir ==  1);
                break;
            case 5: // V axis
                js.VUp   = (dir == -1);
                js.VDown = (dir ==  1);
                break;
            }
        }

        // Hat 0 -> POV directions
        if (dev.num_hats > 0)
        {
            uint8_t hat = SDL_GetJoystickHat(dev.joystick, 0);
            js.PovUp      = (hat & SDL_HAT_UP)    != 0;
            js.PovDown    = (hat & SDL_HAT_DOWN)  != 0;
            js.PovLeft    = (hat & SDL_HAT_LEFT)  != 0;
            js.PovRight   = (hat & SDL_HAT_RIGHT) != 0;

            // Diagonal combinations
            js.PovUpLeft  = (hat & (SDL_HAT_UP   | SDL_HAT_LEFT))  == (SDL_HAT_UP   | SDL_HAT_LEFT);
            js.PovUpRight = (hat & (SDL_HAT_UP   | SDL_HAT_RIGHT)) == (SDL_HAT_UP   | SDL_HAT_RIGHT);
            js.PovDnLeft  = (hat & (SDL_HAT_DOWN | SDL_HAT_LEFT))  == (SDL_HAT_DOWN | SDL_HAT_LEFT);
            js.PovDnRight = (hat & (SDL_HAT_DOWN | SDL_HAT_RIGHT)) == (SDL_HAT_DOWN | SDL_HAT_RIGHT);
        }
        else
        {
            js.PovUp = js.PovDown = js.PovLeft = js.PovRight = false;
            js.PovUpLeft = js.PovUpRight = js.PovDnLeft = js.PovDnRight = false;
        }

        // Buttons 0-31
        int maxBtn = (dev.num_buttons < 32) ? dev.num_buttons : 32;
        for (int b = 0; b < maxBtn; b++)
            js.Button[b] = SDL_GetJoystickButton(dev.joystick, b) != 0;
        for (int b = maxBtn; b < 32; b++)
            js.Button[b] = false;
    }
}

// Control codes match S9xGetState / TranslateKey conventions:
//   0=Left, 1=Right, 2=Up, 3=Down (stick axes)
//   4=PovLeft, 5=PovRight, 6=PovUp, 7=PovDown
//   8..39 = Button 0..31
//   41=ZUp, 42=ZDown, 43=RUp, 44=RDown, 45=UUp, 46=UDown, 47=VUp, 48=VDown
//   49=PovDnLeft, 50=PovDnRight, 51=PovUpLeft, 52=PovUpRight

void SDLInput_PollForBinding()
{
    // Process events for hot-plug during binding capture
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_JOYSTICK_ADDED)
            OpenDevice(event.jdevice.which);
        else if (event.type == SDL_EVENT_JOYSTICK_REMOVED)
            CloseDevice(event.jdevice.which);
    }

    const int threshold_percent = S9X_JOY_NEUTRAL;

    for (auto &pair : s_devices)
    {
        auto &dev = pair.second;
        int slot = dev.slot;
        if (slot < 0 || slot >= 16)
            continue;

        auto &bs = s_bindState[slot];

        // Check axes for transitions
        for (int a = 0; a < dev.num_axes && a < 6; a++)
        {
            int16_t val = SDL_GetJoystickAxis(dev.joystick, a);
            int dir = AxisDirection(val, dev.axes[a].initial, threshold_percent);

            bool neg = (dir == -1);
            bool pos = (dir == 1);

            // Control codes for each axis pair
            short negCode, posCode;
            switch (a)
            {
            case 0: negCode = 0;  posCode = 1;  break; // X: Left/Right
            case 1: negCode = 2;  posCode = 3;  break; // Y: Up/Down
            case 2: negCode = 41; posCode = 42; break; // Z
            case 3: negCode = 43; posCode = 44; break; // R
            case 4: negCode = 45; posCode = 46; break; // U
            case 5: negCode = 47; posCode = 48; break; // V
            default: continue;
            }

            if (neg && !bs.axis[a].negative)
                JoystickChanged((short)slot, negCode);
            bs.axis[a].negative = neg;

            if (pos && !bs.axis[a].positive)
                JoystickChanged((short)slot, posCode);
            bs.axis[a].positive = pos;
        }

        // Check hat 0
        if (dev.num_hats > 0)
        {
            uint8_t hat = SDL_GetJoystickHat(dev.joystick, 0);
            bool up    = (hat & SDL_HAT_UP)    != 0;
            bool down  = (hat & SDL_HAT_DOWN)  != 0;
            bool left  = (hat & SDL_HAT_LEFT)  != 0;
            bool right = (hat & SDL_HAT_RIGHT) != 0;

            // Cardinal directions
            if (up && !bs.povUp)       JoystickChanged((short)slot, 6);
            if (down && !bs.povDown)   JoystickChanged((short)slot, 7);
            if (left && !bs.povLeft)   JoystickChanged((short)slot, 4);
            if (right && !bs.povRight) JoystickChanged((short)slot, 5);

            // Diagonals
            bool upLeft  = up && left;
            bool upRight = up && right;
            bool dnLeft  = down && left;
            bool dnRight = down && right;
            if (upLeft && !bs.povUpLeft)   JoystickChanged((short)slot, 51);
            if (upRight && !bs.povUpRight) JoystickChanged((short)slot, 52);
            if (dnLeft && !bs.povDnLeft)   JoystickChanged((short)slot, 49);
            if (dnRight && !bs.povDnRight) JoystickChanged((short)slot, 50);

            bs.povUp = up; bs.povDown = down;
            bs.povLeft = left; bs.povRight = right;
            bs.povUpLeft = upLeft; bs.povUpRight = upRight;
            bs.povDnLeft = dnLeft; bs.povDnRight = dnRight;
        }

        // Check buttons
        int maxBtn = (dev.num_buttons < 32) ? dev.num_buttons : 32;
        for (int b = 0; b < maxBtn; b++)
        {
            bool pressed = SDL_GetJoystickButton(dev.joystick, b) != 0;
            if (pressed && !bs.button[b])
                JoystickChanged((short)slot, (short)(8 + b));
            bs.button[b] = pressed;
        }
    }
}

void SDLInput_InitBindingState()
{
    // Reset all binding capture state
    memset(s_bindState, 0, sizeof(s_bindState));

    // Do one poll to capture current state so we don't
    // immediately trigger on existing button presses
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_JOYSTICK_ADDED)
            OpenDevice(event.jdevice.which);
        else if (event.type == SDL_EVENT_JOYSTICK_REMOVED)
            CloseDevice(event.jdevice.which);
    }

    const int threshold_percent = S9X_JOY_NEUTRAL;

    for (auto &pair : s_devices)
    {
        auto &dev = pair.second;
        int slot = dev.slot;
        if (slot < 0 || slot >= 16)
            continue;

        auto &bs = s_bindState[slot];

        for (int a = 0; a < dev.num_axes && a < 6; a++)
        {
            int16_t val = SDL_GetJoystickAxis(dev.joystick, a);
            int dir = AxisDirection(val, dev.axes[a].initial, threshold_percent);
            bs.axis[a].negative = (dir == -1);
            bs.axis[a].positive = (dir == 1);
        }

        if (dev.num_hats > 0)
        {
            uint8_t hat = SDL_GetJoystickHat(dev.joystick, 0);
            bs.povUp    = (hat & SDL_HAT_UP)    != 0;
            bs.povDown  = (hat & SDL_HAT_DOWN)  != 0;
            bs.povLeft  = (hat & SDL_HAT_LEFT)  != 0;
            bs.povRight = (hat & SDL_HAT_RIGHT) != 0;
            bs.povUpLeft  = bs.povUp && bs.povLeft;
            bs.povUpRight = bs.povUp && bs.povRight;
            bs.povDnLeft  = bs.povDown && bs.povLeft;
            bs.povDnRight = bs.povDown && bs.povRight;
        }

        int maxBtn = (dev.num_buttons < 32) ? dev.num_buttons : 32;
        for (int b = 0; b < maxBtn; b++)
            bs.button[b] = SDL_GetJoystickButton(dev.joystick, b) != 0;
    }
}

std::string SDLInput_GetDeviceName(int slot)
{
    auto *dev = FindDeviceBySlot(slot);
    if (!dev)
        return "";
    return dev->name;
}

std::vector<SDLDeviceListEntry> SDLInput_GetDeviceList()
{
    std::vector<SDLDeviceListEntry> list;
    for (auto &pair : s_devices)
    {
        auto &dev = pair.second;
        if (dev.slot < 0)
            continue;
        list.push_back({dev.slot, dev.is_gamepad, dev.name});
    }
    std::sort(list.begin(), list.end(),
              [](const SDLDeviceListEntry &a, const SDLDeviceListEntry &b) { return a.slot < b.slot; });
    return list;
}

bool SDLInput_IsGamepad(int slot)
{
    auto *dev = FindDeviceBySlot(slot);
    if (!dev)
        return false;
    return dev->is_gamepad;
}

bool SDLInput_AutoMapGamepad(int slot, SJoypad &out)
{
    auto *dev = FindDeviceBySlot(slot);
    if (!dev || !dev->is_gamepad || !dev->gamepad)
        return false;

    int num_bindings = 0;
    auto sdl_bindings = SDL_GetGamepadBindings(dev->gamepad, &num_bindings);
    if (!sdl_bindings)
        return false;

    // Build a lookup map: (output_type, output_button) -> binding
    std::map<std::pair<int, int>, SDL_GamepadBinding> bmap;
    for (int i = 0; i < num_bindings; i++)
        bmap[{sdl_bindings[i]->output_type, sdl_bindings[i]->output.button}] = *sdl_bindings[i];
    SDL_free(sdl_bindings);

    // Helper to convert an SDL binding to our WORD joystick key format
    auto bindingToWord = [slot](const SDL_GamepadBinding &b) -> WORD {
        WORD key = 0x8000 | ((slot & 0xF) << 8);
        if (b.input_type == SDL_GAMEPAD_BINDTYPE_BUTTON)
        {
            key |= (8 + b.input.button); // buttons start at code 8
        }
        else if (b.input_type == SDL_GAMEPAD_BINDTYPE_HAT)
        {
            // Map hat direction to POV control codes
            switch (b.input.hat.hat_mask)
            {
            case SDL_HAT_UP:    key |= 6; break;
            case SDL_HAT_DOWN:  key |= 7; break;
            case SDL_HAT_LEFT:  key |= 4; break;
            case SDL_HAT_RIGHT: key |= 5; break;
            default:            return 0;
            }
        }
        else if (b.input_type == SDL_GAMEPAD_BINDTYPE_AXIS)
        {
            // Map axis to directional control codes
            int axis = b.input.axis.axis;
            // axis_min < axis_max means positive direction, else negative
            bool positive = (b.input.axis.axis_min < b.input.axis.axis_max);
            switch (axis)
            {
            case 0: key |= positive ? 1 : 0; break;  // X: Right/Left
            case 1: key |= positive ? 3 : 2; break;  // Y: Down/Up
            case 2: key |= positive ? 42 : 41; break; // Z
            case 3: key |= positive ? 44 : 43; break; // R
            case 4: key |= positive ? 46 : 45; break; // U
            case 5: key |= positive ? 48 : 47; break; // V
            default: return 0;
            }
        }
        return key;
    };

    auto getBinding = [&](SDL_GamepadButton btn) -> WORD {
        auto it = bmap.find({SDL_GAMEPAD_BINDTYPE_BUTTON, btn});
        if (it == bmap.end())
            return 0;
        return bindingToWord(it->second);
    };

    // Map SNES buttons to SDL gamepad buttons
    // Note: SNES and Xbox have swapped A/B and X/Y positions
    out.Up     = getBinding(SDL_GAMEPAD_BUTTON_DPAD_UP);
    out.Down   = getBinding(SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    out.Left   = getBinding(SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    out.Right  = getBinding(SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    out.A      = getBinding(SDL_GAMEPAD_BUTTON_EAST);   // Xbox B = SNES A
    out.B      = getBinding(SDL_GAMEPAD_BUTTON_SOUTH);  // Xbox A = SNES B
    out.X      = getBinding(SDL_GAMEPAD_BUTTON_NORTH);  // Xbox Y = SNES X
    out.Y      = getBinding(SDL_GAMEPAD_BUTTON_WEST);   // Xbox X = SNES Y
    out.L      = getBinding(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    out.R      = getBinding(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    out.Start  = getBinding(SDL_GAMEPAD_BUTTON_START);
    out.Select = getBinding(SDL_GAMEPAD_BUTTON_BACK);
    out.Enabled = TRUE;

    // Diagonals not auto-mapped (they use combined hat directions)
    out.Left_Up    = 0;
    out.Left_Down  = 0;
    out.Right_Up   = 0;
    out.Right_Down = 0;

    return true;
}
