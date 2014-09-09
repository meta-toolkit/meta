/**
 * @file graph-test.cpp
 * @author Sean Massung
 */

#include <iostream>

#include "meta.h"
#include "graph/directed_graph.h"
#include "graph/undirected_graph.h"

using namespace meta;

int main(int argc, char* argv[])
{
    graph::undirected_graph<> dg;
    dg.insert(graph::default_node{"hi"});
    std::cout << "size: " << dg.size() << std::endl;

    graph::directed_graph<> ug;
    ug.insert(graph::default_node{"yo"});
    std::cout << "size: " << ug.size() << std::endl;
}
