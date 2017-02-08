/**
 * @file cooccurrence_counter.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_EMBEDDINGS_COOCCURRENCE_COUNTER_H_
#define META_EMBEDDINGS_COOCCURRENCE_COUNTER_H_

#include <atomic>

#include "meta/config.h"

#include "meta/analyzers/token_stream.h"
#include "meta/corpus/corpus.h"
#include "meta/embeddings/cooccur_record.h"
#include "meta/hashing/probe_map.h"
#include "meta/io/packed.h"
#include "meta/parallel/semaphore.h"

namespace meta
{
namespace embeddings
{

/**
 * A (target, context) pair used as the key in a cooccurrence hash table.
 */
struct cooccurrence_key
{
    constexpr cooccurrence_key(uint64_t targ, uint64_t ctx)
        : target{targ}, context{ctx}
    {
        // nothing
    }

    uint64_t target;
    uint64_t context;
};

inline bool operator==(const cooccurrence_key& a, const cooccurrence_key& b)
{
    return std::tie(a.target, a.context) == std::tie(b.target, b.context);
}

inline bool operator<(const cooccurrence_key& a, const cooccurrence_key& b)
{
    return std::tie(a.target, a.context) < std::tie(b.target, b.context);
}

template <class OutputStream>
uint64_t packed_write(OutputStream& os, const cooccurrence_key& key)
{
    auto bytes = io::packed::write(os, key.target);
    return bytes + io::packed::write(os, key.context);
}
}

namespace hashing
{
template <>
struct key_traits<embeddings::cooccurrence_key>
{
    static constexpr bool inlineable = true;
    constexpr static embeddings::cooccurrence_key sentinel()
    {
        return {key_traits<uint64_t>::sentinel(),
                key_traits<uint64_t>::sentinel()};
    }
};

template <>
struct is_contiguously_hashable<embeddings::cooccurrence_key>
{
    const static constexpr bool value = true;
};
}

namespace embeddings
{

/**
 * A chunk of cooccurrence records on disk.
 */
struct cooccurrence_chunk
{
    cooccurrence_chunk(const std::string& file, uint64_t bytes)
        : path{file}, size{bytes}
    {
        // nothing
    }

    std::string path;
    uint64_t size;
};

inline bool operator<(const cooccurrence_chunk& a, const cooccurrence_chunk& b)
{
    // merge smaller chunks first
    return a.size > b.size;
}

/**
 * An iterator adhering to the ChunkIterator concept for multiway_merge
 * support on in-memory cooccurrence data.
 */
class memory_cooccur_iterator
{
  public:
    using map_type = hashing::probe_map<cooccurrence_key, double>;
    using memory_chunk_type = map_type::storage_type::vector_type;
    using count_type = std::pair<cooccurrence_key, double>;

    memory_cooccur_iterator() = default;

    memory_cooccur_iterator(memory_chunk_type&& items)
        : items_{std::move(items)}, idx_{0}
    {
        // nothing
    }

    memory_cooccur_iterator& operator++()
    {
        ++idx_;
        if (idx_ >= items_.size())
        {
            items_.clear();
            idx_ = 0;
        }

        return *this;
    }

    cooccur_record operator*() const
    {
        const auto& item = items_[idx_];
        return {item.first.target, item.first.context, item.second};
    }

    uint64_t total_bytes() const
    {
        return sizeof(count_type) * items_.size();
    }

    uint64_t bytes_read() const
    {
        return sizeof(count_type) * idx_;
    }

    bool operator==(const memory_cooccur_iterator& other) const
    {
        return items_.empty() && other.items_.empty();
    }

    bool operator!=(const memory_cooccur_iterator& other) const
    {
        return !(*this == other);
    }

  private:
    memory_chunk_type items_;
    std::size_t idx_{0};
};

/**
 * Management class for cooccurrence counting. This class maintains the
 * shared state across all threads used for parallel cooccurrence counting.
 */
class cooccurrence_counter
{
  public:
    using memory_chunk_type = memory_cooccur_iterator::memory_chunk_type;

    struct configuration
    {
        std::string prefix;
        std::size_t max_ram = 4096u * 1024u * 1024u; // 4GB
        std::size_t merge_fanout = 8;
        std::size_t window_size = 15;
        bool break_on_tags = false;
    };

    cooccurrence_counter(configuration config, parallel::thread_pool& pool);

    ~cooccurrence_counter();

    void count(corpus::corpus& docs,
               const analyzers::token_stream& stream);

  private:
    void flush_chunk(memory_chunk_type&& chunk);
    void memory_merge_chunks();
    void maybe_merge();

    friend class cooccurrence_buffer;
    const std::string prefix_;
    std::size_t max_ram_;
    const std::size_t merge_fanout_;
    const std::size_t window_size_;
    const bool break_on_tags_;
    const hashing::probe_map<std::string, uint64_t> vocab_;
    parallel::thread_pool& pool_;
    std::size_t chunk_num_{0};
    std::atomic_size_t num_tokenizing_{0};
    std::size_t num_pending_{0};
    std::vector<memory_cooccur_iterator> memory_chunks_;
    std::priority_queue<cooccurrence_chunk> chunks_;
    std::mutex chunk_mutex_;
    std::condition_variable chunk_cond_;
    std::mutex io_mutex_;
};

class cooccurrence_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}


}
#endif
