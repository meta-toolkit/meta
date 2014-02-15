/**
 * @file multi_analyzer.cpp
 */

#include "analyzers/multi_analyzer.h"

namespace meta {
namespace analyzers {

multi_analyzer::multi_analyzer(
        const std::vector<std::shared_ptr<analyzer>> & toks):
    _analyzers{toks}
{ /* nothing */ }

void multi_analyzer::tokenize(corpus::document & doc)
{
    for(auto & tok: _analyzers)
        tok->tokenize(doc);
}

}
}
