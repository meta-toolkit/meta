/**
 * @file range.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_RANGE_H_
#define META_UTIL_RANGE_H_

#include <cmath>
#include <iterator>

#include "meta/config.h"

namespace meta
{
namespace util
{

/**
 * Implements a range that spans a loop's extension and termination conditions,
 * most useful for iterating over a range of numbers with a range-based for
 * loop.
 */
template <class T>
class basic_range
{
  public:
    /**
     * Iterator to traverse the generic range.
     */
    template <class Plus>
    class iterator_t
    {
      public:
        /// convenience typedef for the current iterator type
        using self_type = iterator_t;
        /// convenience typedef for the contained value of the range
        using value_type = T;
        /// a reference to the contained type
        using reference = const T&;
        /// a pointer to the contained type
        using pointer = const T*;
        /// the category for this iterator
        using iterator_category = std::forward_iterator_tag;
        /// the difference type for this iterator
        using difference_type = std::ptrdiff_t;

        friend class basic_range<T>;

        /**
         * Prefix increment.
         * @return the current iterator after incrementing
         */
        self_type& operator++()
        {
            _curr = _plus(_curr, _range->_step);
            ++_idx;
            return *this;
        }

        /**
         * Postfix increment.
         * @return the old iterator
         */
        self_type operator++(int)
        {
            self_type temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * Dereference operator.
         * @return a reference to the current value
         */
        reference operator*()
        {
            return _curr;
        }

        /**
         * Member access operator.
         * @return a pointer to the current value (member access)
         */
        pointer operator->()
        {
            return &_curr;
        }

        /**
         * Equality operator.
         * @param lhs
         * @param rhs
         * @return whether the two iterators are equivalent
         */
        friend bool operator==(const iterator_t& lhs, const iterator_t& rhs)
        {
            return lhs._range == rhs._range && lhs._idx == rhs._idx;
        }

        /**
         * Inequality operator.
         * @param lhs
         * @param rhs
         * @return whether the two iterators are not equivalent (defined
         * in terms of iterator_t::operator==).
         */
        friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs)
        {
            return !(lhs == rhs);
        }

      private:
        /**
         * Constructs a new iterator.
         *
         * @param range The range we are iterating over
         * @param start The start of the range
         * @param idx The index to begin from
         */
        iterator_t(const basic_range<T>* range, const T& start, size_t idx)
            : _curr(start), _idx(idx), _range(range)
        {
            // nothing
        }

        /// the current value
        T _curr;
        /// the current index into the ragne
        size_t _idx;
        /// the functor to be used for incrementing the current value
        Plus _plus;
        /// a back-pointer to the range this iterator is operating over
        const basic_range<T>* _range;
    };

    /// the iterator for the range class
    using iterator = iterator_t<std::plus<T>>;
    /// the const_iterator for the range class (same as the iterator)
    using const_iterator = iterator;

    /// the reverse_iterator for the range class
    using reverse_iterator = iterator_t<std::minus<T>>;
    /// the const_reverse_iterator for the range class
    using const_reverse_iterator = reverse_iterator;

    friend iterator;
    friend reverse_iterator;

    /**
     * @return an iterator the beginning of the range
     */
    iterator begin() const
    {
        return iterator(this, _begin, 0);
    }

    /**
     * @return an iterator to the end of the range
     */
    iterator end() const
    {
        return iterator(this, _end, _num);
    }

    /**
     * @return a reverse_iterator to the beginning of the range
     */
    reverse_iterator rbegin() const
    {
        return reverse_iterator(this, _end, 0);
    }

    /**
     * @return a reverse_iterator to the end of the range
     */
    reverse_iterator rend() const
    {
        return reverse_iterator(this, _begin, _num);
    }

    /**
     * Constructs a range.
     *
     * @param begin The starting value for the range
     * @param end The ending value for the range
     * @param step The step size of the range
     */
    basic_range(const T& begin, const T& end, const T& step)
        : _begin(begin), _end(end), _num((end - begin) / step + 1), _step(step)
    {
        // nothing
    }

    basic_range(const basic_range&) = default;
    basic_range(basic_range&&) = default;
    basic_range& operator=(const basic_range&) = default;
    basic_range& operator=(basic_range&&) = default;

    /**
     * Defaulted destructor.
     */
    ~basic_range() = default;

  private:
    /// the beginning value for the range
    T _begin;
    /// the ending value for the range
    T _end;
    /// the number of values contained in the range
    size_t _num;
    /// the step size of the range
    T _step;
};

/**
 * Constructs a range based on a beginning and ending value.
 * @param begin The desired start for the range
 * @param end The desired end for the range
 * @return a new range between begin and end
 */
template <class T>
basic_range<T> range(const T& begin, const T& end)
{
    return basic_range<T>(begin, end, T{1});
}

/**
 * Constructs a range based on a beginning value, and ending value, and a
 * step size.
 * @param begin The desired start for the range
 * @param end The desired end for the range
 * @param step The desired step size for the range
 * @return a new range between begin and end that increments by step each
 * time
 */
template <class T>
basic_range<T> range(const T& begin, const T& end, const T& step)
{
    return basic_range<T>(begin, end, step);
}
}
}

#endif
