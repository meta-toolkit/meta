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

template <class T>
template <class Analyzer>
void analyzer_factory<T>::register_analyzer()
{
    // this-> needed to find the add() method in dependent base class
    this->add(Analyzer::id, make_analyzer<Analyzer>);
}

template <class T>
analyzer_factory<T>::analyzer_factory()
{
    // built-in analyzers
    register_analyzer<ngram_word_analyzer<T>>();
}

template class analyzer_factory<uint64_t>;
template class analyzer_factory<double>;
}
}
