/**
 * @file model.tcc
 * @author Sean Massung
 */

#include <random>

#include "meta/stats/multinomial.h"
#include "meta/util/progress.h"

namespace meta
{
namespace graph
{
namespace algorithms
{
template <class Graph>
void random_graph(Graph& g, uint64_t num_nodes, uint64_t num_edges)
{
    uint64_t start_id = g.size();
    for (uint64_t i = start_id; i < start_id + num_nodes; ++i)
        g.emplace(std::to_string(i));

    uint64_t possible = g.size() * (g.size() - 1) - g.num_edges();
    if (num_edges > possible)
        throw graph_algorithm_exception{
            "impossible to add required number of edges to graph"};

    std::default_random_engine gen;
    std::uniform_int_distribution<uint64_t> dist(0, g.size() - 1);
    uint64_t edges_added = 0;
    while (edges_added != num_edges)
    {
        node_id src{dist(gen)};
        node_id dest{dist(gen)};

        if (src == dest || g.edge(src, dest))
            continue;

        g.add_edge(src, dest);
        ++edges_added;
    }
}

template <class Graph>
void watts_strogatz(Graph& g, uint64_t num_nodes, uint64_t num_neighbors,
                    uint64_t num_random_edges)
{
    if (g.size() != 0)
        throw graph_algorithm_exception{
            "watts-strogatz graph generation must be called on an empty graph"};

    if (num_neighbors % 2 != 0)
        throw graph_algorithm_exception{
            "num_neighbors for watts-strogatz graph model must be even"};

    for (uint64_t i = 0; i < num_nodes; ++i)
        g.emplace(std::to_string(i));

    for (uint64_t i = 0; i < num_nodes; ++i)
    {
        for (uint64_t j = 1; j <= num_neighbors / 2; ++j)
        {
            auto src = node_id{i};
            auto dest = node_id{(i + j) % g.size()};
            g.add_edge(src, dest);
            if (!g.edge(dest, src))
                g.add_edge(dest, src);
        }
    }

    random_graph(g, 0, num_random_edges);
}

template <class Graph>
void preferential_attachment(
    Graph& g, uint64_t num_nodes, uint64_t node_edges,
    std::function<double(node_id)> attr /* = return 1.0 */)
{
    if (g.size() != 0)
        throw graph_algorithm_exception{"preferential attachment graph "
                                        "generation must be called on an empty "
                                        "graph"};

    if (node_edges > num_nodes)
        throw graph_algorithm_exception{
            "num_nodes should be significantly higher than node_edges"};

    // first, create a complete graph of node_edges nodes
    stats::multinomial<node_id> probs;
    for (uint64_t i = 0; i < node_edges; ++i)
    {
        g.emplace(std::to_string(i));
        probs.increment(node_id{i}, attr(node_id{i}));
    }

    for (uint64_t i = 0; i < node_edges; ++i)
        for (uint64_t j = i + 1; j < node_edges; ++j)
            g.add_edge(node_id{i}, node_id{j});

    // now, add a single node each time step, connecting to node_edges nodes
    std::default_random_engine gen;
    printing::progress prog{" Generating graph ", num_nodes};
    for (uint64_t i = node_edges; i < num_nodes; ++i)
    {
        prog(i);
        g.emplace(std::to_string(i));
        auto src = node_id{i};
        for (uint64_t j = 0; j < node_edges; ++j)
        {
            auto dest = probs(gen);
            try
            {
                g.add_edge(src, dest);
            }
            catch (...)
            { /* ignore */
            }
        }
        probs.increment(src, attr(src));
    }
    prog.end();
}
}
}
}
