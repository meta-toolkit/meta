/**
 * @file embedding_vocab.cpp
 * @author Chase Geigle
 *
 * This tool builds the vocabulary file for the other word embedding tools.
 */

#include "cpptoml.h"
#include "meta/analyzers/all.h"
#include "meta/analyzers/token_stream.h"
#include "meta/corpus/corpus_factory.h"
#include "meta/hashing/probe_map.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/util/progress.h"

using namespace meta;

namespace
{
    const std::string SGNS  = "sgns";
    const std::string GLOVE = "glove";
}

hashing::probe_map<std::string, uint64_t> reduce_vocab(hashing::probe_map<std::string, uint64_t> vocab,
                                                       unsigned int min_reduce)
{
    hashing::probe_map<std::string, uint64_t> temp_map;

    for (auto idx : vocab)
    {
        if (idx.value() > min_reduce)
        {
            temp_map[idx.key()] = idx.value();
        }
    }

    return temp_map;
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

    auto embed_cfg = config->get_table("embeddings");
    auto prefix = *embed_cfg->get_as<std::string>("prefix");
    auto vocab_filename = prefix + "/vocab.bin";

    auto vocab_cfg = embed_cfg->get_table("vocab");

    auto algorithm = embed_cfg->get_as<std::string>("algorithm").value_or(GLOVE);
    auto min_count = vocab_cfg->get_as<int64_t>("min-count").value_or(100);
    auto max_size = vocab_cfg->get_as<int64_t>("max-size")
                        .value_or(std::numeric_limits<int64_t>::max());

    // Override if we have an SGNS max size set
    max_size = vocab_cfg->get_as<int64_t>("max-size")
                        .value_or(max_size);

    auto sgns_cfg = vocab_cfg->get_table("sgns");
    auto min_reduce = sgns_cfg ? sgns_cfg->get_as<unsigned int>("min-reduce").value_or(1) : 1;
    auto min_reduce_threshold = sgns_cfg ? sgns_cfg->get_as<double>("min-reduce-threshold").value_or(0.7) : 0.7;

    auto stream = analyzers::load_filters(*config, *embed_cfg);
    hashing::probe_map<std::string, uint64_t> vocab;

    LOG(info) << "Using algorithm: " << algorithm << ENDLG;

    {
        auto docs = corpus::make_corpus(*config);
        printing::progress progress{" > Building vocabulary: ", docs->size()};
        for (uint64_t i = 0; docs->has_next(); ++i)
        {
            progress(i);
            auto doc = docs->next();
            stream->set_content(analyzers::get_content(doc));

            while (*stream)
            {
                ++vocab[stream->next()];

                if (algorithm == SGNS && (vocab.size() >= min_reduce_threshold * max_size))
                {
                    vocab = reduce_vocab(vocab, min_reduce);
                    min_reduce++;
                }
            }
        }
    }

    LOG(info) << "Found " << vocab.size() << " unique words" << ENDLG;

    LOG(progress) << "> Sorting vocab...\n" << ENDLG;
    auto items = std::move(vocab).extract();

    auto begin = std::begin(items);
    auto end = std::end(items);
    auto middle = end;
    if (items.size() > static_cast<std::size_t>(max_size))
        middle = begin + max_size;

    using count_t = std::pair<std::string, uint64_t>;

    // partial sort to avoid doing redundant work if our desired vocab
    // size is smaller than items.size()
    std::partial_sort(begin, middle, end, [](const count_t& a, const count_t& b)
                    {
                        return a.second > b.second;
                    });

    // truncate the vocabulary if needed: locate the position one past the
    // last element that is greater than the threshold value
    auto it
        = std::lower_bound(begin, middle, static_cast<uint64_t>(min_count) - 1,
                           [](const count_t& a, uint64_t thresh)
                           {
                               // comparison is reversed since we're
                               // working on reverse-sorted data
                               return a.second > thresh;
                           });

    auto size = static_cast<uint64_t>(std::distance(begin, it));

    LOG(info) << "Vocab truncated to size " << size << ENDLG;

    {
        filesystem::make_directory(prefix);
        std::ofstream output{vocab_filename, std::ios::binary};
        printing::progress progress{" > Writing vocab: ", size};

        io::packed::write(output, size);
        for (uint64_t i = 0; begin != it; ++begin, ++i)
        {
            progress(i);
            io::packed::write(output, begin->first);
            io::packed::write(output, begin->second);
        }
    }

    return 0;
}
