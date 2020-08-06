/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <sys/stat.h>
#include <errno.h>

#include "gtk_compat.h"
#include "gtk_s9x.h"
#include "display.h"
#include "memmap.h"
#include "snapshot.h"
#include "cheats.h"

static char buf[PATH_MAX];

const char *S9xChooseMovieFilename(bool8 read_only)
{
    static char path[PATH_MAX];

    if (!gui_config->rom_loaded)
        return "";

    auto filename = top_level->open_movie_dialog(read_only);
    strcpy(path, filename.c_str());

    return path;
}

const char *S9xGetFilenameInc(const char *e, enum s9x_getdirtype dirtype)
{
    static char filename[PATH_MAX + 1];
    char dir[_MAX_DIR + 1];
    char drive[_MAX_DRIVE + 1];
    char fname[_MAX_FNAME + 1];
    char ext[_MAX_EXT + 1];
    unsigned int i = 0;
    struct stat buf;
    const char *d;

    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
    d = S9xGetDirectory(dirtype);

    do
    {
        snprintf(filename, PATH_MAX, "%s" SLASH_STR "%s%03d%s", d, fname, i, e);
        i++;
    } while (stat(filename, &buf) == 0 && i != 0); /* Overflow? ...riiight :-) */

    return (filename);
}

const char *S9xGetDirectory(enum s9x_getdirtype dirtype)
{
    static char path[PATH_MAX + 1];

    switch (dirtype)
    {
    case HOME_DIR:
        snprintf(path, PATH_MAX + 1, "%s", get_config_dir().c_str());
        break;

    case SNAPSHOT_DIR:
        snprintf(path, PATH_MAX + 1, "%s", gui_config->savestate_directory.c_str());
        break;

    case PATCH_DIR:
        snprintf(path, PATH_MAX + 1, "%s", gui_config->patch_directory.c_str());
        break;

    case CHEAT_DIR:
        snprintf(path, PATH_MAX + 1, "%s", gui_config->cheat_directory.c_str());
        break;

    case SRAM_DIR:
        snprintf(path, PATH_MAX + 1, "%s", gui_config->sram_directory.c_str());
        break;

    case SCREENSHOT_DIR:
    case SPC_DIR:
        snprintf(path, PATH_MAX + 1, "%s", gui_config->export_directory.c_str());
        break;

    default:
        path[0] = '\0';
    }

    /* Check if directory exists, make it and/or set correct permissions */
    if (dirtype != HOME_DIR && path[0] != '\0')
    {
        struct stat file_info;

        if (stat(path, &file_info) == -1)
        {
            mkdir(path, 0755);
            chmod(path, 0755);
        }
        else if (!(file_info.st_mode & 0700))
            chmod(path, file_info.st_mode | 0700);
    }

    /* Anything else, use ROM filename path */
    if (path[0] == '\0')
    {
        char *loc;

        strcpy(path, Memory.ROMFilename);

        loc = strrchr(path, SLASH_CHAR);

        if (loc == NULL)
        {
            if (getcwd(path, PATH_MAX + 1) == NULL)
            {
                strcpy(path, getenv("HOME"));
            }
        }
        else
        {
            path[loc - path] = '\0';
        }
    }

    return path;
}

const char *S9xGetFilename(const char *ex, enum s9x_getdirtype dirtype)
{
    static char filename[PATH_MAX + 1];
    char dir[_MAX_DIR + 1];
    char drive[_MAX_DRIVE + 1];
    char fname[_MAX_FNAME + 1];
    char ext[_MAX_EXT + 1];

    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);

    snprintf(filename, sizeof(filename), "%s" SLASH_STR "%s%s",
             S9xGetDirectory(dirtype), fname, ex);

    return (filename);
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
    static char filename[PATH_MAX];
    const char *base, *ext;

    if (!(base = strrchr(f, SLASH_CHAR)))
        base = f;
    else
        base += 1;

    ext = strrchr(f, '.');

    if (!ext)
        snprintf(filename, PATH_MAX, "%s", base);
    else
    {
        int len = ext - base;
        strncpy(filename, base, len);
        filename[len] = '\0';
    }

    return filename;
}

static int file_exists(const char *name)
{
    FILE *f = NULL;

    f = fopen(name, "r");

    if (!f)
        return 0;
    else
    {
        fclose(f);
        return 1;
    }
}

bool8 S9xOpenSnapshotFile(const char *fname, bool8 read_only, STREAM *file)
{
    char filename[PATH_MAX + 1];
    char drive[_MAX_DRIVE + 1];
    char dir[_MAX_DIR + 1];
    char ext[_MAX_EXT + 1];

    _splitpath(fname, drive, dir, filename, ext);

    if (*drive || *dir == '/' || (*dir == '.' && (*(dir + 1) == '/')))
    {
        snprintf(filename, PATH_MAX + 1, "%s", fname);

        if (!file_exists(filename))
        {
            if (!*ext)
                strcat(filename, ".s9x");
        }
    }
    else
    {
        strcpy(filename, S9xGetDirectory(SNAPSHOT_DIR));
        strcat(filename, SLASH_STR);
        strcat(filename, fname);

        if (!file_exists(filename))
        {
            if (!*ext)
                strcat(filename, ".s9x");
        }
    }

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
        sprintf(buf, "%s loaded", filename);
        S9xSetInfoString(buf);
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
        sprintf(buf, "%s saved", filename);
        S9xSetInfoString(buf);
    }
    else
    {
        fprintf(stderr, "Couldn't save state file: %s\n", filename);
    }
}

/* QuickSave/Load from S9x base controls.cpp */
void S9xQuickSaveSlot(int slot)
{
    char def[PATH_MAX];
    char filename[PATH_MAX];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char ext[_MAX_EXT];

    if (!gui_config->rom_loaded)
        return;

    _splitpath(Memory.ROMFilename, drive, dir, def, ext);

    snprintf(filename, PATH_MAX, "%s%s%s.%03d",
             S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR, def,
             slot);

    if (S9xFreezeGame(filename))
    {
        snprintf(buf, PATH_MAX, "%s.%03d saved", def, slot);

        S9xSetInfoString(buf);
    }
}

void S9xQuickLoadSlot(int slot)
{
    char def[PATH_MAX];
    char filename[PATH_MAX];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char ext[_MAX_EXT];

    if (!gui_config->rom_loaded)
        return;

    _splitpath(Memory.ROMFilename, drive, dir, def, ext);

    snprintf(filename, PATH_MAX, "%s%s%s.%03d",
             S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR, def,
             slot);

    if (file_exists(filename))
        S9xFreezeGame(S9xGetFilename(".undo", SNAPSHOT_DIR));

    if (S9xUnfreezeGame(filename))
    {
        snprintf(buf, PATH_MAX, "%s.%03d loaded", def, slot);
        S9xSetInfoString(buf);
        return;
    }

    static const char *digits = "t123456789";

    _splitpath(Memory.ROMFilename, drive, dir, def, ext);

    snprintf(filename, PATH_MAX, "%s%s%s.zs%c",
             S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR,
             def, digits[slot]);

    if (file_exists(filename))
        S9xFreezeGame(S9xGetFilename(".undo", SNAPSHOT_DIR));

    if (S9xUnfreezeGame(filename))
    {
        snprintf(buf, PATH_MAX,
                 "Loaded ZSNES freeze file %s.zs%c",
                 def, digits[slot]);
        S9xSetInfoString(buf);
        return;
    }

    S9xMessage(S9X_ERROR,
               S9X_FREEZE_FILE_NOT_FOUND,
               "Freeze file not found");
}
