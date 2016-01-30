/**
 * @file farm_hash.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_HASHING_FARM_HASH_H_
#define META_HASHING_FARM_HASH_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <utility>

namespace meta
{
namespace hashing
{

namespace farm
{

template <class T>
inline T fetch(const uint8_t* data)
{
    static_assert(std::is_integral<T>::value, "fetch only defined for ints");
    T result;
    std::memcpy(&result, data, sizeof(T));
    return result;
}

inline uint32_t rotate(uint32_t val, int shift)
{
    // Avoid shifting by 32: doing so yields an undefined result.
    return shift == 0 ? val : ((val >> shift) | (val << (32 - shift)));
}

inline uint64_t rotate(uint64_t val, int shift)
{
    // Avoid shifting by 64: doing so yields an undefined result.
    return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

// Some primes between 2^63 and 2^64 for various uses.
const static constexpr uint64_t k0 = 0xc3a5c85c97cb3127ULL;
const static constexpr uint64_t k1 = 0xb492b66fbe98f273ULL;
const static constexpr uint64_t k2 = 0x9ae16a3b2f90404fULL;

// Magic numbers for 32-bit hashing. Copied from Murmur3.
const static constexpr uint32_t c1 = 0xcc9e2d51;
const static constexpr uint32_t c2 = 0x1b873593;

// A 32-bit to 32-bit integer hash copied from Murmur3.
inline uint32_t fmix(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

inline uint64_t shift_mix(uint64_t val)
{
    return val ^ (val >> 47);
}

struct u128
{
    uint64_t low;
    uint64_t high;

    u128() = default;
    u128(uint64_t l, uint64_t h) : low{l}, high{h}
    {
        // nothing
    }
};

inline uint64_t hash_len_16(uint64_t u, uint64_t v,
                            uint64_t mul = 0x9ddfea08eb382d69ULL)
{
    // Murmur-inspired hashing.
    auto a = (u ^ v) * mul;
    a ^= (a >> 47);
    auto b = (v ^ a) * mul;
    b ^= (b >> 47);
    b *= mul;
    return b;
}

inline uint64_t hash_len_0_to_16(const uint8_t* data, std::size_t len)
{
    if (len >= 8)
    {
        auto mul = k2 + len * 2;
        auto a = fetch<uint64_t>(data) + k2;
        auto b = fetch<uint64_t>(data + len - 8);
        auto c = rotate(b, 37) * mul + a;
        auto d = (rotate(a, 25) + b) * mul;
        return hash_len_16(c, d, mul);
    }
    if (len >= 4)
    {
        auto mul = k2 + len * 2;
        uint64_t a = fetch<uint32_t>(data);
        auto b = len + (a << 3);
        auto c = fetch<uint32_t>(data + len - 4);
        return hash_len_16(b, c, mul);
    }
    if (len > 0)
    {
        auto a = data[0];
        auto b = data[len >> 1];
        auto c = data[len - 1];
        uint32_t y = static_cast<uint32_t>(a) + (static_cast<uint32_t>(b) << 8);
        auto z = static_cast<uint32_t>(len + (static_cast<uint32_t>(c) << 2));
        return shift_mix(y * k2 ^ z * k0) * k2;
    }
    return k2;
}

inline uint64_t hash_len_17_to_32(const uint8_t* data, std::size_t len)
{
    auto mul = k2 + len * 2;
    auto a = fetch<uint64_t>(data) * k1;
    auto b = fetch<uint64_t>(data + 8);
    auto c = fetch<uint64_t>(data + len - 8) * mul;
    auto d = fetch<uint64_t>(data + len - 16) * k2;
    return hash_len_16(rotate(a + b, 43) + rotate(c, 30) + d,
                       a + rotate(b + k2, 18) + c, mul);
}

inline u128 weak_hash_len_32_with_seeds(const uint64_t* buffer, uint64_t a,
                                        uint64_t b)
{
    a += buffer[0];
    b = rotate(b + a + buffer[3], 21);
    auto c = a;
    a += buffer[1];
    a += buffer[2];
    b += rotate(a, 44);
    return {a + buffer[3], b + c};
}

inline uint64_t hash_len_33_to_64(const uint8_t* data, std::size_t len)
{
    auto mul = k2 + len * 2;
    auto a = fetch<uint64_t>(data) * k2;
    auto b = fetch<uint64_t>(data + 8);
    auto c = fetch<uint64_t>(data + len - 8) * mul;
    auto d = fetch<uint64_t>(data + len - 16) * k2;
    auto y = rotate(a + b, 43) + rotate(c, 30) + d;
    auto z = hash_len_16(y, a + rotate(b + k2, 18) + c, mul);
    auto e = fetch<uint64_t>(data + 16) * mul;
    auto f = fetch<uint64_t>(data + 24);
    auto g = (y + fetch<uint64_t>(data + len - 32)) * mul;
    auto h = (z + fetch<uint64_t>(data + len - 24)) * mul;
    return hash_len_16(rotate(e + f, 43) + rotate(g, 30) + h,
                       e + rotate(f + a, 18) + g, mul);
}
}

/**
 * Implementation of FarmHash64. Specifically, the farmhashna variant.
 *
 * Based on code Copyright (c) 2014 Google, Inc. which was released under
 * the MIT license (see LICENSE.mit).
 *
 * @see https://github.com/google/farmhash/
 *
 * Based largely on work done by Geoff Romer and released under an Apache
 * license.
 * @see https://github.com/google/hashing-demo/blob/master/LICENSE
 * @see https://github.com/google/hashing-demo/blob/master/n3980-farmhash.h
 */
