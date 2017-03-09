/**
 * @file postings_inverter.tcc
 * @author Chase Geigle
 */

#include <algorithm>
#include <cassert>

#include "meta/index/chunk_reader.h"
#include "meta/index/disk_index.h"
#include "meta/index/postings_inverter.h"
#include "meta/parallel/thread_pool.h"

namespace meta
{
namespace index
{

template <class Index>
postings_inverter<Index>::producer::producer(postings_inverter* parent,
                                             uint64_t ram_budget)
    : max_size_{static_cast<uint64_t>(0.9 * ram_budget)}, // (1)
      parent_{parent}
{
    //
    // This is a pile of heuristics.
    //
    // (1) (in the initializer list above) helps guard against
    // fragmentation/poor estimates of additional heap usage that isn't
    // accounted for by the hash table or the postings arena. It's mostly
    // just a safeguard to try to make sure we never *actually* hit the RAM
    // limit.
    //
    // (2) is based on observations taken from the old implementation that
    // allowed for the postings hash table to resize. In that setting,
    // things were flushing when there was about a 50-50 split between hash
    // table RAM and postings buffer RAM. This might need to be adjusted,
    // but it's working fine for now.
    //
    // (3) is based on the fact that we know that the robinhood_set will
    // use power-of-2 table sizes and so will round up the number of
    // buckets up to the next larger power of two on a reserve.
    //
    auto table_budget = max_size_ / 2; // (2)
    auto buckets = table_budget / (2 * sizeof(std::size_t)
                                   + pdata_.max_load_factor()
                                         * sizeof(postings_buffer_type));
    // (3)
    auto lower_pow2_buckets
        = static_cast<uint64_t>(std::pow(2, std::floor(std::log2(buckets))));
    max_table_size_
        = static_cast<uint64_t>(pdata_.max_load_factor() * lower_pow2_buckets);

    pdata_.reserve(max_table_size_);
    chunk_size_ = pdata_.bytes_used();
    assert(chunk_size_ < max_size_);
    arena_ = make_unique<util::arena<8>>(max_size_ - chunk_size_);
}

template <class Index>
template <class Container>
void postings_inverter<Index>::producer::
operator()(const secondary_key_type& key, const Container& counts)
{
    for (const auto& count : counts)
    {
        postings_buffer_type pb{count.first, allocator_type{*arena_}};
        auto it = pdata_.find(pb);
        if (it == pdata_.end())
        {
            // check if we are going to cause the table to resize
            if (pdata_.size() == max_table_size_)
                flush_chunk();

            try
            {
                pb.write_count(key, count.second);
            }
            catch (const util::arena_bad_alloc&)
            {
                // if we ran out of room in the arena, we need to flush the
                // current chunk, after which the write_count should always
                // succeed
                flush_chunk();
                pb.write_count(key, count.second);
            }

            chunk_size_ += pb.bytes_used();
            pdata_.emplace(std::move(pb));
        }
        else
        {
            try
            {
                const auto old_size = it->bytes_used();
                // note: we can modify elements in this set because we do
                // not change how comparisons are made (the primary_key
                // value)
                const_cast<postings_buffer_type&>(*it).write_count(
                    key, count.second);
                chunk_size_ += (it->bytes_used() - old_size);
            }
            catch (const util::arena_bad_alloc&)
            {
                // if we ran out of room in the arena, we need to flush the
                // current chunk, after which the postings data table will
                // be empty. Thus, we need to write the count data to the
                // temporary postings buffer we created, and then move that
                // temporary into the table.
                flush_chunk();
                pb.write_count(key, count.second);
                chunk_size_ += pb.bytes_used();
                pdata_.emplace(std::move(pb));
            }
        }

        // this should probably never happen
        if (chunk_size_ >= max_size_)
            flush_chunk();
    }
}

template <class Index>
void postings_inverter<Index>::producer::flush_chunk()
{
    if (pdata_.empty())
        return;

    {
        // extract the keys, emptying the hash set
        auto pdata = pdata_.extract();
        std::sort(pdata.begin(), pdata.end());
        parent_->write_chunk(pdata);
    }

    // clear the arena
    arena_->reset();

    // reserving the same table size as we had before will ensure that the
    // values array we extracted above will be re-allocated
    pdata_.reserve(max_table_size_);
    chunk_size_ = pdata_.bytes_used();
}

template <class Index>
postings_inverter<Index>::producer::~producer()
{
    flush_chunk();
}

template <class Index>
postings_inverter<Index>::postings_inverter(const std::string& prefix,
                                            unsigned writers)
    : prefix_{prefix}, sem_{writers}
{
    // nothing
}

template <class Index>
auto postings_inverter<Index>::make_producer(uint64_t ram_budget) -> producer
{
    return {this, ram_budget};
}

template <class Index>
template <class Allocator>
void postings_inverter<Index>::write_chunk(
    std::vector<postings_buffer_type, Allocator>& pdata)
{
    // ensure we don't get too many writer threads by waiting on the
    // semaphore
    parallel::semaphore::wait_guard guard{sem_};

    auto chunk_num = chunk_num_.fetch_add(1);

    util::optional<chunk_t> top;
    {
        std::lock_guard<std::mutex> lock{mutables_};
        if (!chunks_.empty())
        {
            top = chunks_.top();
            chunks_.pop();
        }
    }

    if (!top) // pqueue was empty
    {
        std::string chunk_name
            = prefix_ + "/chunk-" + std::to_string(chunk_num);
        {
            std::ofstream outfile{chunk_name, std::ios::binary};
            for (auto& p : pdata)
                p.write_packed(outfile);
        }
        pdata.clear();

        std::lock_guard<std::mutex> lock{mutables_};
        chunks_.emplace(chunk_name);
    }
    else // we can merge with an existing chunk
    {
        top->memory_merge_with(pdata);

        std::lock_guard<std::mutex> lock{mutables_};
        chunks_.emplace(*top);
    }
}

template <class Index>
void postings_inverter<Index>::merge_chunks()
{
    std::vector<std::string> to_merge;
    to_merge.reserve(chunks_.size());
    while (!chunks_.empty())
    {
        to_merge.emplace_back(chunks_.top().path());
        chunks_.pop();
    }

    std::ofstream outfile{prefix_ + "/postings.index", std::ios::binary};
    unique_primary_keys_ = multiway_merge<index_pdata_type>(
        outfile, to_merge.begin(), to_merge.end());
}

template <class Index>
uint64_t postings_inverter<Index>::unique_primary_keys() const
{
    if (!unique_primary_keys_)
        throw postings_inverter_exception{
            "merge has not been called before requesting unique primary keys"};
    return *unique_primary_keys_;
}

template <class Index>
uint64_t postings_inverter<Index>::final_size() const
{
    if (!chunks_.empty())
        throw postings_inverter_exception{
            "merge not complete before final_size() called"};
    return filesystem::file_size(prefix_ + "/postings.index");
}

template <class Index>
uint32_t postings_inverter<Index>::size() const
{
    return chunk_num_.load();
}
}
}
