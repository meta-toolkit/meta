/**
 * @file ngram_analyzer.cpp
 * @author Sean Massung
 */

#include "analyzers/ngram/ngram_analyzer.h"

namespace meta
{
namespace analyzers
{

template <class T>
ngram_analyzer<T>::ngram_analyzer(uint16_t n)
    : n_val_{n}
{
    /* nothing */
}

template <class T>
uint16_t ngram_analyzer<T>::n_value() const
{
    return n_val_;
}

template <class T>
std::string
    ngram_analyzer<T>::wordify(const std::deque<std::string>& words) const
{
    std::string result = "";
    for (auto& word : words)
        result += (word + "_");
    return result.substr(0, result.size() - 1);
}

template class ngram_analyzer<uint64_t>;
template class ngram_analyzer<double>;
}
}
