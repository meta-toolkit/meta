/**
 * @file social-network.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <fstream>
#include <string>

#include "meta.h"
#include "graph/undirected_graph.h"
#include "graph/algorithms/algorithms.h"
#include "logging/logger.h"

using namespace meta;

template <class Graph>
void write_json(const Graph& g, const std::string& filename)
{
    std::ofstream out{filename};

    // write nodes

    out << "{\n";
    out << "  \"nodes\":[\n";
    for (auto& n : g)
    {
        out << "    {\"name\": \"" + n.label + "\"}";
        if (n.id != g.size() - 1)
            out << ",";
        out << "\n";
    }

    // write edges

    out << "  ],\n";
    out << "  \"links\":[\n";

    auto e = g.edges_begin();
    for (uint64_t i = 0; e != g.edges_end(); ++i, ++e)
    {
        out << "    {\"source\":" + std::to_string(e->src) + ", ";
        out << "\"target\":" + std::to_string(e->dest) + "}";

        if (i != g.num_edges() - 1)
            out << ",";
        out << "\n";
    }

    out << "  ]\n";
    out << "}";
}

template <class Graph>
void write_centrality(const Graph& g,
                      const graph::algorithms::centrality_result& centrality,
                      const std::string& filename)
{
    std::ofstream out{filename};
    for (auto& p : centrality)
        out << g.node(p.first).label << " " << p.second << std::endl;
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

    logging::set_cerr_logging();
    using namespace graph;

    auto g = undirected_graph<>::load(argv[1]);
 // write_json(g, "yelp-social.json");
 // write_centrality(g, algorithms::degree_centrality(g),
 //                  "degree-centrality.txt");
    write_centrality(g, algorithms::betweenness_centrality(g),
                     "betweenness-centrality.txt");
}
