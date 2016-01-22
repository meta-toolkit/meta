/**
 * @file analyzer_factory.cpp
 * @author Chase Geigle
 */

#include "meta/analyzers/analyzer_factory.h"
#include "meta/analyzers/all.h"

namespace meta
{
namespace analyzers
{

template <class Analyzer>
void analyzer_factory::register_analyzer()
{
    // this-> needed to find the add() method in dependent base class
    this->add(Analyzer::id, make_analyzer<Analyzer>);
}

analyzer_factory::analyzer_factory()
{
    // built-in analyzers
    register_analyzer<ngram_word_analyzer>();
}
}
}
