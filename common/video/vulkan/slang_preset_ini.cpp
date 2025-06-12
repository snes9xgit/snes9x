#include "slang_preset_ini.hpp"
#include "slang_helpers.hpp"
#include <fstream>
#include <cstring>

static std::string trim_comments(std::string str)
{
    for (auto &comment : { "//", "#" })
    {
        auto location = str.rfind(comment);

        if (location != std::string::npos)
            str = str.substr(0, location);
    }

    return trim(str);
}

static std::string trim_quotes(std::string str)
{
    if (str.length() > 1 && str.front() == '\"' && str.back() == '\"')
        return str.substr(1, str.length() - 2);
    return str;
}

bool IniFile::load_file(std::string filename)
{
    std::ifstream file;
    // Windows ifstream wrapper wants c_str
    file.open(filename.c_str(), std::ios_base::binary);
    if (!file.is_open())
    {
        printf("No file %s\n", filename.c_str());
        return false;
    }

    std::string line;
    while (true)
    {
        if (file.eof())
            break;

        std::getline(file, line);

        line = trim(line);

        if (line.find("#reference") == 0)
        {
            std::string reference_path(trim_comments(line.substr(11)));
            reference_path = trim_quotes(reference_path);
            canonicalize(reference_path, filename);
            printf("Loading file %s\n", reference_path.c_str());
            if (!load_file(reference_path))
            {
                printf("Fail %s\n", reference_path.c_str());
                return false;
            }
        }

        line = trim_comments(line);

        if (line.empty())
            continue;

        auto equals = line.find('=');
        if (equals != std::string::npos)
        {
            auto left_side = trim_quotes(trim(line.substr(0, equals)));
            auto right_side = trim_quotes(trim(line.substr(equals + 1)));

            keys.insert_or_assign(left_side, std::make_pair(right_side, filename));
        }
    }

    return true;
}

std::string IniFile::get_string(const std::string &key, std::string default_value = "")
{
    auto it = keys.find(key);
    if (it == keys.end())
        return default_value;

    return it->second.first;
}

int IniFile::get_int(const std::string &key, int default_value = 0)
{
    auto it = keys.find(key);
    if (it == keys.end())
        return default_value;

    return std::stoi(it->second.first);
}

float IniFile::get_float(const std::string &key, float default_value = 0.0f)
{
   auto it = keys.find(key);
   if (it == keys.end())
        return default_value;

    return std::stof(it->second.first);
}

std::string IniFile::get_source(const std::string &key)
{
    auto it = keys.find(key);
    if (it == keys.end())
        return "";

    return it->second.second;
}

bool IniFile::get_bool(const std::string &key, bool default_value = false)
{
   auto it = keys.find(key);
   if (it == keys.end())
        return default_value;

    std::string lower = it->second.first;
    for (auto &c : lower)
        c = tolower(c);

    const char *true_strings[] = { "true", "1", "yes", "on", "y"};
    for (auto &s : true_strings)
        if (lower == s)
            return true;
    return false;
}

bool IniFile::exists(const std::string &key)
{
    auto it = keys.find(key);
    if (it == keys.end())
        return false;
    return true;
}