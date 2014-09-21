/**
 * @file graph-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>

#include "meta.h"
#include "graph/undirected_graph.h"
#include "graph/algorithms.h"

using namespace meta;

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

    auto g3 = graph::undirected_graph<>::load(argv[1]);
    degree_dist(g3, "g3-degrees.dat");
    std::cout << "g3 CC: " << graph::algorithms::clustering_coefficient(g3)
              << std::endl;
}
