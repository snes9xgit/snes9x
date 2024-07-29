#include "Snes9xController.hpp"
#include "EmuConfig.hpp"
#include "SoftwareScalers.hpp"
#include "fscompat.h"
#include <filesystem>
namespace fs = std::filesystem;

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "gfx.h"
#include "snapshot.h"
#include "controls.h"
#include "cheats.h"
#include "movie.h"
#include "display.h"
#include "conffile.h"
#include "statemanager.h"

Snes9xController *g_snes9xcontroller = nullptr;
StateManager g_state_manager;

Snes9xController::Snes9xController()
{
    init();
}

Snes9xController::~Snes9xController()
{
    deinit();
}

Snes9xController *Snes9xController::get()
{
    if (!g_snes9xcontroller)
    {
        g_snes9xcontroller = new Snes9xController();
    }

    return g_snes9xcontroller;
}

void Snes9xController::init()
{
    Settings.MouseMaster = true;
    Settings.SuperScopeMaster = true;
    Settings.JustifierMaster = true;
    Settings.MultiPlayer5Master = true;
    Settings.Transparency = true;
    Settings.Stereo = true;
    Settings.ReverseStereo = false;
    Settings.SixteenBitSound = true;
    Settings.StopEmulation = true;
    Settings.HDMATimingHack = 100;
    Settings.SkipFrames = 0;
    Settings.TurboSkipFrames = 9;
    Settings.NetPlay = false;

    Settings.UpAndDown = false;
    Settings.InterpolationMethod = DSP_INTERPOLATION_GAUSSIAN;
    Settings.FrameTime = 16639;
    Settings.FrameTimeNTSC = 16639;
    Settings.FrameTimePAL = 20000;
    Settings.DisplayFrameRate = false;
    Settings.DisplayTime = false;
    Settings.DisplayFrameRate = false;
    Settings.DisplayPressedKeys = false;
    Settings.DisplayIndicators = true;
    Settings.SoundPlaybackRate = 48000;
    Settings.SoundInputRate = 32040;
    Settings.BlockInvalidVRAMAccess = true;
    Settings.SoundSync = false;
    Settings.Mute = false;
    Settings.DynamicRateControl = false;
    Settings.DynamicRateLimit = 5;
    Settings.SuperFXClockMultiplier = 100;
    Settings.MaxSpriteTilesPerLine = 34;
    Settings.OneClockCycle = 6;
    Settings.OneSlowClockCycle = 8;
    Settings.TwoClockCycles = 12;
    Settings.ShowOverscan = false;
    Settings.InitialInfoStringTimeout = 120;

    CPU.Flags = 0;

    rewind_buffer_size = 0;
    rewind_frame_interval = 5;

    Memory.Init();
    S9xInitAPU();
    S9xInitSound(0);
    S9xSetSamplesAvailableCallback([](void *data) {
        ((Snes9xController *)data)->SamplesAvailable();
    }, this);

    S9xGraphicsInit();
    S9xInitInputDevices();
    S9xUnmapAllControls();
    S9xCheatsEnable();

    active = false;
}

void Snes9xController::deinit()
{
    if (active)
        S9xAutoSaveSRAM();
    S9xGraphicsDeinit();
    S9xDeinitAPU();
}

