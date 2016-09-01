/**
 * @file directed_graph.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DIRECTED_GRAPH_H_
#define META_DIRECTED_GRAPH_H_

#include <cstddef>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "meta/config.h"
#include "meta/graph/default_edge.h"
#include "meta/graph/default_node.h"
#include "meta/graph/graph.h"
#include "meta/meta.h"
#include "meta/util/optional.h"

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
    using vec_t = std::vector<std::pair<Node, adjacency_list>>;
    using graph<Node, Edge>::nodes_;
    using graph<Node, Edge>::num_edges_;
    using graph<Node, Edge>::size;
    using graph<Node, Edge>::add_edge;

    /**
     * @param id The node id to get outgoing nodes from
     * @return the outgoing edges and node_ids to the given node
     */
    const adjacency_list& adjacent(node_id id) const;

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
    virtual void add_edge(Edge edge, node_id source, node_id dest) override;

#include "meta/graph/node_iterator.h"
    typedef node_iterator<typename vec_t::iterator> iterator;
    typedef node_iterator<typename vec_t::const_iterator> const_iterator;

    /**
     * @return an iterator to the beginning ("first" node) of this graph
     */
    iterator begin()
    {
        return {nodes_.begin()};
    }

    const_iterator begin() const
    {
        return {nodes_.cbegin()};
    }

    /**
     * @return an iterator that represents one past the last node of this graph
     */
    iterator end()
    {
        return {nodes_.end()};
    }

    const_iterator end() const
    {
        return {nodes_.cend()};
    }

#include "meta/graph/edge_iterator.h"
    typedef edge_iterator<typename vec_t::iterator> e_iterator;
    typedef edge_iterator<typename vec_t::const_iterator> const_e_iterator;

    /**
     * @return an iterator to the beginning ("first" edge) of this graph
     */
    e_iterator edges_begin()
    {
        return {this, nodes_.begin(), nodes_.end()};
    }

    const_e_iterator edges_begin() const
    {
        return {this, nodes_.cbegin(), nodes_.cend()};
    }

    /**
     * @return an iterator that represents one past the last node of this graph
     */
    e_iterator edges_end()
    {
        return {this, nodes_.end(), nodes_.end()};
    }

    const_e_iterator edges_end() const
    {
        return {this, nodes_.cend(), nodes_.cend()};
    }

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

#include "meta/graph/directed_graph.tcc"
#endif
