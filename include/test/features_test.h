/**
 * @file features_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FEATURES_TEST_H_
#define META_FEATURES_TEST_H_

#include "test/unit_test.h"

namespace meta
{
namespace testing
{

/**
 * Runs all the feature selection tests.
 * @return the number of tests failed
 */
int features_tests();

}
}
#endif
