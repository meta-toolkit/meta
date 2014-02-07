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
    : _path{path}, _start{nullptr}, _size{filesystem::file_size(path)}
{
    _file_descriptor = open(_path.c_str(), O_RDONLY);
    if (_file_descriptor < 0)
        throw mmap_file_exception{"error obtaining file descriptor for "
                                  + _path};

    _start = (char*)mmap(nullptr, _size, PROT_READ, MAP_SHARED,
                         _file_descriptor, 0);
    if (_start == nullptr)
    {
        close(_file_descriptor);
        throw mmap_file_exception("error memory-mapping " + _path);
    }
}

mmap_file::mmap_file(mmap_file&& other)
    : _path{std::move(other._path)},
      _start{std::move(other._start)},
      _size{std::move(other._size)},
      _file_descriptor{std::move(other._file_descriptor)}
{
    other._start = nullptr;
}

char mmap_file::operator[](uint64_t index) const
{
    if (index > _size)
        throw mmap_file_exception{"index out of bounds"};

    return _start[index];
}

char* mmap_file::begin() const
{
    return _start;
}

mmap_file& mmap_file::operator=(mmap_file&& other)
{
    if (this != &other)
    {
        if (_start)
        {
            munmap(_start, _size);
            close(_file_descriptor);
        }
        _path = std::move(other._path);
        _start = std::move(other._start);
        _size = std::move(other._size);
        _file_descriptor = std::move(other._file_descriptor);
        other._start = nullptr;
    }
    return *this;
}

uint64_t mmap_file::size() const
{
    return _size;
}

std::string mmap_file::path() const
{
    return _path;
}

mmap_file::~mmap_file()
{
    if (_start != nullptr)
    {
        munmap(_start, _size);
        close(_file_descriptor);
    }
}
}
}
