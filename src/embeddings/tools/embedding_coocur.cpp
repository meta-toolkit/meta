/**
 * @file embedding_coocur.cpp
 * @author Chase Geigle
 *
 * This tool builds the weighted coocurrence matrix for the GloVe training
 * method.
 */

#include <deque>

#include "cpptoml.h"
#include "meta/analyzers/all.h"
#include "meta/analyzers/token_stream.h"
#include "meta/corpus/corpus_factory.h"
#include "meta/hashing/probe_map.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/util/multiway_merge.h"
#include "meta/util/progress.h"
#include "meta/util/printing.h"

using namespace meta;

namespace meta
{
namespace hashing
{
template <class K, class V>
struct key_traits<std::pair<K, V>>
{
    static constexpr bool inlineable
        = key_traits<K>::inlineable && key_traits<V>::inlineable;

    constexpr static std::pair<K, V> sentinel()
    {
        return {key_traits<K>::sentinel(), key_traits<V>::sentinel()};
    }
};
}
}

struct coocur_record
{
    uint64_t target;
    uint64_t context;
    double weight;

    void merge_with(coocur_record&& other)
    {
        weight += other.weight;
    }

    template <class OutputStream>
    uint64_t write(OutputStream& os) const
    {
        auto bytes = io::packed::write(os, target);
        bytes += io::packed::write(os, context);
        bytes += io::packed::write(os, weight);
        return bytes;
    }
};

bool operator==(const coocur_record& a, const coocur_record& b)
{
    return std::tie(a.target, a.context) == std::tie(b.target, b.context);
}

bool operator!=(const coocur_record& a, const coocur_record& b)
{
    return !(a == b);
}

bool operator<(const coocur_record& a, const coocur_record& b)
{
    return std::tie(a.target, a.context) < std::tie(b.target, b.context);
}

class coocur_chunk_iterator
{
  public:
    using value_type = coocur_record;

    coocur_chunk_iterator(const std::string& filename)
        : path_{filename},
          input_{make_unique<std::ifstream>(filename, std::ios::binary)},
          total_bytes_{filesystem::file_size(filename)},
          bytes_read_{0}
    {
        ++(*this);
    }

    coocur_chunk_iterator() = default;
    coocur_chunk_iterator(coocur_chunk_iterator&&) = default;

    ~coocur_chunk_iterator()
    {
        if (input_)
        {
            input_ = nullptr;
            filesystem::delete_file(path_);
        }
    }

    coocur_chunk_iterator& operator++()
    {
        if (input_->get() == EOF)
            return *this;

        input_->unget();
        bytes_read_ += io::packed::read(*input_, record_.target);
        bytes_read_ += io::packed::read(*input_, record_.context);
        bytes_read_ += io::packed::read(*input_, record_.weight);
        return *this;
    }

    coocur_record& operator*()
    {
        return record_;
    }

    const coocur_record& operator*() const
    {
        return record_;
    }

    bool operator==(const coocur_chunk_iterator& other) const
    {
        if (!other.input_)
        {
            return !input_ || !static_cast<bool>(*input_);
        }
        else
        {
            return std::tie(path_, bytes_read_)
                   == std::tie(other.path_, other.bytes_read_);
        }
    }

    uint64_t total_bytes() const
    {
        return total_bytes_;
    }

    uint64_t bytes_read() const
    {
        return bytes_read_;
    }

  private:
    std::string path_;
    std::unique_ptr<std::ifstream> input_;
    coocur_record record_;
    uint64_t total_bytes_;
    uint64_t bytes_read_;
};

bool operator!=(const coocur_chunk_iterator& a, const coocur_chunk_iterator& b)
{
    return !(a == b);
}

class coocur_buffer
{
  public:
    coocur_buffer(std::size_t max_ram, util::string_view prefix)
        : max_bytes_{max_ram},
          prefix_{prefix.to_string()},
          coocur_{static_cast<std::size_t>(max_bytes_ / sizeof(count_t))}
    {
        // nothing
    }

    void flush()
    {
        LOG(info) << "Flushing buffer of size: "
                  << printing::bytes_to_units(coocur_.bytes_used()) << " with "
                  << coocur_.size() << " unique pairs" << ENDLG;

        {
            auto items = std::move(coocur_).extract();
            std::sort(items.begin(), items.end(),
                      [](const count_t& a, const count_t& b)
                      {
                          return a.first < b.first;
                      });

            std::ofstream output{prefix_ + "/chunk-"
                                     + std::to_string(chunk_num_),
                                 std::ios::binary};
            for (const auto& pr : items)
            {
                io::packed::write(output, pr.first.first);
                io::packed::write(output, pr.first.second);
                io::packed::write(output, pr.second);
            }
        }

        coocur_ = map_t{static_cast<std::size_t>(max_bytes_ / sizeof(count_t))};
        ++chunk_num_;
    }

    void operator()(uint64_t target, uint64_t context, double weight)
    {
        auto it = coocur_.find(std::make_pair(target, context));
        if (it == coocur_.end())
        {
            maybe_flush();
            coocur_[std::make_pair(target, context)] = weight;
        }
        else
        {
            it->value() += weight;
        }
    }

