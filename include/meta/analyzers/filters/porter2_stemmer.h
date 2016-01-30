/**
 * @file porter2_stemmer.h
 * @author Sean Massung
 * @date September 2012
 *
 * Implementation of
 * http://snowball.tartarus.org/algorithms/english/stemmer.html
 */

#ifndef PORTER2_STEMMER_H_
#define PORTER2_STEMMER_H_

#include <string>
#include "meta/util/string_view.h"

namespace meta
{
namespace analyzers
{
namespace filters
{
namespace porter2
{
/**
 * Stems a word in-place using the porter2 algorithm.
 * @param word The word to stem
 */
void stem(std::string& word);
}
}
}
}
#endif
