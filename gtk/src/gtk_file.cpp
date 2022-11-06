/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <sys/stat.h>
#include <errno.h>
#include <string>
#include <filesystem>

#include "gtk_compat.h"
#include "gtk_s9x.h"
#include "display.h"
#include "memmap.h"
#include "snapshot.h"
#include "cheats.h"

namespace fs = std::filesystem;
using namespace std::literals;

static std::string info_string;

const char *S9xGetFilenameInc(const char *e, enum s9x_getdirtype dirtype)
{
    static std::string filename;

    fs::path rom_filename(Memory.ROMFilename);

    fs::path filename_base(S9xGetDirectory(dirtype));
    filename_base /= rom_filename.filename();

    fs::path new_filename;

    int i = 0;
    do
    {
        std::string new_extension = std::to_string(i);
        while (new_extension.length() < 3)
            new_extension = "0"s + new_extension;
        new_extension += "."s + e;

        new_filename = filename_base;
        new_filename.replace_extension(new_extension);

        i++;
    } while (fs::exists(new_filename));

    filename = new_filename;
    return filename.c_str();
}

const char *S9xGetDirectory(enum s9x_getdirtype dirtype)
{
    static std::string dirname;

    switch (dirtype)
    {
    case HOME_DIR:
        dirname = get_config_dir();
        break;

    case SNAPSHOT_DIR:
        dirname = gui_config->savestate_directory;
        break;

    case PATCH_DIR:
        dirname = gui_config->patch_directory;
        break;

    case CHEAT_DIR:
        dirname = gui_config->cheat_directory;
        break;

    case SRAM_DIR:
        dirname = gui_config->sram_directory;
        break;

    case SCREENSHOT_DIR:
    case SPC_DIR:
        dirname = gui_config->export_directory;
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
            fs::create_directory(path);
        }
        else if ((fs::status(path).permissions() & fs::perms::owner_write) == fs::perms::none)
        {
            fs::permissions(path, fs::perms::owner_write, fs::perm_options::add);
        }
    }

    /* Anything else, use ROM filename path */
    if (dirname == "")
    {
        fs::path path(Memory.ROMFilename);

        path.remove_filename();

        if (!fs::is_directory(path))
            dirname = fs::current_path();
        else
            dirname = path;
    }

    return dirname.c_str();
}

const char *S9xGetFilename(const char *ex, enum s9x_getdirtype dirtype)
{
    static std::string filename;
    fs::path path(S9xGetDirectory(dirtype));
    path /= fs::path(Memory.ROMFilename).filename();
    //Fixes issue with MSU-1 on linux
    if(ex[0] == '-') {
        path.replace_extension("");
        filename = path.string();
        filename.append(ex);
    } else {
        path.replace_extension(ex);
        filename = path.string();
    }
    return filename.c_str();
}

const char *S9xBasename(const char *f)
{
    const char *p;

    if ((p = strrchr(f, '/')) != NULL || (p = strrchr(f, '\\')) != NULL)
        return (p + 1);

    return f;
}

const char *S9xBasenameNoExt(const char *f)
{
    static std::string filename;
    filename = fs::path(f).stem();
    return filename.c_str();
}

bool8 S9xOpenSnapshotFile(const char *filename, bool8 read_only, STREAM *file)
{
#ifdef ZLIB
    if (read_only)
    {
        if ((*file = OPEN_STREAM(filename, "rb")))
            return (true);
        else
            fprintf(stderr,
                    "Failed to open file stream for reading. (%s)\n",
                    zError(errno));
    }
    else
    {
        if ((*file = OPEN_STREAM(filename, "wb")))
        {
            return (true);
        }
        else
        {
            fprintf(stderr,
                    "Couldn't open stream with zlib. (%s)\n",
                    zError(errno));
        }
    }

    fprintf(stderr, "zlib: Couldn't open snapshot file:\n%s\n", filename);

#else
    char command[PATH_MAX];

    if (read_only)
    {
        sprintf(command, "gzip -d <\"%s\"", filename);
        if (*file = popen(command, "r"))
            return (true);
    }
    else
    {
        sprintf(command, "gzip --best >\"%s\"", filename);
        if (*file = popen(command, "wb"))
            return (true);
    }

    fprintf(stderr, "gzip: Couldn't open snapshot file:\n%s\n", filename);

#endif

    return (false);
}

void S9xCloseSnapshotFile(STREAM file)
{
#ifdef ZLIB
    CLOSE_STREAM(file);
#else
    pclose(file);
#endif
}

void S9xAutoSaveSRAM()
{
    Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR));
    S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
}

void S9xLoadState(const char *filename)
{
    S9xFreezeGame(S9xGetFilename(".undo", SNAPSHOT_DIR));

    if (S9xUnfreezeGame(filename))
    {
        info_string = filename + " loaded"s;
        S9xSetInfoString(info_string.c_str());
    }
    else
    {
        fprintf(stderr, "Failed to load state file: %s\n", filename);
    }
}

void S9xSaveState(const char *filename)
{
    if (S9xFreezeGame(filename))
    {
        info_string = filename + " saved"s;
        S9xSetInfoString(info_string.c_str());
    }
    else
    {
        fprintf(stderr, "Couldn't save state file: %s\n", filename);
    }
}

static fs::path save_slot_path(int slot)
{
    std::string extension = std::to_string(slot);
    while (extension.length() < 3)
        extension = "0"s + extension;
    fs::path path(S9xGetDirectory(SNAPSHOT_DIR));
    path /= fs::path(Memory.ROMFilename).filename();
    path.replace_extension(extension);
    return path;
}

/* QuickSave/Load from S9x base controls.cpp */
void S9xQuickSaveSlot(int slot)
{
    if (!gui_config->rom_loaded)
        return;

    auto filename = save_slot_path(slot);

    if (S9xFreezeGame(filename.c_str()))
    {
        info_string = filename.filename().string() + " saved";
        S9xSetInfoString(info_string.c_str());
    }
}

void S9xQuickLoadSlot(int slot)
{
    if (!gui_config->rom_loaded)
        return;

    auto filename = save_slot_path(slot);

    if (fs::exists(filename))
        S9xFreezeGame(S9xGetFilename(".undo", SNAPSHOT_DIR));

    if (S9xUnfreezeGame(filename.c_str()))
    {
        info_string = filename.filename().string() + " loaded";
        S9xSetInfoString(info_string.c_str());
        return;
    }

    S9xMessage(S9X_ERROR,
               S9X_FREEZE_FILE_NOT_FOUND,
               "Freeze file not found");
}
