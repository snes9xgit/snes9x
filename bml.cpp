#include <vector>
#include <string.h>
#include <stdio.h>

#include "port.h"
#include "bml.h"

bml_node::bml_node()
{
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

static inline int isalnum(char c)
{
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'));
}

static inline int bml_valid (char c)
{
    return (isalnum (c) || c == '-');
}

static std::string trim(std::string str)
{
    int start;
    int end;
    for (start = 0; str[start] && start != (int)str.length() && isblank (str[start]); start++) {}
    if (start >= (int)str.length())
        return std::string("");
    for (end = str.length() - 1; isblank (str[end]) || str[end] == '\n' || str[end] == '\r'; end--) {}
    return str.substr(start, end - start + 1);
}

static inline unsigned int bml_read_depth(char *data)
{
    unsigned int depth;
    for (depth = 0; isblank(data[depth]); depth++) {}
    return depth;
}

static void bml_parse_depth(bml_node &node, char **data)
{
    unsigned int depth = bml_read_depth(*data);
    *data += depth;
    node.depth = depth;
}

static void bml_parse_name(bml_node &node, char **data)
{
    int len;

    for (len = 0; bml_valid(*(*data + len)); len++) {};

    node.name = trim(std::string(*data, len));
    *data += len;
}

static void bml_parse_data(bml_node &node, char **data)
{
    char *p = *data;
    int len;

    if (p[0] == '=' && p[1] == '\"')
    {
        len = 2;
        while (p[len] && p[len] != '\"' && !islf(p[len]))
            len++;
        if (p[len] != '\"')
            return;

        node.data = std::string(p + 2, len - 2);
        *data += len + 1;
    }
    else if (*p == '=')
    {
        len = 1;
        while (p[len] && !islf(p[len]) && p[len] != '"' && p[len] != ' ')
            len++;
        if (p[len] == '\"')
            return;
        node.data = std::string(p + 1, len - 1);
        *data += len;
    }
    else if (*p == ':')
    {
        len = 1;
        while (p[len] && !islf(p[len]))
            len++;
        node.data = std::string(p + 1, len - 1);
        *data += len;
    }

    return;
}

static void bml_skip_empty(char **data)
{
    char *p = *data;

    while (*p)
    {
        for (; *p && isblank (*p) ; p++) {}

        if (!islf(p[0]) && (p[0] != '/' && p[1] != '/'))
            return;

        /* Skip comment data */
        while (*p && *p != '\r' && *p != '\n')
            p++;

        /* If we found new-line, try to skip more */
        if (*p)
        {
            p++;
            *data = p;
        }
    }
}

static char *bml_read_line (char **data)
{
    char *line;
    char *p;

    bml_skip_empty (data);

    line = *data;

    if (line == NULL || *line == '\0')
        return NULL;

    p = strpbrk (line, "\r\n\0");

    if (p == NULL)
        return NULL;

    if (islf (*p))
    {
        *p = '\0';
        p++;
    }

    *data = p;

    return line;
}

static void bml_parse_attr(bml_node &node, char **data)
{
    char *p = *data;
    int len;

    while (*p && !islf(*p))
    {
        if (*p != ' ')
            return;

        while (isblank(*p))
            p++;
        if (p[0] == '/' && p[1] == '/')
            break;

        bml_node n;
        len = 0;
        while (bml_valid(p[len]))
            len++;
        if (len == 0)
            return;
        n.name = trim(std::string(p, len));
        p += len;
        bml_parse_data(n, &p);
        n.depth = bml_attr_type;
        node.child.push_back(n);
    }

    *data = p;
}

static int contains_space (const char *str)
{
    for (int i = 0; str[i]; i++)
    {
        if (isblank (str[i]))
            return 1;
    }

    return 0;
}

static void bml_print_node(bml_node &node, int depth)
{
    int i;

    for (i = 0; i < depth * 2; i++)
    {
        printf (" ");
    }

    if (!node.name.empty())
        printf ("%s", node.name.c_str());

    if (!node.data.empty())
    {
        if (contains_space(node.data.c_str()))
            printf ("=\"%s\"", node.data.c_str());
        else
            printf (": %s", node.data.c_str());
    }
    for (i = 0; i < (int) node.child.size () && node.child[i].depth == bml_attr_type; i++)
    {
        if (!node.child[i].name.empty())
        {
            printf (" %s", node.child[i].name.c_str());
            if (!node.child[i].data.empty())
            {
                if (contains_space(node.child[i].data.c_str()))
                    printf ("=\"%s\"", node.child[i].data.c_str());
                else
                    printf ("=%s", node.child[i].data.c_str());
            }
        }
    }

    if (depth >= 0)
        printf ("\n");

    for (; i < (int) node.child.size(); i++)
    {
        bml_print_node (node.child[i], depth + 1);
    }

    if (depth == 0)
        printf ("\n");
}

void bml_node::print()
{
    bml_print_node(*this, -1);
}

static bml_node bml_parse_node(char **doc)
{
    char *line;
    bml_node node;

    if ((line = bml_read_line(doc)))
    {
        bml_parse_depth(node, &line);
        bml_parse_name(node, &line);
        bml_parse_data(node, &line);
        bml_parse_attr(node, &line);
    }
    else
        return node;

    bml_skip_empty(doc);
    while (*doc && (int)bml_read_depth(*doc) > node.depth)
    {
        bml_node child = bml_parse_node(doc);

        if (child.depth != -1)
            node.child.push_back(child);

        bml_skip_empty(doc);
    }

    return node;
}

void bml_node::parse(char *doc)
{
    bml_node node;
    char *ptr = doc;

    while ((node = bml_parse_node (&ptr)).depth != -1)
    {
        child.push_back(node);
    }

    return;
}

bml_node *bml_node::find_subnode (std::string name)
{
    unsigned int i;

    for (i = 0; i < child.size (); i++)
    {
        if (name.compare(child[i].name) == 0)
            return &child[i];
    }

    return NULL;
}

bool bml_node::parse_file(const char *filename)
{
    FILE *file = NULL;
    char *buffer = NULL;
    int file_size = 0;

    file = fopen(filename, "rb");

    if (!file)
        return false;

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = new char[file_size + 1];
    fread(buffer, file_size, 1, file);
    buffer[file_size] = '\0';
    fclose(file);

    parse(buffer);
    delete[] buffer;

    return true;
}
