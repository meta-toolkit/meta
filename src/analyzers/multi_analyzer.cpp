/**
 * @file multi_analyzer.cpp
 */

#include "analyzers/multi_analyzer.h"

namespace meta {
namespace analyzers {

multi_analyzer::multi_analyzer(
        std::vector<std::unique_ptr<analyzer>>&& toks):
    _analyzers{std::move(toks)}
{ /* nothing */ }

multi_analyzer::multi_analyzer(const multi_analyzer& other)
{
    _analyzers.reserve(other._analyzers.size());
    for (const auto & an : other._analyzers)
        _analyzers.emplace_back(an->clone());
}

void multi_analyzer::tokenize(corpus::document & doc)
{
    for(auto & tok: _analyzers)
        tok->tokenize(doc);
}

}
}
