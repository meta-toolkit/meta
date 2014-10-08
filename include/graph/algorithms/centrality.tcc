/**
 * @file centrality.tcc
 * @author Sean Massung
 */

#include <stack>
#include <queue>
#include <vector>
#include <unordered_map>

#include "parallel/parallel_for.h"

namespace meta
{
namespace graph
{
namespace algorithms
{
template <class Graph>
centrality_result degree_centrality(const Graph& g)
{
    centrality_result res;
    res.reserve(g.size());
    for (auto& n : g)
        res.emplace_back(n.id, g.adjacent(n.id).size());

    std::sort(res.begin(), res.end(), [&](auto a, auto b)
              {
        return a.second > b.second;
    });
    return res;
}

template <class Graph>
centrality_result betweenness_centrality(const Graph& g)
{
    centrality_result cb;
    cb.reserve(g.size());
    for (auto& n : g)
        cb.emplace_back(n.id, 0.0);

    std::mutex print_mut; // progress mutex
    std::mutex calc_mut;  // centrality calculation mutex

    printing::progress prog{" Calculating betweenness centrality ", g.size()};
    size_t done = 0;
    parallel::parallel_for(g.begin(), g.end(), [&](auto n)
                           {
        internal::betweenness_step(g, cb, n.id, calc_mut);
        std::lock_guard<std::mutex> lock{print_mut};
        prog(++done);
    });
    prog.end();

    std::sort(cb.begin(), cb.end(), [&](auto a, auto b)
              {
        return a.second > b.second;
    });
    return cb;
}

namespace internal
{
template <class Graph>
void betweenness_step(const Graph& g, centrality_result& cb, node_id n,
                      std::mutex& calc_mut)
{
    std::stack<node_id> stack;
    std::unordered_map<node_id, std::vector<node_id>> parent;
    std::vector<double> sigma(g.size(), 0.0);
    sigma[n] = 1.0;
    std::vector<double> d(g.size(), -1.0);
    d[n] = 0;
    std::queue<node_id> queue;
    queue.push(n);
    while (!queue.empty())
    {
        auto v = queue.front();
        queue.pop();
        stack.push(v);
        for (auto& neighbor : g.adjacent(v))
        {
            auto w = neighbor.first;
            // w found for the first time?
            if (d[w] < 0)
            {
                queue.push(w);
                d[w] = d[v] + 1;
            }
            // shortest path to w via v?
            if (d[w] == d[v] + 1)
            {
                sigma[w] = sigma[w] + sigma[v];
                parent[w].push_back(v);
            }
        }
    }

    std::vector<double> delta(g.size(), 0);
    // S returns vertices in order of non-increasing distance from n
    while (!stack.empty())
    {
        auto w = stack.top();
        stack.pop();
        for (auto& v : parent[w])
            delta[v] += (sigma[v] / sigma[w]) * (1.0 + delta[w]);
        if (w != n)
        {
            std::lock_guard<std::mutex> lock{calc_mut};
            cb[w].second += delta[w];
        }
    }
}
}
}
}
}
