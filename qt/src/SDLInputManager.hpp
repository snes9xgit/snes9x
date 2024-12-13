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
    SDL_JoystickID instance_id;

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

    std::vector<bool> buttons;
};

struct SDLInputManager
{
    SDLInputManager();
    ~SDLInputManager();

    std::optional<SDL_Event> ProcessEvent();
    std::vector<std::pair<int, std::string>> getXInputControllers();
    void ClearEvents();
    void AddDevice(int i);
    void RemoveDevice(int i);
    void PrintDevices();
    int FindFirstOpenIndex();

    struct DiscreteAxisEvent
    {
        int joystick_num;
        int axis;
        int direction;
        int pressed;
    };
    std::optional<DiscreteAxisEvent> DiscretizeJoyAxisEvent(SDL_Event &event);

    struct DiscreteHatEvent
    {
        int joystick_num;
        int hat;
        int direction;
        bool pressed;
    };
    std::optional<DiscreteHatEvent> DiscretizeHatEvent(SDL_Event &event);

    std::map<SDL_JoystickID, SDLInputDevice> devices;
};
