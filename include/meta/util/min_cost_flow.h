//
// Created by lolik111 on 29.11.17.
//


//
// Created by lolik111 on 28.11.17.
//

#ifndef FAST_EMD_MIN_COST_FLOW_H
#define FAST_EMD_MIN_COST_FLOW_H

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <list>
#include <vector>

namespace meta
{
namespace util
{
template<typename CostType>
struct edge;

template<typename CostType>
struct edge0;

template<typename CostType>
struct edge1;

template<typename CostType>
struct edge2;

template<typename DistType>
struct edge3;

template<typename NumT>
class min_cost_flow
{

public:

    // e - supply(positive) and demand(negative).
    // c[i] - edges that goes from node i. first is the second nod
    // x - the flow is returned in it

    NumT emd_hat(const std::vector<NumT> &supply_orig,
                 const std::vector<NumT> &demand_orig,
                 const std::vector<NumT> &supply,
                 const std::vector<NumT> &demand,
                 const std::vector<std::vector<NumT>> &cost,
                 NumT extra_mass_penalty);

    NumT compute_min_cost_flow(std::vector<NumT> &e,
                               const std::vector<std::list<edge<NumT>>> &c,
                               std::vector<std::list<edge0<NumT>>> &x);


private:

    size_t _num_nodes;
    std::vector<size_t> _nodes_to_demand;

    template<typename T>
    static T integral_emd_hat(const std::vector<T> &supply_orig,
                          const std::vector<T> &demand_orig,
                          const std::vector<T> &supply,
                          const std::vector<T> &demand,
                          const std::vector<std::vector<T>> &cost,
                          T extra_mass_penalty);

    void
    compute_shortest_path(std::vector<NumT> &d, std::vector<size_t> &prev,

                          size_t from,
                          std::vector<std::list<edge1<NumT>>> &cost_forward,
                          std::vector<std::list<edge2<NumT>>> &cost_backward,

                          const std::vector<NumT> &e, size_t &l);

    void heap_decrease_key(std::vector<edge3<NumT>> &demand,
                           std::vector<size_t> &nodes_to_demand, size_t v,
                           NumT alt);

    void heap_remove_first(std::vector<edge3<NumT>> &demand,
                           std::vector<size_t> &nodes_to_demand);

    void heapify(std::vector<edge3<NumT>> &demand,
                 std::vector<size_t> &nodes_to_demand, size_t i);

    void swap_heap(std::vector<edge3<NumT>> &demand,
                   std::vector<size_t> &nodes_to_demand, size_t i, size_t j);

    size_t LEFT(size_t i)
    {
        return 2 * (i + 1) - 1;
    }

    size_t RIGHT(size_t i)
    {
        return 2 * (i + 1); // 2*(i+1)+1-1
    }

    size_t PARENT(size_t i)
    {
        return (i - 1) / 2;
    }

};
// end min_cost_flow

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
}
}

#include "min_cost_flow.tcc"

#endif //FAST_EMD_MIN_COST_FLOW_H
