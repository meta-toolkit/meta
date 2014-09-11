/**
 * @file node_iterator.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

// forward declare both graph types so they can be used in constructor
template <class A, class B>
class undirected_graph;
template <class A, class B>
class directed_graph;

class node_iterator : public std::iterator<std::forward_iterator_tag, Node>
{
  public:
    typedef node_iterator self_type;
    typedef Node value_type;
    typedef Node& reference;
    typedef Node* pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    friend bool operator==(const self_type& lhs, const self_type& rhs)
    {
        return lhs.cur_id_ == rhs.cur_id_;
    }

    friend bool operator!=(const self_type& lhs, const self_type& rhs)
    {
        return !(lhs == rhs);
    }

    node_iterator(undirected_graph<Node, Edge>* handle, node_id idx)
        : nodes_{handle->nodes_}, cur_id_{idx}
    {
    }

    node_iterator(directed_graph<Node, Edge>* handle, node_id idx)
        : nodes_{handle->nodes_}, cur_id_{idx}
    {
    }

    self_type operator++()
    {
        ++cur_id_;
        return *this;
    }

    self_type operator++(int)
    {
        self_type saved{*this};
        ++(*this);
        return saved;
    }

    reference operator*() { return nodes_[cur_id_].first; }

    pointer operator->() { return &nodes_[cur_id_].first; }

  private:
    std::vector<std::pair<Node, adjacency_list>>& nodes_;
    node_id cur_id_;
};
