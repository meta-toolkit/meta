/**
 * @file mmap_file.cpp
 * @author Sean Massung
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "io/mmap_file.h"
#include "util/filesystem.h"

namespace meta
{
namespace io
{

mmap_file::mmap_file(const std::string& path)
    : path_{path}, start_{nullptr}, size_{filesystem::file_size(path)}
{
    file_descriptor_ = open(path_.c_str(), O_RDONLY);
    if (file_descriptor_ < 0)
        throw mmap_file_exception{"error obtaining file descriptor for "
                                  + path_};

    start_ = (char*)mmap(nullptr, size_, PROT_READ, MAP_SHARED,
                         file_descriptor_, 0);
    if (start_ == nullptr)
    {
        close(file_descriptor_);
        throw mmap_file_exception("error memory-mapping " + path_);
    }
}

mmap_file::mmap_file(mmap_file&& other)
    : path_{std::move(other.path_)},
      start_{std::move(other.start_)},
      size_{std::move(other.size_)},
      file_descriptor_{std::move(other.file_descriptor_)}
{
    other.start_ = nullptr;
}

char mmap_file::operator[](uint64_t index) const
{
    if (index > size_)
        throw mmap_file_exception{"index out of bounds"};

    return start_[index];
}

char* mmap_file::begin() const
{
    return start_;
}

mmap_file& mmap_file::operator=(mmap_file&& other)
{
    if (this != &other)
    {
        if (start_)
        {
            munmap(start_, size_);
            close(file_descriptor_);
        }
        path_ = std::move(other.path_);
        start_ = std::move(other.start_);
        size_ = std::move(other.size_);
        file_descriptor_ = std::move(other.file_descriptor_);
        other.start_ = nullptr;
    }
    return *this;
}

uint64_t mmap_file::size() const
{
    return size_;
}

std::string mmap_file::path() const
{
    return path_;
}

mmap_file::~mmap_file()
{
    if (start_ != nullptr)
    {
        munmap(start_, size_);
        close(file_descriptor_);
    }
}
}
}
