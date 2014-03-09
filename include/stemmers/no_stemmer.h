/**
 * @file no_stemmer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_NO_STEMMER_H_
#define _META_NO_STEMMER_H_

#include <string>

namespace meta
{
namespace stemmers
{

/**
 * A function object that performs no stemming other than conversion to
 * lower case. Typically used as a policy for tokenizers, but it is not
 * limited to this use case.
 */
struct no_stemmer
{
    void operator()(std::string& term) const;
};
}
}

#endif
