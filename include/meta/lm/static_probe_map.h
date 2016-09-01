/**
 * @file static_probe_map.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_STATIC_PROBE_MAP_H_
#define META_STATIC_PROBE_MAP_H_

#include <utility>

#include "meta/config.h"
#include "meta/lm/lm_node.h"
#include "meta/lm/token_list.h"
#include "meta/util/disk_vector.h"
#include "meta/util/optional.h"

namespace meta
{
namespace lm
{
/**
 * Represents language model probabilities as string -> (prob, backoff) values.
 * For space and time efficiency, this class only stores the uint64_t hash of
 * the string keys, so it is not possible to query which keys exist in the
 * table. The values of (prob, backoff) are stored as two packed floats in a
 * uint64_t. The use of uint64_t allows storage to exist in a util::disk_vector,
 * which makes loading after the initial creation relatively fast.
 */
class static_probe_map
{
    static_assert(sizeof(float) == 4, "two floats need to occupy 8 bytes!");

  public:
    /**
     * Constructor.
     * @param num_elems The number of elements that will be stored in this map.
     * Note that the storage required will be more than this amount in order to
     * have an acceptable load factor. If num_elems is zero, binary LM files
     * are loaded.
     */
    static_probe_map(const std::string& filename, uint64_t num_elems = 0);

    /**
     * Default move constructor.
     */
    static_probe_map(static_probe_map&&) = default;

    /**
     * @param key The key to search for in the map (vector of word ids)
     * @return an optional language model node containing the probability and
     * backoff value for the key
     */
    util::optional<lm_node> find(const std::vector<term_id>& key) const;

    /**
     * @param key The string key to insert (though only a uint64_t hash is
     * stored; if the hash already exists, an exception is thrown)
     * @param prob The probability of the key in this LM
     * @param backoff The backoff probability for this LM
     */
    void insert(const token_list& key, float prob, float backoff);

  private:
    /**
     * Helper function to create hasher and hash a list of word ids
     */
    uint64_t hash(const std::vector<term_id>& tokens) const;

    /// Helper function to find a node given the hash value
    util::optional<lm_node> find_hash(uint64_t hashed) const;

    /// A seed for the string hash function
    static constexpr uint64_t seed_ = 0x2bedf99b3aa222d9;

    /// The internal map representing std::string -> lm_node pairs
    util::disk_vector<uint64_t> table_;
};

/**
 * Basic exception for static_probe_map interactions.
 */
class static_probe_map_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#endif