void Snes9xController::updateSettings(const EmuConfig * const config)
{
    Settings.UpAndDown = config->allow_opposing_dpad_directions;

    Settings.InterpolationMethod = config->sound_filter;

    if (config->fixed_frame_rate == 0.0)
    {
        Settings.FrameTimeNTSC = 16639;
        Settings.FrameTimePAL = 20000;
        Settings.FrameTime = Settings.FrameTimeNTSC;
    }
    else
    {
        Settings.FrameTimeNTSC = Settings.FrameTimePAL = Settings.FrameTime =
            1000000 / config->fixed_frame_rate;
    }

    Settings.TurboSkipFrames = config->fast_forward_skip_frames;

    Settings.DisplayTime = config->show_time;

    if (config->display_messages == EmuConfig::eInscreen)
        Settings.AutoDisplayMessages = true;
    else
        Settings.AutoDisplayMessages = false;

    Settings.DisplayFrameRate = config->show_frame_rate;

    Settings.DisplayPressedKeys = config->show_pressed_keys;

    Settings.DisplayIndicators = config->show_indicators;

    if (Settings.SoundPlaybackRate != config->playback_rate || Settings.SoundInputRate != config->input_rate)
    {
        Settings.SoundInputRate = config->input_rate;
        Settings.SoundPlaybackRate = config->playback_rate;
        S9xUpdateDynamicRate();
    }

    Settings.BlockInvalidVRAMAccess = !config->allow_invalid_vram_access;

    Settings.SoundSync = config->speed_sync_method == EmuConfig::eSoundSync;

    Settings.Mute = config->mute_audio;

    Settings.DynamicRateControl = config->dynamic_rate_control;

    Settings.DynamicRateLimit = config->dynamic_rate_limit * 1000;

    Settings.SuperFXClockMultiplier  = config->superfx_clock_multiplier;

    if (rewind_buffer_size != config->rewind_buffer_size && active)
    {
        g_state_manager.init(config->rewind_buffer_size * 1048576);
    }
    rewind_buffer_size = config->rewind_buffer_size;
    rewind_frame_interval = config->rewind_frame_interval;

    if (config->remove_sprite_limit)
        Settings.MaxSpriteTilesPerLine = 128;
    else
        Settings.MaxSpriteTilesPerLine = 34;

    const int overclock_cycles[4][2] = { { 6, 8 }, { 6, 6 }, { 3, 4 }, { 1, 1 } };
    Settings.OneClockCycle = overclock_cycles[config->overclock][0];
    Settings.OneSlowClockCycle = overclock_cycles[config->overclock][1];
    Settings.TwoClockCycles = overclock_cycles[config->overclock][0] * 2;

    Settings.ShowOverscan = config->show_overscan;

    high_resolution_effect = config->high_resolution_effect;

    config_folder = config->findConfigDir();

    auto doFolder = [&](int location, std::string &dest, const std::string &src, const char *subfolder_name)
    {
        if (location == EmuConfig::eROMDirectory)
            dest = "";
        else if (location == EmuConfig::eConfigDirectory)
            dest = config_folder + "/" + subfolder_name;
        else
            dest = src;
    };

    doFolder(config->sram_location, sram_folder, config->sram_folder, "sram");
    doFolder(config->state_location, state_folder, config->state_folder, "state");
    doFolder(config->cheat_location, cheat_folder, config->cheat_folder, "cheat");
    doFolder(config->patch_location, patch_folder, config->patch_folder, "patch");
    doFolder(config->export_location, export_folder, config->export_folder, "export");
}

bool Snes9xController::openFile(std::string filename)
{
    if (active)
        S9xAutoSaveSRAM();
    active = false;
    auto result = Memory.LoadROM(filename.c_str());
    if (result)
    {
        active = true;
        Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR).c_str());
    }
    return active;
}

void Snes9xController::mainLoop()
{
    if (!active)
        return;

    if (rewind_buffer_size > 0)
    {
        if (rewinding)
        {
            uint16 joypads[8];
            for (int i = 0; i < 8; i++)
                joypads[i] = MovieGetJoypad(i);

            rewinding = g_state_manager.pop();

            for (int i = 0; i < 8; i++)
                MovieSetJoypad(i, joypads[i]);
        }
        else if (IPPU.TotalEmulatedFrames % rewind_frame_interval == 0)
            g_state_manager.push();

        if (rewinding)
            Settings.Mute |= 0x80;
        else
            Settings.Mute &= ~0x80;
    }

    S9xMainLoop();
}

void Snes9xController::setPaused(bool paused)
{
    Settings.Paused = paused;
}

void Snes9xController::updateSoundBufferLevel(int empty, int total)
{
    S9xUpdateDynamicRate(empty, total);
}

bool8 S9xDeinitUpdate(int width, int height)
{
    static int last_width = 0;
    static int last_height = 0;
    int yoffset = 0;

    auto &display = Snes9xController::get()->screen_output_function;
    if (display == nullptr)
        return true;

    if (width < 256 || height < 224)
        return false;

    if (last_height > height)
        memset(GFX.Screen + GFX.RealPPL * height, 0, GFX.Pitch * (last_height - height));

    last_width = width;
    last_height = height;

    if (Settings.ShowOverscan)
    {
        if (height == SNES_HEIGHT)
        {
            yoffset = -8;
            height = SNES_HEIGHT_EXTENDED;
        }
        if (height == SNES_HEIGHT * 2)
        {
            yoffset = -16;
            height = SNES_HEIGHT_EXTENDED * 2;
        }
    }
    else
    {
        if (height == SNES_HEIGHT_EXTENDED)
        {
            yoffset = 7;
            height = SNES_HEIGHT;
        }
        if (height == SNES_HEIGHT_EXTENDED * 2)
        {
            yoffset = 14;
            height = SNES_HEIGHT * 2;
        }
    }

    uint16_t *screen_view = GFX.Screen + (yoffset * (int)GFX.RealPPL);

    auto hires_effect = Snes9xController::get()->high_resolution_effect;
    if (!Settings.Paused)
    {
        if (hires_effect == EmuConfig::eScaleUp)
        {
            S9xForceHires(screen_view, GFX.Pitch, width, height);
            last_width = width;
        }
        else if (hires_effect == EmuConfig::eScaleDown)
        {
            S9xMergeHires(screen_view, GFX.Pitch, width, height);
            last_width = width;
        }
    }

    display(screen_view, width, height, GFX.Pitch, Settings.PAL ? 50.0 : 60.098813);

    return true;
}

