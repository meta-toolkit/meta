/**
 * @file ngram_analyzer.cpp
 * @author Sean Massung
 */

#include "analyzers/ngram/ngram_analyzer.h"

namespace meta {
namespace analyzers {

ngram_analyzer::ngram_analyzer(uint16_t n):
    _n_val{n}
{ /* nothing */ }

uint16_t ngram_analyzer::n_value() const
{
    return _n_val;
}

std::string ngram_analyzer::wordify(
        const std::deque<std::string> & words) const
{
    std::string result = "";
    for(auto & word: words)
        result += (word + "_");
    return result.substr(0, result.size() - 1);
}

}
}
