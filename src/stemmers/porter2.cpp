/**
 * @file porter2.cpp
 */

#include "stemmers/porter2.h"

namespace meta {
namespace stemmers {

std::string porter2::stem( const std::string & to_stem ) const {
    return Porter2Stemmer::stem( Porter2Stemmer::trim( to_stem ) );
}

}
}
