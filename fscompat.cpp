/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <cstring>
#include <string>

#include "display.h"
#include "fscompat.h"
#include "port.h"
#include "memmap.h"

using std::string;

bool SplitPath::ext_is(const string &other)
{
    if (strcasecmp(ext.c_str(), other.c_str()) == 0)
        return true;
    if (other[0] != '.' && (strcasecmp(other.c_str(), &(ext.c_str()[1])) == 0))
        return true;
    return false;
}

std::string makepath(const SplitPath &path)
{
    return makepath(path.drive, path.dir, path.stem, path.ext);
}

std::string S9xGetFilename(string filename, string ext, enum s9x_getdirtype dirtype)
{
    auto path = splitpath(filename);
    auto dir  = S9xGetDirectory(dirtype);
    return makepath(path.drive, dir, path.stem, ext);
}

std::string S9xGetFilename(string ext, enum s9x_getdirtype dirtype)
{
    return S9xGetFilename(Memory.ROMFilename, ext, dirtype);
}

std::string S9xBasename(std::string filename)
{
    auto path = splitpath(filename);
    return path.stem + path.ext;
}

std::string S9xBasenameNoExt(std::string filename)
{
    return splitpath(filename).stem;
}

#if __cplusplus >= 201703L && 0
#include <filesystem>
namespace fs = std::filesystem;

SplitPath splitpath(string str)
{
    SplitPath output{};
    fs::path path(str);

    if (path.has_root_name())
        output.drive = path.root_name().string();

    if (path.has_filename())
    {
        output.stem = path.stem().string();
        output.ext  = path.extension().string();
        path.remove_filename();
    }

    if (!path.empty())
        output.dir = path.string();

    return output;
}

string makepath(const string &drive, const string &dir, const string &stem, const string &ext)
{
    auto dot_position = ext.find('.');

    if (dot_position == string::npos)
    {
        fs::path path(drive);
        path = path / dir / stem;
        path.replace_extension(ext);
        return path.string();
    }

    auto filename = stem + ext;
    fs::path path(drive);
    path = path / dir / filename;
    return path.string();
}

#else
constexpr auto npos = std::string::npos;

SplitPath splitpath(string path)
{
    SplitPath output{};

#ifdef _WIN32
    if (path.length() > 2 && path[1] == ':')
    {
        output.drive = path.substr(0, 2);
        path = path.substr(2);
    }
#endif
    auto slash = path.rfind(SLASH_CHAR);
    auto dot   = path.rfind('.');

    if (dot != npos && slash != npos && dot < slash)
    {
        dot = npos;
    }
    else if (dot != npos)
    {
        output.ext = path.substr(dot);
    }

    if (slash != npos)
    {
        output.dir = path.substr(0, slash + 1);
        output.stem = path.substr(slash + 1, dot - slash - 1);
    }
    else
    {
        output.stem = path.substr(0, dot);
    }

    return output;
}

string makepath(const string &drive, const string &dir, const string &stem, const string &ext)
{
    string output;

    if (!drive.empty())
    {
        if (!(dir.length() > 2 && dir[1] == ':'))
        {
            output += drive + ":";
            if (!dir.empty() && dir[0] != SLASH_CHAR)
                output += SLASH_CHAR;
        }
    }

    if (!dir.empty())
    {
        output += dir;
        if (output[output.length() - 1] != SLASH_CHAR)
            output += SLASH_CHAR;
    }

    if (!stem.empty())
    {
        output += stem;
    }

    if (!ext.empty())
    {
        if (ext.find('.') == string::npos)
            output += '.';
        output += ext;
    }

    return output;
}

#ifndef _WIN32
void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
    char *slash = strrchr((char *)path, SLASH_CHAR);
    char *dot = strrchr((char *)path, '.');

    *drive = '\0';

    if (dot && slash && dot < slash)
    {
        dot = 0;
    }

    if (!slash)
    {
        *dir = '\0';
        strcpy(fname, path);

        if (dot)
        {
            fname[dot - path] = '\0';
            strcpy(ext, dot + 1);
        }
        else
        {
            *ext = '\0';
        }
    }
    else
    {
        strcpy(dir, path);
        dir[slash - path] = '\0';
        strcpy(fname, slash + 1);

        if (dot)
        {
            fname[(dot - slash) - 1] = '\0';
            strcpy(ext, dot + 1);
        }
        else
        {
            *ext = '\0';
        }
    }
}

void _makepath(char *path, const char *drive, const char *dir, const char *fname, const char *ext)
{
    if (dir && *dir)
    {
        strcpy(path, dir);
        strcat(path, "/");
    }
    else
        *path = '\0';

    strcat(path, fname);

    if (ext && *ext)
    {
        if (!strchr(ext, '.'))
            strcat(path, ".");
        strcat(path, ext);
    }
}
#endif
#endif