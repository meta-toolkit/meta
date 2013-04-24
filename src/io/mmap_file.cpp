/**
 * @file mmap_file.cpp
 */

#include "io/mmap_file.h"

namespace meta {
namespace io {

using std::string;

mmap_file::mmap_file(string path):
    _path(path), _start(nullptr), _file_descriptor(-1)
{

    struct stat st;
    stat(_path.c_str(), &st);
    _size = st.st_size; 

     _file_descriptor = open(_path.c_str(), O_RDONLY);
    if(_file_descriptor < 0)
        throw mmap_file_exception("error obtaining file descriptor for " + _path);

    _start = (char*) mmap(nullptr, _size, PROT_READ, MAP_SHARED, _file_descriptor, 0);
    if(_start == nullptr)
    {
        close(_file_descriptor);
        throw mmap_file_exception("error memory-mapping " + _path);
    }
}

unsigned int mmap_file::size() const
{
    return _size;
}

string mmap_file::path() const
{
    return _path;
}

char* mmap_file::start() const
{
    return _start;
}

mmap_file::~mmap_file()
{
    if(_start != nullptr)
    {
        munmap(_start, _size);
        close(_file_descriptor);
    }
}

}
}
