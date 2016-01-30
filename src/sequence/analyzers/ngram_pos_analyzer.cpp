/**
 * @file ngram_pos_analyzer.cpp
 */

#include <vector>
#include "cpptoml.h"
#include "meta/corpus/document.h"
#include "meta/sequence/sequence.h"
#include "meta/sequence/crf/tagger.h"
#include "meta/analyzers/token_stream.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"

namespace meta
{
namespace analyzers
{

const util::string_view ngram_pos_analyzer::id = "ngram-pos";

ngram_pos_analyzer::ngram_pos_analyzer(uint16_t n,
                                       std::unique_ptr<token_stream> stream,
                                       const std::string& crf_prefix)
    : base{n},
      stream_{std::move(stream)},
      crf_{std::make_shared<sequence::crf>(crf_prefix)},
      seq_analyzer_{[&]()
                    {
                        auto ana = sequence::default_pos_analyzer();
                        ana.load(crf_prefix);
                        return ana;
                    }()}
{
    // nothing
}

ngram_pos_analyzer::ngram_pos_analyzer(const ngram_pos_analyzer& other)
    : base{other.n_value()},
      stream_{other.stream_->clone()},
      crf_{other.crf_},
      seq_analyzer_{other.seq_analyzer_}
{
    // nothing
}

void ngram_pos_analyzer::tokenize(const corpus::document& doc,
                                  featurizer& counts)
{
    // first, get tokens
    stream_->set_content(get_content(doc));
    std::vector<sequence::sequence> sentences;
    sequence::sequence seq;

    // put tokens into sequences, excluding sentence markers
    while (*stream_)
    {
        auto next = stream_->next();
        if (next.empty() || next == " " || next == "<s>")
            continue;

        if (next == "</s>")
            sentences.emplace_back(std::move(seq));
        else
            seq.add_observation(
                {sequence::symbol_t{next}, sequence::tag_t{"[unknown]"}});
    }

    auto tagger = crf_->make_tagger();
    for (auto& seq : sentences)
    {
        // generate CRF features
        seq_analyzer_.analyze(seq);

        // POS-tag sentence
        tagger.tag(seq);

        // create ngrams
        for (size_t i = this->n_value() - 1; i < seq.size(); ++i)
        {
            std::string combined = seq_analyzer_.tag(seq[i].label());
            for (size_t j = 1; j < this->n_value(); ++j)
            {
                std::string next = seq_analyzer_.tag(seq[i - j].label());
                combined = next + "_" + combined;
            }

            counts(combined, 1ul);
        }
    }
}

template <>
std::unique_ptr<analyzer>
make_analyzer<ngram_pos_analyzer>(const cpptoml::table& global,
                                  const cpptoml::table& config)
{
    auto n_val = config.get_as<int64_t>("ngram");
    if (!n_val)
        throw analyzer_exception{
            "ngram size needed for ngram pos analyzer in config file"};

    auto crf_prefix = config.get_as<std::string>("crf-prefix");
    if (!crf_prefix)
        throw analyzer_exception{
            "ngram-pos analyzer must contain a prefix to a crf model"};

    auto filts = load_filters(global, config);
    return make_unique<ngram_pos_analyzer>(*n_val, std::move(filts),
                                           *crf_prefix);
}
}

namespace sequence
{
void register_analyzers()
{
    using namespace analyzers;
    register_analyzer<ngram_pos_analyzer>();
}
}
}
