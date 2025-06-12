#pragma once
#include <unordered_map>
#include <string>

struct IniFile
{
    bool load_file(std::string filename);
    std::string get_string(const std::string &key, std::string default_string);
    int get_int(const std::string &key, int default_int);
    float get_float(const std::string &key, float default_float);
    bool get_bool(const std::string &key, bool default_bool);
    std::string get_source(const std::string &key);
    bool exists(const std::string &key);
    std::unordered_map<std::string, std::pair<std::string, std::string>> keys;
};