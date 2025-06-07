#include "slang_helpers.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <cmath>
#include <cctype>

using std::string;
using std::string_view;
using std::vector;
namespace fs = std::filesystem;

int mipmap_levels_for_size(int width, int height)
{
    return (int)log2(width > height ? width : height);
}

void trim(string_view &view)
{
    while (!view.empty() && isspace((unsigned char)view.at(0)))
        view.remove_prefix(1);

    while (!view.empty() && isspace((unsigned char)view.at(view.length() - 1)))
        view.remove_suffix(1);
}

string trim(const string& str)
{
    string_view sv(str);
    trim(sv);
    return string(sv);
}

int get_significant_digits(const string_view &view)
{
    auto pos = view.rfind('.');
    if (pos == string_view::npos)
        return 0;
    return view.length() - pos - 1;
}

vector<string> split_string_quotes(const string_view &view)
{
    vector<string> tokens;
    size_t pos = 0;

    while (pos < view.length())
    {
        size_t indexa = view.find_first_not_of("\t\r\n ", pos);
        size_t indexb = 0;
        if (indexa == string::npos)
            break;

        if (view.at(indexa) == '\"')
        {
            indexa++;
            indexb = view.find_first_of('\"', indexa);
            if (indexb == string::npos)
                break;
        }
        else
        {
            indexb = view.find_first_of("\t\r\n ", indexa);
            if (indexb == string::npos)
                indexb = view.length();
        }

        if (indexb > indexa)
            tokens.push_back(string{view.substr(indexa, indexb - indexa)});
        pos = indexb + 1;
    }

    return tokens;
}

vector<string> split_string(const string_view &str, unsigned char delim)
{
    vector<string> tokens;
    size_t pos = 0;

    while (pos < str.length())
    {
        size_t index = str.find(delim, pos);
        if (index == string::npos)
        {
            if (pos < str.length())
            {
                tokens.push_back(string{str.substr(pos)});
            }

            break;
        }
        else if (index > pos)
        {
            tokens.push_back(string{str.substr(pos, index - pos)});
        }

        pos = index + 1;
    }

    return tokens;
}

bool ends_with(const string &str, const string &ext)
{
    if (ext.length() > str.length())
        return false;

    auto icmp = [](const unsigned char a, const unsigned char b) -> bool {
        return std::tolower(a) == std::tolower(b);
    };

    return std::equal(ext.crbegin(), ext.crend(), str.crbegin(), icmp);
}

void canonicalize(string &filename, const string &base)
{
    fs::path path(filename);

    if (path.is_relative())
    {
        fs::path base_path(base);
        base_path.remove_filename();
        path = fs::weakly_canonical(base_path / path);
        filename = path.string();
    }
}
