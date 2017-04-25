#include <fcntl.h>
#ifdef USE_JOYSTICK
#   include "SDL.h"
#endif

#include "gtk_s9xcore.h"
#include "gtk_s9x.h"
#include "gtk_config.h"
#include "gtk_control.h"

const BindingLink b_links[] =
{
        /* Joypad-specific bindings. "Joypad# " will be prepended */
        { "b_up",        "Up"          },
        { "b_down",      "Down"        },
        { "b_left",      "Left"        },
        { "b_right",     "Right"       },
        { "b_start",     "Start"       },
        { "b_select",    "Select"      },
        { "b_a",         "A"           },
        { "b_b",         "B"           },
        { "b_x",         "X"           },
        { "b_y",         "Y"           },
        { "b_l",         "L"           },
        { "b_r",         "R"           },
        { "b_a_turbo",   "Turbo A"     },
        { "b_b_turbo",   "Turbo B"     },
        { "b_x_turbo",   "Turbo X"     },
        { "b_y_turbo",   "Turbo Y"     },
        { "b_l_turbo",   "Turbo L"     },
        { "b_r_turbo",   "Turbo R"     },
        { "b_a_sticky",  "Sticky A"    },
        { "b_b_sticky",  "Sticky B"    },
        { "b_x_sticky",  "Sticky X"    },
        { "b_y_sticky",  "Sticky Y"    },
        { "b_l_sticky",  "Sticky L"    },
        { "b_r_sticky",  "Sticky R"    },

        /* Emulator based bindings */
        { "b_open_rom",            "GTK_open_rom"      },
        { "b_enable_turbo",        "EmuTurbo"          },
        { "b_toggle_turbo",        "ToggleEmuTurbo"    },
        { "b_pause",               "GTK_pause"         },
        { "b_decrease_frame_rate", "DecFrameRate"      },
        { "b_increase_frame_rate", "IncFrameRate"      },
        { "b_decrease_frame_time", "DecFrameTime"      },
        { "b_increase_frame_time", "IncFrameTime"      },
        { "b_hardware_reset",      "Reset"             },
        { "b_soft_reset",          "SoftReset"         },
        { "b_quit",                "GTK_quit"          },
        { "b_bg_layer_0",          "ToggleBG0"         },
        { "b_bg_layer_1",          "ToggleBG1"         },
        { "b_bg_layer_2",          "ToggleBG2"         },
        { "b_bg_layer_3",          "ToggleBG3"         },
        { "b_sprites",             "ToggleSprites"     },
        { "b_bg_layering_hack",    "BGLayeringHack"    },
        { "b_screenshot",          "Screenshot"        },
        { "b_fullscreen",          "GTK_fullscreen"    },
        { "b_save_0",              "QuickSave000"      },
        { "b_save_1",              "QuickSave001"      },
        { "b_save_2",              "QuickSave002"      },
        { "b_save_3",              "QuickSave003"      },
        { "b_save_4",              "QuickSave004"      },
        { "b_save_5",              "QuickSave005"      },
        { "b_save_6",              "QuickSave006"      },
        { "b_save_7",              "QuickSave007"      },
        { "b_save_8",              "QuickSave008"      },
        { "b_save_9",              "QuickSave009"      },
        { "b_load_0",              "QuickLoad000"      },
        { "b_load_1",              "QuickLoad001"      },
        { "b_load_2",              "QuickLoad002"      },
        { "b_load_3",              "QuickLoad003"      },
        { "b_load_4",              "QuickLoad004"      },
        { "b_load_5",              "QuickLoad005"      },
        { "b_load_6",              "QuickLoad006"      },
        { "b_load_7",              "QuickLoad007"      },
        { "b_load_8",              "QuickLoad008"      },
        { "b_load_9",              "QuickLoad009"      },
        { "b_sound_channel_0",     "SoundChannel0"     },
        { "b_sound_channel_1",     "SoundChannel1"     },
        { "b_sound_channel_2",     "SoundChannel2"     },
        { "b_sound_channel_3",     "SoundChannel3"     },
        { "b_sound_channel_4",     "SoundChannel4"     },
        { "b_sound_channel_5",     "SoundChannel5"     },
        { "b_sound_channel_6",     "SoundChannel6"     },
        { "b_sound_channel_7",     "SoundChannel7"     },
        { "b_all_sound_channels",  "SoundChannelsOn"   },
        { "b_save_spc",            "GTK_save_spc"      },
        { "b_begin_recording_movie", "BeginRecordingMovie" },
        { "b_stop_recording_movie", "EndRecordingMovie" },
        { "b_load_movie",          "LoadMovie" },
        { "b_seek_to_frame",       "GTK_seek_to_frame" },
        { "b_swap_controllers",    "GTK_swap_controllers" },
        { "b_rewind",              "GTK_rewind"        },

        { NULL, NULL }
};

