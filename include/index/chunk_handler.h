/**
 * @file chunk_handler.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_CHUNK_HANDLER_H_
#define META_INDEX_CHUNK_HANDLER_H_

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#include "index/chunk.h"
#include "util/optional.h"

namespace meta
{
namespace index
{

/**
 * An interface for writing and merging inverted chunks of postings_data for a
 * disk_index.
 */
template <class Index>
class chunk_handler
{
  public:
    using index_pdata_type = typename Index::index_pdata_type;
    using primary_key_type = typename index_pdata_type::primary_key_type;
    using secondary_key_type = typename index_pdata_type::secondary_key_type;
    using chunk_t = chunk<primary_key_type, secondary_key_type>;

    /**
     * The object that is fed postings_data by the index.
     */
    class producer
    {
      public:
        /**
         * @param parent A back-pointer to the handler this producer is
         * operating on
         */
        producer(chunk_handler* parent);

        /**
         * Handler for when a given secondary_key has been processed and is
         * ready to be added to the in-memory chunk.
         * @param key The secondary key used to index the counts container
         * @param counts A collection of (primary_key_type, count) pairs
         */
        template <class Container>
        void operator()(const secondary_key_type& key, const Container& counts);

        /**
         * Destroys the producer, writing to disk any chunk data still resident
         * in memory.
         */
        ~producer();

      private:
        /**
         * Flushes the current in-memory chunk to disk.
         */
        void flush_chunk();

        /// Current in-memory chunk
        std::unordered_set<index_pdata_type> pdata_;

        /// Current size of the in-memory chunk
        uint64_t chunk_size_;

        /// Maximum allowed size of a chunk in bytes before it is written
        const static uint64_t constexpr max_size = 1024 * 1024 * 128; // 128 MB

        /// Back-pointer to the handler this producer is operating on
        chunk_handler* parent_;
    };

    /**
     * Constructs a chunk_handler that writes to the given prefix.
     * @param prefix The prefix for all chunks to be written
     */
    chunk_handler(const std::string& prefix);

    /**
     * Creates a producer for this chunk_handler. Producers are designed to
     * be thread-local buffers of chunks that write to disk when their
     * buffer is full.
     * @return a new producer
     */
    producer make_producer();

    /**
     * @return the number of chunks this handler has written to disk.
     */
    uint32_t size() const;

    /**
     * @return the size, in bytes, of the last chunk written to disk after
     * merging.
     */
    uint64_t final_size() const;

    /**
     * Merge the remaining on-disk chunks.
     */
    void merge_chunks();

    /**
     * @return the number of unique primary keys seen while merging chunks.
     */
    uint64_t unique_primary_keys() const;

    /**
     * Simple exception class for chunk_handler interactions
     */
    class chunk_handler_exception : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

  private:
    /**
     * @param pdata The collection of postings_data objects to combine into a
     * chunk
     */
    void write_chunk(std::vector<index_pdata_type>& pdata);

    /// The prefix for all chunks to be written
    std::string prefix_;

    /// The current chunk number
    std::atomic<uint32_t> chunk_num_{0};

    /// Queue of chunks on disk that need to be merged */
    std::priority_queue<chunk_t> chunks_;

    /// Mutex used for protecting the chunk queue
    mutable std::mutex mutables_;

    /// Number of unique primary keys encountered while merging
    util::optional<uint64_t> unique_primary_keys_;
};
}
}

#include "index/chunk_handler.tcc"
#endif
