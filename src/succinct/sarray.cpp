/**
 * @file sarray.cpp
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/succinct/sarray.h"
#include "meta/util/likely.h"

namespace meta
{
namespace succinct
{

using namespace sarray_detail;

sarray_builder::sarray_builder(const std::string& prefix, uint64_t num_ones,
                               uint64_t num_bits)
    : low_stream_{low_file(prefix), std::ios::binary},
      high_stream_{high_file(prefix), std::ios::binary},
      nb_stream_{num_bits_file(prefix), std::ios::binary},
      num_ones_{num_ones}
{
    auto ratio = num_bits / num_ones;
    low_bits_ = ratio ? static_cast<uint8_t>(broadword::msb(ratio)) : 0;
    low_mask_ = (1ull << low_bits_) - 1;

    low_builder_
        = make_unique<builder_type>(make_bit_vector_builder(low_stream_));
    high_builder_
        = make_unique<builder_type>(make_bit_vector_builder(high_stream_));
}

void sarray_builder::operator()(uint64_t one_pos)
{
    if (low_bits_)
    {
        low_builder_->write_bits({one_pos & low_mask_, low_bits_});
    }

    // determine the next bit position to set in the upper bit
    // array
    uint64_t upper_bit_pos = (one_pos >> low_bits_) + num_calls_;
    uint64_t word_idx = upper_bit_pos / 64;
    uint64_t word_pos = upper_bit_pos % 64;

    // write full words until we're at the correct word index
    for (; high_word_idx_ < word_idx; ++high_word_idx_)
    {
        high_builder_->write_bits({curr_high_word_, 64});
        curr_high_word_ = 0;
    }

    // set the correct bit in the current word
    curr_high_word_ |= 1ull << word_pos;
    high_word_pos_ = word_pos + 1;

    ++num_calls_;
    if (META_UNLIKELY(num_calls_ > num_ones_))
        throw std::out_of_range{
            "more positions given than bits in sarray building"};
}

sarray_builder::~sarray_builder()
{
    high_builder_->write_bits(
        {curr_high_word_, static_cast<uint8_t>(high_word_pos_)});

    high_builder_ = nullptr;
    low_builder_ = nullptr;

    if (!low_bits_)
        io::write_binary(low_stream_, uint64_t{0});

    io::packed::write(nb_stream_, 64 * high_word_idx_ + high_word_pos_);
    io::packed::write(nb_stream_, low_bits_);
}

sarray::sarray(const std::string& prefix)
    : high_bits_{high_file(prefix)}, low_bits_{low_file(prefix)}
{
    std::ifstream num_bits{num_bits_file(prefix), std::ios::binary};
    io::packed::read(num_bits, high_bit_count_);
    io::packed::read(num_bits, num_low_bits_);
}

bit_vector_view sarray::high_bits() const
{
    return {{high_bits_.begin(), high_bits_.end()}, high_bit_count_};
}

bit_vector_view sarray::low_bits() const
{
    return {{low_bits_.begin(), low_bits_.end()}, 64 * low_bits_.size()};
}

uint8_t sarray::num_low_bits() const
{
    return num_low_bits_;
}

sarray_rank::sarray_rank(const std::string& prefix, const sarray& sarr)
    : sarray_{&sarr}, high_bit_zeroes_{prefix + "/rank", sarr.high_bits()}
{
    // nothing
}

/// @see https://github.com/ot/succinct/blob/master/elias_fano.hpp
uint64_t sarray_rank::rank(uint64_t i) const
{
    auto num_low_bits = sarray_->num_low_bits();
    auto high_query = i >> num_low_bits;

    // make sure we don't query off the end of the zero index
    if (high_query >= high_bit_zeroes_.num_positions())
        return size();

    auto high_pos = high_bit_zeroes_.select(high_query);

    uint64_t rank = high_pos - high_query;

    auto high_bvv = sarray_->high_bits();
    auto low_bvv = sarray_->low_bits();
    uint64_t low_val = i & ((1ull << num_low_bits) - 1);
    while (high_pos > 0 && high_bvv[high_pos - 1]
           && low_bvv.extract((rank - 1) * num_low_bits, num_low_bits)
                  >= low_val)
    {
        --rank;
        --high_pos;
    }

    return rank;
}

uint64_t sarray_rank::size() const
{
    return sarray_->high_bits().size() - high_bit_zeroes_.num_positions() + 1;
}

sarray_select::sarray_select(const std::string& prefix, const sarray& sarr)
    : sarray_{&sarr}, high_bit_ones_{prefix + "/select", sarr.high_bits()}
{
    // nothing
}

uint64_t sarray_select::select(uint64_t i) const
{
    uint8_t num_low_bits = sarray_->num_low_bits();
    return (high_bit_ones_.select(i) - i) << num_low_bits
           | sarray_->low_bits().extract(i * num_low_bits, num_low_bits);
}

uint64_t sarray_select::size() const
{
    return high_bit_ones_.num_positions();
}
}
}
