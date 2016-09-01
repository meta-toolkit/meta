/**
 * @file hash_traits.h
 * @author Chase Geigle
 */

#ifndef META_HASHING_HASH_TRAITS_H_
#define META_HASHING_HASH_TRAITS_H_

#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "meta/config.h"
#include "meta/hashing/hash_storage.h"

namespace meta
{
namespace hashing
{

/**
 * A traits class indicating whether a type can be inlined as a key in a
 * probing-based hash table. The default implementation provided here
 * allows all integral types to be inlined.
 *
 * You should specialize this trait as you see fit to allow inlining of
 * keys into the table for efficiency when it makes sense. Typically, keys
 * are good to inline when they are small, but can be expensive to inline
 * when they are large object (e.g. std::string).
 *
 * The class provides the following members:
 * - inlineable: a boolean indicating whether this type was inlineable
 * - sentinel(): if inlineable, provides a sentinel value to sacrifice in
 *   order to flag empty cells in the table. By default, this is the
 *   largest representable T according to numeric_limits.
 */
template <class T>
struct key_traits
{
    static constexpr bool inlineable = std::is_integral<T>::value;

    constexpr static T sentinel()
    {
        return std::numeric_limits<T>::max();
    }
};

/**
 * A traits class for types that may be used as keys or values in the
 * probing hash tables.
 *
 * This traits class is used to specify the desired underlying storage type
 * for hash sets. By default, it computes an appropriate storage type based
 * on whether the type is key_inlineable.
 *
 * This should be specialized if you desire custom behavior.
 *
 * The class provides the following members:
 * - storage_type: the type to use to store the keys in the hash set
 * - probe_entry: the type stored in the table's probing array
 */
template <class T>
struct hash_traits
{
    template <class ProbingStrategy, class Hash, class KeyEqual>
    using storage_type =
        typename std::conditional<key_traits<T>::inlineable,
                                  inline_key_storage<T, ProbingStrategy, Hash,
                                                     KeyEqual>,
                                  external_key_storage<T, ProbingStrategy, Hash,
                                                       KeyEqual>>::type;

    using probe_entry =
        typename std::conditional<key_traits<T>::inlineable, T, hash_idx>::type;
};

/**
 * Specialization of hash_traits for the hash *table* classes, and is
 * implemented by delegating to hash_traits<T> for the types inside the
 * pair to be stored.
 *
 * This specialization provides the following members:
 * - storage_type: the selected storage type for the key, value pairs in
 *   the table
 * - key_sentinel(): if the key of the key, value pair is inlineable, this
 *   provides a sentinel value for the key by delegating to
 *   hash_traits<K>::sentinel().
 * - value_sentinel(): if the value of the key, value pair is inlineable,
 *   this provides a sentinel value for the value by delegating to
 *   hash_traits<V>::sentinel().
 *
 * Typically, you should only have to specialize hash_traits<K> and
 * hash_traits<V> for the K, V pair types you care about, but you may
 * choose to provide a specialization for a specific kv_pair<K, V> if you
 * so desire.
 */
template <class K, class V>
struct hash_traits<kv_pair<K, V>>
{
    template <class ProbingStrategy, class Hash, class KeyEqual>
    using key_inlineable_storage = typename std::
        conditional<sizeof(V) <= 8,
                    inline_key_value_storage<K, V, ProbingStrategy, Hash,
                                             KeyEqual>,
                    inline_key_external_value_storage<K, V, ProbingStrategy,
                                                      Hash, KeyEqual>>::type;

    template <class ProbingStrategy, class Hash, class KeyEqual>
    using storage_type = typename std::
        conditional<key_traits<K>::inlineable,
                    key_inlineable_storage<ProbingStrategy, Hash, KeyEqual>,
                    external_key_value_storage<K, V, ProbingStrategy, Hash,
                                               KeyEqual>>::type;

    using key_inlineable_probe_entry =
        typename std::conditional<sizeof(V) <= 8, std::pair<K, V>,
                                  std::pair<K, std::size_t>>::type;

    using probe_entry =
        typename std::conditional<key_traits<K>::inlineable,
                                  key_inlineable_probe_entry, hash_idx>::type;
};
}
}
#endif
