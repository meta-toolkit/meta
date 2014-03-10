/**
 * @file ranker_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
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

template <class Ranker, class Index>
void test_rank(Ranker& r, Index& idx);

int ranker_tests();
}
}

#endif
