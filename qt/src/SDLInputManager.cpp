#include "SDLInputManager.hpp"
#include "SDL.h"
#include "SDL_events.h"
#include "SDL_gamecontroller.h"
#include "SDL_joystick.h"

#include <algorithm>
#include <optional>

SDLInputManager::SDLInputManager()
{
    SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
}

SDLInputManager::~SDLInputManager()
{
}

void SDLInputManager::AddDevice(int device_index)
{
    SDLInputDevice d;
    if (!d.open(device_index))
        return;
    d.index = FindFirstOpenIndex();

    printf("Slot %d: %s: ", d.index, SDL_JoystickName(d.joystick));
    printf("%zu axes, %zu buttons, %zu hats, %s API\n", d.axes.size(), d.buttons.size(), d.hats.size(), d.is_controller ? "Controller" : "Joystick");

    devices.insert({ d.instance_id, d });
}

void SDLInputManager::RemoveDevice(int instance_id)
{
    auto iter = devices.find(instance_id);
    if (iter == devices.end())
        return;

    auto &d = iter->second;

    if (d.is_controller)
        SDL_GameControllerClose(d.controller);
    else
        SDL_JoystickClose(d.joystick);

    devices.erase(iter);
    return;
}

void SDLInputManager::ClearEvents()
{
    std::optional<SDL_Event> event;

    while ((event = ProcessEvent()))
    {
    }
}


std::optional<SDLInputManager::DiscreteHatEvent>
SDLInputManager::DiscretizeHatEvent(SDL_Event &event)
{
    auto &device = devices.at(event.jhat.which);
    auto &hat = event.jhat.hat;
    auto new_state = event.jhat.value;
    auto &old_state = device.hats[hat].state;

    if (old_state == new_state)
        return std::nullopt;

    DiscreteHatEvent dhe{};
    dhe.hat = hat;
    dhe.joystick_num = device.index;

    for (auto &s : { SDL_HAT_UP, SDL_HAT_DOWN, SDL_HAT_LEFT, SDL_HAT_RIGHT })
        if ((old_state & s) != (new_state & s))
        {
            dhe.direction = s;
            dhe.pressed = (new_state & s);
            old_state = new_state;
            return dhe;
        }

    return std::nullopt;
}

std::optional<SDLInputManager::DiscreteAxisEvent>
SDLInputManager::DiscretizeJoyAxisEvent(SDL_Event &event)
{
    auto &device = devices.at(event.jaxis.which);
    auto &axis = event.jaxis.axis;
    auto now = event.jaxis.value;
    auto &then = device.axes[axis].last;
    auto center = device.axes[axis].initial;

    int offset = now - center;

    auto pressed = [&](int axis) -> int {
        if (axis > (center + (32767 - center) / 3)) // TODO threshold
            return 1;
        if (axis < (center - (center + 32768) / 3)) // TODO threshold
            return -1;
        return 0;
    };

    auto was_pressed_then = pressed(then);
    auto is_pressed_now   = pressed(now);

    if (was_pressed_then == is_pressed_now)
    {
        then = now;
        return std::nullopt;
    }

    DiscreteAxisEvent dae;
    dae.axis = axis;
    dae.direction = is_pressed_now ? is_pressed_now : was_pressed_then;
    dae.pressed = (is_pressed_now != 0);
    dae.joystick_num = device.index;
    then = now;

    return dae;
}

std::optional<SDL_Event> SDLInputManager::ProcessEvent()
{
    SDL_Event event{};

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_JOYAXISMOTION:
            return event;
        case SDL_JOYHATMOTION:
            return event;
        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN:
            return event;
        case SDL_JOYDEVICEADDED:
            AddDevice(event.jdevice.which);
            return event;
        case SDL_JOYDEVICEREMOVED:
            RemoveDevice(event.jdevice.which);
            return event;
        }
    }

    return std::nullopt;
}

void SDLInputManager::PrintDevices()
{
    for (auto &pair : devices)
    {
        auto &d = pair.second;
        printf("%s: \n", SDL_JoystickName(d.joystick));
        printf(" Index: %d\n"
               " Instance ID: %d\n"
               " Controller %s\n"
               " SDL Joystick Number: %d\n",
               d.index,
               d.instance_id,
               d.is_controller ? "yes" : "no",
               d.sdl_joystick_number);
    }
}

int SDLInputManager::FindFirstOpenIndex()
{
    for (int i = 0;; i++)
    {
        if (std::none_of(devices.begin(), devices.end(), [i](auto &d) -> bool {
            return (d.second.index == i);
        }))
            return i;
    }
    return -1;
}

bool SDLInputDevice::open(int joystick_num)
{
    sdl_joystick_number = joystick_num;
    is_controller = SDL_IsGameController(joystick_num);

    if (is_controller)
    {
        controller = SDL_GameControllerOpen(joystick_num);
        joystick = SDL_GameControllerGetJoystick(controller);
    }
    else
    {
        joystick = SDL_JoystickOpen(joystick_num);
        controller = nullptr;
    }

    if (!joystick)
        return false;

    auto num_axes = SDL_JoystickNumAxes(joystick);
    axes.resize(num_axes);
    for (int i = 0; i < num_axes; i++)
    {
        SDL_JoystickGetAxisInitialState(joystick, i, &axes[i].initial);
        axes[i].last = axes[i].initial;
    }

    buttons.resize(SDL_JoystickNumButtons(joystick));
    hats.resize(SDL_JoystickNumHats(joystick));
    instance_id = SDL_JoystickInstanceID(joystick);

    return true;
}

std::vector<std::pair<int, std::string>> SDLInputManager::getXInputControllers()
{
    std::vector<std::pair<int, std::string>> list;

    for (auto &d : devices)
    {
        if (!d.second.is_controller)
            continue;

        list.push_back(std::pair<int, std::string>(d.first, SDL_JoystickName(d.second.joystick)));
        auto bind = SDL_GameControllerGetBindForButton(d.second.controller, SDL_CONTROLLER_BUTTON_A);
    }

    return list;
}