class farm_hash
{
  private:
    // hashing state consists of 56 bytes for input data longer than 64
    // bytes
    uint64_t x_;
    uint64_t y_;
    uint64_t z_;
    farm::u128 v_;
    farm::u128 w_;

    // we maintain a buffer of 64 bytes since farm operates in chunks of
    // that size or less
    std::array<uint64_t, 8> buffer_;

    // buf_pos_ is where we'll start copying input data into the buffer
    uint8_t* buf_pos_;

    // indicates whether we've done the initialization mix or not. this
    // allows us to skip that block of code if the key is <= 64 bytes long
    bool mixed_;

    inline void handle_block_64()
    {
        x_ = farm::rotate(x_ + y_ + v_.low + buffer_[1], 37) * farm::k1;
        y_ = farm::rotate(y_ + v_.high + buffer_[6], 42) * farm::k1;
        x_ ^= w_.high;
        y_ += v_.low + buffer_[5];
        z_ = farm::rotate(z_ + w_.low, 33) * farm::k1;
        v_ = farm::weak_hash_len_32_with_seeds(buffer_.data(),
                                               v_.high * farm::k1, x_ + w_.low);
        w_ = farm::weak_hash_len_32_with_seeds(buffer_.data() + 4, z_ + w_.high,
                                               y_ + buffer_[2]);
        std::swap(z_, x_);
    }

    inline uint64_t finalize(std::size_t len)
    {
        // the last bit of FarmHash operates on the last 64 bytes of input,
        // in order. We have that, but it's not in the correct order within
        // buffer_ since it's a circular buffer. Rotate fixes that and puts
        // all the last 64 bytes in "chronological" order.
        uint8_t* buf_start = reinterpret_cast<uint8_t*>(buffer_.data());
        std::rotate(buf_start, buf_start + len, buf_start + 64);

        auto mul = farm::k1 + ((z_ & 0xff) << 1);
        w_.low += ((len - 1) & 63);
        v_.low += w_.low;
        w_.low += v_.low;
        x_ = farm::rotate(x_ + y_ + v_.low + buffer_[1], 37) * mul;
        y_ = farm::rotate(y_ + v_.high + buffer_[6], 42) * mul;
        x_ ^= w_.high * 9;
        y_ += v_.low * 9 + buffer_[5];
        z_ = farm::rotate(z_ + w_.low, 33) * mul;
        v_ = farm::weak_hash_len_32_with_seeds(buffer_.data(), v_.high * mul,
                                               x_ + w_.low);
        w_ = farm::weak_hash_len_32_with_seeds(buffer_.data() + 4, z_ + w_.high,
                                               y_ + buffer_[2]);
        std::swap(z_, x_);
        return farm::hash_len_16(farm::hash_len_16(v_.low, w_.low, mul)
                                     + farm::shift_mix(y_) * farm::k0 + z_,
                                 farm::hash_len_16(v_.high, w_.high, mul) + x_,
                                 mul);
    }

