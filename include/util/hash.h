/**
 * @file hash.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_HASH_H_
#define META_UTIL_HASH_H_

#include <cstdint>

namespace meta
{
namespace util
{

/**
 * Implementation of MurmurHash3. Depending on the template parameter, it
 * will return a 32-bit or 64-bit hash value.
 */
template <std::size_t = sizeof(std::size_t)>
struct murmur_hash;

namespace
{
inline uint32_t rotl(uint32_t x, int8_t r)
{
    return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl(uint64_t x, int8_t r)
{
    return (x << r) | (x >> (64 - r));
}

inline uint32_t fmix(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

inline uint64_t fmix(uint64_t h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdLLU;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53LLU;
    h ^= h >> 33;

    return h;
}
}

/**
 * Murmur3Hash for 32-bit outputs. Based on MurmurHash3_x86_32.
 */
template <>
struct murmur_hash<4>
{
    constexpr murmur_hash() = default;

    std::size_t operator()(const uint8_t* data, int len, uint32_t seed)
    {
        std::size_t out = seed;

        const auto nblocks = len / 4;

        constexpr uint32_t c1 = 0xcc9e2d51;
        constexpr uint32_t c2 = 0x1b873593;

        auto blocks = reinterpret_cast<const uint32_t*>(data + nblocks * 4);

        for (int i = -nblocks; i; ++i)
        {
            auto k1 = blocks[i];

            k1 *= c1;
            k1 = rotl(k1, 15);
            k1 *= c2;

            out ^= k1;
            out = rotl(out, 13);
            out = out * 5 + 0xe6546b64;
        }

        const uint8_t* tail = data + nblocks * 4;

        uint32_t k1 = 0;
        switch (len & 3)
        {
            case 3:
                k1 ^= tail[2] << 16;
            case 2:
                k1 ^= tail[1] << 8;
            case 1:
                k1 ^= tail[0];
                k1 *= c1;
                k1 = rotl(k1, 15);
                k1 *= c2;
                out ^= k1;
        }

        out ^= len;

        return fmix(out);
    }
};

/**
 * MurmurHash3 for 64-bit outputs. Based on MurmurHash3_x64_128.
 */
template <>
struct murmur_hash<8>
{
    constexpr murmur_hash() = default;

    std::size_t operator()(const uint8_t* data, int len, uint64_t seed)
    {
        const auto nblocks = len / 16;

        auto h1 = seed;
        auto h2 = seed;

        const uint64_t c1 = 0x87c37b91114253d5LLU;
        const uint64_t c2 = 0x4cf5ad432745937fLLU;

        auto blocks = reinterpret_cast<const uint64_t*>(data);

        for (int i = 0; i < nblocks; ++i)
        {
            auto k1 = blocks[i * 2];
            auto k2 = blocks[i * 2 + 1];

            k1 *= c1;
            k1 = rotl(k1, 31);
            k1 *= c2;
            h1 ^= k1;

            h1 = rotl(h1, 27);
            h1 += h2;
            h1 = h1 * 5 + 0x52dce729;

            k2 *= c2;
            k2 = rotl(k2, 33);
            k2 *= c1;
            h2 ^= k2;

            h2 = rotl(h2, 31);
            h2 += h1;
            h2 = h2 * 5 + 0x38495ab5;
        }

        auto tail = data + nblocks * 16;

        uint64_t k1 = 0;
        uint64_t k2 = 0;

        switch (len & 15)
        {
            case 15:
                k2 ^= static_cast<uint64_t>(tail[14]) << 48;
            case 14:
                k2 ^= static_cast<uint64_t>(tail[13]) << 40;
            case 13:
                k2 ^= static_cast<uint64_t>(tail[12]) << 32;
            case 12:
                k2 ^= static_cast<uint64_t>(tail[11]) << 24;
            case 11:
                k2 ^= static_cast<uint64_t>(tail[10]) << 16;
            case 10:
                k2 ^= static_cast<uint64_t>(tail[9]) << 8;
            case 9:
                k2 ^= static_cast<uint64_t>(tail[8]);
                k2 *= c2;
                k2 = rotl(k2, 33);
                k2 *= c1;
                h2 ^= k2;

            case 8:
                k1 ^= static_cast<uint64_t>(tail[7]) << 56;
            case 7:
                k1 ^= static_cast<uint64_t>(tail[6]) << 48;
            case 6:
                k1 ^= static_cast<uint64_t>(tail[5]) << 40;
            case 5:
                k1 ^= static_cast<uint64_t>(tail[4]) << 32;
            case 4:
                k1 ^= static_cast<uint64_t>(tail[3]) << 24;
            case 3:
                k1 ^= static_cast<uint64_t>(tail[2]) << 16;
            case 2:
                k1 ^= static_cast<uint64_t>(tail[1]) << 8;
            case 1:
                k1 ^= static_cast<uint64_t>(tail[0]);
                k1 *= c1;
                k1 = rotl(k1, 31);
                k1 *= c2;
                h1 ^= k1;
        }

        h1 ^= len;
        h2 ^= len;

        h1 += h2;
        h2 += h1;

        h1 = fmix(h1);
        h2 = fmix(h2);

        h1 += h2;
        // h2 += h1, unneeded since we only want 64-bits.

        return h1;
    }
};
}
}
#endif
