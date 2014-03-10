/**
 * @file chunk_handler.tcc
 * @author Chase Geigle
 */

#include <algorithm>

#include "index/chunk_handler.h"
#include "index/disk_index.h"
#include "parallel/thread_pool.h"

namespace meta
{
namespace index
{

template <class Index>
chunk_handler<Index>::producer::producer(chunk_handler* parent)
    : chunk_size_{0}, parent_{parent}
{
    // nothing
}

template <class Index>
template <class Container>
void chunk_handler<Index>::producer::operator()(const secondary_key_type& key,
                                                const Container& counts)
{
    for (const auto& count : counts)
    {
        index_pdata_type pd{count.first};
        pd.increase_count(key, count.second);
        auto it = pdata_.find(pd);
        if (it == pdata_.end())
        {
            chunk_size_ += pd.bytes_used();
            pdata_.emplace(pd);
        }
        else
        {
            chunk_size_ -= it->bytes_used();

            // note: we can modify elements in this set because we do not change
            // how comparisons are made (the primary_key value)
            const_cast<index_pdata_type&>(*it).increase_count(key,
                                                              count.second);
            chunk_size_ += it->bytes_used();
        }

        if (chunk_size_ >= max_size)
            flush_chunk();
    }
}

template <class Index>
void chunk_handler<Index>::producer::flush_chunk()
{
    if (chunk_size_ == 0)
        return;

    std::vector<index_pdata_type> pdata;
    for (auto it = pdata_.begin(); it != pdata_.end(); it = pdata_.erase(it))
        pdata.emplace_back(std::move(*it));
    pdata_.clear();
    std::sort(pdata.begin(), pdata.end());
    parent_->write_chunk(pdata);
    chunk_size_ = 0;
}

template <class Index>
chunk_handler<Index>::producer::~producer()
{
    flush_chunk();
}

template <class Index>
chunk_handler<Index>::chunk_handler(const std::string& prefix)
    : prefix_{prefix}
{
    // nothing
}

template <class Index>
auto chunk_handler<Index>::make_producer() -> producer
{
    return {this};
}

template <class Index>
void chunk_handler<Index>::write_chunk(std::vector<index_pdata_type>& pdata)
{
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
        std::string chunk_name = prefix_ + "/chunk-"
                                 + std::to_string(chunk_num);
        io::compressed_file_writer outfile{chunk_name,
                                           io::default_compression_writer_func};
        for (auto& p : pdata)
            outfile << p;

        outfile.close(); // close so we can read the file size in chunk ctr
        std::ofstream termfile{chunk_name + ".numterms"};
        termfile << pdata.size();
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
void chunk_handler<Index>::merge_chunks()
{
    size_t remaining = chunks_.size() - 1;
    std::mutex mutex;
    auto task = [&]()
    {
        while (true)
        {
            util::optional<chunk_t> first;
            util::optional<chunk_t> second;
            {
                std::lock_guard<std::mutex> lock{mutables_};
                if (chunks_.size() < 2)
                    return;
                first = chunks_.top();
                chunks_.pop();
                second = chunks_.top();
                chunks_.pop();
                LOG(progress) << "> Merging " << first->path() << " ("
                              << printing::bytes_to_units(first->size())
                              << ") and " << second->path() << " ("
                              << printing::bytes_to_units(second->size())
                              << "), " << --remaining << " remaining        \r"
                              << ENDLG;
            }
            first->merge_with(*second);
            {
                std::lock_guard<std::mutex> lock{mutex};
                chunks_.push(*first);
            }
        }
    };

    parallel::thread_pool pool;
    auto thread_ids = pool.thread_ids();
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < thread_ids.size(); ++i)
        futures.emplace_back(pool.submit_task(task));

    for (auto& fut : futures)
        fut.get();

    LOG(progress) << '\n' << ENDLG;

    if (chunks_.empty())
        throw chunk_handler_exception{"there were no chunks to merge"};

    uint64_t unique_keys;
    std::ifstream termfile{chunks_.top().path() + ".numterms"};
    termfile >> unique_keys;
    termfile.close();
    filesystem::delete_file(chunks_.top().path() + ".numterms");
    filesystem::rename_file(chunks_.top().path(), prefix_ + "/postings.index");

    unique_primary_keys_ = unique_keys;
}

template <class Index>
uint64_t chunk_handler<Index>::unique_primary_keys() const
{
    if (!unique_primary_keys_)
        throw chunk_handler_exception{
            "merge has not been called before requesting unique primary keys"};
    return *unique_primary_keys_;
}

template <class Index>
uint64_t chunk_handler<Index>::final_size() const
{
    if (chunks_.size() != 1)
        throw chunk_handler_exception{
            "merge not complete before final_size() called"};
    return chunks_.top().size();
}

template <class Index>
uint32_t chunk_handler<Index>::size() const
{
    return chunk_num_.load();
}
}
}
