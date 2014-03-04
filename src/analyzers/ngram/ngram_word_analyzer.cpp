/**
 * @file ngram_word_analyzer.cpp
 * @author Sean Massung
 */

#include <string>
#include <vector>

#include "corpus/document.h"
#include "analyzers/ngram/ngram_word_analyzer.h"
#include "analyzers/token_stream.h"

namespace meta {
namespace analyzers {

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

void ngram_word_analyzer::tokenize(corpus::document & doc)
{
    // first, get tokens
    stream_->set_content(get_content(doc));
    std::vector<std::string> tokens;
    while (*stream_)
        tokens.push_back(stream_->next());

    // second, create ngrams from them
    for(size_t i = n_value() - 1; i < tokens.size(); ++i)
    {
        std::string combined = tokens[i];
        for(size_t j = 1; j < n_value(); ++j)
            combined = tokens[i - j] + "_" + combined;

        doc.increment(combined, 1);
    }
}

}
}
