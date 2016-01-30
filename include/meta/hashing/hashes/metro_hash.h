/**
 * @file metro_hash.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_HASHING_METRO_HASH_H_
#define META_HASHING_METRO_HASH_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>

namespace meta
{
namespace hashing
{

namespace metro
{
inline uint64_t rotr(uint64_t x, uint8_t r)
{
    return (x >> r) | (x << (64 - r));
}

template <class T>
inline T read(const uint8_t*& data)
{
    T ret = *reinterpret_cast<const T*>(data);
    data += sizeof(T);
    return ret;
}
}

/**
 * Port of the incremental MetroHash64, which was released under the
 * MIT License (see LICENSE.mit in the main source directory).
 *
 * Original code is Copyright (c) 2015 J. Andrew Rodgers
 * @see https://github.com/jandrewrogers/MetroHash
 */
class metro_hash
{
  public:
    using result_type = uint64_t;

    metro_hash(uint64_t seed) : buflen_{0}, seed_{(seed + k2) * k0}, big_{false}
    {
        std::fill(std::begin(state_), std::end(state_), seed_);
    }

    void operator()(const void* key, std::size_t len)
    {
        auto data = reinterpret_cast<const uint8_t*>(key);
        auto end = data + len;

        // if the input buffer is partially filled
        if (buflen_ > 0 && buflen_ < 32)
        {
            uint64_t copylen = 32 - buflen_ > len ? len : 32 - buflen_;
            std::copy(data, data + copylen, buffer_.data() + buflen_);

            data += copylen;
            buflen_ += copylen;

            // if the buffer still isn't filled, bail out for now
            if (buflen_ < 32)
                return;

            // otherwise, we can process the full 32-byte input buffer
            const uint8_t* buf = buffer_.data();
            handle_block_32(buf);
            buflen_ = 0;
        }

        // now, process the remaining 32-byte blocks
        const auto nblocks = (end - data) / 32;
        for (int i = 0; i < nblocks; ++i)
            handle_block_32(data);

        // copy over the remaining 31 bytes or less for finalizing or use
        // on the next call to operator()
        if (end - data)
        {
            buflen_ = static_cast<std::size_t>(end - data);
            assert(buflen_ < 32);
            std::copy(data, end, buffer_.begin());
        }
    }

    explicit operator std::size_t()
    {
        // loop finalizer
        if (big_)
        {
            state_[2]
                ^= metro::rotr((state_[0] + state_[3]) * k0 + state_[1], 37)
                   * k1;
            state_[3]
                ^= metro::rotr((state_[1] + state_[2]) * k1 + state_[0], 37)
                   * k0;
            state_[0]
                ^= metro::rotr((state_[0] + state_[2]) * k0 + state_[3], 37)
                   * k1;
            state_[1]
                ^= metro::rotr((state_[1] + state_[3]) * k1 + state_[2], 37)
                   * k0;

            state_[0] = seed_ + (state_[0] ^ state_[1]);
        }

        // handle any leftover bytes
        const uint8_t* data = buffer_.data();
        auto end = data + buflen_;

        if (end - data >= 16)
        {
            state_[1] = state_[0] + metro::read<uint64_t>(data) * k2;
            state_[1] = metro::rotr(state_[1], 29) * k3;

            state_[2] = state_[0] + metro::read<uint64_t>(data) * k2;
            state_[2] = metro::rotr(state_[2], 29) * k3;

            state_[1] ^= metro::rotr(state_[1] * k0, 21) + state_[2];
            state_[2] ^= metro::rotr(state_[2] * k3, 21) + state_[1];
            state_[0] += state_[2];
        }

        if (end - data >= 8)
        {
            state_[0] += metro::read<uint64_t>(data) * k3;
            state_[0] ^= metro::rotr(state_[0], 55) * k1;
        }

        if (end - data >= 4)
        {
            state_[0] += metro::read<uint32_t>(data) * k3;
            state_[0] ^= metro::rotr(state_[0], 26) * k1;
        }

        if (end - data >= 2)
        {
            state_[0] += metro::read<uint16_t>(data) * k3;
            state_[0] ^= metro::rotr(state_[0], 48) * k1;
        }

        if (end - data >= 1)
        {
            state_[0] += *(data++) * k3;
            state_[0] ^= metro::rotr(state_[0], 37) * k1;
        }

        state_[0] ^= metro::rotr(state_[0], 28);
        state_[0] *= k0;
        state_[0] ^= metro::rotr(state_[0], 29);

        return state_[0];
    }

  private:
    inline void handle_block_32(const uint8_t*& data)
    {
        big_ = true;
        state_[0] += metro::read<uint64_t>(data) * k0;
        state_[0] = metro::rotr(state_[0], 29) + state_[2];
        state_[1] += metro::read<uint64_t>(data) * k1;
        state_[1] = metro::rotr(state_[1], 29) + state_[3];
        state_[2] += metro::read<uint64_t>(data) * k2;
        state_[2] = metro::rotr(state_[2], 29) + state_[0];
        state_[3] += metro::read<uint64_t>(data) * k3;
        state_[3] = metro::rotr(state_[3], 29) + state_[1];
    }

    const static constexpr uint64_t k0 = 0xD6D018F5;
    const static constexpr uint64_t k1 = 0xA2AA033B;
    const static constexpr uint64_t k2 = 0x62992FC1;
    const static constexpr uint64_t k3 = 0x30BC5B29;

    std::array<uint64_t, 4> state_;
    std::array<uint8_t, 32> buffer_;
    uint64_t buflen_;
    uint64_t seed_;
    bool big_;
};
}
}
#endif
