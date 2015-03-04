/**
 * @file analyzer_factory.cpp
 * @author Chase Geigle
 */

#include "analyzers/analyzer_factory.h"
#include "analyzers/all.h"

namespace meta
{
namespace analyzers
{

template <class Analyzer>
void analyzer_factory::register_analyzer()
{
    add(Analyzer::id, make_analyzer<Analyzer>);
}

analyzer_factory::analyzer_factory()
{
    // built-in analyzers
    register_analyzer<ngram_word_analyzer>();
    register_analyzer<libsvm_analyzer>();
}
}
}
