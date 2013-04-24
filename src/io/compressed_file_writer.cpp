/**
 * @file compressed_file_writer.cpp
 */

#include "io/compressed_file_writer.h"

namespace meta {
namespace io {

using std::string;

compressed_file_writer::compressed_file_writer(const string & filename)
{
    _outfile = fopen(filename.c_str(), "w");
    _char_cursor = 0;
    _bit_cursor = 0;
    _buffer_size = 1024 * 1024 * 8,
    _buffer = new unsigned char[_buffer_size];

    // disable buffering
    if(setvbuf(_outfile, nullptr, _IONBF, 0) != 0)
        throw compressed_file_writer_exception("error disabling buffering (setvbuf)");

    // zero out, we'll only write ones
    memset(_buffer, 0, _buffer_size);
}

compressed_file_writer::~compressed_file_writer()
{
    // write the remaining bits, up to the nearest byte
    fwrite(_buffer, 1, _char_cursor + 1, _outfile);
    delete [] _buffer;
    fclose(_outfile);
}

void compressed_file_writer::write(unsigned int value)
{
    int length = log2(value);

    for(int bit = 0; bit < length; ++bit)
        write_bit(false);

    write_bit(true);

    for(int bit = length - 1; bit >= 0; --bit)
        write_bit(value & 1 << bit);
}

void compressed_file_writer::write_bit(bool bit)
{
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
