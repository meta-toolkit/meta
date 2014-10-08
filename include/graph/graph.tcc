/**
 * @file graph.tcc
 * @author Sean Massung
 */

#include <algorithm>

namespace meta
{
namespace graph
{
template <class Node, class Edge>
Node& graph<Node, Edge>::node(node_id id)
{
    if (id >= size())
        throw graph_exception{"node_id out of range"};

    return nodes_[id].first;
}

template <class Node, class Edge>
const Node& graph<Node, Edge>::node(node_id id) const
{
    if (id >= size())
        throw graph_exception{"node_id out of range"};

    return nodes_[id].first;
}

template <class Node, class Edge>
typename util::optional<Edge> graph<Node, Edge>::edge(node_id source,
                                                      node_id dest) const
{
    if (source >= size() || dest >= size())
        throw graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const std::pair<node_id, Edge>& p)
                           {
        return p.first == dest;
    });
    if (it != list.end())
        return util::optional<Edge>{it->second};

    return util::optional<Edge>{util::nullopt};
}

template <class Node, class Edge>
uint64_t graph<Node, Edge>::num_edges() const
{
    return num_edges_;
}

template <class Node, class Edge>
uint64_t graph<Node, Edge>::size() const
{
    return nodes_.size();
}

template <class Node, class Edge>
void graph<Node, Edge>::add_edge(node_id source, node_id dest)
{
    add_edge(Edge{}, source, dest);
}

template <class Node, class Edge>
template <class... Args>
node_id graph<Node, Edge>::emplace(Args&&... args)
{
    return insert(Node(std::forward<Args>(args)...));
}
}
}
