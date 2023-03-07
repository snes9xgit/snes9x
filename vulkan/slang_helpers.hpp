#pragma once
#include <string>
#include <vector>
#include <string_view>

int get_significant_digits(const std::string_view &view);
std::string trim(const std::string &str);
void trim(std::string_view &view);
std::vector<std::string> split_string(const std::string_view &str, unsigned char delim);
std::vector<std::string> split_string_quotes(const std::string_view &view);
bool ends_with(const std::string &str, const std::string &ext);
void canonicalize(std::string &filename, const std::string &base);
int mipmap_levels_for_size(int width, int height);