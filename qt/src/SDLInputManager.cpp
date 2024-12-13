#include "SDLInputManager.hpp"

#include <algorithm>
#include <optional>

SDLInputManager::SDLInputManager()
{
    SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK);
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

    printf("Slot %d: %s: ", d.index, SDL_GetJoystickName(d.joystick));
    printf("%zu axes, %zu buttons, %zu hats, %s API\n", d.axes.size(), d.buttons.size(), d.hats.size(), d.is_gamepad ? "Controller" : "Joystick");

    devices.insert({ d.instance_id, d });
}

void SDLInputManager::RemoveDevice(int instance_id)
{
    auto iter = devices.find(instance_id);
    if (iter == devices.end())
        return;

    auto &d = iter->second;

    if (d.is_gamepad)
        SDL_CloseGamepad(d.gamepad);
    else
        SDL_CloseJoystick(d.joystick);

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
        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
            return event;
        case SDL_EVENT_JOYSTICK_HAT_MOTION:
            return event;
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            return event;
        case SDL_EVENT_JOYSTICK_ADDED:
            AddDevice(event.jdevice.which);
            return event;
        case SDL_EVENT_JOYSTICK_REMOVED:
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
        printf("%s: \n", SDL_GetJoystickName(d.joystick));
        printf(" Index: %d\n"
               " Instance ID: %d\n"
               " Controller %s\n"
               " SDL Joystick Number: %d\n",
               d.index,
               d.instance_id,
               d.is_gamepad ? "yes" : "no",
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
    is_gamepad = SDL_IsGamepad(joystick_num);

    if (is_gamepad)
    {
        gamepad = SDL_OpenGamepad(joystick_num);
        joystick = SDL_GetGamepadJoystick(gamepad);
    }
    else
    {
        joystick = SDL_OpenJoystick(joystick_num);
        gamepad = nullptr;
    }

    if (!joystick)
        return false;

    auto num_axes = SDL_GetNumJoystickAxes(joystick);
    axes.resize(num_axes);
    for (int i = 0; i < num_axes; i++)
    {
        SDL_GetJoystickAxisInitialState(joystick, i, &axes[i].initial);
        axes[i].last = axes[i].initial;
    }

    buttons.resize(SDL_GetNumJoystickButtons(joystick));
    hats.resize(SDL_GetNumJoystickHats(joystick));
    instance_id = SDL_GetJoystickID(joystick);

    return true;
}

std::vector<std::pair<int, std::string>> SDLInputManager::getXInputControllers()
{
    std::vector<std::pair<int, std::string>> list;

    for (auto &d : devices)
    {
        if (!d.second.is_gamepad)
            continue;

        list.push_back(std::pair<int, std::string>(d.first, SDL_GetJoystickName(d.second.joystick)));
        auto bind = SDL_GetGamepadButton(d.second.gamepad, SDL_GAMEPAD_BUTTON_EAST);
    }

    return list;
}
