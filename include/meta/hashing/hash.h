/**
 * @file hash.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_HASHING_HASH_H_
#define META_HASHING_HASH_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <random>

#include "hashes/farm_hash.h"
#include "hashes/metro_hash.h"
#include "hashes/murmur_hash.h"
#include "meta/config.h"

namespace meta
{
namespace hashing
{

template <std::size_t Size = sizeof(std::size_t)>
struct default_hasher;

template <>
struct default_hasher<4>
{
    using type = murmur_hash<4>;
};

template <>
struct default_hasher<8>
{
    using type = farm_hash_seeded;
};

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
    const static constexpr std::size_t value
        = Size + static_add<Sizes...>::value;
};

template <>
struct static_add<>
{
    const static constexpr std::size_t value = 0;
};
}

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
    const static constexpr bool value
        = is_contiguously_hashable<T>::value
          && is_contiguously_hashable<U>::value
          && sizeof(T) + sizeof(U) == sizeof(std::pair<T, U>);
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
    const static constexpr bool value
        = is_contiguously_hashable<T>::value
          && sizeof(T) * N == sizeof(std::array<T, N>);
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
hash_append(HashAlgorithm& h, T (&a)[N]);

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
hash_append(HashAlgorithm& h, const std::basic_string<Char, Traits, Alloc>& s);

template <class HashAlgorithm, class Char, class Traits, class Alloc>
typename std::enable_if<!is_contiguously_hashable<Char>::value>::type
hash_append(HashAlgorithm& h, const std::basic_string<Char, Traits, Alloc>& s);

template <class HashAlgorithm, class T1, class T2, class... Ts>
void hash_append(HashAlgorithm& h, const T1& first, const T2& second,
                 const Ts&... ts);

template <class HashAlgorithm, class T, class Alloc>
typename std::enable_if<is_contiguously_hashable<T>::value>::type
hash_append(HashAlgorithm& h, const std::vector<T, Alloc>& v);

template <class HashAlgorithm, class T, class Alloc>
typename std::enable_if<!is_contiguously_hashable<T>::value>::type
hash_append(HashAlgorithm& h, const std::vector<T, Alloc>& v);

// begin implementations for hash_append

template <class HashAlgorithm, class T, std::size_t N>
typename std::enable_if<!is_contiguously_hashable<T>::value>::type
hash_append(HashAlgorithm& h, T (&a)[N])
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
hash_append(HashAlgorithm& h, const std::basic_string<Char, Traits, Alloc>& s)
{
    h(s.data(), s.size() * sizeof(Char));
    hash_append(h, s.size());
}

template <class HashAlgorithm, class Char, class Traits, class Alloc>
typename std::enable_if<!is_contiguously_hashable<Char>::value>::type
hash_append(HashAlgorithm& h, const std::basic_string<Char, Traits, Alloc>& s)
{
    for (const auto& c : s)
        hash_append(h, c);
    hash_append(h, s.size());
}

template <class HashAlgorithm, class T, class Alloc>
typename std::enable_if<is_contiguously_hashable<T>::value>::type
hash_append(HashAlgorithm& h, const std::vector<T, Alloc>& v)
{
    h(v.data(), v.size() * sizeof(T));
    hash_append(h, v.size());
}

template <class HashAlgorithm, class T, class Alloc>
typename std::enable_if<!is_contiguously_hashable<T>::value>::type
hash_append(HashAlgorithm& h, const std::vector<T, Alloc>& v)
{
    for (const auto& val : v)
        hash_append(h, val);
    hash_append(h, v.size());
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
 * A generic, manually seeded hash function.
 */
template <class HashAlgorithm = typename default_hasher<>::type,
          class SeedType = uint64_t>
class seeded_hash
{
  public:
    using result_type = typename HashAlgorithm::result_type;

    seeded_hash(SeedType seed) : seed_{seed}
    {
        // nothing
    }

    template <class T>
    result_type operator()(const T& t) const
    {
        HashAlgorithm h(seed_);
        using hashing::hash_append;
        hash_append(h, t);
        return static_cast<result_type>(h);
    }

    SeedType seed() const
    {
        return seed_;
    }

  private:
    SeedType seed_;
};

/**
 * A generic, randomly seeded hash function.
 * @see
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html#seeding
 */
template <class HashAlgorithm = typename default_hasher<>::type>
struct hash
{
    using result_type = typename HashAlgorithm::result_type;

    template <class T>
    result_type operator()(const T& t) const
    {
        auto seed = detail::get_process_seed();
        HashAlgorithm h(seed);
        using hashing::hash_append;
        hash_append(h, t);
        return static_cast<result_type>(h);
    }
};
}
}
#endif
