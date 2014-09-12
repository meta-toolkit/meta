/**
 * @file algorithms.tcc
 * @author Sean Massung
 */

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
    if(adj.empty())
        return 0.0;

    if(adj.size() == 1)
        return 1.0;

    double numerator = 0.0;
    for(size_t i = 0; i < adj.size(); ++i)
    {
        for(size_t j = i; j < adj.size(); ++j)
        {
            if(graph.edge(adj[i].first, adj[j].first))
                ++numerator;
        }
    }

    return (2.0 * numerator) / (adj.size() * (adj.size() - 1));
}
}
}
}