bool8 S9xContinueUpdate(int width, int height)
{
    return S9xDeinitUpdate(width, height);
}

void S9xSyncSpeed()
{
    if (Settings.TurboMode)
    {
        IPPU.FrameSkip++;
        if ((IPPU.FrameSkip > Settings.TurboSkipFrames) && !Settings.HighSpeedSeek)
        {
            IPPU.FrameSkip = 0;
            IPPU.SkippedFrames = 0;
            IPPU.RenderThisFrame = true;
        }
        else
        {
            IPPU.SkippedFrames++;
            IPPU.RenderThisFrame = false;
        }

        return;
    }

    IPPU.RenderThisFrame = true;

}

void S9xParsePortConfig(ConfigFile&, int)
{
}

std::string S9xGetDirectory(s9x_getdirtype dirtype)
{
    std::string dirname;
    auto c = Snes9xController::get();

    switch (dirtype)
    {
    case HOME_DIR:
        dirname = c->config_folder;
        break;

    case SNAPSHOT_DIR:
        dirname = c->state_folder;
        break;

    case PATCH_DIR:
        dirname = c->patch_folder;
        break;

    case CHEAT_DIR:
        dirname = c->cheat_folder;
        break;

    case SRAM_DIR:
        dirname = c->sram_folder;
        break;

    case SCREENSHOT_DIR:
    case SPC_DIR:
        dirname = c->export_folder;
        break;

    default:
        dirname = "";
    }

    /* Check if directory exists, make it and/or set correct permissions */
    if (dirtype != HOME_DIR && dirname != "")
    {
        fs::path path(dirname);

        if (!fs::exists(path))
        {
            fs::create_directories(path);
        }
        else if ((fs::status(path).permissions() & fs::perms::owner_write) == fs::perms::none)
        {
            fs::permissions(path, fs::perms::owner_write, fs::perm_options::add);
        }
    }

    /* Anything else, use ROM filename path */
    if (dirname == "" && !Memory.ROMFilename.empty())
    {
        fs::path path(Memory.ROMFilename);

        path.remove_filename();

        if (!fs::is_directory(path))
            dirname = fs::current_path().u8string();
        else
            dirname = path.u8string();
    }

    return dirname;
}

void S9xInitInputDevices()
{
}

void S9xHandlePortCommand(s9xcommand_t, short, short)
{
}

bool S9xPollButton(unsigned int, bool *)
{
    return false;
}

void S9xToggleSoundChannel(int c)
{
    static int sound_switch = 255;

    if (c == 8)
        sound_switch = 255;
    else
        sound_switch ^= 1 << c;

    S9xSetSoundControl(sound_switch);
}

std::string S9xGetFilenameInc(std::string e, enum s9x_getdirtype dirtype)
{
    fs::path rom_filename(Memory.ROMFilename);

    fs::path filename_base(S9xGetDirectory(dirtype));
    filename_base /= rom_filename.filename();

    fs::path new_filename;

    if (e[0] != '.')
        e = "." + e;
    int i = 0;
    do
    {
        std::string new_extension = std::to_string(i);
        while (new_extension.length() < 3)
            new_extension = "0" + new_extension;
        new_extension += e;

        new_filename = filename_base;
        new_filename.replace_extension(new_extension);

        i++;
    } while (fs::exists(new_filename));

    return new_filename.u8string();
}

bool8 S9xInitUpdate()
{
    return true;
}

void S9xExtraUsage()
{
}

bool8 S9xOpenSoundDevice()
{
    return true;
}

bool S9xPollAxis(unsigned int axis, short *value)
{
    return true;
}

void S9xParseArg(char *argv[], int &index, int argc)
{
}

void S9xExit()
{
}

bool S9xPollPointer(unsigned int, short *, short *)
{
    return false;
}

void Snes9xController::SamplesAvailable()
{
    static std::vector<int16_t> data;
    if (sound_output_function)
    {
        int samples = S9xGetSampleCount();
        if (data.size() < samples)
            data.resize(samples);
        S9xMixSamples((uint8_t *)data.data(), samples);
        sound_output_function(data.data(), samples);
    }
    else
    {
        S9xClearSamples();
    }
}

