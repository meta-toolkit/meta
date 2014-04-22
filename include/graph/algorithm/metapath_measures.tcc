/**
 * @file metapath_measures.tcc
 * @author Sean Massung
 */

#include <algorithm>
#include "util/progress.h"

namespace meta
{
namespace graph
{
namespace algorithm
{
template <class Graph>
metapath_measures
    <Graph>::metapath_measures(Graph& g, const metapath_t& metapath)
    : g_(g), metapath_{metapath}
{/* nothing */
}

template <class Graph>
auto metapath_measures<Graph>::path_count() -> measure_result
{
    printing::progress prog{"Calculating PathCount ", g_.size()};
    measure_result result;
    for (node_id id{0}; id < g_.size(); ++id)
    {
        prog(id);
        bfs_match(id, id, result, 0);
    }

    return result;
}

template <class Graph>
auto metapath_measures<Graph>::symmetric_random_walk() -> measure_result
{
    auto pc_fwd = random_walk();
    std::reverse(metapath_.begin(), metapath_.end());
    auto pc_bwd = random_walk(); // TODO assumes symmetric
    std::reverse(metapath_.begin(), metapath_.end());

    measure_result result;
    for (auto& fwd : pc_fwd)
    {
        node_id src_id = fwd.first;
        for (auto& fwd_dest : fwd.second)
        {
            node_id dest_id = fwd_dest.first;
            result[src_id][dest_id] = fwd_dest.second + pc_bwd[dest_id][src_id];
        }
    }
    return result;
}

template <class Graph>
auto metapath_measures<Graph>::random_walk() -> measure_result
{
    auto pc_fwd = path_count();
    measure_result result;
    for (auto& fwd : pc_fwd)
    {
        node_id src_id = fwd.first;
        for (auto& fwd_dest : fwd.second)
        {
            result[src_id][fwd_dest.first] = fwd_dest.second
                                             / meta_degree(src_id, pc_fwd);
        }
    }
    return result;
}

template <class Graph>
auto metapath_measures<Graph>::normalized_path_count() -> measure_result
{
    auto pc_fwd = path_count();
    std::reverse(metapath_.begin(), metapath_.end());
    auto pc_bwd = path_count(); // TODO assumes symmetric
    std::reverse(metapath_.begin(), metapath_.end());

    measure_result result;
    for (auto& fwd : pc_fwd)
    {
        node_id src_id = fwd.first;
        for (auto& fwd_dest : fwd.second)
        {
            node_id dest_id = fwd_dest.first;
            double numerator = fwd_dest.second + pc_bwd[dest_id][src_id];
            double denominator = meta_degree(src_id, pc_fwd)
                                 + meta_degree(dest_id, pc_bwd);
            result[src_id][dest_id] = numerator / denominator;
        }
    }
    return result;
}

template <class Graph>
uint64_t metapath_measures
    <Graph>::meta_degree(node_id id, const measure_result& result)
{
    uint64_t sum = 0;
    for (auto& res : result.at(id))
        sum += res.second;
    return sum;
}

template <class Graph>
void metapath_measures<Graph>::bfs_match(node_id orig_id, node_id id,
                                         measure_result& result, uint64_t depth)
{
    auto node = g_.node(id);
    // if at end of path
    if (depth == metapath_.size() - 1)
    {
        if (node.type == metapath_.back() && id != orig_id)
            ++result[orig_id][id];
    }
    else if (node.type == metapath_[depth])
    {
        for (auto& p : g_.adjacent(id))
            bfs_match(orig_id, p.first, result, depth + 1);
    }
}
}
}
}
