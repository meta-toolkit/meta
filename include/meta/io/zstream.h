/**
 * @file zstream.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_IO_ZSTREAM_H_
#define META_IO_ZSTREAM_H_

#include <istream>
#include <ostream>
#include <type_traits>

#include "meta/config.h"

namespace meta
{
namespace io
{

namespace detail
{
template <class StreamBuf>
struct has_bytes_read
{
    template <class T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().bytes_read()),
                              uint64_t>::type;

    template <class>
    static constexpr auto check(...) -> std::false_type;

    using type = decltype(check<StreamBuf>(nullptr));
    const static constexpr bool value = type::value;
};
}

template <class StreamBase, class ZStreamBuf>
class zfstream : public StreamBase
{
  public:
    using streambuf_type = ZStreamBuf;

    explicit zfstream(const std::string& name, const char* openmode)
        : StreamBase{&buffer_}, buffer_{name.c_str(), openmode}
    {
        if (buffer_.is_open())
            this->clear();
        else
            this->setstate(std::ios::badbit);
    }

    streambuf_type* rdbuf() const
    {
        return const_cast<streambuf_type*>(&buffer_);
    }

    void flush()
    {
        buffer_.sync();
    }

    template <class T = ZStreamBuf>
    typename std::enable_if<detail::has_bytes_read<T>::value
                                && std::is_same<StreamBase,
                                                std::istream>::value,
                            uint64_t>::type
    bytes_read() const
    {
        return buffer_.bytes_read();
    }

  private:
    streambuf_type buffer_;
};

/**
 * A base class for an input stream that reads from a compressed streambuf
 * object.
 */
template <class ZStreamBuf>
class zifstream : public zfstream<std::istream, ZStreamBuf>
{
  public:
    explicit zifstream(const std::string& name)
        : zfstream<std::istream, ZStreamBuf>(name, "rb")
    {
        // nothing
    }
};

/**
 * A base class for an output stream that writes to a compressed streambuf
 * object.
 */
template <class ZStreamBuf>
class zofstream : public zfstream<std::ostream, ZStreamBuf>
{
  public:
    explicit zofstream(const std::string& name)
        : zfstream<std::ostream, ZStreamBuf>(name, "wb")
    {
        // nothing
    }
};
}
}
#endif