/* Where the page breaks occur in the preferences pane */
const int b_breaks[] =
{
        12, /* End of main buttons */
        24, /* End of turbo/sticky buttons */
        35, /* End of base emulator buttons */
        43, /* End of Graphic options */
        63, /* End of save/load states */
        72, /* End of sound buttons */
        79, /* End of miscellaneous buttons */
        -1
};

static int joystick_lock = 0;

bool
S9xPollButton (uint32 id, bool *pressed)
{
    return true;
}

bool
S9xPollAxis (uint32 id, int16 *value)
{
    return true;
}

bool
S9xPollPointer (uint32 id, int16 *x, int16 *y)
{
    *x = top_level->mouse_loc_x;
    *y = top_level->mouse_loc_y;

    return true;
}

bool
S9xIsMousePluggedIn (void)
{
    enum controllers ctl;
    int8 id1, id2, id3, id4;

    for (int i = 0; i <= 1; i++)
    {
        S9xGetController (i, &ctl, &id1, &id2, &id3, &id4);
        if (ctl == CTL_MOUSE)
            return true;
    }

    return false;
}

bool
S9xGrabJoysticks (void)
{
    if (joystick_lock)
        return FALSE;

    joystick_lock++;

    return TRUE;
}

void
S9xReleaseJoysticks (void)
{
    joystick_lock--;

    return;
}

static void
swap_controllers_1_2 (void)
{
    JoypadBinding interrim;

    memcpy (&interrim,
            &gui_config->pad[0],
            sizeof (JoypadBinding));
    memcpy (&gui_config->pad[0],
            &gui_config->pad[1],
            sizeof (JoypadBinding));
    memcpy (&gui_config->pad[1],
            &interrim,
            sizeof (JoypadBinding));

    gui_config->rebind_keys ();

    return;
}

void
S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
    static bool quit_binding_down = FALSE;

    if (data1 == TRUE)
    {
        if (cmd.port[0] == PORT_QUIT)
            quit_binding_down = TRUE;
        else if (cmd.port[0] == PORT_REWIND)
            top_level->user_rewind = TRUE;
    }

    if (data1 == FALSE) /* Release */
    {
        if (cmd.port[0] != PORT_QUIT)
        {
            quit_binding_down = FALSE;
        }

        if (cmd.port[0] == PORT_COMMAND_FULLSCREEN)
        {
            top_level->toggle_fullscreen_mode ();
        }

        else if (cmd.port[0] == PORT_COMMAND_SAVE_SPC)
        {
            top_level->save_spc_dialog ();
        }

        else if (cmd.port[0] == PORT_OPEN_ROM)
        {
            top_level->open_rom_dialog ();
        }

        else if (cmd.port[0] == PORT_PAUSE)
        {
            if (!(top_level->user_pause))
                top_level->pause_from_user ();
            else
                top_level->unpause_from_user ();
        }

        else if (cmd.port[0] == PORT_REWIND)
        {
            top_level->user_rewind = FALSE;
        }

        else if (cmd.port[0] == PORT_SEEK_TO_FRAME)
        {
            top_level->movie_seek_dialog ();
        }

        else if (cmd.port[0] == PORT_SWAP_CONTROLLERS)
        {
            swap_controllers_1_2 ();
        }

        else if (cmd.port[0] == PORT_QUIT)
        {
            if (quit_binding_down)
                S9xExit ();
        }
    }

    return;
}

