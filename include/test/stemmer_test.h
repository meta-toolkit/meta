/**
 * @file stemmer_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_STEMMER_TEST_H_
#define _META_STEMMER_TEST_H_

#include <fstream>

#include "test/unit_test.h"
#include "stemmers/porter2.h"
#include "stemmers/no_stemmer.h"

namespace meta
{
namespace testing
{

template <class Stemmer>
void test_stem(Stemmer& stemmer, std::ifstream& in, bool do_stem);

int stemmer_tests();
}
}

#endif
