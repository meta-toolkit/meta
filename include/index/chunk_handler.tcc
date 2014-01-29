/**
 * @file chunk_handler.tcc
 * @author Chase Geigle
 */

#include <algorithm>
#include "index/chunk_handler.h"
#include "index/disk_index.h"

namespace meta
{
namespace index
{

template <class Index>
chunk_handler<Index>::chunk_handler(Index* idx,
                                    std::atomic<uint32_t>& chunk_num)
    : chunk_size_{0}, idx_{idx}, chunk_num_{chunk_num}
{
    // nothing
}

template <class Index>
template <class Container>
void chunk_handler<Index>::operator()(const secondary_key_type& key,
                                      const Container& counts)
{
    for (const auto& count : counts)
    {
        index_pdata_type pd{count.first};
        pd.increase_count(key, count.second);
        auto it = pdata_.find(pd);
        if(it == pdata_.end())
        {
            chunk_size_ += pd.bytes_used();
            pdata_.emplace(pd);
        }
        else
        {
            chunk_size_ -= it->bytes_used();

            // note: we can modify elements in this set because we do not change
            // how comparisons are made (the primary_key value)
            const_cast<index_pdata_type&>(*it).increase_count(key, count.second);
            chunk_size_ += it->bytes_used();
        }

        if(chunk_size_ >= max_size)
            flush_chunk();

    }
}

template <class Index>
void chunk_handler<Index>::flush_chunk()
{
    if (chunk_size_ == 0)
        return;

    std::vector<index_pdata_type> pdata;
    for (auto it = pdata_.begin(); it != pdata_.end(); it = pdata_.erase(it))
        pdata.emplace_back(std::move(*it));
    pdata_.clear();
    std::sort(pdata.begin(), pdata.end());
    idx_->write_chunk(chunk_num_.fetch_add(1), pdata);
    chunk_size_ = 0;
}

template <class Index>
chunk_handler<Index>::~chunk_handler()
{
    flush_chunk();
}
}
}
