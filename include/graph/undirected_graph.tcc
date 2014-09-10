/**
 * @file unundirected_graph.tcc
 * @author Sean Massung
 */

namespace meta
{
namespace graph
{

template <class Node, class Edge>
auto undirected_graph<Node, Edge>::adjacent(node_id id) const -> const
    adjacency_list &
{
    if (id >= size())
        throw undirected_graph_exception{"node_id out of range"};

    return nodes_.at(id).second;
}


template <class Node, class Edge>
node_id undirected_graph<Node, Edge>::insert(Node node)
{
    node.id = size();
    nodes_.emplace_back(node, adjacency_list{});
    return node.id;
}

template <class Node, class Edge>
void undirected_graph<Node, Edge>::add_edge(const Edge& edge, node_id source,
                                            node_id dest)
{
    if (source == dest)
        throw undirected_graph_exception{"can not create self-loops"};

    if (source >= size() || dest >= size())
        throw undirected_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const std::pair<node_id, Edge>& p)
                           {
        return p.first == dest;
    });
    if (it != list.end())
        throw undirected_graph_exception{"attempted to add existing edge"};

    ++num_edges_;

    // add connections to both adjacency lists
    list.emplace_back(dest, edge);
    nodes_[dest].second.emplace_back(source, edge);
}
}
}
