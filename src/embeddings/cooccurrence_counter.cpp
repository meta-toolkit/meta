/**
 * @file cooccurrence_counter.cpp
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/embeddings/cooccurrence_counter.h"
#include "meta/analyzers/analyzer.h"
#include "meta/embeddings/cooccur_iterator.h"
#include "meta/logging/logger.h"
#include "meta/util/printing.h"
#include "meta/util/progress.h"

namespace meta
{
namespace embeddings
{

class cooccurrence_buffer
{
  public:
    using count_t = std::pair<cooccurrence_key, double>;
    using map_t = meta::hashing::probe_map<cooccurrence_key, double>;

    cooccurrence_buffer(cooccurrence_counter* counter, std::size_t max_bytes,
                        const analyzers::token_stream& stream)
        : counter_{counter},
          max_bytes_{max_bytes},
          cooccurrences_{
              static_cast<std::size_t>(max_bytes_ / sizeof(count_t))},
          stream_{stream.clone()}
    {
        // nothing
    }

    cooccurrence_buffer(cooccurrence_buffer&&) = default;
    cooccurrence_buffer& operator=(cooccurrence_buffer&&) = default;

    ~cooccurrence_buffer()
    {
        flush();
        --counter_->num_tokenizing_;
    }

    void flush()
    {
        if (!cooccurrences_.empty())
        {
            std::lock_guard<std::mutex> lock{counter_->io_mutex_};
            printing::progress::clear();
            LOG(info) << "Flushing hash table of size: "
                      << printing::bytes_to_units(cooccurrences_.bytes_used())
                      << " with " << cooccurrences_.size() << " unique pairs"
                      << ENDLG;
        }

        {
            auto items = std::move(cooccurrences_).extract();
            std::sort(items.begin(), items.end(),
                      [](const count_t& a, const count_t& b) {
                          return a.first < b.first;
                      });

            counter_->flush_chunk(std::move(items));
        }

        cooccurrences_
            = map_t{static_cast<std::size_t>(max_bytes_ / sizeof(count_t))};
    }

    void operator()(uint64_t target, uint64_t context, double weight)
    {
        cooccurrence_key key{target, context};
        auto it = cooccurrences_.find(key);
        if (it == cooccurrences_.end())
        {
            maybe_flush();
            cooccurrences_[key] = weight;
        }
        else
        {
            it->value() += weight;
        }
    }

  private:
    void maybe_flush()
    {
        // check if inserting a new cooccurrence would cause a resize
        if (cooccurrences_.next_load_factor()
            >= cooccurrences_.max_load_factor())
        {
            // see if the newly resized table would fit in ram
            auto bytes_used
                = cooccurrences_.bytes_used() * cooccurrences_.resize_ratio();

            if (bytes_used >= max_bytes_)
            {
                flush();
            }
        }
    }

    friend class cooccurrence_counter;

    cooccurrence_counter* counter_;
    const std::size_t max_bytes_;
    map_t cooccurrences_;
    std::unique_ptr<analyzers::token_stream> stream_;
};

namespace
{
hashing::probe_map<std::string, uint64_t>
load_vocab(const std::string& filename)
{
    using map_type = hashing::probe_map<std::string, uint64_t>;

    std::ifstream input{filename, std::ios::binary};
    auto size = io::packed::read<uint64_t>(input);
    auto reserve_size = static_cast<std::size_t>(
        std::ceil(size / map_type::default_max_load_factor()));

    printing::progress progress{" > Loading vocab: ", size};
    map_type vocab{reserve_size};
    for (uint64_t tid{0}; tid < size; ++tid)
    {
        progress(tid);
        auto word = io::packed::read<std::string>(input);
        io::packed::read<uint64_t>(input);

        vocab[word] = tid;
    }

    return vocab;
}
}

cooccurrence_counter::cooccurrence_counter(configuration config,
                                           parallel::thread_pool& pool)
    : prefix_{std::move(config.prefix)},
      max_ram_{config.max_ram},
      merge_fanout_{config.merge_fanout},
      window_size_{config.window_size},
      break_on_tags_{config.break_on_tags},
      vocab_{load_vocab(prefix_ + "/vocab.bin")},
      pool_(pool)
{
    LOG(info) << "Loaded vocabulary of size " << vocab_.size() << " occupying "
              << printing::bytes_to_units(vocab_.bytes_used()) << ENDLG;

    if (vocab_.bytes_used() > max_ram_)
        throw cooccurrence_exception{"RAM limit too restrictive"};
    max_ram_ -= vocab_.bytes_used();
}

void cooccurrence_counter::count(corpus::corpus& docs,
                                 const analyzers::token_stream& stream)
{
    if (chunk_num_ != 0)
        throw cooccurrence_exception{
            "cooccurrence_counters may not be re-used"};

    num_tokenizing_ = pool_.size();
    printing::progress progress{" > Counting cooccurrences: ", docs.size()};
    corpus::parallel_consume(
        docs, pool_,
        [&]() {
            return cooccurrence_buffer{this, max_ram_ / pool_.size(), stream};
        },
        [&](cooccurrence_buffer& buffer, const corpus::document& doc) {
            {
                std::lock_guard<std::mutex> lock{io_mutex_};
                progress(doc.id());
            }

            buffer.stream_->set_content(analyzers::get_content(doc));

            std::deque<uint64_t> history;
            while (*buffer.stream_)
            {
                auto tok = buffer.stream_->next();

                if (tok == "<s>" && break_on_tags_)
                {
                    history.clear();
                }
                else if (tok == "</s>" && break_on_tags_)
                {
                    continue;
                }
                else
                {
                    // ignore out-of-vocabulary words
                    auto it = vocab_.find(tok);
                    if (it == vocab_.end())
                        continue;

                    auto tid = it->value();

                    // everything in history is a left-context of tid.
                    // Likewise, tid is a right-context of everything in
                    // history.
                    for (auto it = history.begin(), end = history.end();
                         it != end; ++it)
                    {
                        auto dist = std::distance(it, end);
                        buffer(tid, *it, 1.0 / dist);
                        buffer(*it, tid, 1.0 / dist);
                    }

                    history.push_back(tid);
                    if (history.size() > window_size_)
                        history.pop_front();
                }
            }
        });
}

void cooccurrence_counter::flush_chunk(memory_chunk_type&& chunk)
{
    std::unique_lock<std::mutex> lock{chunk_mutex_};

    if (!chunk.empty())
        memory_chunks_.emplace_back(std::move(chunk));

    ++num_pending_;

    // If this thread added the last expected in-memory chunk, it performs
    // the merging
    if (num_pending_ == num_tokenizing_)
    {
        memory_merge_chunks();
        --num_pending_;
        chunk_cond_.notify_all();
        lock.unlock();

        // co-opt this thread to start merging on-disk chunks if there are
        // enough to start the mergesort
        maybe_merge();
    }
    // otherwise, this thread will wait until the merger thread completes
    else
    {
        chunk_cond_.wait(lock, [&]() { return memory_chunks_.empty(); });
        --num_pending_;
    }
}

void cooccurrence_counter::memory_merge_chunks()
{
    if (!memory_chunks_.empty())
    {
        auto filename = prefix_ + "/chunk-" + std::to_string(chunk_num_++);
        uint64_t total_bytes = 0;
        {
            std::ofstream output{filename, std::ios::binary};
            printing::progress::clear();
            LOG(info) << "Merging " << memory_chunks_.size()
                      << " in-memory chunks..." << ENDLG;
            util::multiway_merge(memory_chunks_.begin(), memory_chunks_.end(),
                                 [&](cooccur_record&& record) {
                                     total_bytes
                                         += io::packed::write(output, record);
                                 },
                                 printing::no_progress_trait{});
        }

        if (total_bytes > 0)
        {
            chunks_.emplace(filename, total_bytes);
        }
        else
        {
            filesystem::delete_file(filename);
        }

        memory_chunks_.clear();
    }
}

void cooccurrence_counter::maybe_merge()
{
    std::unique_lock<std::mutex> lock{chunk_mutex_};
    if (chunks_.size() < merge_fanout_)
        return;

    --num_tokenizing_;

    std::vector<embeddings::destructive_cooccur_iterator> chunks;
    chunks.reserve(merge_fanout_);
    for (std::size_t i = 0; i < merge_fanout_; ++i)
    {
        chunks.emplace_back(chunks_.top().path);
        chunks_.pop();
    }

    auto filename = prefix_ + "/chunk-" + std::to_string(chunk_num_++);
    uint64_t total_bytes = 0;
    {
        std::ofstream output{filename, std::ios::binary};
        printing::progress::clear();
        LOG(info) << "Merging " << chunks.size() << " on-disk chunks..."
                  << ENDLG;
        lock.unlock();

        util::multiway_merge(chunks.begin(), chunks.end(),
                             [&](cooccur_record&& record) {
                                 total_bytes
                                     += io::packed::write(output, record);
                             },
                             printing::no_progress_trait{});
    }

    lock.lock();
    chunks_.emplace(filename, total_bytes);
    ++num_tokenizing_;
    printing::progress::clear();
    LOG(info) << "On-disk merge complete" << ENDLG;
}

cooccurrence_counter::~cooccurrence_counter()
{
    std::vector<embeddings::destructive_cooccur_iterator> chunks;

    chunks.reserve(chunks_.size());
    while (!chunks_.empty())
    {
        chunks.emplace_back(chunks_.top().path);
        chunks_.pop();
    }

    std::ofstream output{prefix_ + "/cooccur.bin", std::ios::binary};
    auto num_records = util::multiway_merge(
        chunks.begin(), chunks.end(),
        [&](cooccur_record&& record) { io::packed::write(output, record); });
    chunks.clear();

    LOG(info) << "Cooccurrence matrix elements: " << num_records << ENDLG;
    LOG(info) << "Cooccurrence matrix size: "
              << printing::bytes_to_units(
                     filesystem::file_size(prefix_ + "/cooccur.bin"))
              << ENDLG;
}
}
}
