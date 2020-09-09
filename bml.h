#ifndef __BML_H
#define __BML_H
#include <vector>
#include <string>
#include <fstream>

struct bml_node
{
    enum node_type {
        CHILD,
        ATTRIBUTE
    };

    bml_node();
    bool parse_file(std::string filename);
    void parse(std::ifstream &fd);
    bml_node *find_subnode(std::string name);
    void print();

    std::string name;
    std::string data;
    int depth;
    std::vector<bml_node> child;
    node_type type;
};

#endif
