/**
 * @file iterator.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_ITERATOR_H_
#define META_UTIL_ITERATOR_H_

#include <iterator>
#include <type_traits>

#include "meta/config.h"
#include "meta/util/comparable.h"

namespace meta
{
namespace util
{

/**
 * Wrapper around an Iterator that, when dereferenced, returns f(*it)
 * where `it` is the wrapped Iterator and `f` is a UnaryFunction.
 */
template <class Iterator, class UnaryFunction>
class transform_iterator
    : public comparable<transform_iterator<Iterator, UnaryFunction>>
{
  public:
    using traits_type = std::iterator_traits<Iterator>;
    using difference_type = typename traits_type::difference_type;
    using value_type = typename std::result_of<UnaryFunction(
        typename traits_type::reference)>::type;
    using pointer = typename std::add_pointer<value_type>::type;
    using reference =
        typename std::add_lvalue_reference<const value_type>::type;
    using iterator_category = typename traits_type::iterator_category;

    transform_iterator(Iterator it, UnaryFunction fun) : it_{it}, fun_(fun)
    {
        // nothing
    }

    transform_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    transform_iterator operator++(int)
    {
        auto tmp = *this;
        ++it_;
        return tmp;
    }

    transform_iterator& operator--()
    {
        --it_;
        return *this;
    }

    transform_iterator operator--(int)
    {
        auto tmp = *this;
        --it_;
        return *tmp;
    }

    transform_iterator& operator+=(difference_type diff)
    {
        it_ += diff;
        return *this;
    }

    transform_iterator operator+(difference_type diff) const
    {
        auto tmp = *this;
        tmp += diff;
        return tmp;
    }

    transform_iterator& operator-=(difference_type diff)
    {
        it_ -= diff;
        return *this;
    }

    transform_iterator operator-(difference_type diff) const
    {
        auto tmp = *this;
        tmp -= diff;
        return tmp;
    }

    difference_type operator-(transform_iterator other) const
    {
        return it_ - other.it_;
    }

    reference operator[](difference_type diff) const
    {
        return fun_(it_[diff]);
    }

    bool operator<(transform_iterator other) const
    {
        return it_ < other.it_;
    }

    value_type operator*() const
    {
        return fun_(*it_);
    }

  private:
    Iterator it_;
    UnaryFunction fun_;
};

/**
 * Helper function to construct a transform_iterator from an Iterator and
 * a UnaryFunction to transform the values of that Iterator.
 */
template <class Iterator, class UnaryFunction>
transform_iterator<Iterator, UnaryFunction>
make_transform_iterator(Iterator it, UnaryFunction&& fun)
{
    return transform_iterator<Iterator, UnaryFunction>(
        it, std::forward<UnaryFunction>(fun));
}
}
}
#endif
