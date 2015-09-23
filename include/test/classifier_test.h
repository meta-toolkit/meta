/**
 * @file classifier_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CLASSIFIER_TEST_H_
#define META_CLASSIFIER_TEST_H_

#include <fstream>
#include <iostream>
#include "test/unit_test.h"
#include "test/inverted_index_test.h"
#include "classify/classifier/all.h"
#include "classify/kernel/all.h"
#include "caching/all.h"
#include "index/ranker/all.h"

namespace meta
{
namespace testing
{
/**
 * Runs the classifier tests.
 * @param type The index type
 * @return the number of tests failed
 */
int run_tests(const std::string& type);

/**
 * Runs the classifier tests.
 * @return the number of tests failed
 */
int classifier_tests();

}
}

#endif
