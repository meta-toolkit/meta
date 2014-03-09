/**
 * @file porter2.cpp
 */

#include "stemmers/porter2.h"

namespace meta
{
namespace stemmers
{

void porter2::operator()(std::string& term) const
{
    Porter2Stemmer::trim(term);
    Porter2Stemmer::stem(term);
}
}
}
