/**
 * @file sparse_vector.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_SPARSE_VECTOR_H_
#define META_UTIL_SPARSE_VECTOR_H_

#include <cstdint>
#include <utility>
#include <vector>

#include "meta/config.h"

namespace meta
{
namespace util
{

/**
 * Represents a sparse vector, indexed by type Index and storing values of
 * type Value. This stores the elements in the vector in sorted order by
 * the Index type.
 */
template <class Index, class Value>
class sparse_vector
{
  public:
    using pair_type = std::pair<Index, Value>;
    using container_type = std::vector<pair_type>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    /**
     * Creates an empty sparse_vector.
     */
    sparse_vector() = default;

    /**
     * Creates a sparse vector with size elements.
     * @param size The number of elements in the new sparse vector.
     */
    sparse_vector(uint64_t size);

    /**
     * Creates a sparse vector from a pair of iterators of pairs.
     * @param begin The iterator to the beginning of the sequence
     * @param end The iterator to the end of the sequence
     */
    template <class Iter>
    sparse_vector(Iter begin, Iter end);

    /**
     * Gets the value associated with a given index in the vector, adding
     * it with a value-initialized Value if not present.
     *
     * @param index The index of the sparse vector to access
     * @return a reference to the value associated with that index into
     * the vector
     */
    Value& operator[](const Index& index);

    /**
     * Gets the value associated with a given index in the vector. If not
     * present, this will return a value-initialized Value and will NOT
     * modify the vector.
     *
     * @param index The index of the sparse vector to access
     * @return the value associated with that index into the vector, or a
     * value-initialized Value if there is none
     */
    Value at(const Index& index) const;

    /**
     * @param index The index of the sparse vector to search for
     * @return an iterator to that position, or the end iterator if it
     * does not exist
     */
    const_iterator find(const Index& index) const;

    /**
     * Adds a new element to the vector, placing it at the back.
     *
     * This assumes that you are adding elements in sorted order by
     * Index---otherwise, the behavior of the vector afterwards is
     * undefined.
     *
     * @param ts The arguments to forward to the pair constructor of the
     * new element
     */
    template <class... Ts>
    void emplace_back(Ts&&... ts);

    /**
     * Reserves space for elements
     * @param size The number of elements to reserve space for
     */
    void reserve(uint64_t size);

    /**
     * Empties the vector.
     */
    void clear();

    /**
     * Optimises the memory usage of the vector.
     */
    void shrink_to_fit();

    /**
     * Condenses the vector to include only non-zero (value initialized)
     * entries.
     */
    void condense();

    /**
     * @return the number of non-zero elements in the vector
     */
    uint64_t size() const;

    /**
     * @return the total capacity of the underlying storage
     */
    uint64_t capacity() const;

    /**
     * @return whether the vector is empty
     */
    bool empty() const;

    /**
     * @return the contents of the vector
     */
    const container_type& contents() const;

    /**
     * @param cont The new contents of the vector
     */
    void contents(container_type cont);

    /**
     * @return an iterator to the beginning of the vector
     */
    iterator begin();

    /**
     * @return a const_iterator to the beginning of the vector
     */
    const_iterator begin() const;

    /**
     * @return a const_iterator to the beginning of the vector
     */
    const_iterator cbegin() const;

    /**
     * @return an iterator to the end of the vector
     */
    iterator end();

    /**
     * @return a const_iterator to the end of the vector
     */
    const_iterator end() const;

    /**
     * @return a const_iterator to the end of the vector
     */
    const_iterator cend() const;

    /**
     * Removes an element.
     */
    iterator erase(iterator pos);

    /**
     * Removes elements in an iterator range.
     */
    iterator erase(iterator first, iterator last);

    sparse_vector& operator+=(const sparse_vector& rhs);
    sparse_vector& operator-=(const sparse_vector& rhs);

  private:
    /**
     * Internal storage for the sparse vector: a sorted vector of pairs.
     */
    container_type storage_;
};

template <class Index, class Value>
sparse_vector<Index, Value> operator+(const sparse_vector<Index, Value>& lhs,
                                      const sparse_vector<Index, Value>& rhs)
{
    return sparse_vector<Index, Value>(lhs) += rhs;
}

template <class Index, class Value>
sparse_vector<Index, Value> operator+(sparse_vector<Index, Value>&& lhs,
                                      const sparse_vector<Index, Value>& rhs)
{
    return lhs += rhs;
}

template <class Index, class Value>
sparse_vector<Index, Value> operator+(const sparse_vector<Index, Value>& lhs,
                                      sparse_vector<Index, Value>&& rhs)
{
    return rhs += lhs;
}

template <class Index, class Value>
sparse_vector<Index, Value> operator+(sparse_vector<Index, Value>&& lhs,
                                      sparse_vector<Index, Value>&& rhs)
{
    return lhs += rhs;
}

template <class Index, class Value>
sparse_vector<Index, Value> operator-(const sparse_vector<Index, Value>& lhs,
                                      const sparse_vector<Index, Value>& rhs)
{
    return sparse_vector<Index, Value>(lhs) -= rhs;
}

template <class Index, class Value>
sparse_vector<Index, Value> operator-(sparse_vector<Index, Value>&& lhs,
                                      const sparse_vector<Index, Value>& rhs)
{
    return lhs -= rhs;
}

template <class Index, class Value>
sparse_vector<Index, Value> operator-(sparse_vector<Index, Value>&& lhs,
                                      sparse_vector<Index, Value>&& rhs)
{
    return lhs -= rhs;
}

template <class SparseVector1, class SparseVector2>
double dot_product(SparseVector1&& first, SparseVector2&& second)
{
    auto first_it = std::begin(first);
    auto first_end = std::end(first);

    auto second_it = std::begin(second);
    auto second_end = std::end(second);

    auto dot = 0.0;
    while (first_it != first_end && second_it != second_end)
    {
        if (first_it->first == second_it->first)
        {
            dot += first_it->second * second_it->second;
            ++first_it;
            ++second_it;
        }
        else if (first_it->first < second_it->first)
        {
            ++first_it;
        }
        else
        {
            ++second_it;
        }
    }

    return dot;
}
}
}
#include "meta/util/sparse_vector.tcc"
#endif
