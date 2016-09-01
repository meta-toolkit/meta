/**
 * @file probe_set.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_HASHING_PROBE_SET_H_
#define META_HASHING_PROBE_SET_H_

#include "meta/config.h"
#include "meta/hashing/hash.h"
#include "meta/hashing/hash_traits.h"
#include "meta/hashing/probing.h"

namespace meta
{
namespace hashing
{

/**
 * An **insert-only** probing hash set.
 *
 * The primary use case for this is for storing in-memory chunks of
 * postings data during indexing, but it could easily be used in other
 * places.
 *
 * The behavior of the set is configurable via the template parameters:
 * - Key: the data type to be stored, which must have a valid hash function
 * - ProbingStrategy: The strategy to use when probing the table (defaults
 *   to probing::binary)
 * - ResizingRatio: The ratio (> 1) to increase the table size by when
 *   resizing (defaults to std::ratio<3, 2>)
 * - Hash: The hash function to use (defaults to hashing::hash<>)
 * - KeyEqual: The comparator to use on keys to determine equality
 *   (defaults to std::equal_to<Key>)
 */
template <class Key, class ProbingStrategy = probing::binary,
          class Hash = hash<>, class KeyEqual = std::equal_to<Key>,
          class Traits = hash_traits<Key>>
class probe_set
    : private Traits::template storage_type<ProbingStrategy, Hash, KeyEqual>
{
  public:
    using storage_type =
        typename Traits::template storage_type<ProbingStrategy, Hash, KeyEqual>;

    using typename storage_type::iterator;
    using typename storage_type::const_iterator;

    using storage_type::default_max_load_factor;
    using storage_type::default_resize_ratio;

    using storage_type::storage_type;
    using storage_type::begin;
    using storage_type::end;
    using storage_type::max_load_factor;
    using storage_type::resize_ratio;
    using storage_type::emplace;
    using storage_type::find;
    using storage_type::empty;
    using storage_type::next_load_factor;
    using storage_type::next_size;
    using storage_type::size;
    using storage_type::capacity;
    using storage_type::clear;
    using storage_type::bytes_used;
    using storage_type::extract_keys;

    probe_set() : storage_type{8}
    {
        // nothing
    }

    /**
     * @param key a reference to the key to be inserted into the table
     * @return an iterator to the item inserted
     */
    iterator insert(const Key& key)
    {
        return emplace(key);
    }
};
}
}
#endif
