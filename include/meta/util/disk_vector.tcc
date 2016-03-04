/**
 * @file disk_vector.tcc
 * @author Sean Massung
 */

#include <sys/stat.h>
#include "meta/io/filesystem.h"
#include "meta/util/disk_vector.h"

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
            auto offset = static_cast<long>(size_bytes - 1);
            if (lseek(file_desc_, offset, SEEK_SET) == -1)
                throw disk_vector_exception{"error lseeking to extend file"};
            if (write(file_desc_, " ", 1) != 1)
                throw disk_vector_exception{
                    "error writing to extend vector file"};
        }
    }
    else
    {
        size_ = actual_size / sizeof(T);
        if (size_ == 0)
            throw disk_vector_exception{"cannot map empty file " + path};
    }

    start_ = (T*)mmap(nullptr, sizeof(T) * size_, PROT_READ | PROT_WRITE,
                      MAP_SHARED, file_desc_, 0);

    if (start_ == MAP_FAILED)
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
        throw disk_vector_exception{"index " + std::to_string(idx)
                                    + " out of range [0, "
                                    + std::to_string(size_) + ")"};
    return start_[idx];
}

template <class T>
const T& disk_vector<T>::at(uint64_t idx) const
{
    if (idx >= size_)
        throw disk_vector_exception{"index " + std::to_string(idx)
                                    + " out of range [0, "
                                    + std::to_string(size_) + ")"};
    return start_[idx];
}

template <class T>
uint64_t disk_vector<T>::size() const
{
    return size_;
}

template <class T>
auto disk_vector<T>::begin() -> iterator
{
    return start_;
}

template <class T>
auto disk_vector<T>::begin() const -> const_iterator
{
    return start_;
}

template <class T>
auto disk_vector<T>::end() const -> const_iterator
{
    return start_ + size_;
}

template <class T>
auto disk_vector<T>::end() -> iterator
{
    return start_ + size_;
}
}
}
