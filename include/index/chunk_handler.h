/**
 * @file chunk_handler.h
 * @author Chase Geigle
 */

#ifndef _META_INDEX_CHUNK_HANDLER_H_
#define _META_INDEX_CHUNK_HANDLER_H_

#include <atomic>
#include <utility>
#include <vector>
#include <unordered_set>

namespace meta
{
namespace index
{

template <class Index>
class chunk_handler
{
  public:
    using index_pdata_type = typename Index::index_pdata_type;
    using secondary_key_type = typename index_pdata_type::secondary_key_type;

    /**
     * Creates a new handler on the given index, using the
     * given atomic to keep track of the current chunk number.
     */
    chunk_handler(Index* idx, std::atomic<uint32_t>& chunk_num);

    /**
     * Handler for when a given primary_key has been processed and is
     * ready to be added to the in-memory chunk.
     */
    template <class Container>
    void operator()(const secondary_key_type& key, const Container& counts);

    /**
     * Destroys the handler, writing to disk any chunk data
     * still resident in memory.
     */
    ~chunk_handler();

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

    /** a back-pointer to the index this handler is operating on */
    Index* idx_;

    /** the current chunk number */
    std::atomic<uint32_t>& chunk_num_;
};
}
}

#include "index/chunk_handler.tcc"
#endif
