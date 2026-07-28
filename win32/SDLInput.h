/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef SDLINPUT_H
#define SDLINPUT_H

#include "SDL3/SDL.h"
#include <map>
#include <vector>
#include <string>

struct SJoypad;

struct SDLDeviceInfo {
    SDL_Joystick *joystick = nullptr;
    SDL_Gamepad *gamepad = nullptr;
    SDL_JoystickID instance_id = 0;
    int slot = -1;
    bool is_gamepad = false;
    int num_axes = 0;
    int num_buttons = 0;
    int num_hats = 0;
    std::string name;

    struct AxisState {
        int16_t initial = 0;
        int16_t last = 0;
    };
    std::vector<AxisState> axes;
};

// Initialize SDL joystick/gamepad subsystem. Call once at startup.
void SDLInput_Init();

// Shutdown SDL joystick/gamepad subsystem. Call at exit.
void SDLInput_Shutdown();

// Detect and open all currently connected joystick devices.
// Populates the global Joystick[] array slots.
void SDLInput_DetectJoypads();

// Poll SDL events and update the global Joystick[] state array.
// Call once per frame in place of the old WinMM polling loop.
void SDLInput_Poll();

// Poll for joystick state changes in the binding capture dialog.
// Calls JoystickChanged(ID, controlCode) on transitions.
void SDLInput_PollForBinding();

// Re-initialize the binding capture state (replaces InitJoystickStruct).
void SDLInput_InitBindingState();

// Get the human-readable device name for a Joystick[] slot.
std::string SDLInput_GetDeviceName(int slot);

struct SDLDeviceListEntry {
    int slot = -1;
    bool is_gamepad = false;
    std::string name;
};

// List all attached devices, sorted by Joystick[] slot.
std::vector<SDLDeviceListEntry> SDLInput_GetDeviceList();

// Check if the device at a Joystick[] slot is a recognized gamepad.
bool SDLInput_IsGamepad(int slot);

// Auto-map a recognized gamepad's buttons to SNES joypad bindings.
// Returns true if the device is a gamepad and mapping was successful.
bool SDLInput_AutoMapGamepad(int slot, SJoypad &out);

// Force SDL to process any pending device-change messages and drain events.
// Call from the main window's WM_DEVICECHANGE handler for instant hot-plug response.
void SDLInput_OnDeviceChange();

#endif // SDLINPUT_H
