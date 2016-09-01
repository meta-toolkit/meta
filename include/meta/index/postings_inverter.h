/**
 * @file posting_inverter.h
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
#include <utility>
#include <vector>

#include "meta/config.h"
#include "meta/hashing/probe_set.h"
#include "meta/index/chunk.h"
#include "meta/index/postings_buffer.h"
#include "meta/parallel/semaphore.h"
#include "meta/util/optional.h"

namespace meta
{
namespace index
{

/**
 * An interface for writing and merging inverted chunks of postings_data for a
 * disk_index.
 */
template <class Index>
class postings_inverter
{
  public:
    using index_pdata_type = typename Index::index_pdata_type;
    using primary_key_type = typename index_pdata_type::primary_key_type;
    using secondary_key_type = typename index_pdata_type::secondary_key_type;
    using chunk_t = chunk<primary_key_type, secondary_key_type>;
    using postings_buffer_type
        = postings_buffer<primary_key_type, secondary_key_type>;

    /**
     * The object that is fed postings_data by the index.
     */
    class producer
    {
      public:
        /**
         * @param parent A back-pointer to the handler this producer is
         * operating on
         * @param ram_budget The **estimated** allowed size of the buffer
         * for this producer
         */
        producer(postings_inverter* parent, uint64_t ram_budget);

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
        hashing::probe_set<postings_buffer_type> pdata_;

        /// Current size of the in-memory chunk
        uint64_t chunk_size_;

        /**
         * Maximum allowed size of a chunk in bytes before it is written.
         * This is an *estimate*, so you should make sure there's some slop
         * in this number to make sure you don't run out of memory.
         */
        uint64_t max_size_;

        /// Back-pointer to the handler this producer is operating on
        postings_inverter* parent_;
    };

    /**
     * Constructs a postings_inverter that writes to the given prefix.
     * @param prefix The prefix for all chunks to be written
     * @param max_writers The maximum number of allowed writing threads
     */
    postings_inverter(const std::string& prefix, unsigned writers = 8);

    /**
     * Creates a producer for this postings_inverter. Producers are designed to
     * be thread-local buffers of chunks that write to disk when their
     * buffer is full.
     * @param ram_bugdet The estimated allowed size of this thread-local
     * buffer
     * @return a new producer
     */
    producer make_producer(uint64_t ram_budget);

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

  private:
    /**
     * @param pdata The collection of postings_data objects to combine into a
     * chunk
     */
    template <class Allocator>
    void write_chunk(std::vector<postings_buffer_type, Allocator>& pdata);

    /// The prefix for all chunks to be written
    std::string prefix_;

    /// The current chunk number
    std::atomic<uint32_t> chunk_num_{0};

    /// Queue of chunks on disk that need to be merged */
    std::priority_queue<chunk_t> chunks_;

    /// Mutex used for protecting the chunk queue
    mutable std::mutex mutables_;

    /// Semaphore used for limiting the number of threads writing to disk
    parallel::semaphore sem_;

    /// Number of unique primary keys encountered while merging
    util::optional<uint64_t> unique_primary_keys_;
};

/**
 * Simple exception class for postings_inverter interactions
 */
class postings_inverter_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
}

#include "meta/index/postings_inverter.tcc"
#endif
