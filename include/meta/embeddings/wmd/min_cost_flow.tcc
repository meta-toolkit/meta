/**
 * @file min_cost_flow.tcc
 * @author lolik111
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <limits>
#include <set>

#include "min_cost_flow.h"

namespace meta
{
namespace embeddings
{
template <typename T>
struct edge
{
    edge(size_t to = 0, T cost = 0) : _to(to), _cost(cost)
    {
    }

    size_t _to;
    T _cost;
};

template <typename T>
struct edge_weighted
{
    edge_weighted(size_t to, T cost, T amount)
        : _to(to), _cost(cost), _amount(amount)
    {
    }

    size_t _to;
    T _cost;
    T _amount;
};

template <typename NumT>
NumT min_cost_flow<NumT>::compute_min_cost_flow(
    std::vector<NumT>& e, const std::vector<std::list<edge<NumT>>>& c,
    std::vector<std::list<edge_weighted<NumT>>>& x)
{

    assert(e.size() == c.size());
    assert(x.size() == c.size());

    _num_nodes = e.size();
    _nodes_to_demand.resize(_num_nodes);

    // reduced costs for forward edges (c[i,j]-pi[i]+pi[j])
    // Note that for forward edges the residual capacity is infinity
    std::vector<std::list<edge<NumT>>> r_cost_forward(_num_nodes);

    // reduced costs and capacity for backward edges (c[j,i]-pi[j]+pi[i])
    // Since the flow at the beginning is 0, the residual capacity is also zero
    std::vector<std::list<edge_weighted<NumT>>> r_cost_cap_backward(_num_nodes);

    for (size_t from = 0; from < _num_nodes; ++from)
    {
        for (auto it = c[from].begin(); it != c[from].end(); ++it)
        {
            // init flow
            x[from].push_back(edge_weighted<NumT>(it->_to, it->_cost, 0));
            x[it->_to].push_back(edge_weighted<NumT>(from, -it->_cost, 0));

            r_cost_forward[from].push_back(edge<NumT>(it->_to, it->_cost));
            r_cost_cap_backward[it->_to].push_back(
                edge_weighted<NumT>(from, -it->_cost, 0));
        }
    }

    // Max supply
    NumT U = 0;
    for (size_t i = 0; i < _num_nodes; ++i)
    {
        if (e[i] > U)
            U = e[i];
    }

    std::vector<NumT> d(_num_nodes);
    std::vector<size_t> prev(_num_nodes);
    NumT delta = 1;
    while (true)
    { // until we break when S or T is empty

        NumT max_supply = 0;
        size_t k = 0;
        for (size_t i = 0; i < _num_nodes; ++i)
        {
            if (e[i] > 0)
            {
                if (max_supply < e[i])
                {
                    max_supply = e[i];
                    k = i;
                }
            }
        }
        if (max_supply == 0)
            break;
        delta = max_supply;

        size_t l;
        compute_shortest_path(d, prev, k, r_cost_forward, r_cost_cap_backward,
                              e, l);

        // find delta (minimum on the path from k to l)
        size_t to = l;
        do
        {
            size_t from = prev[to];
            assert(from != to);

            // residual
            auto itccb = r_cost_cap_backward[from].begin();
            while ((itccb != r_cost_cap_backward[from].end())
                   && (itccb->_to != to))
            {
                ++itccb;
            }
            if (itccb != r_cost_cap_backward[from].end())
            {
                if (itccb->_amount < delta)
                    delta = itccb->_amount;
            }

            to = from;
        } while (to != k);

        // augment delta flow from k to l (backwards actually...)
        to = l;
        do
        {
            size_t from = prev[to];
            assert(from != to);

            auto itx = x[from].begin();
            while (itx->_to != to)
            {
                ++itx;
            }
            itx->_amount += delta;

            // update residual for backward edges
            auto itccb = r_cost_cap_backward[to].begin();
            while ((itccb != r_cost_cap_backward[to].end())
                   && (itccb->_to != from))
            {
                ++itccb;
            }
            if (itccb != r_cost_cap_backward[to].end())
            {
                itccb->_amount += delta;
            }
            itccb = r_cost_cap_backward[from].begin();
            while ((itccb != r_cost_cap_backward[from].end())
                   && (itccb->_to != to))
            {
                ++itccb;
            }
            if (itccb != r_cost_cap_backward[from].end())
            {
                itccb->_amount -= delta;
            }

            // update e
            e[to] += delta;
            e[from] -= delta;

            to = from;
        } while (to != k);
    }

    // compute distance from x
    NumT dist = 0;
    for (size_t from = 0; from < _num_nodes; ++from)
    {
        for (auto it = x[from].begin(); it != x[from].end(); ++it)
        {
            dist += (it->_cost * it->_amount);
        }
    }

    return dist;
}

template <typename NumT>
void min_cost_flow<NumT>::compute_shortest_path(
    std::vector<NumT>& d, std::vector<size_t>& prev, size_t from,
    std::vector<std::list<edge<NumT>>>& cost_forward,
    std::vector<std::list<edge_weighted<NumT>>>& cost_backward,
    const std::vector<NumT>& e, size_t& l)
{
    // Making heap (all inf except 0, so we are saving comparisons...)
    std::vector<edge<NumT>> demand(_num_nodes);

    demand[0]._to = from;
    _nodes_to_demand[from] = 0;
    demand[0]._cost = 0;

    size_t j = 1;
    for (size_t i = 0; i < from; ++i)
    {
        demand[j]._to = i;
        _nodes_to_demand[i] = j;
        demand[j]._cost = std::numeric_limits<NumT>::max();
        ++j;
    }

    for (size_t i = from + 1; i < _num_nodes; ++i)
    {
        demand[j]._to = i;
        _nodes_to_demand[i] = j;
        demand[j]._cost = std::numeric_limits<NumT>::max();
        ++j;
    }

    // main loop
    std::vector<bool> final_nodes_flg(_num_nodes, false);
    do
    {
        size_t u = demand[0]._to;

        d[u] = demand[0]._cost; // final distance
        final_nodes_flg[u] = true;
        if (e[u] < 0)
        {
            l = u;
            break;
        }

        heap_remove_first(demand, _nodes_to_demand);

        // neighbors of capacity
        for (auto it = cost_forward[u].begin(); it != cost_forward[u].end();
             ++it)
        {
            assert(it->_cost >= 0);
            NumT alt = d[u] + it->_cost;
            size_t v = it->_to;
            if ((_nodes_to_demand[v] < demand.size())
                && (alt < demand[_nodes_to_demand[v]]._cost))
            {
                heap_decrease_key(demand, _nodes_to_demand, v, alt);
                prev[v] = u;
            }
        }

        for (auto it = cost_backward[u].begin(); it != cost_backward[u].end();
             ++it)
        {
            if (it->_amount > 0)
            {
                assert(it->_cost >= 0);
                NumT alt = d[u] + it->_cost;
                size_t v = it->_to;
                if ((_nodes_to_demand[v] < demand.size())
                    && (alt < demand[_nodes_to_demand[v]]._cost))
                {
                    heap_decrease_key(demand, _nodes_to_demand, v, alt);
                    prev[v] = u;
                }
            }
        }
    } while (!demand.empty());

    // reduced costs for forward edges (cost[i,j]-pi[i]+pi[j])
    for (size_t node_from = 0; node_from < _num_nodes; ++node_from)
    {

        for (auto it = cost_forward[node_from].begin();
             it != cost_forward[node_from].end(); ++it)
        {
            if (final_nodes_flg[node_from])
            {
                it->_cost += d[node_from] - d[l];
            }
            if (final_nodes_flg[it->_to])
            {
                it->_cost -= d[it->_to] - d[l];
            }
        }
    }

    // reduced costs and capacity for backward edges (c[j,i]-pi[j]+pi[i])
    for (size_t node_from = 0; node_from < _num_nodes; ++node_from)
    {
        for (auto it = cost_backward[node_from].begin();
             it != cost_backward[node_from].end(); ++it)
        {
            if (final_nodes_flg[node_from])
            {
                it->_cost += d[node_from] - d[l];
            }
            if (final_nodes_flg[it->_to])
            {
                it->_cost -= d[it->_to] - d[l];
            }
        }
    }
}

template <typename NumT>
void min_cost_flow<NumT>::heap_decrease_key(
    std::vector<edge<NumT>>& demand, std::vector<size_t>& nodes_to_demand,
    size_t v, NumT alt)
{
    size_t i = nodes_to_demand[v];
    demand[i]._cost = alt;
    while (i > 0 && demand[PARENT(i)]._cost > demand[i]._cost)
    {
        swap_heap(demand, nodes_to_demand, i, PARENT(i));
        i = PARENT(i);
    }
}

template <typename NumT>
void min_cost_flow<NumT>::heap_remove_first(
    std::vector<edge<NumT>>& demand, std::vector<size_t>& nodes_to_demand)
{
    swap_heap(demand, nodes_to_demand, 0, demand.size() - 1);
    demand.pop_back();
    heapify(demand, nodes_to_demand, 0);
}

template <typename NumT>
void min_cost_flow<NumT>::heapify(std::vector<edge<NumT>>& demand,
                                  std::vector<size_t>& nodes_to_demand,
                                  size_t i)
{
    do
    {
        // TODO: change to loop
        size_t l = LEFT(i);
        size_t r = RIGHT(i);
        size_t smallest;
        if ((l < demand.size()) && (demand[l]._cost < demand[i]._cost))
        {
            smallest = l;
        }
        else
        {
            smallest = i;
        }
        if ((r < demand.size()) && (demand[r]._cost < demand[smallest]._cost))
        {
            smallest = r;
        }

        if (smallest == i)
            return;

        swap_heap(demand, nodes_to_demand, i, smallest);
        i = smallest;

    } while (true);
}

template <typename NumT>
void min_cost_flow<NumT>::swap_heap(std::vector<edge<NumT>>& demand,
                                    std::vector<size_t>& nodes_to_demand,
                                    size_t i, size_t j)
{
    edge<NumT> tmp = demand[i];
    demand[i] = demand[j];
    demand[j] = tmp;
    nodes_to_demand[demand[j]._to] = j;
    nodes_to_demand[demand[i]._to] = i;
}

template <typename NumT>
NumT min_cost_flow<NumT>::emd_hat(const std::vector<NumT>& supply,
                                  const std::vector<NumT>& demand,
                                  const std::vector<std::vector<NumT>>& cost)
{
    if (std::is_integral<NumT>::value && std::is_signed<NumT>::value)
    {
        return integral_emd_hat<NumT>(supply, demand, cost);
    }
    else
    {

        const double mult_factor = 1000000;

        // Constructing the input
        const size_t n = supply.size();
        std::vector<int64_t> i_supply(n);
        std::vector<int64_t> i_demand(n);
        std::vector<std::vector<int64_t>> i_cost(n, std::vector<int64_t>(n));

        // Converting to uint64_t
        double sum_supply = 0.0;
        double sum_demand = 0.0;
        double max_cost = cost[0][0];
        for (size_t i = 0; i < n; ++i)
        {
            sum_supply += supply[i];
            sum_demand += demand[i];
            for (size_t j = 0; j < n; ++j)
            {
                if (cost[i][j] > max_cost)
                    max_cost = cost[i][j];
            }
        }
        double max_sum = std::max(sum_supply, sum_demand);
        double supply_demand_norm_factor = mult_factor / max_sum;
        if (max_cost < 1e-12){
            return 0.0;
        }
        double cost_norm_factor = mult_factor / max_cost;
        for (size_t i = 0; i < n; ++i)
        {
            i_supply[i] = static_cast<int64_t>(
                floor(supply[i] * supply_demand_norm_factor + 0.5));
            i_demand[i] = static_cast<int64_t>(
                floor(demand[i] * supply_demand_norm_factor + 0.5));
            for (size_t j = 0; j < n; ++j)
            {
                i_cost[i][j] = static_cast<int64_t>(
                    floor(cost[i][j] * cost_norm_factor + 0.5));
            }
        }

        // computing distance
        double dist = integral_emd_hat<int64_t>(i_supply, i_demand, i_cost);

        dist = dist / supply_demand_norm_factor;
        dist = dist / cost_norm_factor;

        return dist;
    }
}

template <typename NumT>
template <typename T>
T min_cost_flow<NumT>::integral_emd_hat(
    const std::vector<T>& supply_c, const std::vector<T>& demand_c,
    const std::vector<std::vector<T>>& cost_c)
{
    size_t n = supply_c.size();
    assert(demand_c.size() == n);

    // Ensuring that the supplier - supply, have more mass.
    std::vector<T> supply;
    std::vector<T> demand;
    std::vector<std::vector<T>> cost(cost_c);
    T abs_diff_sum_supply_sum_denamd;
    T sum_supply = 0;
    T sum_demand = 0;
    for (size_t i = 0; i < n; ++i)
    {
        sum_supply += supply_c[i];
        sum_demand += demand_c[i];
    }

    if (sum_demand > sum_supply)
    {
        supply = demand_c;
        demand = supply_c;
        // transpose cost
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = 0; j < n; ++j)
            {
                cost[i][j] = cost_c[j][i];
            }
        }
        abs_diff_sum_supply_sum_denamd = sum_demand - sum_supply;
    }
    else
    {
        supply = supply_c;
        demand = demand_c;
        abs_diff_sum_supply_sum_denamd = sum_supply - sum_demand;
    }

    // creating the b vector that contains all vertexes
    std::vector<T> b(2 * n + 2);
    const size_t threshold_node = 2 * n;
    const size_t artificial_node = 2 * n + 1; // need to be last !
    for (size_t i = 0; i < n; ++i)
    {
        b[i] = supply[i];
        b[i + n] = demand[i];
    }

    // remark*) Deficit of the extra mass, as mass that flows to the threshold
    // node can be absorbed from all sources with cost zero
    // This makes sum of b zero.
    b[threshold_node] = -abs_diff_sum_supply_sum_denamd;
    b[artificial_node] = 0;

    T max_cost = 0;
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            assert(cost[i][j] >= 0);
            if (cost[i][j] > max_cost)
                max_cost = cost[i][j];
        }
    }

    std::set<size_t> sources_that_flow_not_only_to_thresh;
    std::set<size_t> sinks_that_get_flow_not_only_from_thresh;
    T pre_flow_cost = 0;

    // regular edges between sinks and sources without threshold edges
    std::vector<std::list<edge<T>>> c(b.size());
    {
        for (size_t i = 0; i < n; ++i)
        {
            if (b[i] == 0)
                continue;
            {
                for (size_t j = 0; j < n; ++j)
                {
                    if (b[j + n] == 0)
                        continue;
                    if (cost[i][j] == max_cost)
                        continue;
                    c[i].push_back(edge<T>(j + n, cost[i][j]));

                    // checking which are not isolated
                    sources_that_flow_not_only_to_thresh.insert(i);
                    sinks_that_get_flow_not_only_from_thresh.insert(j + n);
                }
            }
        }
    }

    // converting all sinks to negative
    for (size_t i = n; i < 2 * n; ++i)
    {
        b[i] = -b[i];
    }

    // add edges from/to threshold node,
    // note that costs are reversed to the paper (see also remark* above)
    // It is important that it will be this way because of remark* above.
    for (size_t i = 0; i < n; ++i)
    {
        c[i].push_back(edge<T>(threshold_node, 0));
        c[threshold_node].push_back(edge<T>(i + n, max_cost));
    }

    // artificial arcs - Note the restriction that only one edge i,j is
    // artificial so I ignore it...
    for (size_t i = 0; i < artificial_node; ++i)
    {
        c[i].push_back(edge<T>(artificial_node, max_cost + 1));
        c[artificial_node].push_back(edge<T>(i, max_cost + 1));
    }

    // remove nodes with supply demand of 0
    // and vertices that are connected only to the
    // threshold vertex
    int current_node_name = 0;
    // Note here it should be vector<int> and not vector<size_t>
    // as I'm using -1 as a special flag !!!
    const int remove_node_flag = -1;
    std::vector<int> nodes_new_names(b.size(), remove_node_flag);
    std::vector<size_t> nodes_old_names;
    nodes_old_names.reserve(b.size());

    for (size_t i = 0; i < n * 2; ++i)
    {
        if (b[i] != 0)
        {
            if (sources_that_flow_not_only_to_thresh.find(i)
                    != sources_that_flow_not_only_to_thresh.end()
                || sinks_that_get_flow_not_only_from_thresh.find(i)
                       != sinks_that_get_flow_not_only_from_thresh.end())
            {
                nodes_new_names[i] = current_node_name;
                nodes_old_names.push_back(i);
                ++current_node_name;
            }
            else
            {
                if (i >= n)
                { // sink
                    pre_flow_cost -= (b[i] * max_cost);
                }
                b[threshold_node] += b[i]; // add mass(i<n) or deficit (i>=n)
            }
        }
    }

    nodes_new_names[threshold_node] = current_node_name;
    nodes_old_names.push_back(threshold_node);
    ++current_node_name;
    nodes_new_names[artificial_node] = current_node_name;
    nodes_old_names.push_back(artificial_node);
    ++current_node_name;

    std::vector<T> bb(current_node_name);
    size_t j = 0;
    for (size_t i = 0; i < b.size(); ++i)
    {
        if (nodes_new_names[i] != remove_node_flag)
        {
            bb[j] = b[i];
            ++j;
        }
    }

    std::vector<std::list<edge<T>>> cc(bb.size());
    for (size_t i = 0; i < c.size(); ++i)
    {
        if (nodes_new_names[i] == remove_node_flag)
            continue;
        for (auto it = c[i].begin(); it != c[i].end(); ++it)
        {
            if (nodes_new_names[it->_to] != remove_node_flag)
            {
                cc[nodes_new_names[i]].push_back(
                    edge<T>(nodes_new_names[it->_to], it->_cost));
            }
        }
    }

    min_cost_flow<T> mcf;
    T my_dist;
    std::vector<std::list<edge_weighted<T>>> flows(bb.size());

    T mcf_dist = mcf.compute_min_cost_flow(bb, cc, flows);

    my_dist = pre_flow_cost + // pre-flowing on cases where it was possible
              mcf_dist;       // solution of the transportation problem

    return my_dist;
}
}
}

// Copyright (c) 2009-2012, Ofir Pele
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//    * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//    * Neither the name of the The Hebrew University of Jerusalem nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
