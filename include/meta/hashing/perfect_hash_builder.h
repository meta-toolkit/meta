/**
 * @file perfect_hash_builder.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_HASHING_PERFECT_HASH_BUILDER_H_
#define META_HASHING_PERFECT_HASH_BUILDER_H_

#include <cstdint>
#include <string>
#include <vector>

#include "meta/config.h"

namespace meta
{
namespace hashing
{

/**
 * Constructs a minimal perfect hash using a streaming variant of the
 * hash, displace, and compress algorithm. Each key is first hashed to a
 * bucket (where the number of buckets is < the number of total keys). The
 * buckets are then sorted by size, and then each bucket's keys are hashed
 * into the range 0, N by iterating through possible seed values for the
 * hash function until there are no collisions with keys that have already
 * been hashed. For performance reasons, N is bigger than the number of
 * keys (defaulting to N = keys / 0.99), so the algorithm results in a
 * perfect, but not minimal, hash function. The perfect hash function is
 * then reduced to a minimal perfect hash function via the use of a
 * succinct rank structure at the cost of some additional space.
 *
 * This is accomplished for data that is larger than available RAM with the
 * following steps:
 *
 * 1. Hash keys to buckets using a fixed size buffer, sorting by bucket id
 *    and spilling chunks to disk when full
 * 2. Merge chunks and records by the bucket id
 * 3. Sort each bucket by size, writing chunks to disk when buffer is full
 * 4. Merge chunks (no record merging)
 * 5. Find appropriate seed values for each bucket
 * 6. Compress the seed values
 * 7. Compress the perfect hash function found into a minimal perfect hash
 *
 * Empirically, our algorithm approaches somewhere in the neighborhood of
 * ~2.7 bits per key with our default settings.
 *
 * @see http://cmph.sourceforge.net/papers/esa09.pdf
 */
template <class K>
class perfect_hash_builder
{
  public:
    struct options
    {
        std::string prefix;
        uint64_t max_ram = 1024 * 1024 * 1024; // 1 GB
        uint64_t num_keys;
        uint64_t num_per_bucket = 4;
        float load_factor = 0.99f;

        options() = default;
        options(const options&) = default;
        options(options&&) = default;
        options& operator=(const options&) = default;
        options& operator=(options&&) = default;
    };

    /**
     * @param opts The options for the builder
     */
    perfect_hash_builder(options opts);

    /**
     * Records observed keys. Should be called *once* per unique key.
     * @param key The key to record
     */
    void operator()(const K& key);

    /**
     * Writes the perfect_hash to disk.
     */
    void write();

  private:
    void flush_chunk();

    void merge_chunks_by_bucket_id();

    template <class Iterator>
    void flush_bucket_chunk(Iterator begin, Iterator end);

    void merge_chunks_by_bucket_size();
    void sort_buckets_by_size();
    void construct_perfect_hash();

    /// The options used during building
    options opts_;

    /// The seed for the bucket hash function
    uint64_t bucket_seed_;

    /// The number of buckets to use during building
    uint64_t num_buckets_;

    /// The current number of chunks that have been flushed to disk
    uint64_t num_chunks_;

    struct hashed_key
    {
        std::size_t idx;
        K key;

        hashed_key(std::size_t index, const K& akey) : idx{index}, key{akey}
        {
            // nothing
        }

        bool operator<(const hashed_key& other) const
        {
            return idx < other.idx;
        }
    };

    /// The buffer used for performing the bucket partitioning
    std::vector<hashed_key> buffer_;
};
}
}

#include "perfect_hash_builder.tcc"
#endif
