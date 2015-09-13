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

#include <array>
#include <cassert>
#include <cstdint>
#include <random>

namespace meta
{
namespace util
{

namespace detail
{
template <bool...>
struct static_and;

template <bool B, bool... Bs>
struct static_and<B, Bs...>
{
    const static constexpr bool value = B && static_and<Bs...>::value;
};

template <>
struct static_and<>
{
    const static constexpr bool value = true;
};

template <std::size_t...>
struct static_add;

template <std::size_t Size, std::size_t... Sizes>
struct static_add<Size, Sizes...>
{
    const static constexpr std::size_t value = Size
                                               + static_add<Sizes...>::value;
};

template <>
struct static_add<>
{
    const static constexpr std::size_t value = 0;
};
}

/**
 * Implementation of MurmurHash3. Depending on the template parameter, it
 * will return a 32-bit or 64-bit hash value.
 */
template <std::size_t = sizeof(std::size_t)>
class murmur_hash;

namespace detail
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
class murmur_hash<4>
{
  private:
    // this *has* to be uint32_t for OS X clang to correctly resolve
    // between the two versions of rotl/fmix in namespace detail above.
    uint32_t out_;
    std::array<uint8_t, 4> buf_;
    std::size_t buflen_;
    std::size_t total_length_;

    const static constexpr uint32_t c1 = 0xcc9e2d51;
    const static constexpr uint32_t c2 = 0x1b873593;

    void handle_block_4(uint32_t block)
    {
        block *= c1;
        block = detail::rotl(block, 15);
        block *= c2;

        out_ ^= block;
        out_ = detail::rotl(out_, 13);
        out_ = out_ * 5 + 0xe6546b64;
    }

  public:
    using result_type = std::size_t;

    murmur_hash(std::size_t seed) : out_(seed), buflen_{0}, total_length_{0}
    {
    }

    void operator()(const void* in, std::size_t len)
    {
        auto data = reinterpret_cast<const uint8_t*>(in);
        total_length_ += len;

        // handle 4-byte blocks at a time, starting from the data we had
        // "left over" from the last call to operator()
        auto end = data + len;
        while (buflen_ > 0 && buflen_ < 4 && data < end)
            buf_[buflen_++] = *(data++);

        if (buflen_ / 4 > 0)
        {
            handle_block_4(reinterpret_cast<const uint32_t*>(buf_.data())[0]);
            buflen_ = 0;
        }

        // now handle the remaining 4-byte blocks in this data
        const auto nblocks = (end - data) / 4;
        auto blocks = reinterpret_cast<const uint32_t*>(data + nblocks * 4);
        for (int i = -nblocks; i; ++i)
            handle_block_4(blocks[i]);

        // copy over the remaining 3 bytes or less for finalizing or use on
        // the next call to operator()
        const uint8_t* tail = data + nblocks * 4;
        if (end - tail)
        {
            buflen_ = end - tail;
            assert(buflen_ < 4);
            std::copy(tail, end, buf_.begin());
        }
    }

    explicit operator std::size_t()
    {
        uint32_t k1 = 0;
        switch (buflen_ & 3)
        {
            case 3:
                k1 ^= buf_[2] << 16;
            case 2:
                k1 ^= buf_[1] << 8;
            case 1:
                k1 ^= buf_[0];
                k1 *= c1;
                k1 = detail::rotl(k1, 15);
                k1 *= c2;
                out_ ^= k1;
        }

        out_ ^= total_length_;

        return detail::fmix(out_);
    }
};

/**
 * MurmurHash3 for 64-bit outputs. Based on MurmurHash3_x64_128.
 */
template <>
class murmur_hash<8>
{
  private:
    uint64_t h1_;
    uint64_t h2_;
    std::array<uint8_t, 16> buf_;
    std::size_t buflen_;
    std::size_t total_length_;

    const static constexpr uint64_t c1 = 0x87c37b91114253d5LLU;
    const static constexpr uint64_t c2 = 0x4cf5ad432745937fLLU;

    inline void handle_block_16(const uint8_t* start)
    {
        auto blocks = reinterpret_cast<const uint64_t*>(start);
        auto k1 = blocks[0];
        auto k2 = blocks[1];

        k1 *= c1;
        k1 = detail::rotl(k1, 31);
        k1 *= c2;
        h1_ ^= k1;

        h1_ = detail::rotl(h1_, 27);
        h1_ += h2_;
        h1_ = h1_ * 5 + 0x52dce729;

        k2 *= c2;
        k2 = detail::rotl(k2, 33);
        k2 *= c1;
        h2_ ^= k2;

        h2_ = detail::rotl(h2_, 31);
        h2_ += h1_;
        h2_ = h2_ * 5 + 0x38495ab5;
    }

  public:
    using result_type = std::size_t;

    murmur_hash(uint64_t seed)
        : h1_{seed}, h2_{seed}, buflen_{0}, total_length_{0}
    {
    }

