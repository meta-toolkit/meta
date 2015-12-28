/**
 * @file search.tcc
 * @author Sean Massung
 */

#include <unordered_set>
#include <unordered_map>
#include <queue>
#include "meta/util/optional.h"

namespace meta
{
namespace graph
{
namespace algorithms
{
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
        util::optional<node_id> best_id;
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

        cur = *best_id;
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
