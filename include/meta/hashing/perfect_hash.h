/**
 * @file perfect_hash.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/config.h"
#include "meta/succinct/compressed_vector.h"
#include "meta/succinct/sarray.h"

namespace meta
{
namespace hashing
{

/**
 * Query class for the minimal perfect hash functions created by
 * perfect_hash_builder. Always returns a number \f$\in [0, N)\f$, even for
 * keys that were not used to construct the minimal perfect hash. It is up
 * to the user to perform any collision resolution for unknown keys; this
 * class simply represents the hash function itself, not a map/table.
 */
template <class K>
class perfect_hash
{
  public:
    perfect_hash(const std::string& prefix)
        : seeds_{prefix + "/seeds"},
          sarray_{prefix + "/sarray"},
          empty_rank_{prefix + "/sarray", sarray_}
    {
        std::ifstream metadata{prefix + "/hash-metadata.bin", std::ios::binary};
        io::packed::read(metadata, bucket_seed_);
        io::packed::read(metadata, num_bins_);
        // nothing
    }

    uint64_t operator()(const K& key) const
    {
        using meta::hashing::hash_append;
        farm_hash_seeded hasher{bucket_seed_};
        hash_append(hasher, key);
        auto hash = static_cast<std::size_t>(hasher);
        auto bucket_id = hash % seeds_.size();
        auto seed = seeds_[bucket_id];
        auto pos = farm::hash_len_16(hash, seed) % num_bins_;
        // the final position is the hash function's position shifted to
        // the left by the number of empty bins that came before it.
        return pos - empty_rank_.rank(pos);
    }

  private:
    /// The seed to use for the bucket hash function
    uint64_t bucket_seed_;
    /// The number of bins for the perfect hash function
    uint64_t num_bins_;
    /// The seeds to use for each bucket
    succinct::compressed_vector seeds_;
    /// The sarray that backs the rank data structure
    succinct::sarray sarray_;
    /// The ranking data structure that counts the number of empty slots
    succinct::sarray_rank empty_rank_;
};
}
}
