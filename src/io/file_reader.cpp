/**
 * @file file_reader.cpp
 * @author Sean Massung
 */

#include "io/file_reader.h"
#include "util/common.h"

namespace meta {
namespace io {

file_reader::file_reader(const std::string & filename):
    _byte{0},
    _size{common::file_size(filename)},
    _file_desc{open(filename.c_str(), O_RDONLY, 0755)}
{ /* nothing */ }

file_reader::~file_reader()
{
    ::close(_file_desc);
}

uint64_t file_reader::byte() const
{
    return _byte;
}

bool file_reader::good() const
{
    return _byte < _size;
}

void file_reader::close()
{
    ::close(_file_desc);
}

}
}
