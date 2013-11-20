/**
 * @file compressed_file_writer.cpp
 * @author Sean Massung
 */

#include "io/compressed_file_writer.h"

namespace meta {
namespace io {

compressed_file_writer::compressed_file_writer(const std::string & filename,
        std::function<uint64_t(uint64_t)> mapping):
    _outfile{fopen(filename.c_str(), "w")},
    _char_cursor{0},
    _bit_cursor{0},
    _buffer_size{1024 * 1024 * 8},
    _buffer{new unsigned char[_buffer_size]},
    _mapping{std::move(mapping)},
    _bit_location{0}
{
    // disable buffering
    if(setvbuf(_outfile, nullptr, _IONBF, 0) != 0)
        throw compressed_file_writer_exception(
                "error disabling buffering (setvbuf)");

    // zero out, we'll only write ones
    memset(_buffer, 0, _buffer_size);
}

uint64_t compressed_file_writer::bit_location() const
{
    return _bit_location;
}

compressed_file_writer::~compressed_file_writer()
{
    // write the remaining bits, up to the nearest byte
    fwrite(_buffer, 1, _char_cursor + 1, _outfile);
    delete [] _buffer;
    fclose(_outfile);
}

void compressed_file_writer::write(uint64_t value)
{
    uint64_t cvalue = _mapping(value);
    uint64_t length = log2(cvalue);

    for(uint64_t bit = 0; bit < length; ++bit)
        write_bit(false);

    write_bit(true);

    for(int64_t bit = length - 1; bit >= 0; --bit)
        write_bit(cvalue & 1 << bit);
}

void compressed_file_writer::write_bit(bool bit)
{
    ++_bit_location;

    if(bit)
        _buffer[_char_cursor] |= (1 << (7 - _bit_cursor));

    if(++_bit_cursor == 8)
    {
        _bit_cursor = 0;
        if(++_char_cursor == _buffer_size)
        {
            _char_cursor = 0;
            write_buffer();
        }
    }
}

void compressed_file_writer::write_buffer() const
{
    if(fwrite(_buffer, 1, _buffer_size, _outfile) != _buffer_size)
        throw compressed_file_writer_exception("error writing to file");
    memset(_buffer, 0, _buffer_size);
}

}
}
