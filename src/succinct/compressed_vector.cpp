/**
 * @file compressed_vector.cpp
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/succinct/compressed_vector.h"

namespace meta
{
namespace succinct
{

compressed_vector::compressed_vector(const std::string& prefix)
    : numbers_{prefix + "/compressed-vec.bin"},
      positions_{prefix + "/sarray"},
      select_{prefix + "/sarray", positions_}
{
    // nothing
}

uint64_t compressed_vector::operator[](uint64_t i) const
{
    bit_vector_view num_bvv{{numbers_.begin(), numbers_.end()},
                            64 * numbers_.size()};
    auto start = select_.select(i);
    auto end = select_.select(i + 1);
    return num_bvv.extract(start, static_cast<uint8_t>(end - start));
}

uint64_t compressed_vector::size() const
{
    return select_.size() - 1;
}
}
}
