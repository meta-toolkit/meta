/**
 * @file compressed_file_reader.cpp
 * @author Sean Massung
 */

#include <limits>
#include "io/compressed_file_reader.h"
#include "io/mmap_file.h"
#include "util/shim.h"

namespace meta
{
namespace io
{

compressed_file_reader::compressed_file_reader(const std::string& filename,
                                               std::function
                                               <uint64_t(uint64_t)> mapping)
    : file_{make_unique<mmap_file>(filename)},
      start_{file_->begin()},
      size_{file_->size()},
      status_{notDone},
      current_value_{0},
      current_char_{0},
      current_bit_{0},
      mapping_{std::move(mapping)}
{
    // initialize the stream
    get_next();
}

compressed_file_reader::compressed_file_reader(const mmap_file& file,
                                               std::function
                                               <uint64_t(uint64_t)> mapping)
    : file_{nullptr},
      start_{file.begin()},
      size_{file.size()},
      status_{notDone},
      current_value_{0},
      current_char_{0},
      current_bit_{0},
      mapping_{std::move(mapping)}
{
    // initialize the stream
    get_next();
}

compressed_file_reader::~compressed_file_reader() = default;

void compressed_file_reader::close()
{
    file_.reset(nullptr); // closes the mmap_file
}

uint64_t compressed_file_reader::bit_location() const
{
    return (current_char_ * 8) + current_bit_;
}

void compressed_file_reader::reset()
{
    current_char_ = 0;
    current_bit_ = 0;
    status_ = notDone;
    get_next();
}

std::string compressed_file_reader::next_string()
{
    uint64_t length = next();
    std::string str;
    for (uint64_t i = 0; i < length; ++i)
        str += static_cast<char>(next());
    return str;
}

void compressed_file_reader::seek(uint64_t bit_offset)
{
    uint64_t byte = bit_offset / 8;
    uint8_t bit = bit_offset % 8;

    if (byte < size_)
    {
        current_char_ = byte;
        current_bit_ = bit;
        status_ = notDone;
        get_next();
    }
    else
        throw compressed_file_reader_exception(
            "error seeking: parameter out of bounds");
}

bool compressed_file_reader::has_next() const
{
    return status_ != readerDone;
}

uint64_t compressed_file_reader::next()
{
    if (status_ == userDone)
        return 0;

    if (status_ == readerDone)
    {
        status_ = userDone;
        return current_value_;
    }

    uint64_t next = mapping_(current_value_);
    get_next();
    return next;
}

void compressed_file_reader::get_next()
{
    uint64_t numberBits = 0;
    while (status_ == 0 && !read_bit())
        ++numberBits;

    current_value_ = 0;
    for (int64_t bit = numberBits - 1; status_ == 0 && bit >= 0; --bit)
    {
        if (read_bit())
            current_value_ |= (1 << bit);
    }

    current_value_ |= (1 << numberBits);
}

bool compressed_file_reader::read_bit()
{
    // (7 - current_Bit) to read from left to right
    bool bit = start_[current_char_] & (1 << (7 - current_bit_));
    if (current_bit_ == 7)
    {
        current_bit_ = 0;
        if (++current_char_ == size_)
            status_ = readerDone;
    }
    else
    {
        ++current_bit_;
    }
    return bit;
}

uint64_t default_compression_reader_func(uint64_t value)
{
    if (value == 1)
        return std::numeric_limits<uint64_t>::max(); // delimiter
    return value - 2;
}
}
}
