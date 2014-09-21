/**
 * @file graph-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>

#include "meta.h"
#include "graph/undirected_graph.h"
#include "graph/algorithms.h"

using namespace meta;

graph::undirected_graph<> load(const std::string& filename)
{
    graph::undirected_graph<> g;

    std::string src;
    std::string dest;
    std::ifstream infile{filename};
    std::unordered_map<std::string, node_id> seen;
    size_t tried = 0;
    size_t errors = 0;
    while (infile >> src >> dest)
    {
        ++tried;

        if (src == dest)
        {
            std::cout << "Found self-loop: " << src << std::endl;
            continue;
        }

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
            ++errors;
            // std::cout << e.what() << std::endl;
        }
    }

    std::cout << "Tried to add " << tried << " total edges." << std::endl;
    std::cout << "Got " << errors << " errors." << std::endl;

    return g;
}

void degree_dist(graph::undirected_graph<>& g, const std::string& outfile)
{
    using pair_t = std::pair<node_id, size_t>;
    std::vector<pair_t> degrees;
    for (auto& node : g)
        degrees.emplace_back(node.id, g.adjacent(node.id).size());

    std::sort(degrees.begin(), degrees.end(),
              [&](const pair_t& a, const pair_t& b)
              {
        return a.second > b.second;
    });

    std::ofstream out{outfile};
    for (auto& c : degrees)
        out << c.first << " " << c.second << "\n";
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " graph-file.txt" << std::endl;
        std::cout << "The file contains space-delimited pairs of vertices, "
                     "representing edges in the graph" << std::endl;
        return 1;
    }

    graph::undirected_graph<> g1;
    graph::algorithms::random_graph(g1, 5242, 14496);
    degree_dist(g1, "g1-degrees.dat");
    std::cout << "g1 CC: " << graph::algorithms::clustering_coefficient(g1)
              << std::endl;

    graph::undirected_graph<> g2;
    graph::algorithms::watts_strogatz(g2, 5242, 4, 4012);
    degree_dist(g2, "g2-degrees.dat");
    std::cout << "g2 CC: " << graph::algorithms::clustering_coefficient(g2)
              << std::endl;

    auto g3 = load(argv[1]);
    degree_dist(g3, "g3-degrees.dat");
    std::cout << "g3 CC: " << graph::algorithms::clustering_coefficient(g3)
              << std::endl;
}
