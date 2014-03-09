/**
 * @file no_stemmer.cpp
 */

#include <algorithm>

#include "stemmers/no_stemmer.h"

namespace meta {
namespace stemmers {

void no_stemmer::operator()(std::string & term) const {
    std::transform(term.begin(), term.end(), term.begin(), ::tolower);
}

}
}
