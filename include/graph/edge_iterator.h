/**
 * @file edge_iterator.h
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

class edge_iterator : public std::iterator<std::forward_iterator_tag, Edge>
{
  public:
    typedef edge_iterator self_type;
    typedef Edge value_type;
    typedef Edge& reference;
    typedef Edge* pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    friend bool operator==(const self_type& lhs, const self_type& rhs)
    {
        return lhs.cur_id_ == rhs.cur_id_ && lhs.end_ == rhs.end_;
    }

    friend bool operator!=(const self_type& lhs, const self_type& rhs)
    {
        return !(lhs == rhs);
    }

    edge_iterator(undirected_graph<Node, Edge>* handle, node_id idx, bool end)
        : nodes_{handle->nodes_}, cur_id_{idx}, end_{end}, is_undirected_{true}
    {
        init(handle->num_edges());
    }

    edge_iterator(directed_graph<Node, Edge>* handle, node_id idx, bool end)
        : nodes_{handle->nodes_}, cur_id_{idx}, end_{end}, is_undirected_{false}
    {
        init(handle->num_edges());
    }

    void init(uint64_t num_edges)
    {
        if (num_edges == 0)
        {
            cur_id_ = nodes_.size();
            end_ = true;
        }
        else if (cur_id_ < nodes_.size())
        {
            iter_ = nodes_[cur_id_].second.begin();
            if (nodes_[cur_id_].second.empty())
                ++(*this);
        }
    }

    self_type operator++()
    {
        if (++iter_ == nodes_[cur_id_].second.end())
        {
            while (++cur_id_ < nodes_.size() && nodes_[cur_id_].second.empty())
                /* nothing */;
            if (cur_id_ < nodes_.size())
                iter_ = nodes_[cur_id_].second.begin();
            else
                end_ = true;
        }

        // use this inequality to not display duplicate edges since each edge is
        // stored twice in an undirected graph
        if (is_undirected_ && !end_ && iter_->first < cur_id_)
            return ++(*this);

        return *this;
    }

    self_type operator++(int)
    {
        self_type saved{*this};
        ++(*this);
        return saved;
    }

    reference operator*() { return iter_->second; }

    pointer operator->() { return &iter_->second; }

  private:
    std::vector<std::pair<Node, adjacency_list>>& nodes_;
    node_id cur_id_;
    typename graph<Node, Edge>::adjacency_list::iterator iter_;
    bool end_;
    bool is_undirected_;
};
