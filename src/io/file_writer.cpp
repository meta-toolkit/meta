/**
 * @file file_writer.cpp
 * @author Sean Massung
 */

#include <type_traits>
#include "io/file_writer.h"

namespace meta {
namespace io {

file_writer::file_writer(const std::string & filename):
    _byte{0},
    _file_desc{open(filename.c_str(), O_RDWR | O_CREAT, 0755)}
{ /* nothing */ }

file_writer::~file_writer()
{
    ::close(_file_desc);
}

uint64_t file_writer::byte() const
{
    return _byte;
}

void file_writer::reset()
{
    lseek(_file_desc, 0, SEEK_SET);
    _byte = 0;
}

void file_writer::close()
{
    ::close(_file_desc);
}

}
}
