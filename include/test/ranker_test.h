/**
 * @file ranker_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_RANKER_TEST_H_
#define META_RANKER_TEST_H_

#include "test/unit_test.h"
#include "test/inverted_index_test.h"
#include "index/ranker/all.h"

namespace meta
{
namespace testing
{
/**
 * Queries an index with its own docs to ensure that the query is the first doc
 * returned.
 * @param r The ranker to test
 * @param idx The index to use
 */
template <class Ranker, class Index>
void test_rank(Ranker& r, Index& idx);

/**
 * Runs all the ranking tests.
 * @return the number of tests failed
 */
int ranker_tests();
}
}

#endif
