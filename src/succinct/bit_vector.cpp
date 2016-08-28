/**
 * @file bit_vector.cpp
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/succinct/bit_vector.h"
#include "meta/math/integer.h"
#include "meta/util/likely.h"

namespace meta
{
namespace succinct
{

bit_vector_view::bit_vector_view(util::array_view<const uint64_t> data,
                                 uint64_t num_bits)
    : data_{data}, num_bits_{num_bits}
{
    if (META_UNLIKELY(math::integer::div_ceil(num_bits_, 64) > data.size()))
        throw std::out_of_range{"not enough bits in array view to "
                                "construct a bit vector view of the "
                                "requested length"};
}

bool bit_vector_view::operator[](uint64_t bit_idx) const
{
    auto word_pos = bit_idx / 64;
    auto bit_pos = bit_idx % 64;

    return (data_[word_pos] >> bit_pos) & 1ull;
}

uint64_t bit_vector_view::extract(uint64_t bit_idx, uint8_t len) const
{
#if DEBUG
    if (META_UNLIKELY(len > 64))
        throw std::out_of_range{"bit length longer than word"};

    if (META_UNLIKELY(bit_idx > size()))
        throw std::out_of_range{"bit index larger than bit count"};
#endif

    auto word_pos = bit_idx / 64;
    auto bit_pos = bit_idx % 64;

    uint64_t bits = 0;
    if (64 - bit_pos >= len)
    {
        // one word contains all we need
        bits |= (data_[word_pos] >> bit_pos);
    }
    else
    {
        // combine the high bits of the current word with the low bits
        // of the next word
        bits |= (data_[word_pos] >> bit_pos);
        bits |= (data_[word_pos + 1] << (64 - bit_pos));
    }

    // mask off only the bits we need
    auto mask = len == 64 ? static_cast<uint64_t>(-1) : (1ull << len) - 1;
    bits &= mask;

    return bits;
}

util::array_view<const uint64_t> bit_vector_view::data() const
{
    return data_;
}

uint64_t bit_vector_view::size() const
{
    return num_bits_;
}
}
}
