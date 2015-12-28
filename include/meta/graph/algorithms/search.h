/**
 * @file search.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GRAPH_ALGORITHMS_SEARCH_H_
#define META_GRAPH_ALGORITHMS_SEARCH_H_

#include "meta/graph/undirected_graph.h"
#include "meta/graph/directed_graph.h"

namespace meta
{
namespace graph
{
namespace algorithms
{
/**
 * @param g
 * @param src
 * @param dest
 * @return a path from src to dest
 */
template <class Graph>
std::vector<node_id> myopic_search(Graph& g, node_id src, node_id dest);

/**
 * @param g
 * @param src
 * @param dest
 * @return the shortest path from src to dest in number of edges
 */
template <class Graph>
std::vector<node_id> bfs(Graph& g, node_id src, node_id dest);
}
}
}

#include "meta/graph/algorithms/search.tcc"
#endif
