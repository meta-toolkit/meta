/**
 * @file gzstream.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include "meta/io/gzstream.h"

namespace meta
{
namespace io
{

gzstreambuf::gzstreambuf(const char* filename, const char* openmode,
                         size_t buffer_size)
    : buffer_(buffer_size), file_{gzopen(filename, openmode)}
{
    auto end = &buffer_.back() + 1;
    setg(end, end, end);

    auto begin = &buffer_.front();
    setp(begin, end - 1);
}

gzstreambuf::~gzstreambuf()
{
    sync();
    gzclose(file_);
    file_ = nullptr;
}

auto gzstreambuf::underflow() -> int_type
{
    if (gptr() && (gptr() < egptr()))
        return traits_type::to_int_type(*gptr());

    if (!is_open())
        return traits_type::eof();

    auto bytes = gzread(file_, &buffer_[0], static_cast<unsigned>(buffer_.size()));
    if (bytes <= 0)
    {
        setg(&buffer_[0], &buffer_[0], &buffer_[0]);
        return traits_type::eof();
    }

    setg(&buffer_[0], &buffer_[0], &buffer_[0] + bytes);

    return traits_type::to_int_type(*gptr());
}

auto gzstreambuf::overflow(int_type ch) -> int_type
{
    if (ch != traits_type::eof())
    {
        *pptr() = traits_type::to_char_type(ch);
        pbump(1);
        if (sync() == 0)
            return ch;
    }

    return traits_type::eof();
}

int gzstreambuf::sync()
{
    auto bytes = static_cast<int>(pptr() - pbase());
    if (bytes > 0)
    {

        if (gzwrite(file_, pbase(), static_cast<unsigned>(bytes)) != bytes)
            return -1;
        pbump(-bytes);
    }

    return 0;
}

bool gzstreambuf::is_open() const
{
    return file_ != nullptr;
}

gzifstream::gzifstream(std::string name)
    : std::istream{&buffer_}, buffer_{name.c_str(), "rb"}
{
    clear();
}

gzstreambuf* gzifstream::rdbuf() const
{
    return const_cast<gzstreambuf*>(&buffer_);
}

void gzifstream::flush()
{
    buffer_.sync();
}

gzofstream::gzofstream(std::string name)
    : std::ostream{&buffer_}, buffer_{name.c_str(), "wb"}
{
    clear();
}

gzstreambuf* gzofstream::rdbuf() const
{
    return const_cast<gzstreambuf*>(&buffer_);
}

void gzofstream::flush()
{
    buffer_.sync();
}
}
}
