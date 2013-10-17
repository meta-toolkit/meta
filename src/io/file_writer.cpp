/**
 * @file file_writer.cpp
 * @author Sean Massung
 */

#include "io/file_writer.h"

namespace meta {
namespace io {

file_writer::file_writer(const std::string & filename):
    _outfile{fopen(filename.c_str(), "w")},
    _buffer_size{1024 * 1024 * 16},  // 16 MB
    _buffer{""}
{
    // disable buffering
    if(setvbuf(_outfile, nullptr, _IONBF, 0) != 0)
        throw file_writer_exception("error disabling buffering (setvbuf)");
}

file_writer::~file_writer()
{
    if(_buffer.size() > 0)
        fwrite(_buffer.c_str(), 1, _buffer.size(), _outfile);
    fclose(_outfile);
}

void file_writer::write(const std::string & data)
{
    if(data.size() > 0)
    {
        _buffer += data;
        if(_buffer.size() > _buffer_size)
        {
            fwrite(_buffer.c_str(), 1, _buffer.size(), _outfile);
            _buffer.clear();
        }
    }
}

}
}
