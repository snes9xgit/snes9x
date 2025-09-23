#pragma once

#include "SDL3/SDL.h"
#include <map>
#include <vector>
#include <string>
#include <optional>

struct SDLInputDevice
{
    bool open(int joystick_num);

    int index;
    int sdl_joystick_number;
    bool is_gamepad;
    SDL_Gamepad *gamepad = nullptr;
    SDL_Joystick *joystick = nullptr;
    SDL_JoystickID instance_id = 0;
    int num_buttons;
    int num_axes;
    int num_hats;

    struct Axis
    {
        int16_t initial;
        int last;
    };
    std::vector<Axis> axes;

    struct Hat
    {
        uint8_t state;
    };
    std::vector<Hat> hats;
};

struct SDLInputManager
{
    SDLInputManager();
    ~SDLInputManager();

    std::optional<SDL_Event> processEvent();
    std::vector<std::pair<int, std::string>> getXInputControllers();
    void clearEvents();
    void addDevice(int i);
    void removeDevice(int i);
    void printDevices();
    int findFirstOpenIndex();
    static std::map<std::pair<int, int>, SDL_GamepadBinding> getXInputButtonBindings(SDL_Gamepad *gamepad);

    struct DiscreteAxisEvent
    {
        int joystick_num;
        int axis;
        int direction;
        int pressed;
    };
    std::vector<DiscreteAxisEvent> discretizeJoyAxisEvent(SDL_Event &event, int threshold_percent = 33);

    struct DiscreteHatEvent
    {
        int joystick_num;
        int hat;
        int direction;
        bool pressed;
    };
    std::vector<DiscreteHatEvent> discretizeHatEvent(SDL_Event &event);

    std::map<SDL_JoystickID, SDLInputDevice> devices;
};
