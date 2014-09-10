/**
 * @file graph-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

#include "meta.h"
#include "graph/undirected_graph.h"

using namespace meta;

graph::undirected_graph<> load(const std::string& filename)
{
    graph::undirected_graph<> g;

    std::string src;
    std::string dest;
    std::ifstream infile{"CA-GrQc.txt"};
    std::unordered_map<std::string, node_id> seen;
    while (infile >> src >> dest)
    {
        if (seen.find(src) == seen.end())
            seen[src] = g.insert(graph::default_node{src});

        if (seen.find(dest) == seen.end())
            seen[dest] = g.insert(graph::default_node{dest});

        try
        {
            g.add_edge(seen[src], seen[dest]);
        }
        catch (graph::undirected_graph_exception& e)
        {
            std::cout << "Warning: attempted to add duplicate edge"
                      << std::endl;
        }
    }

    return g;
}

int main(int argc, char* argv[])
{
    auto g = load("CA-GrQc.txt");

    for(auto& node: g)
    {
        std::cout << node.label << std::endl;
        std::cout << " adj: " << g.adjacent(node.id).size() << std::endl;
    }

    std::cout << "Number of nodes: " << g.size() << std::endl;
}