Binding
S9xGetBindingByName (const char *name)
{
    for (int i = 0; i < NUM_EMU_LINKS; i++)
    {
        if (!strcasecmp (b_links[i + NUM_JOYPAD_LINKS].snes9x_name, name))
        {
            return gui_config->shortcut[i];
        }
    }

    return Binding ();
}

s9xcommand_t
S9xGetPortCommandT (const char *name)
{
    s9xcommand_t cmd;

    cmd.type = S9xButtonPort;
    cmd.multi_press = 0;
    cmd.button_norpt = 0;
    cmd.port[0] = 0;
    cmd.port[1] = 0;
    cmd.port[2] = 0;
    cmd.port[3] = 0;

    if (!strcasecmp (name, "GTK_fullscreen"))
    {
        cmd.port[0] = PORT_COMMAND_FULLSCREEN;
    }

    else if (!strcasecmp (name, "GTK_save_spc"))
    {
        cmd.port[0] = PORT_COMMAND_SAVE_SPC;
    }

    else if (!strcasecmp (name, "GTK_open_rom"))
    {
        cmd.port[0] = PORT_OPEN_ROM;
    }

    else if (!strcasecmp (name, "GTK_pause"))
    {
        cmd.port[0] = PORT_PAUSE;
    }

    else if (!strcasecmp (name, "GTK_seek_to_frame"))
    {
        cmd.port[0] = PORT_SEEK_TO_FRAME;
    }

    else if (!strcasecmp (name, "GTK_quit"))
    {
        cmd.port[0] = PORT_QUIT;
    }

    else if (!strcasecmp (name, "GTK_swap_controllers"))
    {
        cmd.port[0] = PORT_SWAP_CONTROLLERS;
    }

    else if (!strcasecmp (name, "GTK_rewind"))
    {
        cmd.port[0] = PORT_REWIND;
    }

    else
    {
        cmd = S9xGetCommandT (name);
    }

    return cmd;
}

void
S9xProcessEvents (bool8 block)
{
#ifdef USE_JOYSTICK
    JoyEvent event;
    Binding  binding;

    if (S9xGrabJoysticks ())
    {
        for (int i = 0; gui_config->joystick[i]; i++)
        {
            while (gui_config->joystick[i]->get_event (&event))
            {
                binding = Binding (i, event.parameter, 0);
                S9xReportButton (binding.hex (), event.state == JOY_PRESSED ? 1 : 0);
                gui_config->screensaver_needs_reset = TRUE;
            }
        }

        S9xReleaseJoysticks ();
    }
#endif

    return;
}

#ifdef USE_JOYSTICK
static void
poll_joystick_events (void)
{
    SDL_Event event;

    while (SDL_PollEvent (&event))
    {
        if (event.type == SDL_JOYAXISMOTION)
        {
             gui_config->joystick[event.jaxis.which]->handle_event (&event);
        }

        else if (event.type == SDL_JOYHATMOTION)
        {
            gui_config->joystick[event.jhat.which]->handle_event (&event);
        }

        else if (event.type == SDL_JOYBUTTONUP ||
                 event.type == SDL_JOYBUTTONDOWN)
        {
            gui_config->joystick[event.jbutton.which]->handle_event (&event);
        }
    }

    return;
}
#endif

void
S9xInitInputDevices (void)
{
#ifdef USE_JOYSTICK
    SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    for (int i = 0; ; i++)
    {
        gui_config->joystick = (JoyDevice **)
            realloc (gui_config->joystick,
                     sizeof (JoyDevice *) * (i + 1));

        gui_config->joystick[i] = new JoyDevice (i);

        if (!gui_config->joystick[i]->enabled)
        {
            delete gui_config->joystick[i];
            gui_config->joystick[i] = NULL;
            break;
        }
        else
        {
            gui_config->joystick[i]->joynum = i;
        }
    }
#endif

    return;
}

void
S9xDeinitInputDevices (void)
{
#ifdef USE_JOYSTICK
    for (int i = 0; gui_config->joystick[i] != NULL; i++)
    {
        delete gui_config->joystick[i];
    }

    free (gui_config->joystick);

    SDL_Quit ();
#endif

    return;
}

