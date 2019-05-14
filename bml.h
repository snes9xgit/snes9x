#ifndef __BML_H
#define __BML_H
#include <vector>
#include <string>

const int bml_attr_type = -2;

struct bml_node
{
    bml_node();
    bool parse_file(const char *filename);
    void parse(char *buffer);
    bml_node *find_subnode(std::string name);
    void print();

    static const int bml_attr_type = -2;
    std::string name;
    std::string data;
    int depth;
    std::vector<bml_node> child;
};

#endif
