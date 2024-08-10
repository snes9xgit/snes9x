#pragma once
#include <unordered_map>
#include <string>

struct IniFile
{
    IniFile();
    ~IniFile();
    bool load_file(std::string filename);
    std::string get_string(std::string key, std::string default_string);
    int get_int(std::string key, int default_int);
    float get_float(std::string key, float default_float);
    bool get_bool(std::string key, bool default_bool);
    std::string get_source(std::string key);
    bool exists(std::string key);
    std::unordered_map<std::string, std::pair<std::string, std::string>> keys;
};