#ifdef USE_JOYSTICK
JoyDevice::JoyDevice (unsigned int device_num)
{
    enabled = false;
    axis = NULL;
    filedes = NULL;
    mode = JOY_MODE_INDIVIDUAL;

    if ((int) device_num >= SDL_NumJoysticks ())
        return;

    filedes = SDL_JoystickOpen (device_num);

    if (!filedes)
        return;

    enabled = true;

    num_axes = SDL_JoystickNumAxes (filedes);
    num_hats = SDL_JoystickNumHats (filedes);
    axis = (int *) malloc (sizeof (int) * num_axes);
    hat  = (int *) malloc (sizeof (int) * num_hats);
    calibration = (Calibration *) malloc (sizeof (Calibration) * num_axes);

    for (int i = 0; i < num_axes; i++)
    {
        calibration[i].min = -32767;
        calibration[i].max = 32767;
        calibration[i].center = 0;
    }

    memset (axis, 0, sizeof (int) * num_axes);

    return;
}

JoyDevice::~JoyDevice (void)
{
    if (enabled)
    {
        SDL_JoystickClose (filedes);
        free (axis);
        free (hat);
        free (calibration);
    }

    enabled = false;

    return;
}

void
JoyDevice::add_event (unsigned int parameter, unsigned int state)
{
    JoyEvent event = { parameter, state };

    queue.push (event);

    return;
}

void
JoyDevice::register_centers (void)
{
    for (int i = 0; i < num_axes; i++)
    {
        calibration[i].center = SDL_JoystickGetAxis (filedes, i);

        /* Snap centers to specific target points */
        if (calibration[i].center < -24576)
            calibration[i].center = -32768;
        else if (calibration[i].center < -8192)
            calibration[i].center = -16384;
        else if (calibration[i].center < 8192)
            calibration[i].center = 0;
        else if (calibration[i].center < 24576)
            calibration[i].center = 16383;
        else
            calibration[i].center = 32767;
    }

    return;
}

