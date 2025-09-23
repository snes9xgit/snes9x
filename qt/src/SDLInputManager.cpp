#include "SDLInputManager.hpp"

#include <algorithm>
#include <optional>

SDLInputManager::SDLInputManager()
{
    SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK);
}

SDLInputManager::~SDLInputManager() = default;

void SDLInputManager::addDevice(int device_index)
{
    SDLInputDevice d;
    if (!d.open(device_index))
        return;
    d.index = findFirstOpenIndex();

    printf("Slot %d: %s: ", d.index, SDL_GetJoystickName(d.joystick));
    printf("%zu axes, %d buttons, %zu hats, %s API\n", d.axes.size(), d.num_buttons, d.hats.size(), d.is_gamepad ? "Controller" : "Joystick");

    devices.insert({ d.instance_id, d });
}

void SDLInputManager::removeDevice(int instance_id)
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
}

void SDLInputManager::clearEvents()
{
    std::optional<SDL_Event> event;

    while ((event = processEvent()))
    {
    }
}


std::vector<SDLInputManager::DiscreteHatEvent>
SDLInputManager::discretizeHatEvent(SDL_Event &event)
{
    auto &device = devices.at(event.jhat.which);
    auto &hat = event.jhat.hat;
    auto new_state = event.jhat.value;
    auto &old_state = device.hats[hat].state;

    if (old_state == new_state)
        return {};

    std::vector<DiscreteHatEvent> events;

    for (auto &s : { SDL_HAT_UP, SDL_HAT_DOWN, SDL_HAT_LEFT, SDL_HAT_RIGHT })
        if ((old_state & s) != (new_state & s))
        {
            events.emplace_back(device.index, hat, s, new_state & s);
        }

    old_state = new_state;
    return events;
}

std::vector<SDLInputManager::DiscreteAxisEvent>
SDLInputManager::discretizeJoyAxisEvent(SDL_Event &event, int threshold_percent)
{
    auto &device = devices.at(event.jaxis.which);
    auto &axis = event.jaxis.axis;
    auto now = event.jaxis.value;
    auto &then = device.axes[axis].last;
    auto center = device.axes[axis].initial;

    auto get_direction = [&](int pos) -> int {
        if (pos > (center + (32767 - center) * threshold_percent / 100))
            return 1;
        if (pos < (center - (center + 32768) * threshold_percent / 100))
            return -1;
        return 0;
    };

    auto previous_direction = get_direction(then);
    auto current_direction  = get_direction(now);

    if (previous_direction == current_direction)
    {
        then = now;
        return {};
    }

    std::vector<DiscreteAxisEvent> events{};

    if (previous_direction == -1 || current_direction == -1)
    {
        events.emplace_back(device.index, axis, -1, (current_direction == -1));
    }
    if (previous_direction == 1 || current_direction == 1)
    {
        events.emplace_back(device.index, axis, 1, (current_direction == 1));
    }

    then = now;

    return events;
}

std::optional<SDL_Event> SDLInputManager::processEvent()
{
    SDL_Event event{};

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
        case SDL_EVENT_JOYSTICK_HAT_MOTION:
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            return event;
        case SDL_EVENT_JOYSTICK_ADDED:
            addDevice(event.jdevice.which);
            return event;
        case SDL_EVENT_JOYSTICK_REMOVED:
            removeDevice(event.jdevice.which);
            return event;
        default:
            break;
        }
    }

    return std::nullopt;
}

void SDLInputManager::printDevices()
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

int SDLInputManager::findFirstOpenIndex()
{
    for (int i = 0; i < 10000; i++)
    {
        if (std::ranges::none_of(devices, [i](auto &d) -> bool { return d.second.index == i; }))
        {
            return i;
        }
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

    num_axes = SDL_GetNumJoystickAxes(joystick);
    axes.resize(num_axes);
    for (int i = 0; i < num_axes; i++)
    {
        SDL_GetJoystickAxisInitialState(joystick, i, &axes[i].initial);
        axes[i].last = axes[i].initial;
    }

    num_buttons = SDL_GetNumJoystickButtons(joystick);
    num_hats = SDL_GetNumJoystickHats(joystick);
    hats.resize(num_hats);
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

        list.emplace_back(d.first, SDL_GetJoystickName(d.second.joystick));
    }

    return list;
}

std::map<std::pair<int, int>, SDL_GamepadBinding> SDLInputManager::getXInputButtonBindings(SDL_Gamepad *gamepad)
{
    int num_bindings = 0;
    auto sdl_bindings = SDL_GetGamepadBindings(gamepad, &num_bindings);

    std::map<std::pair<int, int>, SDL_GamepadBinding> binding_map;
    for (int i = 0; i < num_bindings; i++)
        binding_map.insert_or_assign({ sdl_bindings[i]->output_type, sdl_bindings[i]->output.button }, *sdl_bindings[i]);

    SDL_free(sdl_bindings);

    return binding_map;
}