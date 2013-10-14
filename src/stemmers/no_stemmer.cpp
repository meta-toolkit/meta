/**
 * @file no_stemmer.h
 */

#include <algorithm>

#include "stemmers/no_stemmer.h"

namespace meta {
namespace stemmers {

std::string no_stemmer::operator()(const std::string & to_stem) const {
    std::string result{to_stem};
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

}
}
