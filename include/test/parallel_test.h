/**
 * @file parallel_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
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
 * Assumes multi-core machine: tests speed; serial should be slower than
 * parallel.
 * @param v A vector of doubles to perform math ops on
 * @return positive number if failed
 */
int test_speed(std::vector<double>& v);

/**
 * Checks that each thread touches each index exactly once.
 * @param v A vector of doubles to perform math ops on
 * @return positive number if failed
 */
int test_correctness(std::vector<double>& v);

/**
 * Tests the threadpool.
 * @return the number of tests failed
 */
int test_threadpool();

/**
 * Tests all the parallel functions.
 * @return the number of tests failed
 */
int parallel_tests();
}
}

#endif
