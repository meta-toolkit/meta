/**
 * @file parallel_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARALLEL_TEST_H_
#define META_PARALLEL_TEST_H_

#include <cmath>
#include <algorithm>
#include <numeric>

#include "test/unit_test.h"
#include "util/time.h"
#include "parallel/parallel_for.h"
#include "parallel/thread_pool.h"

namespace meta
{
namespace testing
{

template <class Type>
void hard_func(Type& x);

template <class Type>
void easy_func(Type& x);

/**
 * Assumes multi-core machine...
 */
int test_speed(std::vector<double>& v);

int test_correctness(std::vector<double>& v);

int test_threadpool();

int parallel_tests();
}
}

#endif
