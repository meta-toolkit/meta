/**
 * @file dense_matrix.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_DENSE_MATRIX_H_
#define META_UTIL_DENSE_MATRIX_H_

#include <cstdint>
#include <vector>

#include "meta/config.h"

namespace meta
{
namespace util
{

/**
 * Simple wrapper class for representing a dense matrix laid out in
 * row-major order (that is, its internal representation is a linear array
 * of the rows).
 */
template <class T>
class dense_matrix
{
  public:
    /**
     * Default constructed matrices are empty.
     */
    dense_matrix() = default;

    /**
     * dense_matrix may be copy constructed.
     */
    dense_matrix(const dense_matrix&) = default;

    /**
     * dense_matrix may be move constructed.
     */
    dense_matrix(dense_matrix&&) = default;

    /**
     * dense_matrix may be copy assigned.
     */
    dense_matrix& operator=(const dense_matrix&) = default;

    /**
     * dense_matrix may be move assigned.
     */
    dense_matrix& operator=(dense_matrix&&) = default;

    /**
     * Constructs a dense_matrix with the specified number of rows and
     * columns. Construction of the matrix value-initializes all elements
     * (just as std::vector does), which ensures that elements are set to
     * 0 when T is a numeric type.
     *
     * @param rows The desired number of rows
     * @param columns The desired number of columns
     */
    dense_matrix(uint64_t rows, uint64_t columns);

    /**
     * Resizes the dense_matrix to have the specified number of rows and
     * columns. *All* elements are reset to be value initialized, not just
     * new ones!
     *
     * @param rows The desired new number of rows
     * @param columns The desired number of columns
     */
    void resize(uint64_t rows, uint64_t columns);

    /**
     * Obtains the column-th element of the row-th row.
     *
     * @param row The row index
     * @param column The column index
     * @return a reference to the element at that position
     */
    T& operator()(uint64_t row, uint64_t column);

    /**
     * Obtains the column-th element of the row-th row.
     *
     * @param row The row index
     * @param column The column index
     * @return a const reference to the element at that position
     */
    const T& operator()(uint64_t row, uint64_t column) const;

    using row_iterator = typename std::vector<T>::iterator;
    using const_row_iterator = typename std::vector<T>::const_iterator;

    /**
     * @param row The row index
     * @return an iterator over the row-th row
     */
    row_iterator begin(uint64_t row);

    /**
     * @param row The row index
     * @return a const iterator over the row-th row
     */
    const_row_iterator begin(uint64_t row) const;

    /**
     * @param row The row index
     * @return an iterator to the end of the row-th row
     */
    row_iterator end(uint64_t row);

    /**
     * @param row The row index
     * @return an iterator to the end of the row-th row
     */
    const_row_iterator end(uint64_t row) const;

    /**
     * @return the number of rows in the matrix
     */
    uint64_t rows() const;

    /**
     * @return the number of columns in the matrix
     */
    uint64_t columns() const;

  private:
    /// the underlying storage for the matrix
    std::vector<T> storage_;
    /// the number of columns in the matrix
    uint64_t columns_;
};
}
}

#include "meta/util/dense_matrix.tcc"

#endif
