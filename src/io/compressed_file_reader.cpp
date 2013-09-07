/**
 * @file compressed_file_reader.cpp
 * @author Sean Massung
 */

#include "io/compressed_file_reader.h"

namespace meta {
namespace io {

compressed_file_reader::compressed_file_reader(const std::string & filename,
        const util::invertible_map<uint64_t, uint64_t> & mapping):
    _status{notDone},
    _current_value{0},
    _current_char{0},
    _current_bit{0},
    _mapping{mapping}
{
    struct stat st;
    stat(filename.c_str(), &st);
    _size = st.st_size;
    
    // get file descriptor
    _fileDescriptor = open(filename.c_str(), O_RDONLY);
    if(_fileDescriptor < 0)
        throw compressed_file_reader_exception(
                "error obtaining file descriptor for " + filename);
    
    // memory map
    _start = (unsigned char*)
        mmap(nullptr, _size, PROT_READ, MAP_SHARED, _fileDescriptor, 0);
    if(_start == nullptr)
    {
        close(_fileDescriptor);
        throw compressed_file_reader_exception("error memory-mapping the file");
    }

    // initialize the stream
    get_next();
}

compressed_file_reader::~compressed_file_reader()
{
    if(_start != nullptr)
    { 
        // unmap memory and close file
        munmap(_start, _size);
        close(_fileDescriptor);
    }
}

void compressed_file_reader::reset()
{
    _current_char = 0;
    _current_bit = 0;
    _status = notDone;
    get_next();
}

void compressed_file_reader::seek(uint64_t position, uint8_t bitOffset)
{
    if(bitOffset <= 7 && position < _size)
    {
        _current_char = position;
        _current_bit = bitOffset;
        _status = notDone;
        get_next();
    }
    else
        throw compressed_file_reader_exception(
            "error seeking; invalid parameters: position = "
            + common::to_string(position)
            + ", bit = "
            + common::to_string(static_cast<int>(bitOffset))
            + " (size is "
            + common::to_string(_size)
            + " bytes)"
        );
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
