/**
 * @file compressed_file_reader.cpp
 * @author Sean Massung
 */

#include "io/compressed_file_reader.h"

namespace meta {
namespace io {

compressed_file_reader::compressed_file_reader(const mmap_file & file,
        const util::invertible_map<uint64_t, uint64_t> & mapping):
    _start{file.start()},
    _size{file.size()},
    _status{notDone},
    _current_value{0},
    _current_char{0},
    _current_bit{0},
    _mapping{mapping}
{
    // initialize the stream
    get_next();
}

void compressed_file_reader::reset()
{
    _current_char = 0;
    _current_bit = 0;
    _status = notDone;
    get_next();
}

void compressed_file_reader::seek(uint64_t bit_offset)
{
    uint64_t byte = bit_offset / 8;
    uint8_t bit = bit_offset % 8;

    if(byte < _size)
    {
        _current_char = byte;
        _current_bit = bit;
        _status = notDone;
        get_next();
    }
    else
        throw compressed_file_reader_exception(
                "error seeking: parameter out of bounds");
}

bool compressed_file_reader::has_next() const
{
    return _status != readerDone;
}

uint64_t compressed_file_reader::next()
{
    if(_status == userDone)
        return 0;

    if(_status == readerDone)
    {
        _status = userDone;
        return _current_value;
    }

    uint64_t next = _mapping.get_key(_current_value);
    get_next();
    return next;
}

void compressed_file_reader::get_next()
{
    uint64_t numberBits = 0;
    while(_status == 0 && !read_bit())
        ++numberBits;

    _current_value = 0;
    for(int64_t bit = numberBits - 1; _status == 0 && bit >= 0; --bit)
    {
        if(read_bit())
            _current_value |= (1 << bit);
    }

    _current_value |= (1 << numberBits);
}

bool compressed_file_reader::read_bit()
{
    // (7 - _currentBit) to read from left to right
    bool bit = _start[_current_char] & (1 << (7 - _current_bit));
    if(_current_bit == 7)
    {
        _current_bit = 0;
        if(++_current_char == _size)
            _status = readerDone;
    }
    else
    {
        ++_current_bit;
    }
    return bit;
}

}
}
