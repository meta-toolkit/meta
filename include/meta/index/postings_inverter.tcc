/**
 * @file postings_inverter.tcc
 * @author Chase Geigle
 */

#include <cassert>
#include <algorithm>

#include "meta/index/chunk_reader.h"
#include "meta/index/postings_inverter.h"
#include "meta/index/disk_index.h"
#include "meta/parallel/thread_pool.h"

namespace meta
{
namespace index
{

template <class Index>
postings_inverter<Index>::producer::producer(postings_inverter* parent,
                                             uint64_t ram_budget)
    : max_size_{ram_budget}, parent_{parent}
{
    chunk_size_ = pdata_.bytes_used();
    assert(chunk_size_ < max_size_);
}

template <class Index>
template <class Container>
void postings_inverter<Index>::producer::
operator()(const secondary_key_type& key, const Container& counts)
{
    for (const auto& count : counts)
    {
        using kv_traits
            = hashing::kv_traits<typename std::decay<decltype(count)>::type>;

        postings_buffer_type pb{kv_traits::key(count)};
        auto it = pdata_.find(pb);
        if (it == pdata_.end())
        {
            // check if we would resize on an insert
            if ((pdata_.size() + 1) / static_cast<double>(pdata_.capacity())
                >= pdata_.max_load_factor())
            {
                // now check if roughly doubling our bytes used is going to
                // cause problems
                auto next_chunk_size = chunk_size_ + pdata_.bytes_used()
                                       + pdata_.bytes_used() / 2;
                if (next_chunk_size >= max_size_)
                {
                    // if so, flush the current chunk before carrying on
                    flush_chunk();
                }
            }

            chunk_size_ -= pdata_.bytes_used();

            pb.write_count(key, static_cast<uint64_t>(kv_traits::value(count)));
            chunk_size_ += pb.bytes_used();
            pdata_.emplace(std::move(pb));

            chunk_size_ += pdata_.bytes_used();
        }
        else
        {
            chunk_size_ -= it->bytes_used();

            // note: we can modify elements in this set because we do not change
            // how comparisons are made (the primary_key value)
            const_cast<postings_buffer_type&>(*it).write_count(
                key, static_cast<uint64_t>(kv_traits::value(count)));

            chunk_size_ += it->bytes_used();
        }

        if (chunk_size_ >= max_size_)
            flush_chunk();
    }
}

template <class Index>
void postings_inverter<Index>::producer::flush_chunk()
{
    if (pdata_.empty())
        return;

    // extract the keys, emptying the hash set
    auto pdata = pdata_.extract_keys();
    std::sort(pdata.begin(), pdata.end());
    parent_->write_chunk(pdata);

    chunk_size_ = pdata_.bytes_used();

    // if the table itself is beyond the maximum chunk size, start over
    // (this should rarely, if ever, happen)
    if (chunk_size_ > max_size_)
    {
        decltype(pdata_) tmp{};
        using std::swap;
        swap(tmp, pdata_);
        chunk_size_ = pdata_.bytes_used();
    }
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