    std::size_t num_chunks() const
    {
        return chunk_num_;
    }

    uint64_t merge_chunks()
    {
        coocur_ = map_t{};
        std::vector<coocur_chunk_iterator> chunks;
        chunks.reserve(num_chunks());

        for (std::size_t i = 0; i < num_chunks(); ++i)
            chunks.emplace_back(prefix_ + "/chunk-" + std::to_string(i));

        std::ofstream output{prefix_ + "/coocur.bin", std::ios::binary};
        return util::multiway_merge(chunks.begin(), chunks.end(),
                                    [&](coocur_record&& record)
                                    {
                                        record.write(output);
                                    });
    }

  private:
    void maybe_flush()
    {
        // check if inserting a new coocurrence would cause a resize
        if (coocur_.next_load_factor() >= coocur_.max_load_factor())
        {
            // see if the newly resized table would fit in ram
            auto bytes_used = coocur_.bytes_used() * coocur_.resize_ratio();

            if (bytes_used >= max_bytes_)
            {
                flush();
            }
        }
    }

    using count_t = std::pair<std::pair<uint64_t, uint64_t>, double>;
    using map_t
        = meta::hashing::probe_map<std::pair<uint64_t, uint64_t>, double>;
    const std::size_t max_bytes_;
    const std::string prefix_;
    map_t coocur_;
    std::size_t chunk_num_ = 0;
};

std::unique_ptr<analyzers::token_stream>
make_stream(const cpptoml::table& config)
{
    std::unique_ptr<analyzers::token_stream> stream;
    auto analyzers = config.get_table_array("analyzers");
    for (const auto& group : analyzers->get())
    {
        auto method = group->get_as<std::string>("method");
        if (!method)
            continue;

        if (*method == analyzers::ngram_word_analyzer::id)
        {
            stream = analyzers::load_filters(config, *group);
            break;
        }
    }
    return stream;
}

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

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    // extract building parameters
    auto embed_cfg = config->get_table("embeddings");
    auto prefix = *embed_cfg->get_as<std::string>("prefix");
    auto vocab_filename = prefix + "/vocab.bin";
    auto window_size = static_cast<std::size_t>(
        embed_cfg->get_as<int64_t>("window-size").value_or(15));
    auto max_ram = static_cast<std::size_t>(
                       embed_cfg->get_as<int64_t>("max-ram").value_or(4096))
                   * 1024 * 1024;

    if (!filesystem::file_exists(vocab_filename))
    {
        LOG(fatal) << "Vocabulary file has not yet been generated, please do "
                      "this before building the coocurrence table"
                   << ENDLG;
        return 1;
    }

    auto vocab = load_vocab(vocab_filename);
    LOG(info) << "Loaded vocabulary of size " << vocab.size() << " occupying "
              << printing::bytes_to_units(vocab.bytes_used()) << ENDLG;

    if (max_ram <= vocab.bytes_used())
    {
        LOG(fatal) << "RAM limit too restrictive" << ENDLG;
        return 1;
    }

    max_ram -= vocab.bytes_used();
    if (max_ram < 1024 * 1024)
    {
        LOG(fatal) << "RAM limit too restrictive" << ENDLG;
        return 1;
    }

    auto stream = make_stream(*config);
    if (!stream)
    {
        LOG(fatal) << "Failed to find an ngram-word analyzer configuration in "
                   << argv[1] << ENDLG;
        return 1;
    }

    coocur_buffer coocur{max_ram, prefix};

    {
        auto docs = corpus::make_corpus(*config);
        printing::progress progress{" > Counting coocurrences: ", docs->size()};
        for (uint64_t i = 0; docs->has_next(); ++i)
        {
            progress(i);
            auto doc = docs->next();
            stream->set_content(analyzers::get_content(doc));

            std::deque<uint64_t> history;
            while (*stream)
            {
                auto tok = stream->next();

                if (tok == "<s>")
                {
                    history.clear();
                }
                else if (tok == "</s>")
                {
                    continue;
                }
                else
                {
                    // ignore out-of-vocabulary words
                    auto it = vocab.find(tok);
                    if (it == vocab.end())
                        continue;

                    auto tid = it->value();

                    // everything in history is a left-context of tid.
                    // Likewise, tid is a right-context of everything in
                    // history.
                    for (auto it = history.begin(), end = history.end();
                         it != end; ++it)
                    {
                        auto dist = std::distance(it, end);
                        coocur(tid, *it, 1.0 / dist);
                        coocur(*it, tid, 1.0 / dist);
                    }

                    history.push_back(tid);
                    if (history.size() > window_size)
                        history.pop_front();
                }
            }
        }
    }

    // flush any remaining elements
    coocur.flush();

    // merge all on-disk chunks
    auto uniq = coocur.merge_chunks();

    LOG(info) << "Coocurrence matrix elements: " << uniq << ENDLG;
    LOG(info) << "Coocurrence matrix size: "
              << printing::bytes_to_units(
                     filesystem::file_size(prefix + "/coocur.bin"))
              << ENDLG;

    return 0;
}