void Snes9xController::clearSoundBuffer()
{
    S9xClearSamples();
}

void S9xMessage(int message_class, int type, const char *message)
{
    if (type == S9X_ROM_INFO)
        S9xSetInfoString(Memory.GetMultilineROMInfo().c_str());

    printf("%s\n", message);
}

const char *S9xStringInput(const char *prompt)
{
    return "";
}

bool8 S9xOpenSnapshotFile(const char *filename, bool8 read_only, STREAM *file)
{
    if (read_only)
    {
        if ((*file = OPEN_STREAM(filename, "rb")))
            return (true);
        else
            fprintf(stderr, "Failed to open file stream for reading.\n");
    }
    else
    {
        if ((*file = OPEN_STREAM(filename, "wb")))
        {
            return (true);
        }
        else
        {
            fprintf(stderr, "Couldn't open stream with zlib.\n");
        }
    }

    fprintf(stderr, "Couldn't open snapshot file:\n%s\n", filename);

    return false;
}

void S9xCloseSnapshotFile(STREAM file)
{
    CLOSE_STREAM(file);
}

void S9xAutoSaveSRAM()
{
    printf("%s\n", S9xGetFilename(".srm", SRAM_DIR).c_str());
    Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR).c_str());
    S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR).c_str());
}

bool Snes9xController::acceptsCommand(const char *command)
{
    auto cmd = S9xGetCommandT(command);
    return !(cmd.type == S9xNoMapping || cmd.type == S9xBadMapping);
}

void Snes9xController::updateBindings(const EmuConfig *const config)
{
    const char *snes9x_names[] = {
        "Up",
        "Down",
        "Left",
        "Right",
        "A",
        "B",
        "X",
        "Y",
        "L",
        "R",
        "Start",
        "Select",
        "Turbo A",
        "Turbo B",
        "Turbo X",
        "Turbo Y",
        "Turbo L",
        "Turbo R",
    };

    S9xUnmapAllControls();

    switch (config->port_configuration)
    {
    case EmuConfig::eTwoControllers:
        S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
        S9xSetController(1, CTL_JOYPAD, 1, 1, 1, 1);
        break;
    case EmuConfig::eMousePlusController:
        S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
        S9xSetController(1, CTL_JOYPAD, 0, 0, 0, 0);
        break;
    case EmuConfig::eSuperScopePlusController:
        S9xSetController(0, CTL_SUPERSCOPE, 0, 0, 0, 0);
        S9xSetController(1, CTL_JOYPAD, 0, 0, 0, 0);
        break;
    case EmuConfig::eControllerPlusMultitap:
        S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
        S9xSetController(1, CTL_MP5, 1, 2, 3, 4);
        break;
    default:
        S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
        S9xSetController(1, CTL_NONE, 0, 0, 0, 0);
    }

    for (int controller_number = 0; controller_number < 5; controller_number++)
    {
        auto &controller = config->binding.controller[controller_number];
        for (int i = 0; i < config->num_controller_bindings; i++)
        {
            for (int b = 0; b < config->allowed_bindings; b++)
            {
                auto binding = controller.buttons[i * config->allowed_bindings + b];
                if (binding.hash() == 0)
                    continue;
                std::string name = "Joypad" +
                                    std::to_string(controller_number + 1) + " " +
                                    snes9x_names[i];
                auto cmd = S9xGetCommandT(name.c_str());
                S9xMapButton(binding.hash(), cmd, false);
            }
        }
    }

    for (int i = 0; i < config->num_shortcuts; i++)
    {
        auto command = S9xGetCommandT(EmuConfig::getShortcutNames()[i]);
        if (command.type == S9xNoMapping)
            continue;

        for (int b = 0; b < 4; b++)
        {
            auto binding = config->binding.shortcuts[i * 4 + b];
            if (binding.type != 0)
                S9xMapButton(binding.hash(), command, false);
        }
    }

    auto cmd = S9xGetCommandT("Pointer Mouse1+Superscope+Justifier1");
    S9xMapPointer(EmuBinding::MOUSE_POINTER, cmd, false);
    mouse_x = mouse_y = 0;
    S9xReportPointer(EmuBinding::MOUSE_POINTER, mouse_x, mouse_y);

    cmd = S9xGetCommandT("{Mouse1 L,Superscope Fire,Justifier1 Trigger}");
    S9xMapButton(EmuBinding::MOUSE_BUTTON1, cmd, false);

    cmd = S9xGetCommandT("{Justifier1 AimOffscreen Trigger,Superscope AimOffscreen}");
    S9xMapButton(EmuBinding::MOUSE_BUTTON3, cmd, false);

    cmd = S9xGetCommandT("{Mouse1 R,Superscope Cursor,Justifier1 Start}");
    S9xMapButton(EmuBinding::MOUSE_BUTTON2, cmd, false);

}