void
JoyDevice::handle_event (SDL_Event *event)
{
    if (event->type == SDL_JOYAXISMOTION)
    {
        int cal_min = calibration[event->jaxis.axis].min;
        int cal_max = calibration[event->jaxis.axis].max;
        int cal_cen = calibration[event->jaxis.axis].center;
        int t = gui_config->joystick_threshold;
        int ax_min = (cal_min - cal_cen) * t / 100 + cal_cen;
        int ax_max = (cal_max - cal_cen) * t / 100 + cal_cen;

        if (mode == JOY_MODE_INDIVIDUAL)
        {
            for (int i = 0; i < NUM_JOYPADS; i++)
            {
                Binding *pad = (Binding *) &(gui_config->pad[i]);

                for (int j = 0; j < NUM_JOYPAD_LINKS; j++)
                {
                    if (pad[j].get_axis () == event->jaxis.axis &&
                        pad[j].get_device () == (unsigned int) (joynum + 1))
                    {
                        t = pad[j].get_threshold ();

                        if (pad[j].is_positive ())
                        {
                            ax_max = (cal_max - cal_cen) * t / 100 + cal_cen;
                        }
                        else if (pad[j].is_negative ())
                        {
                            ax_min = (cal_min - cal_cen) * t / 100 + cal_cen;
                        }
                    }
                }
            }

            for (int i = 0; i < NUM_EMU_LINKS; i++)
            {
                if (gui_config->shortcut[i].get_axis () == event->jaxis.axis &&
                    gui_config->shortcut[i].get_device () ==
                    (unsigned int) (joynum + 1))
                {
                    t = gui_config->shortcut[i].get_threshold ();
                    if (gui_config->shortcut[i].is_positive ())
                    {
                        ax_max = (cal_max - cal_cen) * t / 100 + cal_cen;
                    }
                    else if (gui_config->shortcut[i].is_negative ())
                    {
                        ax_min = (cal_min - cal_cen) * t / 100 + cal_cen;
                    }
                }
            }
        }
        else if (mode == JOY_MODE_CALIBRATE)
        {
            if (event->jaxis.value < calibration[event->jaxis.axis].min)
                calibration[event->jaxis.axis].min = event->jaxis.value;
            if (event->jaxis.value > calibration[event->jaxis.axis].max)
                calibration[event->jaxis.axis].min = event->jaxis.value;
        }

        /* Sanity Check */
        if (ax_min >= cal_cen)
            ax_min = cal_cen - 1;
        if (ax_max <= cal_cen)
            ax_max = cal_cen + 1;

        if (event->jaxis.value <= ax_min &&
                axis[event->jaxis.axis] > ax_min)
        {
            add_event (JOY_AXIS (event->jaxis.axis, AXIS_NEG), 1);
        }

        if (event->jaxis.value > ax_min &&
                axis[event->jaxis.axis] <= ax_min)
        {
            add_event (JOY_AXIS (event->jaxis.axis, AXIS_NEG), 0);
        }

        if (event->jaxis.value >= ax_max &&
                axis[event->jaxis.axis] < ax_max)
        {
            add_event (JOY_AXIS (event->jaxis.axis, AXIS_POS), 1);
        }

        if (event->jaxis.value < ax_max &&
                axis[event->jaxis.axis] >= ax_max)
        {
            add_event (JOY_AXIS (event->jaxis.axis, AXIS_POS), 0);
        }

        axis[event->jaxis.axis] = event->jaxis.value;

    }

    else if (event->type == SDL_JOYBUTTONUP ||
            event->type == SDL_JOYBUTTONDOWN)
    {
        add_event (event->jbutton.button,
                   event->jbutton.state == SDL_PRESSED ? 1 : 0);
    }

    else if (event->type == SDL_JOYHATMOTION)
    {
        if ((event->jhat.value & SDL_HAT_UP) &&
            !(hat[event->jhat.hat] & SDL_HAT_UP))
        {
            add_event (JOY_AXIS (num_axes + (event->jhat.hat * 2), AXIS_POS), 1);
        }

        if (!(event->jhat.value & SDL_HAT_UP) &&
            (hat[event->jhat.hat] & SDL_HAT_UP))
        {
            add_event (JOY_AXIS (num_axes + (event->jhat.hat * 2), AXIS_POS), 0);
        }

        if ((event->jhat.value & SDL_HAT_DOWN) &&
            !(hat[event->jhat.hat] & SDL_HAT_DOWN))
        {
            add_event (JOY_AXIS (num_axes + (event->jhat.hat * 2), AXIS_NEG), 1);
        }

        if (!(event->jhat.value & SDL_HAT_DOWN) &&
            (hat[event->jhat.hat] & SDL_HAT_DOWN))
        {
            add_event (JOY_AXIS (num_axes + (event->jhat.hat * 2), AXIS_NEG), 0);
        }

        if ((event->jhat.value & SDL_HAT_LEFT) &&
            !(hat[event->jhat.hat] & SDL_HAT_LEFT))
        {
            add_event (JOY_AXIS (num_axes + (event->jhat.hat * 2) + 1, AXIS_NEG), 1);
        }

        if (!(event->jhat.value & SDL_HAT_LEFT) &&
            (hat[event->jhat.hat] & SDL_HAT_LEFT))
        {
            add_event (JOY_AXIS (num_axes + (event->jhat.hat * 2) + 1, AXIS_NEG), 0);
        }

        if ((event->jhat.value & SDL_HAT_RIGHT) &&
            !(hat[event->jhat.hat] & SDL_HAT_RIGHT))
        {
            add_event (JOY_AXIS (num_axes + (event->jhat.hat * 2) + 1, AXIS_POS), 1);
        }

        if (!(event->jhat.value & SDL_HAT_RIGHT) &&
            (hat[event->jhat.hat] & SDL_HAT_RIGHT))
        {
            add_event (JOY_AXIS (num_axes + (event->jhat.hat * 2) + 1, AXIS_POS), 0);
        }

        hat[event->jhat.hat] = event->jhat.value;
    }

    return;
}

int
JoyDevice::get_event (JoyEvent *event)
{
    poll_events ();

    if (queue.empty ())
        return 0;

    event->parameter = queue.front ().parameter;
    event->state     = queue.front ().state;

    queue.pop ();

    return 1;
}

void
JoyDevice::poll_events (void)
{
    poll_joystick_events ();

    return;
}

void
JoyDevice::flush (void)
{
    SDL_Event event;

    while (SDL_PollEvent (&event))
    {
    }

    while (!queue.empty ())
        queue.pop ();

    return;
}
#endif
