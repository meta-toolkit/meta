/**
 * @file filedes.h
 * @author Chase Geigle
 *
 * All files in MeTA are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_IO_FILEDES_H_
#define META_IO_FILEDES_H_

#include <cstdint>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <type_traits>
#if _MSC_VER
#include <io.h>
#include <share.h>
#else
#include <unistd.h>
#endif

#include "meta/config.h"

namespace meta
{
namespace io
{

/**
 * Represents the open mode for a file descriptor. Bitwise | can be applied
 * to create a mix of flags, and bitwise & can be performed to check for
 * the presence of a flag.
 */
enum class open_mode : uint8_t
{
    READ = 1,
    WRITE = 1 << 1,
    READ_WRITE = 1 << 2,
    CREATE = 1 << 3
};

inline open_mode& operator|=(open_mode& a, open_mode b)
{
    using T = std::underlying_type<open_mode>::type;
    return a = static_cast<open_mode>(static_cast<T>(a) | static_cast<T>(b));
}

inline open_mode operator|(open_mode a, open_mode b)
{
    return a |= b;
}

inline bool operator&(open_mode a, open_mode b)
{
    using T = std::underlying_type<open_mode>::type;
    return static_cast<bool>(static_cast<T>(a) & static_cast<T>(b));
}

namespace detail
{
#if _MSC_VER
inline int to_flags(open_mode mode)
{
    int flags = 0;
    flags |= (mode & open_mode::READ) ? _O_RDONLY : 0;
    flags |= (mode & open_mode::WRITE) ? _O_WRONLY : 0;
    flags |= (mode & open_mode::READ_WRITE) ? _O_RDWR : 0;
    flags |= (mode & open_mode::CREATE) ? _O_CREAT : 0;
    return flags;
}

inline int to_share(open_mode mode)
{
    if (mode & open_mode::READ)
        return _SH_DENYWR;
    return _SH_DENYRW;
}
#else
inline int to_flags(open_mode mode)
{
    int flags = 0;
    flags |= (mode & open_mode::READ) ? O_RDONLY : 0;
    flags |= (mode & open_mode::WRITE) ? O_WRONLY : 0;
    flags |= (mode & open_mode::READ_WRITE) ? O_RDWR : 0;
    flags |= (mode & open_mode::CREATE) ? O_CREAT : 0;
    return flags;
}
#endif
}

class file_descriptor_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * RAII wrapper class for a system file descriptor.
 *
 * This class is not very full featured; it mainly serves as a porting
 * class for supporting Unix vs Windows.
 */
class file_descriptor
{
  public:
    file_descriptor() : fd_{-1}
    {
        // nothing
    }

    file_descriptor(const char* path, open_mode mode)
    {
        using detail::to_flags;
#ifndef _MSC_VER
        fd_ = ::open(path, to_flags(mode), S_IRUSR | S_IWUSR);
        if (fd_ < 0)
        {
            throw file_descriptor_exception{
                "error obtaining file descriptor for " + std::string(path)};
        }
#else
        using detail::to_share;
        auto err = ::_sopen_s(&fd_, path, to_flags(mode), to_share(mode),
                              _S_IREAD | _S_IWRITE);
        if (fd_ < 0)
        {
            throw file_descriptor_exception{
                "error obtaining file descriptor for " + std::string(path)};
        }
#endif
    }

    file_descriptor(file_descriptor&& other) : fd_{other.fd_}
    {
        other.fd_ = -1;
    }

    file_descriptor& operator=(file_descriptor&& rhs)
    {
        close();
        fd_ = rhs.fd_;
        rhs.fd_ = -1;
        return *this;
    }

    ~file_descriptor()
    {
        close();
    }

    void close()
    {
        if (fd_ < 0)
            return;
#ifndef _MSC_VER
        ::close(fd_);
#else
        ::_close(fd_);
#endif
        fd_ = -1;
    }

    long lseek(long offset)
    {
#ifndef _MSC_VER
        return ::lseek(fd_, offset, SEEK_SET);
#else
        return ::_lseek(fd_, offset, SEEK_SET);
#endif
    }

    long write(const void* buffer, unsigned int count)
    {
#ifndef _MSC_VER
        return ::write(fd_, buffer, count);
#else
        return ::_write(fd_, buffer, count);
#endif
    }

    operator int() const
    {
        return fd_;
    }

  private:
    int fd_;
};
}
}
#endif
