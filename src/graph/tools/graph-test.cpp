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
    size_t tried = 0;
    size_t errors = 0;
    while (infile >> src >> dest)
    {
        if (seen.find(src) == seen.end())
            seen[src] = g.insert(graph::default_node{src});

        if (seen.find(dest) == seen.end())
            seen[dest] = g.insert(graph::default_node{dest});

        ++tried;

        try
        {
            g.add_edge(seen[src], seen[dest]);
        }
        catch (graph::undirected_graph_exception& e)
        {
            ++errors;
            // std::cout << e.what() << std::endl;
        }
    }

    std::cout << "Tried to add " << tried << " total edges." << std::endl;
    std::cout << "Got " << errors << " errors." << std::endl;

    return g;
}

void degree_dist(graph::undirected_graph<>& g)
{
    std::ofstream outfile{"degrees.dat"};
    std::unordered_map<size_t, size_t> counts;
    for(auto& node: g)
        ++counts[g.adjacent(node.id).size()];

    using pair_t = std::pair<size_t, size_t>;
    std::vector<pair_t> sorted{counts.begin(), counts.end()};
    std::sort(sorted.begin(), sorted.end(), [&](const pair_t& a, const pair_t& b)
    {
        return a.first > b.first;
    });

    for(auto& c: sorted)
        outfile << c.first << " " << c.second << "\n";
}

int main(int argc, char* argv[])
{
    auto g = load("CA-GrQc.txt");
    degree_dist(g);
    std::cout << "Number of nodes: " << g.size() << std::endl;

    size_t num_edges = 0;
    for(auto it = g.edges_begin(); it != g.edges_end(); ++it)
        ++num_edges;

    std::cout << "Number of edges: " << num_edges << std::endl;
    std::cout << "Number of edges: " << g.num_edges() << std::endl;
}
