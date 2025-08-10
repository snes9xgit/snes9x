#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
#include <cstdio>

#include "port.h"
#include "bml.h"

bml_node::bml_node()
{
    type = CHILD;
    depth = -1;
}

static inline bool starts_with(const std::string_view &s, const std::string_view &prefix)
{
    if (s.length() < prefix.length())
        return false;

    for (size_t i = 0; i < prefix.length(); i++)
    {
        if (s[i] != prefix[i])
            return false;
    }

    return true;
}

static inline bool islf(const char c)
{
    return (c == '\r' || c == '\n');
}

static inline bool isblank(const char c)
{
    return (c == ' ' || c == '\t');
}

static inline bool isblankorlf(const char c)
{
    return (islf(c) || isblank(c));
}

static inline bool isalnum(const char c)
{
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'));
}

static inline bool bml_valid(char c)
{
    return (isalnum(c) || c == '-');
}

static std::string trim(std::string_view sv)
{
    while (!sv.empty() && isblank(sv.front()))
        sv.remove_prefix(1);
    while (!sv.empty() && isblankorlf(sv.back()))
        sv.remove_suffix(1);
    return std::string(sv);
}

static std::string trim_comments(std::string_view sv)
{
    auto comment_location = sv.find("//");
    if (comment_location != std::string::npos)
        sv.remove_suffix(sv.length() - comment_location);
    while (!sv.empty() && isblankorlf(sv.back()))
        sv.remove_suffix(1);
    return std::string(sv);
}

static inline int bml_read_depth(const std::string_view &str)
{
    auto depth = str.find_first_not_of("\t ");
    return depth == std::string::npos ? -1 : static_cast<int>(depth);
}

static void bml_parse_depth(bml_node &node, std::string_view &line)
{
    auto depth = bml_read_depth(line);
    line.remove_prefix(depth);
    node.depth = depth;
}

static void bml_parse_name(bml_node &node, std::string_view &line)
{
    int len;
    for (len = 0; len < line.length() && bml_valid(line[len]); len++) {};

    node.name = trim(line.substr(0, len));
    line.remove_prefix(len);
}

static void bml_parse_data(bml_node &node, std::string_view &line)
{
    if (starts_with(line, "=\"")) {
        line.remove_prefix(2);
        auto quote_pos = line.find_first_of("\"\n\r");
        if (quote_pos != std::string::npos && line[quote_pos] == '\"') {
            node.data = line.substr(0, quote_pos);
            line.remove_prefix(quote_pos + 1);
        }
    }
    else if (line[0] == '=') {
        line.remove_prefix(1);
        auto end_pos = line.find_first_of("\n\r ");
        node.data = trim(line.substr(0, end_pos));
        line.remove_prefix(end_pos);
    }
    else if (line[0] == ':') {
        line.remove_prefix(1);
        node.data = trim(line);
        line = {};
    }
}

static std::string bml_read_line(std::ifstream &fd)
{
    std::string line;

    while (fd)
    {
        std::getline(fd, line);
        line = trim_comments(line);
        if (!line.empty())
        {
            return line;
        }
    }

    return {};
}

static void bml_parse_attr(bml_node &node, std::string_view &line)
{
    while (!line.empty())
    {
        if (!isblank(line[0]))
            return;

        while (isblank(line[0]))
            line.remove_prefix(1);

        bml_node n;
        int len = 0;
        while (len < line.length() && bml_valid(line[len]))
            len++;
        if (len == 0)
            return;
        n.name = trim(line.substr(0, len));
        line.remove_prefix(len);
        bml_parse_data(n, line);
        n.depth = node.depth + 1;
        n.type = bml_node::ATTRIBUTE;
        node.child.push_back(n);
    }
}

static bool contains_space(const std::string &str)
{
    for (int i = 0; str[i]; i++)
    {
        if (isblank(str[i]))
            return true;
    }

    return false;
}

static void bml_print_node(bml_node &node, int depth)
{
    for (int i = 0; i < depth * 2; i++)
    {
        printf(" ");
    }

    if (!node.name.empty())
        printf("%s", node.name.c_str());

    if (!node.data.empty())
    {
        if (contains_space(node.data))
            printf("=\"%s\"", node.data.c_str());
        else
            printf(": %s", node.data.c_str());
    }
    for (size_t i = 0; i < node.child.size() && node.child[i].type == bml_node::ATTRIBUTE; i++)
    {
        if (!node.child[i].name.empty())
        {
            printf(" %s", node.child[i].name.c_str());
            if (!node.child[i].data.empty())
            {
                if (contains_space(node.child[i].data))
                    printf("=\"%s\"", node.child[i].data.c_str());
                else
                    printf("=%s", node.child[i].data.c_str());
            }
        }
    }

    if (depth >= 0)
        printf("\n");

    for (auto &child : node.child)
        bml_print_node(child, depth + 1);

    if (depth == 0)
        printf("\n");
}

void bml_node::print()
{
    bml_print_node(*this, -1);
}

void bml_node::parse(std::ifstream &fd)
{
    std::stack<bml_node *> nodestack;
    nodestack.push(this);

    while (fd)
    {
        bml_node new_node;
        std::string line = bml_read_line(fd);
        std::string_view sv(line);
        if (line.empty())
            return;

        int line_depth = bml_read_depth(line);
        while (line_depth <= nodestack.top()->depth && nodestack.size() > 1)
            nodestack.pop();

        bml_parse_depth(new_node, sv);
        bml_parse_name(new_node, sv);
        bml_parse_data(new_node, sv);
        bml_parse_attr(new_node, sv);

        nodestack.top()->child.push_back(new_node);
        nodestack.push(&nodestack.top()->child.back());
    }
}

bml_node *bml_node::find_subnode(const std::string &name)
{
    for (auto &i : child)
        if (name == i.name)
            return &i;

    return nullptr;
}

bool bml_node::parse_file(const std::string &filename)
{
    std::ifstream file(filename.c_str(), std::ios_base::binary);

    if (!file)
        return false;

    parse(file);

    return true;
}
