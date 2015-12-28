/**
 * @file dense_matrix.tcc
 * @author Chase Geigle
 */

#include "meta/util/dense_matrix.h"

namespace meta
{
namespace util
{

template <class T>
dense_matrix<T>::dense_matrix(uint64_t rows, uint64_t columns)
    : storage_(rows * columns), columns_{columns}
{
    // nothing: use the fact that std::vector<T> value initializes all
    // elements on construction
}

template <class T>
T& dense_matrix<T>::operator()(uint64_t row, uint64_t column)
{
    return storage_[row * columns_ + column];
}

template <class T>
const T& dense_matrix<T>::operator()(uint64_t row, uint64_t column) const
{
    return storage_[row * columns_ + column];
}

template <class T>
void dense_matrix<T>::resize(uint64_t rows, uint64_t columns)
{
    storage_.resize(rows * columns);
    std::fill(storage_.begin(), storage_.end(), T{});
    columns_ = columns;
}

template <class T>
auto dense_matrix<T>::begin(uint64_t row) -> row_iterator
{
    using diff_type = typename row_iterator::difference_type;
    return storage_.begin() + static_cast<diff_type>(row * columns_);
}

template <class T>
auto dense_matrix<T>::begin(uint64_t row) const -> const_row_iterator
{
    using diff_type = typename const_row_iterator::difference_type;
    return storage_.begin() + static_cast<diff_type>(row * columns_);
}

template <class T>
auto dense_matrix<T>::end(uint64_t row) -> row_iterator
{
    using diff_type = typename row_iterator::difference_type;
    return storage_.begin() + static_cast<diff_type>((row + 1) * columns_);
}

template <class T>
auto dense_matrix<T>::end(uint64_t row) const -> const_row_iterator
{
    using diff_type = typename const_row_iterator::difference_type;
    return storage_.begin() + static_cast<diff_type>((row + 1) * columns_);
}

template <class T>
uint64_t dense_matrix<T>::rows() const
{
    return storage_.size() / columns_;
}

template <class T>
uint64_t dense_matrix<T>::columns() const
{
    return columns_;
}

}
}
