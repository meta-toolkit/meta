/**
 * @file bit_vector.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SUCCINCT_BIT_VECTOR_H_
#define META_SUCCINCT_BIT_VECTOR_H_

#include <cstdint>
#include <ostream>
#include <stdexcept>

#include "meta/config.h"
#include "meta/util/array_view.h"
#include "meta/util/likely.h"

namespace meta
{
namespace succinct
{

/**
 * Represents a collection of bits packed into a word (uint64_t) to be
 * written into a bit_vector.
 */
class packed_bits
{
  public:
    packed_bits(uint64_t word, uint8_t len) : word_{word}, len_{len}
    {
#if DEBUG
        if (META_UNLIKELY(len > 64))
            throw std::invalid_argument{"bit length longer than word"};
#endif

        auto mask = len_ == 64 ? static_cast<uint64_t>(-1) : (1ull << len_) - 1;
        word_ &= mask;
    }

    inline uint64_t word() const
    {
        return word_;
    }

    inline uint8_t size() const
    {
        return len_;
    }

  private:
    uint64_t word_;
    uint8_t len_;
};

/**
 * Writes a word-aligned bit vector to a file to be mapped in later.
 */
template <class WordWriter>
class bit_vector_builder
{
  public:
    bit_vector_builder(WordWriter&& writer)
        : cur_word_{0},
          bit_in_word_{0},
          total_bits_{0},
          writer_{std::forward<WordWriter>(writer)}
    {
        // nothing
    }

    void write_bits(packed_bits bits)
    {
        if (64 - bit_in_word_ >= bits.size())
        {
            // we can fit these bits in the current word
            cur_word_ |= (bits.word() << bit_in_word_);
            bit_in_word_ += bits.size();

            if (bit_in_word_ == 64)
                flush_word();
        }
        else
        {
            // we don't have enough room, so we need to append what we can,
            // flush the word, and then set the current word to the
            // remaining bits we didn't write
            auto num_written = static_cast<uint8_t>(64 - bit_in_word_);
            cur_word_ |= (bits.word() << bit_in_word_);
            flush_word();
            cur_word_ = (bits.word() >> num_written);
            bit_in_word_ = static_cast<uint8_t>(bits.size() - num_written);
        }
        total_bits_ += bits.size();
    }

    uint64_t total_bits() const
    {
        return total_bits_;
    }

    ~bit_vector_builder()
    {
        if (bit_in_word_)
            flush_word();
    }

  private:
    void flush_word()
    {
        writer_(cur_word_);
        bit_in_word_ = 0;
        cur_word_ = 0;
    }

    uint64_t cur_word_;
    uint8_t bit_in_word_;
    uint64_t total_bits_;
    WordWriter writer_;
};

namespace detail
{
template <class T>
struct is_ostream_reference
{
    const static constexpr bool value
        = std::is_convertible<T, std::ostream&>::value;
};

struct ostream_word_writer
{
    ostream_word_writer(std::ostream& out) : out_(out)
    {
        // nothing
    }

    void operator()(uint64_t word)
    {
        out_.write(reinterpret_cast<const char*>(&word), sizeof(uint64_t));
    }

    std::ostream& out_;
};
}

template <class WordWriter,
          class = typename std::
              enable_if<!detail::is_ostream_reference<WordWriter>::value>::type>
bit_vector_builder<WordWriter> make_bit_vector_builder(WordWriter&& writer)
{
    return bit_vector_builder<WordWriter>{std::forward<WordWriter>(writer)};
}

inline bit_vector_builder<detail::ostream_word_writer>
make_bit_vector_builder(std::ostream& os)
{
    detail::ostream_word_writer writer{os};
    return {std::move(writer)};
}

/**
 * Conceptually views a contiguous chunk of words as a (const) bit vector.
 */
class bit_vector_view
{
  public:
    bit_vector_view(util::array_view<const uint64_t> data, uint64_t num_bits);

    bool operator[](uint64_t bit_idx) const;

    uint64_t extract(uint64_t bit_idx, uint8_t len) const;

    util::array_view<const uint64_t> data() const;

    uint64_t size() const;

  private:
    util::array_view<const uint64_t> data_;
    const uint64_t num_bits_;
};
}
}
#endif
