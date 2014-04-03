/**
 * @file compressed_file_writer.cpp
 * @author Sean Massung
 */

#include <cmath>
#include <cstring>
#include <limits>
#include "io/compressed_file_writer.h"

namespace meta
{
namespace io
{

compressed_file_writer::compressed_file_writer(const std::string& filename,
                                               std::function
                                               <uint64_t(uint64_t)> mapping)
    : outfile_{fopen(filename.c_str(), "w")},
      char_cursor_{0},
      bit_cursor_{0},
      buffer_size_{1024 * 1024 * 64}, // 64 MB
      buffer_{new unsigned char[buffer_size_]},
      mapping_{std::move(mapping)},
      bit_location_{0},
      closed_{false}
{
    // disable buffering
    if (setvbuf(outfile_, nullptr, _IONBF, 0) != 0)
        throw compressed_file_writer_exception(
            "error disabling buffering (setvbuf)");

    // zero out, we'll only write ones
    memset(buffer_, 0, buffer_size_);
}

void compressed_file_writer::write(const std::string& str)
{
    uint64_t length = str.size();
    write(length);
    for (auto& ch : str)
    {
        auto uch = static_cast<uint8_t>(ch);
        write(static_cast<uint64_t>(uch));
    }
}

uint64_t compressed_file_writer::bit_location() const
{
    return bit_location_;
}

compressed_file_writer::~compressed_file_writer()
{
    if (!closed_)
        close();
}

void compressed_file_writer::close()
{
    if (!closed_)
    {
        // write the remaining bits, up to the nearest byte
        fwrite(buffer_, 1, char_cursor_ + 1, outfile_);
        delete[] buffer_;
        fclose(outfile_);

        closed_ = true;
    }
}

void compressed_file_writer::write(uint64_t value)
{
    uint64_t cvalue = mapping_(value);
    uint64_t length = std::log2(cvalue);

    for (uint64_t bit = 0; bit < length; ++bit)
        write_bit(false);

    write_bit(true);

    for (int64_t bit = length - 1; bit >= 0; --bit)
        write_bit(cvalue & 1 << bit);
}

void compressed_file_writer::write_bit(bool bit)
{
    ++bit_location_;

    if (bit)
        buffer_[char_cursor_] |= (1 << (7 - bit_cursor_));

    if (++bit_cursor_ == 8)
    {
        bit_cursor_ = 0;
        if (++char_cursor_ == buffer_size_)
        {
            char_cursor_ = 0;
            write_buffer();
        }
    }
}

void compressed_file_writer::write_buffer() const
{
    if (fwrite(buffer_, 1, buffer_size_, outfile_) != buffer_size_)
        throw compressed_file_writer_exception("error writing to file");
    memset(buffer_, 0, buffer_size_);
}

uint64_t default_compression_writer_func(uint64_t key)
{
    if (key == std::numeric_limits<uint64_t>::max()) // delimiter
        return uint64_t{1};
    return key + 2;
}
}
}
