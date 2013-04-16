/**
 * @file porter2.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
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
