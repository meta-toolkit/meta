/**
 * @file graph-test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <string>
#include <random>

#include "meta.h"
#include "util/progress.h"
#include "graph/undirected_graph.h"
#include "graph/directed_graph.h"
#include "graph/algorithms.h"
#include "logging/logger.h"

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

void ring_world(double q)
{
    graph::undirected_graph<> g;
    graph::algorithms::watts_strogatz(g, 10000, 2, 0);
    std::default_random_engine gen;
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    printing::progress prog{"Generating graph: ", g.size()};
    for (uint64_t i = 0; i < g.size(); ++i)
    {
        prog(i);

        // create distribution of possible edges to connect to
        double total_prob = 0.0;
        std::vector<std::pair<node_id, double>> choices;
        choices.reserve(g.size());
        for (uint64_t j = 0; j < g.size(); ++j)
        {
            if (i == j)
                continue;
            double prob
                = std::exp(std::log(static_cast<double>(j - i)) * -1.0 * q);
            choices.emplace_back(node_id{j}, prob);
            total_prob += prob;
        }

        for (auto& p : choices)
            p.second /= total_prob;

        double sum = 0.0;
        double rand = dist(gen);
        for (auto& p : choices)
        {
            sum += p.second;
            if (sum > rand)
            {
                if (!g.edge(p.first, node_id{i}))
                    g.add_edge(p.first, node_id{i});
                break;
            }
        }
    }
    prog.end();

    // run 1000 pairs, recording average path length
    uint64_t num_runs = 1000;
    double length = 0.0;
    std::vector<uint64_t> nodes(g.size());
    std::iota(nodes.begin(), nodes.end(), 0);
    std::shuffle(nodes.begin(), nodes.end(), gen);

    printing::progress bfs_prog{"Finding shortest paths: ", num_runs * 2};
    for (uint64_t i = 0; i < num_runs * 2; i += 2)
    {
        bfs_prog(i);
        auto path = graph::algorithms::myopic_search(g, node_id{nodes[i]},
                                                     node_id{nodes[i + 1]});
        length += (path.size() - 1);
    }
    bfs_prog.end();

    std::cout << q << " " << length / num_runs << std::endl;
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

    undirected_graph<> g1;
    algorithms::random_graph(g1, 5242, 14496);
    degree_dist(g1, "g1-degrees.dat");
    std::cout << "g1 CC: " << algorithms::clustering_coefficient(g1)
              << std::endl;

    undirected_graph<> g2;
    algorithms::watts_strogatz(g2, 5242, 4, 4012);
    degree_dist(g2, "g2-degrees.dat");
    std::cout << "g2 CC: " << algorithms::clustering_coefficient(g2)
              << std::endl;

    auto g3 = undirected_graph<>::load(argv[1]);
    degree_dist(g3, "g3-degrees.dat");
    std::cout << "g3 CC: " << algorithms::clustering_coefficient(g3)
              << std::endl;

    for (double q = 0.05; q < 2.0001; q += 0.05)
        ring_world(q);
}
