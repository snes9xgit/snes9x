#include "ControllerPanel.hpp"
#include "SDLInputManager.hpp"
#include "EmuApplication.hpp"
#include "EmuConfig.hpp"
#include "SDL_gamecontroller.h"
#include <optional>
#include <QtEvents>
#include <QTimer>

ControllerPanel::ControllerPanel(EmuApplication *app)
    : BindingPanel(app)
{
    setupUi(this);
    QObject::connect(controllerComboBox, &QComboBox::currentIndexChanged, [&](int index) {
        BindingPanel::binding = this->app->config->binding.controller[index].buttons;
        fillTable();
        awaiting_binding = false;
    });

    BindingPanel::setTableWidget(tableWidget_controller,
                                 app->config->binding.controller[0].buttons,
                                 app->config->allowed_bindings,
                                 app->config->num_controller_bindings);

    auto action = edit_menu.addAction(QObject::tr("Clear Current Controller"));
    action->connect(action, &QAction::triggered, [&](bool checked) {
        clearCurrentController();
    });

    action = edit_menu.addAction(QObject::tr("Clear All Controllers"));
    action->connect(action, &QAction::triggered, [&](bool checked) {
        clearAllControllers();
    });

    auto swap_menu = edit_menu.addMenu(QObject::tr("Swap With"));
    for (auto i = 0; i < 5; i++)
    {
        action = swap_menu->addAction(QObject::tr("Controller %1").arg(i + 1));
        action->connect(action, &QAction::triggered, [&, i](bool) {
            auto current_index = controllerComboBox->currentIndex();
            if (current_index == i)
                return;
            swapControllers(i, current_index);
            fillTable();
        });
    }

    editToolButton->setMenu(&edit_menu);
    editToolButton->setPopupMode(QToolButton::InstantPopup);

    recreateAutoAssignMenu();
    onJoypadsChanged([&]{ recreateAutoAssignMenu(); });
}

void ControllerPanel::recreateAutoAssignMenu()
{
    auto_assign_menu.clear();
    auto controller_list = app->input_manager->getXInputControllers();

    for (int i = 0; i < app->config->allowed_bindings; i++)
    {
        auto slot_menu = auto_assign_menu.addMenu(tr("Binding Set #%1").arg(i + 1));
        auto default_keyboard = slot_menu->addAction(tr("Default Keyboard"));
        default_keyboard->connect(default_keyboard, &QAction::triggered, [&, slot = i](bool) {
            autoPopulateWithKeyboard(slot);
        });

        for (auto c : controller_list)
        {
            auto controller_item = slot_menu->addAction(c.second.c_str());
            controller_item->connect(controller_item, &QAction::triggered, [&, id = c.first, slot = i](bool) {
                autoPopulateWithJoystick(id, slot);
            });
        }
    }
    autoAssignToolButton->setMenu(&auto_assign_menu);
    autoAssignToolButton->setPopupMode(QToolButton::InstantPopup);
}

void ControllerPanel::autoPopulateWithKeyboard(int slot)
{
    auto &buttons = app->config->binding.controller[controllerComboBox->currentIndex()].buttons;
    const char *button_list[] = { "Up", "Down", "Left", "Right", "d", "c", "s", "x", "z", "a", "Return", "Space" };

    for (int i = 0; i < std::size(button_list); i++)
        buttons[app->config->allowed_bindings * i + slot] = EmuBinding::keyboard(QKeySequence::fromString(button_list[i])[0].key());

    fillTable();
}

void ControllerPanel::autoPopulateWithJoystick(int joystick_id, int slot)
{
    auto &device = app->input_manager->devices[joystick_id];
    auto sdl_controller = device.controller;
    auto &buttons = app->config->binding.controller[controllerComboBox->currentIndex()].buttons;
    const SDL_GameControllerButton list[] = { SDL_CONTROLLER_BUTTON_DPAD_UP,
                                              SDL_CONTROLLER_BUTTON_DPAD_DOWN,
                                              SDL_CONTROLLER_BUTTON_DPAD_LEFT,
                                              SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
                                              // B, A and X, Y are inverted on XInput vs SNES
                                              SDL_CONTROLLER_BUTTON_B,
                                              SDL_CONTROLLER_BUTTON_A,
                                              SDL_CONTROLLER_BUTTON_Y,
                                              SDL_CONTROLLER_BUTTON_X,
                                              SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
                                              SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
                                              SDL_CONTROLLER_BUTTON_START,
                                              SDL_CONTROLLER_BUTTON_BACK };
    for (auto i = 0; i < std::size(list); i++)
    {
        auto sdl_binding = SDL_GameControllerGetBindForButton(sdl_controller, list[i]);
        if (SDL_CONTROLLER_BINDTYPE_BUTTON == sdl_binding.bindType)
            buttons[4 * i + slot] = EmuBinding::joystick_button(device.index, sdl_binding.value.button);
        else if (SDL_CONTROLLER_BINDTYPE_HAT == sdl_binding.bindType)
            buttons[4 * i + slot] = EmuBinding::joystick_hat(device.index, sdl_binding.value.hat.hat, sdl_binding.value.hat.hat_mask);
        else if (SDL_CONTROLLER_BINDTYPE_AXIS == sdl_binding.bindType)
            buttons[4 * i + slot] = EmuBinding::joystick_axis(device.index, sdl_binding.value.axis, sdl_binding.value.axis);
    }
    fillTable();
}

void ControllerPanel::swapControllers(int first, int second)
{
    auto &a = app->config->binding.controller[first].buttons;
    auto &b = app->config->binding.controller[second].buttons;

    int count = std::size(a);
    for (int i = 0; i < count; i++)
    {
        EmuBinding swap = b[i];
        b[i] = a[i];
        a[i] = swap;
    }
}

void ControllerPanel::clearCurrentController()
{
    auto &c = app->config->binding.controller[controllerComboBox->currentIndex()];
    for (auto &b : c.buttons)
        b = {};
    fillTable();
}

void ControllerPanel::clearAllControllers()
{
    for (auto &c : app->config->binding.controller)
        for (auto &b : c.buttons)
            b = {};
    fillTable();
}

void ControllerPanel::showEvent(QShowEvent *event)
{
    BindingPanel::showEvent(event);
    recreateAutoAssignMenu();
}

ControllerPanel::~ControllerPanel()
{
}

