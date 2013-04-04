/**
 * @file porter2.h
 */

#include "stemmers/porter2_stemmer.h"

namespace meta {
namespace stemmers {

/**
 * A function object that uses the Porter2Stemmer library for stemming.
 * Typically used as a policy for tokenizers, but it is not limited to
 * just this use case.
 */
struct porter2 {
    std::string stem( const std::string & to_stem ) const;
};

}
}
