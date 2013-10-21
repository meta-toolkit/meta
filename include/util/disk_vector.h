/**
 * @file disk_vector.h
 * @author Sean Massung
 * 
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DISK_VECTOR_H_
#define _DISK_VECTOR_H_

#include <string>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

namespace meta {
namespace util {

/**
 * disk_vector represents a large constant-size vector that does not necessarily
 * fit in memory.
 */
template <class T>
class disk_vector
{
    public:
        /**
         * @param path The path to save this vector as. If the file
         * exists, it is treated as disk_vector. If the file doesn't exist, a
         * new one is created.
         * @param size The number of elements that will be in this vector
         */
        disk_vector(const std::string & path, uint64_t size);

        /**
         * Destructor.
         */
        ~disk_vector();

        /**
         * @param idx The index of the vector to retrieve
         * @return a reference to the element at position idx in the vector
         * container
         */
        T & operator[](uint64_t idx);

        /**
         * @param idx The index of the vector to retrieve
         * @return a reference to the element at position idx in the vector
         * container
         */
        const T & operator[](uint64_t idx) const;

        /**
         * @param idx The index of the vector to retrieve
         * @return a reference to the element at position idx in the vector
         *
         * The function automatically checks whether idx is within the bounds of
         * valid elements in the vector, throwing an  exception if it is not
         * (i.e., if idx is greater or equal than its size). This is in contrast
         * with member operator[], that does not check against bounds.
         */
        T & at(uint64_t idx);

        /**
         * @param idx The index of the vector to retrieve
         * @return a reference to the element at position idx in the vector
         *
         * The function automatically checks whether idx is within the bounds of
         * valid elements in the vector, throwing an  exception if it is not
         * (i.e., if idx is greater or equal than its size). This is in contrast
         * with member operator[], that does not check against bounds.
         */
        const T & at(uint64_t idx) const;

        /**
         * @return the number of elements this vector stores
         */
        uint64_t size() const;

        /**
         * Provides iterator functionality for the disk_vector class.
         */
        class iterator: public std::iterator<std::random_access_iterator_tag, T>
        {
            friend disk_vector;
            private:
                uint64_t _idx;
                T* _data;

                /** constructor for disk_vector */
                iterator(uint64_t idx, T* data): _idx{idx}, _data{data}
                    { /* nothing */ }

            public:
                /** constructor */
                iterator(): _idx{0}, _data{nullptr} { /* nothing */ }

                /** copy constructor */
                iterator(const iterator & other):
                    _idx{other._idx}, _data{other._data}
                { /* nothing */ }

                /** assignment operator */
                iterator & operator=(iterator other)
                {
                    std::swap(*this, other);
                    return *this;
                }

                /** pre-increment */
                iterator & operator++()
                {
                    ++_idx;
                    return *this;
                }

                /** post-increment */
                iterator operator++(int)
                {
                    iterator save{*this};
                    ++_idx;
                    return save;
                }

                /** pre-decrement */
                iterator & operator--()
                {
                    --_idx;
                    return *this;
                }

                /** post-decrement */
                iterator operator--(int)
                {
                    iterator save{*this};
                    --_idx;
                    return *this;
                }

                /** equality */
                bool operator==(const iterator & other)
                {
                    return other._idx == _idx && other._data == _data;
                }

                /** inequality */
                bool operator!=(const iterator & other)
                {
                    return !(*this == other);
                }

                /** dereference operator */
                T & operator*()
                {
                    return _data[_idx];
                }

                /** arrow operator */
                const T* operator->()
                {
                    return &_data[_idx];
                }

                // TODO not all random_access_iterator functions are defined
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
        /** the path to the file this disk_vector uses for storage */
        std::string _path;

        /** the beginning of where the storage file is memory mapped */
        T* _start;

        /** this size of the memory-mapped file (in regards to T objects) */
        uint64_t _size;

        /** the file descriptor used to open and close the mmap file */
        int _file_desc;

    public:
        /**
         * Basic exception for disk_vector.
         */
        class disk_vector_exception: public std::exception
        {
            public:
                disk_vector_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }
           
            private:
                std::string _error;
        };
};

}
}

#include "disk_vector.tcc"
#endif
