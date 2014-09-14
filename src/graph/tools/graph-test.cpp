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

void degree_dist(graph::undirected_graph<>& g)
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

    std::ofstream outfile{"degrees.dat"};
    for (auto& c : degrees)
        outfile << c.first << " " << c.second << "\n";

    std::cout << "Clustering coefficient of top 5 authors: " << std::endl;
    for (size_t i = 0; i < 5; ++i)
    {
        auto id = degrees[i].first;
        std::cout << (i + 1) << ". id=" << g.node(id).label << " "
                  << graph::algorithms::clustering_coefficient(g, id)
                  << std::endl;
    }

    // print out all clustering coefficients sorted by descending node degree
    std::ofstream ccout{"cc.dat"};
    for (auto& p : degrees)
    {
        ccout << g.adjacent(g.node(p.first).id).size() << " "
              << graph::algorithms::clustering_coefficient(g, p.first)
              << std::endl;
    }
}

void overlap(const graph::undirected_graph<>& g)
{
    std::ofstream out{"overlap.dat"};
    for (auto e = g.edges_begin(); e != g.edges_end(); ++e)
    {
        out << e->src << " " << e->dest << " "
            << graph::algorithms::neighborhood_overlap(g, e->src, e->dest)
            << std::endl;
    }
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

    auto g = load(argv[1]);
    std::cout << "Number of nodes: " << g.size() << std::endl;
    std::cout << "Number of edges: " << g.num_edges() << std::endl;
    degree_dist(g);
    overlap(g);
}