    void operator()(const void* in, std::size_t len)
    {
        auto data = reinterpret_cast<const uint8_t*>(in);
        total_length_ += len;

        // handle 16-byte blocks at a time, starting from the data we had
        // "left over" from the last call to operator()
        auto end = data + len;
        while (buflen_ > 0 && buflen_ < 16 && data < end)
            buf_[buflen_++] = *(data++);

        if (buflen_ / 16 > 0)
        {
            handle_block_16(buf_.data());
            buflen_ = 0;
        }

        // now handle the remaining 16-byte blocks in this data
        const auto nblocks = (end - data) / 16;
        for (int i = 0; i < nblocks; ++i)
        {
            handle_block_16(data);
            data += 16;
        }

        // copy over the remaining 15 bytes or less for finalizing or use
        // on the next call to operator()
        if (end - data)
        {
            buflen_ = end - data;
            assert(buflen_ < 16);
            std::copy(data, end, buf_.begin());
        }
    }

    explicit operator std::size_t()
    {
        uint64_t k1 = 0;
        uint64_t k2 = 0;

        switch (buflen_)
        {
            case 15:
                k2 ^= static_cast<uint64_t>(buf_[14]) << 48;
            case 14:
                k2 ^= static_cast<uint64_t>(buf_[13]) << 40;
            case 13:
                k2 ^= static_cast<uint64_t>(buf_[12]) << 32;
            case 12:
                k2 ^= static_cast<uint64_t>(buf_[11]) << 24;
            case 11:
                k2 ^= static_cast<uint64_t>(buf_[10]) << 16;
            case 10:
                k2 ^= static_cast<uint64_t>(buf_[9]) << 8;
            case 9:
                k2 ^= static_cast<uint64_t>(buf_[8]);
                k2 *= c2;
                k2 = detail::rotl(k2, 33);
                k2 *= c1;
                h2_ ^= k2;

            case 8:
                k1 ^= static_cast<uint64_t>(buf_[7]) << 56;
            case 7:
                k1 ^= static_cast<uint64_t>(buf_[6]) << 48;
            case 6:
                k1 ^= static_cast<uint64_t>(buf_[5]) << 40;
            case 5:
                k1 ^= static_cast<uint64_t>(buf_[4]) << 32;
            case 4:
                k1 ^= static_cast<uint64_t>(buf_[3]) << 24;
            case 3:
                k1 ^= static_cast<uint64_t>(buf_[2]) << 16;
            case 2:
                k1 ^= static_cast<uint64_t>(buf_[1]) << 8;
            case 1:
                k1 ^= static_cast<uint64_t>(buf_[0]);
                k1 *= c1;
                k1 = detail::rotl(k1, 31);
                k1 *= c2;
                h1_ ^= k1;
        }

        h1_ ^= total_length_;
        h2_ ^= total_length_;

        h1_ += h2_;
        h2_ += h1_;

        h1_ = detail::fmix(h1_);
        h2_ = detail::fmix(h2_);

        h1_ += h2_;
        // h2 += h1, unneeded since we only want 64-bits.

        return h1_;
    }
};

template <class T>
struct is_contiguously_hashable
{
    const static constexpr bool value = std::is_integral<T>::value
                                        || std::is_enum<T>::value
                                        || std::is_pointer<T>::value;
};

template <class T>
struct is_contiguously_hashable<const T> : public is_contiguously_hashable<T>
{
};

template <class T>
struct is_contiguously_hashable<const volatile T>
    : public is_contiguously_hashable<T>
{
};

template <class T, std::size_t N>
struct is_contiguously_hashable<T[N]> : public is_contiguously_hashable<T>
{
};

template <class T, class U>
struct is_contiguously_hashable<std::pair<T, U>>
{
    const static constexpr bool value = is_contiguously_hashable<T>::value
                                        && is_contiguously_hashable<U>::value
                                        && sizeof(T) + sizeof(U)
                                               == sizeof(std::pair<T, U>);
};

template <class... Ts>
struct is_contiguously_hashable<std::tuple<Ts...>>
{
    const static constexpr bool value
        = detail::static_and<is_contiguously_hashable<Ts>::value...>::value
          && detail::static_add<sizeof(Ts)...>::value
                 == sizeof(std::tuple<Ts...>);
};

template <class T, std::size_t N>
struct is_contiguously_hashable<std::array<T, N>>
{
    const static constexpr bool value = is_contiguously_hashable<T>::value
                                        && sizeof(T) * N
                                               == sizeof(std::array<T, N>);
};

template <class HashAlgorithm, class T>
inline typename std::enable_if<is_contiguously_hashable<T>::value>::type
    hash_append(HashAlgorithm& h, const T& t)
{
    h(std::addressof(t), sizeof(t));
}

template <class HashAlgorithm, class T>
inline typename std::enable_if<std::is_floating_point<T>::value>::type
    hash_append(HashAlgorithm& h, T t)
{
    // -0 and 0 are the same, but have different bit patterns, so normalize
    // to positive zero before hashing
    if (t == 0)
        t = 0;
    h(std::addressof(t), sizeof(t));
}

template <class HashAlgorithm>
inline void hash_append(HashAlgorithm& h, std::nullptr_t)
{
    const void* p = nullptr;
    h(std::addressof(p), sizeof(p));
}

// all of these hash_appends below need to be forward declared so they can
// find one another in their implementations

template <class HashAlgorithm, class T, std::size_t N>
typename std::enable_if<!is_contiguously_hashable<T>::value>::type
    hash_append(HashAlgorithm& h, T(&a)[N]);

template <class HashAlgorithm, class T, class U>
typename std::enable_if<!is_contiguously_hashable<std::pair<T, U>>::value>::type
    hash_append(HashAlgorithm& h, const std::pair<T, U>& p);

template <class HashAlgorithm, class... Ts>
typename std::enable_if<!is_contiguously_hashable<std::tuple<Ts...>>::value>::
    type
    hash_append(HashAlgorithm& h, const std::tuple<Ts...>& t);

template <class HashAlgorithm, class T, std::size_t N>
typename std::enable_if<!is_contiguously_hashable<std::array<T, N>>::value>::
    type
    hash_append(HashAlgorithm& h, const std::array<T, N>& a);

template <class HashAlgorithm, class Char, class Traits, class Alloc>
typename std::enable_if<is_contiguously_hashable<Char>::value>::type
    hash_append(HashAlgorithm& h,
                const std::basic_string<Char, Traits, Alloc>& s);

template <class HashAlgorithm, class Char, class Traits, class Alloc>
typename std::enable_if<!is_contiguously_hashable<Char>::value>::type
    hash_append(HashAlgorithm& h,
                const std::basic_string<Char, Traits, Alloc>& s);

template <class HashAlgorithm, class T1, class T2, class... Ts>
void hash_append(HashAlgorithm& h, const T1& first, const T2& second,
                 const Ts&... ts);

// begin implementations for hash_append

template <class HashAlgorithm, class T, std::size_t N>
typename std::enable_if<!is_contiguously_hashable<T>::value>::type
    hash_append(HashAlgorithm& h, T(&a)[N])
{
    for (const auto& t : a)
        hash_append(h, t);
}

template <class HashAlgorithm, class T, class U>
typename std::enable_if<!is_contiguously_hashable<std::pair<T, U>>::value>::type
    hash_append(HashAlgorithm& h, const std::pair<T, U>& p)
{
    hash_append(h, p.first, p.second);
}

namespace detail
{
// @see
// http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer
template <std::size_t...>
struct sequence;

template <std::size_t N, std::size_t... S>
struct generate : generate<N - 1, N - 1, S...>
{
    // nothing
};

template <std::size_t... S>
struct generate<0, S...>
{
    using type = sequence<S...>;
};

template <class HashAlgorithm, class... Ts, std::size_t... S>
void hash_tuple(HashAlgorithm& h, const std::tuple<Ts...>& t, sequence<S...>)
{
    hash_append(h, std::get<S>(t)...);
}
}

template <class HashAlgorithm, class... Ts>
typename std::enable_if<!is_contiguously_hashable<std::tuple<Ts...>>::value>::
    type
    hash_append(HashAlgorithm& h, const std::tuple<Ts...>& t)
{
    detail::hash_tuple(h, t, typename detail::generate<sizeof...(Ts)>::type{});
}

template <class HashAlgorithm, class T, std::size_t N>
typename std::enable_if<!is_contiguously_hashable<std::array<T, N>>::value>::
    type
    hash_append(HashAlgorithm& h, const std::array<T, N>& a)
{
    for (const auto& t : a)
        hash_append(h, a);
}

template <class HashAlgorithm, class Char, class Traits, class Alloc>
typename std::enable_if<is_contiguously_hashable<Char>::value>::type
    hash_append(HashAlgorithm& h,
                const std::basic_string<Char, Traits, Alloc>& s)
{
    h(s.data(), s.size() * sizeof(Char));
    hash_append(h, s.size());
}

template <class HashAlgorithm, class Char, class Traits, class Alloc>
typename std::enable_if<!is_contiguously_hashable<Char>::value>::type
    hash_append(HashAlgorithm& h,
                const std::basic_string<Char, Traits, Alloc>& s)
{
    for (const auto& c : s)
        hash_append(h, c);
    hash_append(h, s.size());
}

template <class HashAlgorithm, class T1, class T2, class... Ts>
void hash_append(HashAlgorithm& h, const T1& first, const T2& second,
                 const Ts&... ts)
{
    hash_append(h, first);
    hash_append(h, second, ts...);
}

namespace detail
{
inline uint64_t get_process_seed()
{
    static uint64_t seed = std::random_device{}();
    return seed;
}
}

/**
 * A generic, randomly seeded hash function.
 * @see
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html#seeding
 */
template <class HashAlgorithm = util::murmur_hash<sizeof(std::size_t)>>
struct hash
{
    using result_type = typename HashAlgorithm::result_type;

    template <class T>
    result_type operator()(const T& t) const
    {
        auto seed = detail::get_process_seed();
        HashAlgorithm h(seed);
        using util::hash_append;
        hash_append(h, t);
        return static_cast<result_type>(h);
    }
};
}
}
#endif
