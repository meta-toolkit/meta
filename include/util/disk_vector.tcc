/**
 * @file disk_vector.tcc
 * @author Sean Massung
 */

#include "util/common.h"

namespace meta {
namespace util {

template <class T>
disk_vector<T>::disk_vector(const std::string & path, uint64_t size):
    _path{path + ".vector"},
    _start{nullptr},
    _size{size},
    _file_desc{-1}
{
    _file_desc = open(_path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(_file_desc < 0)
        throw disk_vector_exception{"error obtaining file descriptor for "
            + _path};

    uint64_t size_bytes = sizeof(T) * size;

    // if file doesn't exist yet, make it the correct size by seeking to the end
    // and writing a byte
    uint64_t actual_size = common::file_size(path);
    if(actual_size != size_bytes)
    {
        if(lseek(_file_desc, size_bytes - 1, SEEK_SET) == -1)
            throw disk_vector_exception{"error lseeking to extend file"};
        if(write(_file_desc, " ", 1) != 1)
            throw disk_vector_exception{"error writing to extend vector file"};
    }

    _start = (T*) mmap(nullptr, size_bytes, PROT_READ | PROT_WRITE,
            MAP_SHARED, _file_desc, 0);

    if(_start == nullptr)
        throw disk_vector_exception{"error memory-mapping the file " + _path};
}

template <class T>
disk_vector<T>::~disk_vector()
{
    munmap(_start, sizeof(T) * _size);
    close(_file_desc);
}

template <class T>
T & disk_vector<T>::operator[](uint64_t idx)
{
    return _start[idx];
}

template <class T>
const T & disk_vector<T>::operator[](uint64_t idx) const
{
    return _start[idx];
}

template <class T>
T & disk_vector<T>::at(uint64_t idx)
{
    if(idx >= _size)
        throw disk_vector_exception{"index out of range"};
    return _start[idx];
}

template <class T>
const T & disk_vector<T>::at(uint64_t idx) const
{
    if(idx >= _size)
        throw disk_vector_exception{"index out of range"};
    return _start[idx];
}

template <class T>
uint64_t disk_vector<T>::size() const
{
    return _size;
}

template <class T>
typename disk_vector<T>::iterator disk_vector<T>::begin() const
{
    return iterator{0, _start};
}

template <class T>
typename disk_vector<T>::iterator disk_vector<T>::end() const
{
    return iterator{_size, _start};
}

}
}
