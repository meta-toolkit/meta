/**
 * @file directed_graph.tcc
 * @author Sean Massung
 */

namespace meta
{
namespace graph
{
template <class Node, class Edge>
auto directed_graph
    <Node, Edge>::adjacent(node_id id) const -> const AdjacencyList &
{
    if (id >= size())
        throw directed_graph_exception{"node_id out of range"};

    return nodes_[id].second;
}

template <class Node, class Edge>
Node& directed_graph<Node, Edge>::node(node_id id)
{
    if (id >= size())
        throw directed_graph_exception{"node_id out of range"};

    return nodes_[id].first;
}

template <class Node, class Edge>
typename util::optional<Edge> directed_graph
    <Node, Edge>::edge(node_id source, node_id dest)
{
    if (source >= size() || dest >= size())
        throw directed_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = list.find(dest);
    if (it != list.end())
        return {it->second};

    return {util::nullopt};
}

template <class Node, class Edge>
node_id directed_graph<Node, Edge>::insert(const Node& node)
{
    nodes_.emplace_back(node, AdjacencyList{});
    return node_id{nodes_.size() - 1};
}

template <class Node, class Edge>
void directed_graph
    <Node, Edge>::add_edge(const Edge& edge, node_id source, node_id dest)
{
    if (source >= size() || dest >= size())
        throw directed_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = list.find(dest);
    if (it != list.end())
        throw directed_graph_exception{"attempted to add existing edge"};

    it->emplace(dest, edge);
}

template <class Node, class Edge>
uint64_t directed_graph<Node, Edge>::size() const
{
    return nodes_.size();
}
}
}
