/**
 * @file file_writer.tcc
 * @author Sean Massung
 */

#include <type_traits>
#include "io/file_writer.h"

namespace meta {
namespace io {

file_writer::file_writer(const std::string & filename):
    _file_desc{open(filename.c_str(), O_RDWR | O_CREAT, 0755)}
{ /* nothing */ }

file_writer::~file_writer()
{
    close(_file_desc);
}

template <class T>
void file_writer::write(const T & data)
{
    static_assert(!std::is_same<T, std::string>::value,
            "do not use the templated write() function for strings!");
    ::write(_file_desc, &data, sizeof(T));
}

void file_writer::write(const std::string & str)
{
    write(str.size());
    ::write(_file_desc, str.c_str(), str.size());
}

}
}
