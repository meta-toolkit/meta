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

#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <type_traits>
#ifndef _WIN32
#include <sys/mman.h>
#else
#include "meta/io/mman-win32/mman.h"
#endif
#include <unistd.h>

#include "meta/config.h"
#include "meta/meta.h"

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

    using iterator = T*;
    using const_iterator = const T*;

    /**
     * @return an iterator to the beginning of this container
     */
    iterator begin();

    /**
     * @return an iterator to the beginning of this container (const
     * version)
     */
    const_iterator begin() const;

    /**
     * @return an iterator to the end of this container
     */
    iterator end();

    /**
     * @return an iterator to the end of this container (const version)
     */
    const_iterator end() const;

  private:
    /// the path to the file this disk_vector uses for storage
    std::string path_;

    /// the beginning of where the storage file is memory mapped
    T* start_;

    /// this size of the memory-mapped file (in regards to T objects)
    uint64_t size_;

    /// the file descriptor used to open and close the mmap file
    int file_desc_;
};

/**
 * Basic exception for disk_vector.
 */
class disk_vector_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#include "meta/util/disk_vector.tcc"
#endif
