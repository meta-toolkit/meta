/**
 * @file algorithms.tcc
 * @author Sean Massung
 */

#include <random>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include "util/progress.h"
#include "stats/multinomial.h"

namespace meta
{
namespace graph
{
namespace algorithms
{
template <class Graph>
double clustering_coefficient(const Graph& graph, node_id id)
{
    auto adj = graph.adjacent(id);
    if (adj.empty())
        return 0.0;

    if (adj.size() == 1)
        return 1.0;

    double numerator = 0.0;
    for (size_t i = 0; i < adj.size(); ++i)
    {
        for (size_t j = i; j < adj.size(); ++j)
        {
            if (graph.edge(adj[i].first, adj[j].first))
                ++numerator;
        }
    }

    return (2.0 * numerator) / (adj.size() * (adj.size() - 1));
}

template <class Graph>
double clustering_coefficient(const Graph& graph)
{
    double total = 0.0;
    for (auto& n : graph)
        total += clustering_coefficient(graph, n.id);

    return total / graph.size();
}

template <class Graph>
double neighborhood_overlap(const Graph& graph, node_id src, node_id dest)
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

template <class Graph>
std::vector<node_id> myopic_search(Graph& g, node_id src, node_id dest)
{
    auto cur = src;
    std::vector<node_id> path;
    path.push_back(src);
    while (cur != dest)
    {
        if (path.size() > g.size())
            throw graph_algorithm_exception{"no path found in myopic search"};
        node_id best_id;
        double best_distance = std::numeric_limits<double>::max();
        for (auto& n : g.adjacent(cur))
        {
            double distance = std::abs(static_cast<double>(n.first)
                                       - static_cast<double>(dest));
            if (distance < best_distance)
            {
                best_distance = distance;
                best_id = n.first;
            }
        }

        cur = best_id;
        path.push_back(cur);
    }

    return path;
}

template <class Graph>
std::vector<node_id> bfs(Graph& g, node_id src, node_id dest)
{
    std::unordered_set<node_id> seen;
    std::unordered_map<node_id, node_id> parent;
    std::queue<node_id> q;
    q.push(src);

    // find path
    while (!q.empty())
    {
        auto cur = q.front();
        q.pop();
        seen.insert(cur);
        for (auto& n : g.adjacent(cur))
        {
            if (seen.find(n.first) == seen.end())
            {
                q.push(n.first);
                parent[n.first] = cur;

                if (n.first == dest)
                    break;
            }
        }
    }

    // find parents
    std::vector<node_id> path;
    node_id cur{dest};
    path.push_back(dest);
    while (true)
    {
        path.push_back(parent[cur]);
        cur = parent[cur];
        if (cur == src)
        {
            path.push_back(cur);
            break;
        }
    }

    return path;
}
}
}
}
