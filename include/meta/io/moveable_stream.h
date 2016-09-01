/**
 * @file moveable_stream.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_IO_MOVEABLE_STREAM_H_
#define META_IO_MOVEABLE_STREAM_H_

#include <fstream>

#include "meta/config.h"
#include "meta/util/shim.h"

namespace meta
{
namespace io
{

namespace detail
{
template <class T>
T& get_stream(std::unique_ptr<T>& stream)
{
    return *stream;
}

template <class T>
const T& get_stream(const std::unique_ptr<T>& stream)
{
    return *stream;
}

inline std::ifstream& get_stream(std::ifstream& stream)
{
    return stream;
}

inline const std::ifstream& get_stream(const std::ifstream& stream)
{
    return stream;
}

inline std::ofstream& get_stream(std::ofstream& stream)
{
    return stream;
}

inline const std::ofstream& get_stream(const std::ofstream& stream)
{
    return stream;
}

#if META_HAS_STREAM_MOVE
template <class T, class... Args>
T make_stream(Args&&... args)
{
    return T{std::forward<Args>(args)...};
}

#else
template <class T, class... Args>
std::unique_ptr<T> make_stream(Args&&... args)
{
    return make_unique<T>(std::forward<Args>(args)...);
}
#endif

template <class T>
struct default_openmode;

template <>
struct default_openmode<std::ifstream>
{
    const static constexpr std::ios_base::openmode value = std::ios_base::in;
};

template <>
struct default_openmode<std::ofstream>
{
    const static constexpr std::ios_base::openmode value = std::ios_base::out;
};
}

/**
 * A stupid wrapper around a std::fstream to work around GCC's libstdc++
 * lacking move constructors for std::fstream until GCC 5.
 */
template <class Stream>
class mfstream
{
  public:
    mfstream() : stream_{detail::make_stream<Stream>()}
    {
        // nothing
    }

    explicit mfstream(const char* filename,
                      std::ios_base::openmode mode
                      = detail::default_openmode<Stream>::value)
        : stream_{detail::make_stream<Stream>(filename, mode)}
    {
        // nothing
    }

    explicit mfstream(const std::string& filename,
                      std::ios_base::openmode mode
                      = detail::default_openmode<Stream>::value)
        : mfstream{filename.c_str(), mode}
    {
        // nothing
    }

    operator Stream&()
    {
        return detail::get_stream(stream_);
    }

    operator const Stream&() const
    {
        return detail::get_stream(stream_);
    }

    Stream& stream()
    {
        return detail::get_stream(stream_);
    }

    const Stream& stream() const
    {
        return detail::get_stream(stream_);
    }

    explicit operator bool() const
    {
        return static_cast<bool>(detail::get_stream(stream_));
    }

  private:
#if META_HAS_STREAM_MOVE
    Stream stream_;
#else
    std::unique_ptr<Stream> stream_;
#endif
};

using mifstream = mfstream<std::ifstream>;
using mofstream = mfstream<std::ofstream>;
}
}
#endif
