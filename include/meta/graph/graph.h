/**
 * @file graph.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GRAPH_H_
#define META_GRAPH_H_

#include "meta/config.h"
#include "meta/meta.h"
#include "meta/util/optional.h"

namespace meta
{
namespace graph
{
template <class Node, class Edge>
class graph
{
  public:
    using adjacency_list = std::vector<std::pair<node_id, Edge>>;
    using node_type = Node;
    using edge_type = Edge;

    virtual ~graph() = default;

    /**
     * @param id
     * @return the Node object that the id represents
     */
    virtual Node& node(node_id id);
    virtual const Node& node(node_id id) const;

    /**
     * @param source
     * @param dest
     * @return an optional edge connecting source and dest
     */
    virtual util::optional<Edge> edge(node_id source, node_id dest) const;

    /**
     * @return the size of this graph (number of nodes), which is the
     * range for a valid node_id
     */
    virtual uint64_t size() const;

    /**
     * @return the number of edges in the graph
     */
    virtual uint64_t num_edges() const;

    /**
     * @param node The new object to add into the graph
     * @return the id of the inserted node
     */
    virtual node_id insert(Node node) = 0;

    /**
     * Constructs a node with forwarded arguments.
     * @return the id of the inserted node
     */
    template <class... Args>
    node_id emplace(Args&&... args);

    /**
     * @param edge
     * @param source
     * @param dest
     */
    virtual void add_edge(Edge edge, node_id source, node_id dest) = 0;

    /**
     * Adds a default edge between the two nodes.
     * @param source
     * @param dest
     */
    virtual void add_edge(node_id source, node_id dest);

  protected:
    /// Each Node object is indexed by its id.
    std::vector<std::pair<Node, adjacency_list>> nodes_;

    /// Saves the number of edges in this graph
    uint64_t num_edges_ = 0;
};

/**
 * Basic exception for graph interactions.
 */
class graph_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
#include "meta/graph/graph.tcc"
#endif
