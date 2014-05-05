#include <iostream>
#include <vector>

#include "cpptoml.h"
#include "analyzers/tree/phrase_analyzer.h"
#include "analyzers/token_stream.h"
#include "corpus/corpus.h"
#include "io/mmap_file.h"
#include "logging/logger.h"
#include "sequence/sequence.h"
#include "topics/window_lda.h"
#include "utf/segmenter.h"
#include "utf/utf.h"
#include "util/invertible_map.h"

using namespace meta;

std::string get_content(const corpus::document& doc)
{
    if (doc.contains_content())
        return utf::to_utf8(doc.content(), doc.encoding());

    io::mmap_file file{doc.path()};
    return utf::to_utf8({file.begin(), file.size()}, doc.encoding());
}

/**
 * Convert a corpus to a list of sequences, one sequence for each document.
 * Each sequence in the list represents a single document, and each
 * observation in that sequence encodes the words that occur in some
 * contiguous window in the document.
 */
topics::window_lda::dataset
    tree_yield_sequences(corpus::corpus* docs, analyzers::analyzer* ana)
{
    topics::window_lda::dataset dset;
    analyzers::phrase_analyzer ph_ana;

    while (docs->has_next())
    {
        auto doc = docs->next();
        ph_ana.tokenize(doc);
        auto phrases = ph_ana.phrases();

        sequence::sequence seq;
        for (const auto& phrase : phrases)
        {
            corpus::document d{"[NONE]", 0};
            d.content(phrase, docs->encoding());
            ana->tokenize(d);

            sequence::observation::feature_vector fvect;
            fvect.reserve(d.counts().size());
            for (const auto& p : d.counts())
                fvect.emplace_back(dset.vocab_map(p.first), p.second);
            std::sort(fvect.begin(), fvect.end());

            sequence::observation obs{"[NONE]"};
            obs.features(fvect);
            seq.add_observation(std::move(obs));
        }
        dset.add_sequence(std::move(seq));
    }

    return dset;
}

std::vector<std::vector<sequence::sequence>>
    chunker_sequences(corpus::corpus* docs)
{
    std::vector<std::vector<sequence::sequence>> sequences;
    sequences.reserve(docs->size());
    utf::segmenter segmenter;

    while (docs->has_next())
    {
        auto doc = docs->next();
        auto content = get_content(doc);
        segmenter.set_content(content);

        std::vector<sequence::sequence> seqs;
        for (const auto& sent : segmenter.sentences())
        {
            sequence::sequence seq;
            for (const auto& word : segmenter.words(sent))
            {
                auto wrd = segmenter.content(word);
                seq.add_symbol(wrd);
            }
            seqs.emplace_back(std::move(seq));
        }
        sequences.emplace_back(std::move(seqs));
    }

    return sequences;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto docs = corpus::corpus::load(argv[1]);

    auto config = cpptoml::parse_file(argv[1]);
    auto grp = config.get_group("window-lda");
    auto type = *grp->get_as<std::string>("type");
    auto prefix = *grp->get_as<std::string>("model-prefix");

    topics::window_lda::dataset dset;
    if (type == "tree-yield")
    {
        auto ana = analyzers::analyzer::load(config);
        dset = tree_yield_sequences(docs.get(), ana.get());
    }
    else
    {
        LOG(error) << "No valid type specified for windowing" << ENDLG;
        return 1;
    }

    uint64_t iters = *grp->get_as<int64_t>("max-iters");
    uint64_t burn_in = *grp->get_as<int64_t>("burn-in");
    auto alpha = *grp->get_as<double>("alpha");
    auto beta = *grp->get_as<double>("beta");
    uint64_t topics = *grp->get_as<int64_t>("topics");

    topics::window_lda lda{topics, alpha, beta};
    lda.learn(dset, burn_in, iters);
    lda.save(prefix, dset);
}
