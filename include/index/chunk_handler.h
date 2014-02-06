/**
 * @file chunk_handler.h
 * @author Chase Geigle
 */

#ifndef _META_INDEX_CHUNK_HANDLER_H_
#define _META_INDEX_CHUNK_HANDLER_H_

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

template <class Index>
class chunk_handler
{
  public:
    using index_pdata_type = typename Index::index_pdata_type;
    using primary_key_type = typename index_pdata_type::primary_key_type;
    using secondary_key_type = typename index_pdata_type::secondary_key_type;
    using chunk_t = chunk<primary_key_type, secondary_key_type>;

    class producer
    {
      public:
        producer(chunk_handler* parent);

        /**
         * Handler for when a given secondary_key has been processed and is
         * ready to be added to the in-memory chunk.
         */
        template <class Container>
        void operator()(const secondary_key_type& key, const Container& counts);

        /**
         * Destroys the producer, writing to disk any chunk data
         * still resident in memory.
         */
        ~producer();

      private:
        /**
         * Flushes the current in-memory chunk to disk.
         */
        void flush_chunk();

        /** the current in-memory chunk */
        std::unordered_set<index_pdata_type> pdata_;

        /** the current size of the in-memory chunk */
        uint64_t chunk_size_;

        const static uint64_t constexpr max_size = 1024 * 1024 * 128; // 128 MB

        /** a back-pointer to the handler this producer is operating on */
        chunk_handler* parent_;
    };

    /**
     * Constructs a chunk_handler that writes to the given prefix.
     * @param prefix the prefix for all chunks to be written
     */
    chunk_handler(const std::string& prefix);

    /**
     * Creates a producer for this chunk_handler. Producers are designed to
     * be thread-local buffers of chunks that write to disk when their
     * buffer is full.
     */
    producer make_producer();

    /**
     * Gets the number of chunks this handler has written to disk.
     */
    uint32_t size() const;

    /**
     * Gets the size, in bytes, of the last chunk written to disk after
     * merging.
     */
    uint64_t final_size() const;

    /**
     * Merge the remaining on-disk chunks.
     */
    void merge_chunks();

    /**
     * Gets the number of unique primary keys seen while merging chunks.
     */
    uint64_t unique_primary_keys() const;

    class chunk_handler_exception : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

  private:
    void write_chunk(std::vector<index_pdata_type>& pdata);

    /** the prefix for all chunks to be written */
    std::string prefix_;

    /** the current chunk number */
    std::atomic<uint32_t> chunk_num_{0};

    /** the queue of chunks on disk that need to be merged */
    std::priority_queue<chunk_t> chunks_;

    /** the mutex used for protecting the chunk queue */
    mutable std::mutex mutables_;

    /** the number of unique primary keys encountered while merging */
    util::optional<uint64_t> unique_primary_keys_;
};
}
}

#include "index/chunk_handler.tcc"
#endif
