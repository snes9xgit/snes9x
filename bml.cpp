#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
#include <stdio.h>

#include "port.h"
#include "bml.h"

bml_node::bml_node()
{
    type = CHILD;
    depth = -1;
}

static inline int islf(char c)
{
    return (c == '\r' || c == '\n');
}

static inline int isblank(char c)
{
    return (c == ' ' || c == '\t');
}

static inline int isblankorlf(char c)
{
    return (islf(c) || isblank(c));
}

static inline int isalnum(char c)
{
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'));
}

static inline int bml_valid(char c)
{
    return (isalnum(c) || c == '-');
}

static std::string trim(std::string str)
{
    int start;
    int end;
    for (start = 0; str[start] && start != (int)str.length() && isblank(str[start]); start++) {}
    if (start >= (int)str.length())
        return std::string("");
    for (end = str.length() - 1; isblankorlf(str[end]); end--) {}
    return str.substr(start, end - start + 1);
}

static std::string trimcomments(std::string str)
{
    int end = str.length();
    size_t comment = str.find("//");
    if (comment != std::string::npos)
        end = comment;

    for (int i = end - 1; i >= 0; i--)
    {
        if (!isblankorlf(str[i]))
        {
            end = i + 1;
            break;
        }
    }

    return str.substr(0, end);
}

static inline int bml_read_depth(std::string &data)
{
    size_t depth;
    for (depth = 0; isblank(data[depth]) && depth < data.length(); depth++) {}
    return depth == data.length() ? -1 : depth;
}

static void bml_parse_depth(bml_node &node, std::string &line)
{
    unsigned int depth = bml_read_depth(line);
    line.erase(0, depth);
    node.depth = depth;
}

static void bml_parse_name(bml_node &node, std::string &line)
{
    int len;

    for (len = 0; bml_valid(line[len]); len++) {};

    node.name = trim(line.substr(0, len));
    line.erase(0, len);
}

static void bml_parse_data(bml_node &node, std::string &line)
{
    int len;

    if (line[0] == '=' && line[1] == '\"')
    {
        len = 2;
        while (line[len] && line[len] != '\"' && !islf(line[len]))
            len++;
        if (line[len] != '\"')
            return;

        node.data = line.substr(2, len - 2);
        line.erase(0, len + 1);
    }
    else if (line[0] == '=')
    {
        len = 1;
        while (line[len] && !islf(line[len]) && line[len] != '"' && line[len] != ' ')
            len++;
        if (line[len] == '\"')
            return;
        node.data = line.substr(1, len - 1);
        line.erase(0, len);
    }
    else if (line[0] == ':')
    {
        len = 1;
        while (line[len] && !islf(line[len]))
            len++;
        node.data = trim(line.substr(1, len - 1));
        line.erase(0, len);
    }

    return;
}

static std::string bml_read_line(std::ifstream &fd)
{
    std::string line;

    while (fd)
    {
        std::getline(fd, line);
        line = trimcomments(line);
        if (!line.empty())
        {
            return line;
        }
    }

    return std::string("");
}

static void bml_parse_attr(bml_node &node, std::string &line)
{
    int len;

    while (line.length() > 0)
    {
        if (!isblank(line[0]))
            return;

        while (isblank(line[0]))
            line.erase(0, 1);

        bml_node n;
        len = 0;
        while (bml_valid(line[len]))
            len++;
        if (len == 0)
            return;
        n.name = trim(line.substr(0, len));
        line.erase(0, len);
        bml_parse_data(n, line);
        n.depth = node.depth + 1;
        n.type = bml_node::ATTRIBUTE;
        node.child.push_back(n);
    }
}

static int contains_space(const char *str)
{
    for (int i = 0; str[i]; i++)
    {
        if (isblank(str[i]))
            return 1;
    }

    return 0;
}

static void bml_print_node(bml_node &node, int depth)
{
    int i;

    for (i = 0; i < depth * 2; i++)
    {
        printf(" ");
    }

    if (!node.name.empty())
        printf("%s", node.name.c_str());

    if (!node.data.empty())
    {
        if (contains_space(node.data.c_str()))
            printf("=\"%s\"", node.data.c_str());
        else
            printf(": %s", node.data.c_str());
    }
    for (i = 0; i < (int)node.child.size() && node.child[i].type == bml_node::ATTRIBUTE; i++)
    {
        if (!node.child[i].name.empty())
        {
            printf(" %s", node.child[i].name.c_str());
            if (!node.child[i].data.empty())
            {
                if (contains_space(node.child[i].data.c_str()))
                    printf("=\"%s\"", node.child[i].data.c_str());
                else
                    printf("=%s", node.child[i].data.c_str());
            }
        }
    }

    if (depth >= 0)
        printf("\n");

    for (; i < (int)node.child.size(); i++)
    {
        bml_print_node(node.child[i], depth + 1);
    }

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
        bml_node newnode;
        std::string line = bml_read_line(fd);
        if (line.empty())
            return;

        int line_depth = bml_read_depth(line);
        while (line_depth <= nodestack.top()->depth && nodestack.size() > 1)
            nodestack.pop();

        bml_parse_depth(newnode, line);
        bml_parse_name(newnode, line);
        bml_parse_data(newnode, line);
        bml_parse_attr(newnode, line);

        nodestack.top()->child.push_back(newnode);
        nodestack.push(&nodestack.top()->child.back());
    }

    return;
}

bml_node *bml_node::find_subnode(std::string name)
{
    unsigned int i;

    for (i = 0; i < child.size(); i++)
    {
        if (name.compare(child[i].name) == 0)
            return &child[i];
    }

    return NULL;
}

bool bml_node::parse_file(std::string filename)
{
    std::ifstream file(filename.c_str(), std::ios_base::binary);

    if (!file)
        return false;

    parse(file);

    return true;
}
