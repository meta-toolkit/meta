/**
 * @file no_stemmer.h
 */

#include <algorithm>

#include "stemmers/no_stemmer.h"

namespace meta {
namespace stemmers {

std::string no_stemmer::stem( const std::string & to_stem ) const {
    std::string result{ to_stem };
    std::transform( result.begin(), result.end(), result.begin(), ::tolower );
    return result;
}

}
}
