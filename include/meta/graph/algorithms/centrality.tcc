/**
 * @file centrality.tcc
 * @author Sean Massung
 */

#include <stack>
#include <queue>
#include <vector>
#include <unordered_map>

#include "meta/parallel/parallel_for.h"

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

    using pair_t = std::pair<node_id, double>;
    std::sort(res.begin(), res.end(), [&](const pair_t& a, const pair_t& b)
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
    parallel::parallel_for(g.begin(), g.end(), [&](decltype(*g.begin()) n)
                           {
                               internal::betweenness_step(g, cb, n.id,
                                                          calc_mut);
                               std::lock_guard<std::mutex> lock{print_mut};
                               prog(++done);
                           });
    prog.end();

    using pair_t = std::pair<node_id, double>;
    std::sort(cb.begin(), cb.end(), [&](const pair_t& a, const pair_t& b)
              {
                  return a.second > b.second;
              });
    return cb;
}

template <class DirectedGraph>
centrality_result
page_rank_centrality(const DirectedGraph& g, double damp /* = 0.85 */,
                     const stats::multinomial<node_id>& jump_dist /* = {} */,
                     uint64_t max_iters /* = 100 */)
{
    if (damp < 0.0 || damp > 1.0)
        throw graph_exception{"PageRank dampening factor must be on [0, 1]"};

    std::vector<double> v(g.size(), 1.0 / g.size());
    std::vector<double> w(g.size(), 0.0);

    parallel::thread_pool pool;

    printing::progress prog{" > Calculating PageRank centrality ", max_iters};
    for (uint64_t iter = 0; iter < max_iters; ++iter)
    {
        prog(iter);
        w.assign(w.size(), 0.0);

        using node = typename DirectedGraph::node_type;
        parallel::parallel_for(
            g.begin(), g.end(), pool, [&](const node& curr)
            {
                double sum = 0.0;
                for (const auto& n : g.incoming(curr.id))
                {
                    auto adj_size = g.adjacent(n).size();
                    if (adj_size != 0)
                        sum += v[n] / adj_size;
                }
                if (jump_dist.counts() == 0.0)
                {
                    w[curr.id] = (1.0 - damp) / g.size() + damp * sum;
                }
                else
                {
                    w[curr.id] = (1.0 - damp) * jump_dist.probability(curr.id)
                                 + damp * sum;
                }
            });
        v.swap(w);
    }
    prog.end();

    centrality_result evc;
    evc.reserve(g.size());
    node_id id{0};
    for (auto& n : v)
        evc.emplace_back(id++, n);

    using pair_t = std::pair<node_id, double>;
    std::sort(evc.begin(), evc.end(), [&](const pair_t& a, const pair_t& b)
              {
                  return a.second > b.second;
              });
    return evc;
}

template <class Graph>
centrality_result eigenvector_centrality(const Graph& g,
                                         uint64_t max_iters /* = 100 */)
{
    std::vector<double> v(g.size(), 1.0);
    std::vector<double> w(g.size(), 0.0);

    printing::progress prog{" Calculating eigenvector centrality ", max_iters};
    for (uint64_t iter = 0; iter < max_iters; ++iter)
    {
        prog(iter);
        w.assign(w.size(), 0.0);
        for (uint64_t i = 0; i < g.size(); ++i)
            for (auto& n : g.adjacent(node_id{i}))
                w[n.first] += v[i];
        v.swap(w);
    }
    prog.end();

    centrality_result evc;
    evc.reserve(g.size());
    node_id id{0};
    double sum = std::accumulate(v.begin(), v.end(), 0.0);
    for (auto& n : v)
        evc.emplace_back(id++, n / sum);

    using pair_t = std::pair<node_id, double>;
    std::sort(evc.begin(), evc.end(), [&](const pair_t& a, const pair_t& b)
              {
                  return a.second > b.second;
              });
    return evc;
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
