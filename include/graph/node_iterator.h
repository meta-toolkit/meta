/**
 * @file node_iterator.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

template <class Iter>
class node_iterator : public std::iterator<std::forward_iterator_tag, Node>
{
  public:
    typedef std::vector<std::pair<Node, adjacency_list>> vec_t;
    typedef node_iterator self_type;
    typedef typename std::conditional<
        std::is_same<Iter, typename vec_t::const_iterator>::value,
        const Node,
        Node>::type value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef std::forward_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    friend bool operator==(const self_type& lhs, const self_type& rhs)
    {
        return lhs.iter_ == rhs.iter_;
    }

    friend bool operator!=(const self_type& lhs, const self_type& rhs)
    {
        return !(lhs == rhs);
    }

    node_iterator(const Iter& iter) : iter_{iter} {}

    self_type operator++()
    {
        ++iter_;
        return *this;
    }

    self_type operator++(int)
    {
        self_type saved{*this};
        ++(*this);
        return saved;
    }

    reference operator*() { return iter_->first; }

    pointer operator->() { return &iter_->first; }

  private:
    Iter iter_;
};
