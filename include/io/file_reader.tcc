/**
 * @file file_reader.tcc
 */

namespace meta {
namespace io {

file_reader::file_reader(const std::string & filename):
    _file_desc{open(filename.c_str(), O_RDONLY, 0755)}
{ /* nothing */ }

file_reader::~file_reader()
{
    close(_file_desc);
}

template <class T>
void file_reader::read(T & elem)
{
    ::read(_file_desc, &elem, sizeof(T));
}

void file_reader::read(std::string & str)
{
    std::string::size_type length;
    read(length);
    char* buf = new char[length];
    ::read(_file_desc, buf, length);
    str.assign(buf, length); // TODO move?
    delete [] buf;
}

}
}
