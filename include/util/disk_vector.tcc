/**
 * @file disk_vector.tcc
 * @author Sean Massung
 */

#include "util/filesystem.h"

namespace meta
{
namespace util
{

template <class T>
disk_vector<T>::disk_vector(const std::string& path, uint64_t size /* = 0 */)
    : path_{path}, start_{nullptr}, size_{size}, file_desc_{-1}
{
    file_desc_ = open(path_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file_desc_ < 0)
        throw disk_vector_exception{"error obtaining file descriptor for "
                                    + path_};

    uint64_t actual_size = filesystem::file_size(path_);
    if (size_ != 0)
    {
        uint64_t size_bytes = sizeof(T) * size_;

        // if file doesn't exist yet, make it the correct size by seeking to the
        // end and writing a byte
        if (actual_size != size_bytes)
        {
            if (lseek(file_desc_, size_bytes - 1, SEEK_SET) == -1)
                throw disk_vector_exception{"error lseeking to extend file"};
            if (write(file_desc_, " ", 1) != 1)
                throw disk_vector_exception{
                    "error writing to extend vector file"};
        }
    }
    else
        size_ = actual_size / sizeof(T);

    start_ = (T*)mmap(nullptr, sizeof(T) * size_, PROT_READ | PROT_WRITE,
                      MAP_SHARED, file_desc_, 0);

    if (start_ == nullptr)
        throw disk_vector_exception{"error memory-mapping the file " + path_};
}

template <class T>
disk_vector<T>::disk_vector(disk_vector&& other)
    : path_{std::move(other.path_)},
      start_{std::move(other.start_)},
      size_{std::move(other.size_)},
      file_desc_{std::move(other.file_desc_)}
{
    other.start_ = nullptr;
}

template <class T>
disk_vector<T>& disk_vector<T>::operator=(disk_vector&& other)
{
    if (this != &other)
    {
        if (start_)
        {
            munmap(start_, sizeof(T) * size_);
            close(file_desc_);
        }
        path_ = std::move(other.path_);
        start_ = std::move(other.start_);
        size_ = std::move(other.size_);
        file_desc_ = std::move(other.file_desc_);
        other.start_ = nullptr;
    }
    return *this;
}

template <class T>
disk_vector<T>::~disk_vector()
{
    if (!start_)
        return;
    munmap(start_, sizeof(T) * size_);
    close(file_desc_);
}

template <class T>
T& disk_vector<T>::operator[](uint64_t idx)
{
    return start_[idx];
}

template <class T>
const T& disk_vector<T>::operator[](uint64_t idx) const
{
    return start_[idx];
}

template <class T>
T& disk_vector<T>::at(uint64_t idx)
{
    if (idx >= size_)
        throw disk_vector_exception{"index out of range"};
    return start_[idx];
}

template <class T>
const T& disk_vector<T>::at(uint64_t idx) const
{
    if (idx >= size_)
        throw disk_vector_exception{"index out of range"};
    return start_[idx];
}

template <class T>
uint64_t disk_vector<T>::size() const
{
    return size_;
}

template <class T>
typename disk_vector<T>::iterator disk_vector<T>::begin() const
{
    return iterator{0, start_};
}

template <class T>
typename disk_vector<T>::iterator disk_vector<T>::end() const
{
    return iterator{size_, start_};
}
}
}
