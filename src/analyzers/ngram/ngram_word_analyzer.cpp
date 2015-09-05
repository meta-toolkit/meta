/**
 * @file ngram_word_analyzer.cpp
 * @author Sean Massung
 */

#include <string>
#include <vector>

#include "cpptoml.h"
#include "corpus/document.h"
#include "analyzers/ngram/ngram_word_analyzer.h"
#include "analyzers/token_stream.h"

namespace meta
{
namespace analyzers
{

const std::string ngram_word_analyzer::id = "ngram-word";

ngram_word_analyzer::ngram_word_analyzer(uint16_t n,
                                         std::unique_ptr<token_stream> stream)
    : base{n}, stream_{std::move(stream)}
{
    // nothing
}

ngram_word_analyzer::ngram_word_analyzer(const ngram_word_analyzer& other)
    : base{other.n_value()}, stream_{other.stream_->clone()}
{
    // nothing
}

void ngram_word_analyzer::tokenize(corpus::document& doc)
{
    stream_->set_content(get_content(doc));
    std::deque<std::string> tokens;
    while (*stream_)
    {
        tokens.emplace_back(stream_->next());
        if (tokens.size() == n_value())
        {
            auto combined = std::move(tokens.front());
            tokens.pop_front();
            for (const auto& token : tokens)
                combined += "_" + token;

            doc.increment(combined, 1);
        }
    }
}

template <>
std::unique_ptr<analyzer>
    make_analyzer<ngram_word_analyzer>(const cpptoml::table& global,
                                       const cpptoml::table& config)
{
    auto n_val = config.get_as<int64_t>("ngram");
    if (!n_val)
        throw analyzer_exception{
            "ngram size needed for ngram word analyzer in config file"};

    auto filts = load_filters(global, config);
    return make_unique<ngram_word_analyzer>(*n_val, std::move(filts));
}
}
}
