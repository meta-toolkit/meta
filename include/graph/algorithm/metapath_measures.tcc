/**
 * @file metapath_measures.tcc
 * @author Sean Massung
 */

#include <algorithm>
#include <iostream>
#include "util/progress.h"

namespace meta
{
namespace graph
{
namespace algorithm
{
template <class Graph>
metapath_measures<Graph>::metapath_measures(Graph& g, const metapath& mpath)
    : g_(g), mpath_{mpath}
{/* nothing */
}

template <class Graph>
auto metapath_measures<Graph>::path_count(bool is_weighted) -> measure_result
{
    printing::progress prog{"Calculating PathCount ", g_.size()};
    measure_result result;
    if(mpath_[2] == "similarity")
        is_weighted = true;
    for (node_id id{0}; id < g_.size(); ++id)
    {
        prog(id);
        bfs_match(id, id, result, 0, is_weighted);
    }
    return result;
}

template <class Graph>
auto metapath_measures<Graph>::symmetric_random_walk() -> measure_result
{
    auto pc_fwd = random_walk();
    mpath_.reverse();
    auto pc_bwd = random_walk();
    mpath_.reverse(); // undo previous reverse

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
    auto pc = path_count();
    measure_result result;
    for (auto& src : pc)
    {
        node_id src_id = src.first;
        double total_num_paths = meta_degree(src_id, pc);
        for (auto& dest : src.second)
        {
            node_id dest_id = dest.first;
            if(total_num_paths != 0)
                result[src_id][dest_id] = dest.second / total_num_paths;
        }
    }
    return result;
}

template <class Graph>
auto metapath_measures<Graph>::normalized_path_count() -> measure_result
{
    auto pc_fwd = path_count();
    mpath_.reverse();
    auto pc_bwd = path_count();
    mpath_.reverse(); // undo previous reverse

    measure_result result;
    for (auto& fwd : pc_fwd)
    {
        node_id src_id = fwd.first;
        for (auto& fwd_dest : fwd.second)
        {
            node_id dest_id = fwd_dest.first;
            double numerator = fwd_dest.second + pc_bwd[dest_id][src_id];
            double denominator
                = (pc_fwd[src_id][src_id] + pc_bwd[dest_id][dest_id]);

            if (denominator == 0)
                continue;

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
void metapath_measures
    <Graph>::bfs_match(node_id orig_id, node_id id, measure_result& result,
                       uint64_t depth, bool is_weighted)
{
    auto node = g_.node(id);
    // if at end of path
    if (depth == mpath_.size() - 1)
    {
        if (node.type == mpath_[depth])
        {
            cur_weight_ += node.weight;

            cur_path_.push_back(node.name);
            if(is_weighted)
                result[orig_id][id] += cur_weight_;
            else
                ++result[orig_id][id];

            cur_weight_ = 0;

            // print path found that ends here
            if (print_paths)
            {
                for (auto& n : cur_path_)
                    std::cout << n << " ";
                std::cout << std::endl;
                cur_path_.pop_back();
            }
        }
    }
    else if (node.type == mpath_[depth])
    {
        cur_weight_ += node.weight;

        if (print_paths)
            cur_path_.push_back(node.name);
        if (mpath_.edge_dir(depth) == metapath::direction::backward)
        {
            for (auto& p : g_.incoming(id))
                bfs_match(orig_id, p, result, depth + 1, is_weighted);
        }
        else
        {
            for (auto& p : g_.outgoing(id))
                bfs_match(orig_id, p.first, result, depth + 1, is_weighted);
        }
        if (print_paths)
            cur_path_.pop_back();
    }
}
}
}
}
