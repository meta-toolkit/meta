/**
 * @file chunk_handler.tcc
 * @author Chase Geigle
 */

#include <cassert>
#include <algorithm>

#include "index/chunk_handler.h"
#include "index/disk_index.h"
#include "parallel/thread_pool.h"

namespace meta
{
namespace index
{

template <class Index>
chunk_handler<Index>::producer::producer(chunk_handler* parent,
                                         uint64_t ram_budget)
    : max_size_{ram_budget}, parent_{parent}
{
    // sizeof(size_t): list size per bucket
    // sizeof(void*): head pointer per bucket
    chunk_size_ = pdata_.bucket_count() * (sizeof(size_t) + sizeof(void*));
    assert(chunk_size_ < max_size_);
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
            // sizeof(size_t): list size per bucket
            // sizeof(void*): head pointer per bucket
            chunk_size_ -= pdata_.bucket_count()
                           * (sizeof(size_t) + sizeof(void*));

            // sizeof(void*): next pointer per element
            chunk_size_ += pd.bytes_used() + sizeof(void*);
            // 25% slop factor
            chunk_size_ += (pd.bytes_used() + sizeof(void*)) / 4;

            pdata_.emplace(pd);

            // sizeof(size_t): list size per bucket
            // sizeof(void*): head pointer per bucket
            chunk_size_ += pdata_.bucket_count()
                           * (sizeof(size_t) + sizeof(void*));
        }
        else
        {
            chunk_size_ -= it->bytes_used() + sizeof(void*);
            chunk_size_ -= (it->bytes_used() + sizeof(void*)) / 4;

            // note: we can modify elements in this set because we do not change
            // how comparisons are made (the primary_key value)
            const_cast<index_pdata_type&>(*it)
                .increase_count(key, count.second);

            chunk_size_ += it->bytes_used() + sizeof(void*);
            chunk_size_ += (it->bytes_used() + sizeof(void*)) / 4;
        }

        if (chunk_size_ >= max_size_)
            flush_chunk();
    }
}

template <class Index>
void chunk_handler<Index>::producer::flush_chunk()
{
    if (pdata_.empty())
        return;

    std::vector<index_pdata_type> pdata;
    for (auto it = pdata_.begin(); it != pdata_.end(); it = pdata_.erase(it))
        pdata.emplace_back(std::move(*it));

    pdata_.clear();
    std::sort(pdata.begin(), pdata.end());
    parent_->write_chunk(pdata);

    // sizeof(size_t): list size per bucket
    // sizeof(void*): head pointer per bucket
    chunk_size_ = pdata_.bucket_count() * (sizeof(size_t) + sizeof(void*));
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
auto chunk_handler<Index>::make_producer(uint64_t ram_budget) -> producer
{
    return {this, ram_budget};
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

namespace detail
{
template <class Index>
struct input_chunk
{
    std::unique_ptr<std::ifstream> file;
    std::string path;
    typename Index::index_pdata_type postings;
    uint64_t total_bytes;
    uint64_t bytes_read;

    input_chunk(const std::string& filename)
        : file{make_unique<std::ifstream>(filename, std::ios::binary)},
          path{filename},
          total_bytes{filesystem::file_size(path)},
          bytes_read{0}
    {
        ++(*this);
    }

    ~input_chunk()
    {
        if (file)
        {
            file = nullptr;
            filesystem::delete_file(path);
        }
    }

    input_chunk(input_chunk&&) = default;

    input_chunk& operator=(input_chunk&& rhs)
    {
        if (file)
        {
            file = nullptr;
            filesystem::delete_file(path);
        }

        file = std::move(rhs.file);
        path = std::move(rhs.path);
        postings = std::move(rhs.postings);
        total_bytes = rhs.total_bytes;
        bytes_read = rhs.bytes_read;

        return *this;
    }

    operator bool() const
    {
        return static_cast<bool>(*file);
    }

    bool operator<(const input_chunk& other) const
    {
        return postings < other.postings;
    }

    void operator++()
    {
        bytes_read += postings.read_packed(*file);
    }
};
}

template <class Index>
void chunk_handler<Index>::merge_chunks()
{
    using input_chunk = detail::input_chunk<Index>;
    std::vector<input_chunk> to_merge;
    to_merge.reserve(chunks_.size());
    while (!chunks_.empty())
    {
        to_merge.emplace_back(chunks_.top().path());
        chunks_.pop();
    }

    printing::progress progress{
        " > Merging postings: ",
        std::accumulate(to_merge.begin(), to_merge.end(), 0ul,
                        [](uint64_t acc, const input_chunk& chunk)
                        {
                            return acc + chunk.total_bytes;
                        })};
    std::ofstream outfile{prefix_ + "/postings.index", std::ios::binary};
    unique_primary_keys_ = 0;

    uint64_t total_read
        = std::accumulate(to_merge.begin(), to_merge.end(), 0ul,
                          [](uint64_t acc, const input_chunk& chunk)
                          {
                              return acc + chunk.bytes_read;
                          });
    while (!to_merge.empty())
    {
        progress(total_read);
        ++(*unique_primary_keys_);

        std::sort(to_merge.begin(), to_merge.end());

        // gather all postings that match the smallest primary key, reading
        // a new postings from the corresponding file
        auto range = std::equal_range(to_merge.begin(), to_merge.end(),
                                      *to_merge.begin());
        auto min_pk = range.first->postings.primary_key();

        using count_t = typename index_pdata_type::count_t;
        std::vector<count_t> to_write;
        to_write.reserve(std::distance(range.first, range.second));
        std::for_each(range.first, range.second, [&](input_chunk& chunk)
                      {
                          to_write.emplace_back(chunk.postings.counts());
                          auto before = chunk.bytes_read;
                          ++chunk;
                          total_read += (chunk.bytes_read - before);
                      });

        // merge them all into one big counts vector
        count_t counts;
        std::for_each(to_write.begin(), to_write.end(), [&](count_t& pd)
                      {
                          std::move(pd.begin(), pd.end(),
                                    std::back_inserter(counts));
                          count_t{}.swap(pd);
                      });

        // write out the merged counts
        index_pdata_type output{std::move(min_pk)};
        output.set_counts(counts);
        count_t{}.swap(counts);
        output.write_packed(outfile);

        // remove all empty chunks from the input
        to_merge.erase(std::remove_if(to_merge.begin(), to_merge.end(),
                                      [](const input_chunk& chunk)
                                      {
                                          return !chunk;
                                      }),
                       to_merge.end());
    }
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
    if (!chunks_.empty())
        throw chunk_handler_exception{
            "merge not complete before final_size() called"};
    return filesystem::file_size(prefix_ + "/postings.index");
}

template <class Index>
uint32_t chunk_handler<Index>::size() const
{
    return chunk_num_.load();
}
}
}
