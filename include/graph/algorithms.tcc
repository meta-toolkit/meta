/**
 * @file algorithms.tcc
 * @author Sean Massung
 */

#include <unordered_set>

namespace meta
{
namespace graph
{
namespace algorithms
{
template <class UndirectedGraph>
double clustering_coefficient(const UndirectedGraph& graph, node_id id)
{
    auto adj = graph.adjacent(id);
    if (adj.empty())
        return 0.0;

    if (adj.size() == 1)
        return 1.0;

    double numerator = 0.0;
    for (size_t i = 0; i < adj.size(); ++i)
    {
        for (size_t j = i + 1; j < adj.size(); ++j)
        {
            // count if two neighbors are connected
            if (graph.edge(adj[i].first, adj[j].first))
                ++numerator;
        }
    }

    return (2.0 * numerator) / (adj.size() * (adj.size() - 1));
}
template <class UndirectedGraph>
double neighborhood_overlap(const UndirectedGraph& graph, node_id src,
                            node_id dest)
{
    if (!graph.edge(src, dest))
        throw graph_algorithm_exception{
            "neighborhood_overlap must be called on neighboring nodes"};

    double num_shared = 0.0;
    std::unordered_set<node_id> total;
    for (auto& p : graph.adjacent(src))
    {
        total.insert(p.first);
        if (graph.edge(dest, p.first))
            ++num_shared;
    }

    if (num_shared == 0.0)
        return 0.0;

    for (auto& p : graph.adjacent(dest))
        total.insert(p.first);

    // minus 2 so src doesn't count dest and vice versa
    return num_shared / (total.size() - 2);
}
}
}
}
