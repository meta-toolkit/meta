/**
 * @file compressed_vector.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SUCCINCT_COMPRESSED_VECTOR_H_
#define META_SUCCINCT_COMPRESSED_VECTOR_H_

#include <fstream>

#include "meta/config.h"
#include "meta/succinct/bit_vector.h"
#include "meta/succinct/broadword.h"
#include "meta/succinct/sarray.h"
#include "meta/util/disk_vector.h"

namespace meta
{
namespace succinct
{

/**
 * Compressed, \f$O(1)\f$ time random-access sequences of unsigned 64-bit
 * numbers. In order for this to work, the total sum of the minimal binary
 * representation length for each integer (excluding leading zeroes) must
 * fit in a 64-bit integer. Otherwise, the behavior is not defined.
 */
class compressed_vector
{
  public:
    compressed_vector(const std::string& prefix);

    uint64_t operator[](uint64_t i) const;

    uint64_t size() const;

  private:
    util::disk_vector<uint64_t> numbers_;
    sarray positions_;
    sarray_select select_;
};

template <class ForwardIterator>
void make_compressed_vector(const std::string& prefix, ForwardIterator begin,
                            ForwardIterator end)
{
    filesystem::make_directory(prefix);
    std::ofstream bv_stream{prefix + "/compressed-vec.bin", std::ios::binary};
    auto bv_builder = make_bit_vector_builder(bv_stream);

    uint64_t num_elems = 0;
    uint64_t num_bits = 0;
    for (auto it = begin; it != end; ++it)
    {
        uint64_t word = *it;
        num_bits += (word) ? broadword::msb(word) : 1;
        ++num_elems;
    }

    {
        filesystem::make_directory(prefix + "/sarray");
        sarray_builder s_builder{prefix + "/sarray", num_elems + 1, num_bits};
        s_builder(bv_builder.total_bits());
        for (auto it = begin; it != end; ++it)
        {
            uint64_t word = *it;
            uint64_t len = (word) ? broadword::msb(word) : 1;
            bv_builder.write_bits({word, static_cast<uint8_t>(len)});

            s_builder(bv_builder.total_bits());
        }
    }

    sarray select{prefix + "/sarray"};
    sarray_select{prefix + "/sarray", select};
}
}
}
#endif
