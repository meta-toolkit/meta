/**
 * @file disk_vector.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DISK_VECTOR_H_
#define META_DISK_VECTOR_H_

#include <type_traits>
#include <string>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "meta.h"

namespace meta
{
namespace util
{

/**
 * disk_vector represents a large constant-size vector that does not necessarily
 * fit in memory.
 */
template <class T>
class disk_vector
{
    static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value
                  || std::is_base_of<util::numeric, T>::value,
                  "disk_vector templated types must be integral types");

  public:
    /**
     * @param path The path to save this vector as. If the file
     * exists, it is treated as disk_vector. If the file doesn't exist, a
     * new one is created.
     * @param size The number of elements that will be in this vector. If not
     * specified, the disk_vector assumes that the file already exists.
     */
    disk_vector(const std::string& path, uint64_t size = 0);

    /**
     * Move constructor.
     */
    disk_vector(disk_vector&&);

    /**
     * Move assignment operator.
     */
    disk_vector& operator=(disk_vector&&);

    /**
     * Destructor.
     */
    ~disk_vector();

    /**
     * @param idx The index of the vector to retrieve
     * @return a reference to the element at position idx in the vector
     * container
     */
    T& operator[](uint64_t idx);

    /**
     * @param idx The index of the vector to retrieve
     * @return a reference to the element at position idx in the vector
     * container
     */
    const T& operator[](uint64_t idx) const;

    /**
     * @param idx The index of the vector to retrieve
     * @return a reference to the element at position idx in the vector
     *
     * The function automatically checks whether idx is within the bounds of
     * valid elements in the vector, throwing an  exception if it is not
     * (i.e., if idx is greater or equal than its size). This is in contrast
     * with member operator[], that does not check against bounds.
     */
    T& at(uint64_t idx);

    /**
     * @param idx The index of the vector to retrieve
     * @return a reference to the element at position idx in the vector
     *
     * The function automatically checks whether idx is within the bounds of
     * valid elements in the vector, throwing an  exception if it is not
     * (i.e., if idx is greater or equal than its size). This is in contrast
     * with member operator[], that does not check against bounds.
     */
    const T& at(uint64_t idx) const;

    /**
     * @return the number of elements this vector stores
     */
    uint64_t size() const;

    /**
     * Provides iterator functionality for the disk_vector class.
     */
    class iterator : public std::iterator<std::random_access_iterator_tag, T>
    {
        /// Need to access disk_vector representation
        friend disk_vector;

      private:
        /// The current index this iterator is at
        uint64_t idx_;

        /// The current element this iterator is at
        T* data_;

        /**
         * Constructor for disk_vector to use.
         * @param idx The index to start out at
         * @param data The data element to initially contain
         */
        iterator(uint64_t idx, T* data) : idx_{idx}, data_{data}
        {
            /* nothing */
        }

      public:
        /// Constructor.
        iterator() : idx_{0}, data_{nullptr}
        {
            /* nothing */
        }

        /// Copy constructor.
        iterator(const iterator& other) : idx_{other.idx_}, data_{other.data_}
        {
            /* nothing */
        }

        /// assignment operator.
        iterator& operator=(iterator other)
        {
            std::swap(*this, other);
            return *this;
        }

        /// Pre-increment.
        iterator& operator++()
        {
            ++idx_;
            return *this;
        }

        /// Post-increment.
        iterator operator++(int)
        {
            iterator save{*this};
            ++idx_;
            return save;
        }

        /// Pre-decrement.
        iterator& operator--()
        {
            --idx_;
            return *this;
        }

        /// Post-decrement.
        iterator operator--(int)
        {
            iterator save{*this};
            --idx_;
            return *this;
        }

        /// Equality.
        bool operator==(const iterator& other)
        {
            return other.idx_ == idx_ && other.data_ == data_;
        }

        /// Inequality.
        bool operator!=(const iterator& other)
        {
            return !(*this == other);
        }

        /// Dereference operator.
        T& operator*()
        {
            return data_[idx_];
        }

        /// Arrow operator.
        const T* operator->()
        {
            return &data_[idx_];
        }

        /// Operator<.
        bool operator<(const iterator& other) const
        {
            return idx_ < other.idx_;
        }

        /// Operator>.
        bool operator>(const iterator& other) const
        {
            return idx_ > other.idx_;
        }

        /// Operator<=.
        bool operator<=(const iterator& other) const
        {
            return idx_ <= other.idx_;
        }

        /// Operator>=.
        bool operator>=(const iterator& other) const
        {
            return idx_ >= other.idx_;
        }
    };

    /**
     * @return an iterator to the beginning of this container
     */
    iterator begin() const;

    /**
     * @return an iterator to the end of this container
     */
    iterator end() const;

  private:
    /// the path to the file this disk_vector uses for storage
    std::string path_;

    /// the beginning of where the storage file is memory mapped
    T* start_;

    /// this size of the memory-mapped file (in regards to T objects)
    uint64_t size_;

    /// the file descriptor used to open and close the mmap file
    int file_desc_;

  public:
    /**
     * Basic exception for disk_vector.
     */
    class disk_vector_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };
};
}
}

#include "disk_vector.tcc"
#endif
