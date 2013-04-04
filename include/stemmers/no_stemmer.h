/**
 * @file no_stemmer.h
 */

#include <string>

namespace meta {
namespace stemmers {
    
/**
 * A function object that performs no stemming other than conversion to
 * lower case. Typically used as a policy for tokenizers, but it is not
 * limited to this use case.
 */
struct no_stemmer {
    std::string stem( const std::string & to_stem ) const;
};

}
}
