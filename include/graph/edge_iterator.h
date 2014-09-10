/**
 * @file edge_iterator.h
 * @author Sean Massung
 */

class edge_iterator : public std::iterator<std::forward_iterator_tag, Edge>
{
  public:
    typedef undirected_graph<Node, Edge> graph_t;
    typedef typename graph_t::adjacency_list::iterator al_iter;

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

    edge_iterator(graph_t* handle, node_id idx, bool end)
        : nodes_{handle->nodes_}, cur_id_{idx}, end_{end}
    {
        if(handle->num_edges() == 0)
        {
            cur_id_ = nodes_.size();
            end_ = true;
        }
        else if (idx < nodes_.size())
        {
            iter_ = nodes_[idx].second.begin();
            if (nodes_[idx].second.empty())
                ++(*this);
        }
    }

    self_type operator++()
    {
        if (++iter_ == nodes_[cur_id_].second.end())
        {
            while (++cur_id_ < nodes_.size()
                   && nodes_[cur_id_].second.empty())
                /* nothing */;
            if (cur_id_ < nodes_.size())
                iter_ = nodes_[cur_id_].second.begin();
            else
                end_ = true;
        }

        // use this inequality to not display duplicate edges since each edge is
        // stored twice
        if(!end_ && iter_->first < cur_id_)
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
    al_iter iter_;
    bool end_;
};
