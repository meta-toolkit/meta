/**
 * @file graph-test.cpp
 * @author Sean Massung
 */

#include "graph/directed_graph.h"

using namespace meta;

int main()
{
    graph::directed_graph<> g;
    graph::default_node n{"name"};
    g.insert(n);
}
