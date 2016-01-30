/**
 * @file edge_iterator.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

// forward declare (if necessary) both graph types so they can be used in
// edge_iterator constructor
#ifndef META_UNDIRECTED_GRAPH_H_
template <class A, class B>
class undirected_graph;
#endif
#ifndef META_DIRECTED_GRAPH_H_
template <class A, class B>
class directed_graph;
#endif

template <class Iter>
class edge_iterator : public std::iterator<std::forward_iterator_tag, Edge>
{
  public:
    typedef typename graph<Node, Edge>::adjacency_list adj_list;
    typedef edge_iterator self_type;
    typedef typename std::
        conditional<std::is_same<Iter, typename vec_t::const_iterator>::value,
                    const Edge, Edge>::type value_type;
    typedef value_type& reference;
    typedef value_type* pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    friend bool operator==(const self_type& lhs, const self_type& rhs)
    {
        return lhs.iter_ == rhs.iter_ && lhs.end_ == rhs.end_;
    }

    friend bool operator!=(const self_type& lhs, const self_type& rhs)
    {
        return !(lhs == rhs);
    }

    edge_iterator(const undirected_graph<Node, Edge>* handle, const Iter& iter,
                  const Iter& end_iter)
        : iter_{iter},
          end_{iter == end_iter},
          beg_iter_{iter},
          end_iter_{end_iter},
          is_undirected_{true}
    {
        init(handle->num_edges());
    }

    edge_iterator(const directed_graph<Node, Edge>* handle, const Iter& iter,
                  const Iter& end_iter)
        : iter_{iter},
          end_{iter == end_iter},
          beg_iter_{iter},
          end_iter_{end_iter},
          is_undirected_{false}
    {
        init(handle->num_edges());
    }

    void init(uint64_t num_edges)
    {
        if (num_edges == 0)
        {
            iter_ = end_iter_;
            end_ = true;
        }
        else if (iter_ != end_iter_)
        {
            al_iter_ = iter_->second.begin();
            if (iter_->second.empty())
                ++(*this);
        }
    }

    self_type operator++()
    {
        if (++al_iter_ == iter_->second.end())
        {
            while (++iter_ != end_iter_ && iter_->second.empty())
                /* nothing */;
            if (iter_ != end_iter_)
                al_iter_ = iter_->second.begin();
            else
                end_ = true;
        }

        // use this inequality to not display duplicate edges since each edge is
        // stored twice in an undirected graph
        if (is_undirected_ && !end_
            && al_iter_->first
               < static_cast<uint64_t>(std::distance(beg_iter_, iter_)))
            return ++(*this);

        return *this;
    }

    self_type operator++(int)
    {
        self_type saved{*this};
        ++(*this);
        return saved;
    }

    reference operator*()
    {
        return al_iter_->second;
    }

    pointer operator->()
    {
        return &al_iter_->second;
    }

  private:
    Iter iter_;
    typename std::
        conditional<std::is_same<Iter, typename vec_t::const_iterator>::value,
                    typename adj_list::const_iterator,
                    typename adj_list::iterator>::type al_iter_;
    bool end_;
    Iter beg_iter_;
    Iter end_iter_;
    bool is_undirected_;
};
