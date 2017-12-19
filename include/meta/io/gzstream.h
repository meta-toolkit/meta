/**
 * @file gzstream.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_GZSTREAM_H_
#define META_UTIL_GZSTREAM_H_

#include <zlib.h>

#include <streambuf>
#include <vector>

#include "meta/config.h"
#include "meta/io/zstream.h"

namespace meta
{
namespace io
{

class gzstreambuf : public std::streambuf
{
  public:
    gzstreambuf(const char* filename, const char* openmode,
                size_t buffer_size = 512);

    ~gzstreambuf();

    int_type underflow() override;

    int_type overflow(int_type ch) override;

    int sync() override;

    bool is_open() const;

  private:
    std::vector<char> buffer_;
    gzFile file_;
};

/**
 * An ifstream that can read gz compressed files.
 */
using gzifstream = zifstream<gzstreambuf>;

/**
 * An ofstream that can write gz compressed files.
 */
using gzofstream = zofstream<gzstreambuf>;
}
}
#endif