void Snes9xController::reportBinding(EmuBinding b, bool active)
{
    S9xReportButton(b.hash(), active);
}

void Snes9xController::reportMouseButton(int button, bool pressed)
{
    S9xReportButton(EmuBinding::MOUSE_POINTER + button, pressed);
}

void Snes9xController::reportPointer(int x, int y)
{
    mouse_x += x;
    mouse_y += y;
    S9xReportPointer(EmuBinding::MOUSE_POINTER, mouse_x, mouse_y);
}

static fs::path save_slot_path(int slot)
{
    std::string extension = std::to_string(slot);
    while (extension.length() < 3)
        extension = "0" + extension;
    fs::path path(S9xGetDirectory(SNAPSHOT_DIR));
    path /= fs::path(Memory.ROMFilename).filename();
    path.replace_extension(extension);
    return path;
}

void Snes9xController::loadUndoState()
{
    S9xUnfreezeGame(S9xGetFilename(".undo", SNAPSHOT_DIR).c_str());
}

std::string Snes9xController::getStateFolder()
{
    return S9xGetDirectory(SNAPSHOT_DIR);
}

bool Snes9xController::slotUsed(int slot)
{
    return fs::exists(save_slot_path(slot));
}

bool Snes9xController::loadState(int slot)
{
    return loadState(save_slot_path(slot).u8string());
}

bool Snes9xController::loadState(std::string filename)
{
    if (!active)
        return false;

    S9xFreezeGame(S9xGetFilename(".undo", SNAPSHOT_DIR).c_str());

    if (S9xUnfreezeGame(filename.c_str()))
    {
        auto info_string = filename + " loaded";
        S9xSetInfoString(info_string.c_str());
        return true;
    }
    else
    {
        fprintf(stderr, "Failed to load state file: %s\n", filename.c_str());
        return false;
    }
}

bool Snes9xController::saveState(std::string filename)
{
    if (!active)
        return false;

    if (S9xFreezeGame(filename.c_str()))
    {
        auto info_string = filename + " saved";
        S9xSetInfoString(info_string.c_str());
        return true;
    }
    else
    {
        fprintf(stderr, "Couldn't save state file: %s\n", filename.c_str());
        return false;
    }
}

void Snes9xController::mute(bool muted)
{
    Settings.Mute = muted;
}

bool Snes9xController::isAbnormalSpeed()
{
    return (Settings.TurboMode || rewinding);
}

void Snes9xController::reset()
{
    S9xReset();
}

void Snes9xController::softReset()
{
    S9xSoftReset();
}

bool Snes9xController::saveState(int slot)
{
    return saveState(save_slot_path(slot).u8string());
}

void Snes9xController::setMessage(std::string message)
{
    S9xSetInfoString(message.c_str());
}

std::vector<std::tuple<bool, std::string, std::string>> Snes9xController::getCheatList()
{
    std::vector<std::tuple<bool, std::string, std::string>> cheat_list;

    cheat_list.reserve(Cheat.group.size());

    for (auto &c : Cheat.group)
        cheat_list.push_back({ c.enabled, c.name, S9xCheatGroupToText(c) });

    return std::move(cheat_list);
}

void Snes9xController::disableAllCheats()
{
    for (size_t i = 0; i < Cheat.group.size(); i++)
    {
        S9xDisableCheatGroup(i);
    }
}

void Snes9xController::enableCheat(int index)
{
    S9xEnableCheatGroup(index);
}

void Snes9xController::disableCheat(int index)
{
    S9xDisableCheatGroup(index);
}

bool Snes9xController::addCheat(std::string description, std::string code)
{
    return S9xAddCheatGroup(description, code) >= 0;
}

void Snes9xController::deleteCheat(int index)
{
    S9xDeleteCheatGroup(index);
}

void Snes9xController::deleteAllCheats()
{
    S9xDeleteCheats();
}

int Snes9xController::tryImportCheats(std::string filename)
{
    return S9xImportCheatsFromDatabase(filename);
}

std::string Snes9xController::validateCheat(std::string code)
{
    return S9xCheatValidate(code);
}

int Snes9xController::modifyCheat(int index, std::string name, std::string code)
{
    return S9xModifyCheatGroup(index, name, code);
}

std::string Snes9xController::getContentFolder()
{
    return S9xGetDirectory(ROMFILENAME_DIR);
}