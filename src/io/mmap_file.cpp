/**
 * @file mmap_file.cpp
 */

#include "io/mmap_file.h"

namespace meta {
namespace io {

using std::string;

MmapFile::MmapFile(string path):
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

unsigned int MmapFile::size() const
{
    return _size;
}

string MmapFile::path() const
{
    return _path;
}

char* MmapFile::start() const
{
    return _start;
}

MmapFile::~MmapFile()
{
    if(_start != nullptr)
    {
        munmap(_start, _size);
        close(_file_descriptor);
    }
}

}
}
