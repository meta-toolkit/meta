/**
 * @file graph_test.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <string>
#include <random>

#include "meta/logging/logger.h"
#include "meta/meta.h"
#include "meta/graph/algorithms/algorithms.h"
#include "meta/graph/directed_graph.h"
#include "meta/graph/undirected_graph.h"
#include "meta/util/progress.h"

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

void simulations()
{
    using namespace graph;
    uint64_t num_nodes = 5000;
    uint64_t num_edges = 10;

    // Uniform

    undirected_graph<> g1;
    algorithms::preferential_attachment(g1, num_nodes, num_edges);
    degree_dist(g1, "pa-uniform.dat");

    // Standard normal

    std::default_random_engine gen;
    std::normal_distribution<> std_norm{0, 1};
    undirected_graph<> g2;
    algorithms::preferential_attachment(g2, num_nodes, num_edges,
                                        [&](node_id)
                                        {
        return std::abs(std_norm(gen));
    });
    degree_dist(g2, "pa-norm01.dat");

    // Beta(a,b) = Gamma(a,1) / (Gamma(a,1) + Gamma(b,1))

    std::gamma_distribution<> gamma1{0.5, 1.0};
    std::gamma_distribution<> gamma2{0.5, 1.0};
    undirected_graph<> g3;
    algorithms::preferential_attachment(g3, num_nodes, num_edges,
                                        [&](node_id)
                                        {
        auto x = gamma1(gen);
        return x / (x + gamma2(gen));
    });
    degree_dist(g3, "pa-beta-half-half.dat");

    // piecewise

    undirected_graph<> g4;
    algorithms::preferential_attachment(g4, num_nodes, num_edges,
                                        [&](node_id id)
                                        {
        return (id % 2) ? 0.1 : 1000.0;
    });
    degree_dist(g4, "pa-piecewise.dat");

    // increasing

    undirected_graph<> g5;
    algorithms::preferential_attachment(g5, num_nodes, num_edges,
                                        [&](node_id id)
                                        {
        return id * 10;
    });
    degree_dist(g5, "pa-increasing.dat");

    undirected_graph<> g6;
    algorithms::preferential_attachment(g6, num_nodes, num_edges,
                                        [&](node_id id)
                                        {
        return 1.0 / (id + 1);
    });
    degree_dist(g6, "pa-skew.dat");
}

void hybrid(const std::string& graph_file)
{
    using namespace graph;
    auto g = undirected_graph<>::load(graph_file);
    std::vector<uint64_t> counts(g.size(), 0);
    for (auto& n : g)
        ++counts[g.adjacent(n.id).size()];

    std::ofstream out{"degrees.txt"};
    for (auto& c : counts)
        out << c << std::endl;
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
    // simulations();
    hybrid(argv[1]);
}
