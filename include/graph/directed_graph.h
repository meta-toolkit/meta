/**
 * @file graph.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DIRECTED_GRAPH_H_
#define META_DIRECTED_GRAPH_H_

#include <stdexcept>
#include <vector>
#include <unordered_set>
#include "meta.h"
#include "util/optional.h"
#include "graph/graph.h"
#include "graph/default_node.h"
#include "graph/default_edge.h"

namespace meta
{
namespace graph
{
/**
 * A (currently) simple class to represent a directed graph in memory.
 */
template <class Node = default_node, class Edge = default_edge>
class directed_graph : public graph<Node, Edge>
{
  public:
    using adjacency_list = typename graph<Node, Edge>::adjacency_list;
    using graph<Node, Edge>::nodes_;
    using graph<Node, Edge>::num_edges_;
    using graph<Node, Edge>::size;
    using graph<Node, Edge>::add_edge;

    /**
     * @param id The node id to get outgoing nodes from
     * @return the outgoing edges and node_ids to the given node
     */
    const adjacency_list& outgoing(node_id id) const;

    /**
     * @param id The node id to get incoming nodes to
     * @return a collection of node_ids that are incoming to the parameter node
     */
    const std::vector<node_id>& incoming(node_id id) const;

    /**
     * @param node The new object to add into the graph
     * @return the id of the inserted node
     */
    virtual node_id insert(Node node) override;

    /**
     * @param edge
     * @param source
     * @param dest
     */
    virtual void add_edge(const Edge& edge, node_id source,
                          node_id dest) override;

    #include "node_iterator.h"
    #include "edge_iterator.h"

    /**
     * @return an iterator to the beginning ("first" node) of this graph
     */
    node_iterator begin() { return {this, node_id{0}}; }

    /**
     * @return an iterator that represents one past the last node of this graph
     */
    node_iterator end() { return {this, node_id{nodes_.size()}}; }

    /**
     * @return an iterator to the beginning ("first" edge) of this graph
     */
    edge_iterator edges_begin() { return {this, node_id{0}, false}; }

    /**
     * @return an iterator that represents one past the last node of this graph
     */
    edge_iterator edges_end() { return {this, node_id{nodes_.size()}, true}; }

  private:
    /**
     * Each Node object is indexed by its id. This structure keeps track of
     * incoming nodes to a specific node_id.
     */
    std::vector<std::vector<node_id>> incoming_;
};

/**
 * Basic exception for directed_graph interactions.
 */
class directed_graph_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#include "graph/directed_graph.tcc"
#endif
