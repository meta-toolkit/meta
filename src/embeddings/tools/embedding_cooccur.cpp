/**
 * @file embedding_cooccur.cpp
 * @author Chase Geigle
 *
 * This tool builds the weighted cooccurrence matrix for the GloVe training
 * method.
 */

#include <deque>

#include "cpptoml.h"
#include "meta/analyzers/analyzer.h"
#include "meta/corpus/corpus_factory.h"
#include "meta/embeddings/cooccurrence_counter.h"
#include "meta/io/filesystem.h"
#include "meta/logging/logger.h"

using namespace meta;

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
    auto window_size
        = embed_cfg->get_as<std::size_t>("window-size").value_or(15);
    auto max_ram = embed_cfg->get_as<std::size_t>("max-ram").value_or(4096)
                   * 1024 * 1024;
    auto merge_fanout
        = embed_cfg->get_as<std::size_t>("merge-fanout").value_or(8);
    auto break_on_tags
        = embed_cfg->get_as<bool>("break-on-tags").value_or(false);

    if (!filesystem::file_exists(vocab_filename))
    {
        LOG(fatal) << "Vocabulary file has not yet been generated, please do "
                      "this before building the cooccurrence table"
                   << ENDLG;
        return 1;
    }

    auto stream = analyzers::load_filters(*config, *embed_cfg);
    if (!stream)
    {
        LOG(fatal) << "Failed to find an ngram-word analyzer configuration in "
                   << argv[1] << ENDLG;
        return 1;
    }

    auto num_threads
        = embed_cfg->get_as<std::size_t>("num-threads")
              .value_or(std::max(1u, std::thread::hardware_concurrency()));

    {
        embeddings::cooccurrence_counter::configuration cooccur_config;
        cooccur_config.prefix = prefix;
        cooccur_config.max_ram = max_ram;
        cooccur_config.merge_fanout = merge_fanout;
        cooccur_config.window_size = window_size;
        cooccur_config.break_on_tags = break_on_tags;

        parallel::thread_pool pool{num_threads};
        embeddings::cooccurrence_counter counter{cooccur_config, pool};

        auto docs = corpus::make_corpus(*config);
        counter.count(*docs, *stream);
    }

    return 0;
}
