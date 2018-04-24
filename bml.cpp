#include <ctype.h>
#include <vector>
#include <string.h>
#include <stdio.h>

#include "bml.h"

static inline bml_node *bml_node_new(void)
{
    bml_node *node = new bml_node;

    node->data = NULL;
    node->name = NULL;
    node->depth = -1;
}

static char *strndup_trim (char *str, int len)
{
    int start;
    int end;

    for (start = 0; str[start] && start != len && isblank (str[start]); start++) {}
    if (!str[start] || start >= len)
        return strdup ("");

    for (end = len - 1; isblank (str[end]) || str[end] == '\n' || str[end] == '\r'; end--) {}

    return strndup (str + start, end - start + 1);
}

static inline int bml_valid (char c)
{
    return (isalnum (c) || c == '-');
}

static inline int islf(char c)
{
    return (c == '\r' || c == '\n');
}

static inline unsigned int bml_read_depth (char *data)
{
    unsigned int depth;
    for (depth = 0; isblank (data[depth]); depth++) {}
    return depth;
}

static unsigned int bml_parse_depth (bml_node *node, char **data)
{
    unsigned int depth = bml_read_depth (*data);
    *data += depth;
    node->depth = depth;
}

static char *bml_parse_name (bml_node *node, char **data)
{
    int len;

    for (len = 0; bml_valid(*(*data + len)); len++) {};

    node->name = strndup_trim (*data, len);
    *data += len;
}

static void bml_parse_data (bml_node *node, char **data)
{
    char *p = *data;
    int len;

    if (p[0] == '=' && p[1] == '\"')
    {
        len = 2;
        while (p[len] && !islf (p[len]))
            len++;
        if (p[len] != '\"')
            return;

        node->data = strndup (p + 2, len - 2);
        *data += len + 1;
    }
    else if (*p == '=')
    {
        len = 1;
        while (p[len] && !islf (p[len]) && p[len] != '"' && p[len] != ' ')
            len++;
        if (p[len] == '\"')
            return;
        node->data = strndup_trim (p + 1, len - 1);
        *data += len;
    }
    else if (*p == ':')
    {
        len = 1;
        while (p[len] && !islf (p[len]))
            len++;
        node->data = strndup_trim (p + 1, len - 1);
        *data += len;
    }

    return;
}

static void bml_skip_empty (char **data)
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

static void bml_parse_attr (bml_node *node, char **data)
{
    char *p = *data;
    bml_node *n;
    int len;

    while (*p && !islf (*p))
    {
        if (*p != ' ')
            return;

        while (isblank (*p))
            p++;
        if (p[0] == '/' && p[1] == '/')
            break;

        n = bml_node_new ();
        len = 0;
        while (bml_valid (p[len]))
           len++;
        if (len == 0)
            return;
        n->name = strndup_trim (p, len);
        p += len;
        bml_parse_data (n, &p);
        node->attr.push_back (n);
    }

    *data = p;
}

static int contains_space (char *str)
{
    for (int i = 0; str[i]; i++)
    {
        if (isblank (str[i]))
            return 1;
    }

    return 0;
}

static void bml_print_node (bml_node *node, int depth)
{
    if (!node)
        return;

    for (int i = 0; i < depth * 2; i++)
    {
        printf (" ");
    }

    if (node->name)
        printf ("%s", node->name);

    if (node->data)
    {
        if (contains_space (node->data))
            printf (": \"%s\"", node->data);
        else
            printf (": %s", node->data);
    }

    for (int i = 0; i < node->attr.size(); i++)
    {
        if (node->attr[i]->name)
        {
            printf (" %s", node->attr[i]->name);
            if (node->attr[i]->data)
            {
                if (contains_space (node->attr[i]->data))
                    printf ("=\"%s\"", node->attr[i]->data);
                else
                    printf ("=%s", node->attr[i]->data);
            }
        }
    }

    if (depth >= 0)
        printf ("\n");

    for (int i = 0; i < node->child.size(); i++)
    {
        bml_print_node (node->child[i], depth + 1);
    }

    if (depth == 0)
        printf ("\n");
}

void bml_print_node (bml_node *node)
{
    bml_print_node (node, -1);
}

static bml_node *bml_parse_node (char **doc)
{
    char *line;
    bml_node *node = NULL;

    if ((line = bml_read_line (doc)))
    {
        node = bml_node_new ();

        bml_parse_depth (node, &line);
        bml_parse_name  (node, &line);
        bml_parse_data  (node, &line);
        bml_parse_attr  (node, &line);
    }
    else
        return NULL;

    bml_skip_empty (doc);
    while (*doc && bml_read_depth (*doc) > node->depth)
    {
        bml_node *child = bml_parse_node (doc);

        if (child)
            node->child.push_back (child);

        bml_skip_empty (doc);
    }

    return node;
}

void bml_free_node (bml_node *node)
{
    delete[] (node->name);
    delete[] (node->data);

    for (int i = 0; i < node->child.size(); i++)
    {
        bml_free_node (node->child[i]);
        delete node->child[i];
    }

    for (int i = 0; i < node->attr.size(); i++)
    {
        bml_free_node (node->attr[i]);
        delete node->attr[i];
    }

    return;
}

bml_node *bml_parse (char **doc)
{
    bml_node *root = NULL;
    bml_node *node = NULL;
    char *ptr = *doc;

    root = bml_node_new ();

    while ((node = bml_parse_node (&ptr)))
    {
        root->child.push_back (node);
    }

    if (!root->child.size())
    {
        delete root;
        root = NULL;
    }

    return root;
}

bml_node *bml_parse_file (char *filename)
{
    FILE *file = NULL;
    char *buffer = NULL;
    int file_size = 0;
    bml_node *node = NULL;

    file = fopen (filename, "r");

    if (!file)
        return NULL;

    fseek (file, 0, SEEK_END);
    file_size = ftell (file);
    fseek (file, 0, SEEK_SET);

    buffer = new char[file_size + 1];
    fread (buffer, file_size, 1, file);
    buffer[file_size] = '\0';

    fclose (file);

    node = bml_parse (&buffer);
    delete[] buffer;

    return node;
}