  public:
    using result_type = std::size_t;

    farm_hash()
        : buf_pos_{reinterpret_cast<uint8_t*>(buffer_.data())}, mixed_{false}
    {
        // nothing
    }

    inline void operator()(const void* in, std::size_t len)
    {
        auto data = reinterpret_cast<const uint8_t*>(in);
        // determine how much space we've got left in the buffer
        auto buf_start = reinterpret_cast<uint8_t*>(buffer_.data());
        auto buf_remaining
            = static_cast<std::size_t>(buf_start + 64 - buf_pos_);

        // if we can fit the entire key into our buffer, so do so and bail
        if (len <= buf_remaining)
        {
            std::copy(data, data + len, buf_pos_);
            buf_pos_ += len;
            return;
        }

        std::size_t bytes = 0;
        std::copy(data, data + buf_remaining, buf_pos_);
        bytes += buf_remaining;

        // we have more than 64 total bytes to be hashed, so check if we
        // need to do our initialization mix
        if (!mixed_)
        {
            const constexpr uint64_t seed = 81;
            x_ = seed;
            y_ = seed * farm::k1 + 113;
            z_ = farm::shift_mix(y_ * farm::k2 + 113) * farm::k2;
            v_.low = v_.high = 0;
            w_.low = w_.high = 0;
            x_ = x_ * farm::k2 + buffer_[0];
            mixed_ = true;
        }

        // start hashing out of buffer_ blocks of size 64, refilling as
        // necessary
        handle_block_64();
        while (len - bytes > 64)
        {
            std::copy(data + bytes, data + bytes + 64, buf_start);
            bytes += 64;
            handle_block_64();
        }
        std::copy(data + bytes, data + len, buf_start);
        buf_pos_ = buf_start + (len - bytes);
    }

    inline explicit operator result_type()
    {
        auto buf_start = reinterpret_cast<uint8_t*>(buffer_.data());
        auto len = static_cast<std::size_t>(buf_pos_ - buf_start);

        if (!mixed_)
        {
            if (len <= 32)
            {
                if (len <= 16)
                {
                    return farm::hash_len_0_to_16(buf_start, len);
                }
                else
                {
                    return farm::hash_len_17_to_32(buf_start, len);
                }
            }
            else
            {
                return farm::hash_len_33_to_64(buf_start, len);
            }
        }
        else
        {
            return finalize(len);
        }
    }
};

/**
 * A seeded version of farm_hash.
 */
class farm_hash_seeded : public farm_hash
{
  private:
    // the initial seeds for the algorithm, used during finalization
    farm::u128 seed_;

  public:
    using farm_hash::result_type;

    farm_hash_seeded(uint64_t seed) : farm_hash_seeded{farm::k2, seed}
    {
        // nothing
    }

    farm_hash_seeded(uint64_t seed0, uint64_t seed1) : seed_{seed0, seed1}
    {
        // nothing
    }

    inline explicit operator result_type()
    {
        uint64_t result
            = static_cast<std::size_t>(static_cast<farm_hash&>(*this));
        return farm::hash_len_16(result - seed_.low, seed_.high);
    }
};
}
}
#endif
