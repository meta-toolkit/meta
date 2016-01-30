/**
 * @file multi_analyzer.cpp
 */

#include "meta/analyzers/multi_analyzer.h"

namespace meta
{
namespace analyzers
{

multi_analyzer::multi_analyzer(std::vector<std::unique_ptr<analyzer>>&& toks)
    : analyzers_{std::move(toks)}
{
    /* nothing */
}

multi_analyzer::multi_analyzer(const multi_analyzer& other)
{
    analyzers_.reserve(other.analyzers_.size());
    for (const auto& an : other.analyzers_)
        analyzers_.emplace_back(an->clone());
}

void multi_analyzer::tokenize(const corpus::document& doc, featurizer& counts)
{
    for (auto& tok : analyzers_)
        tok->tokenize(doc, counts);
}
}
}
