/**
 * @file filesystem_test.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FILESYSTEM_TEST_H_
#define META_FILESYSTEM_TEST_H_

#include "test/unit_test.h"

namespace meta
{
namespace testing
{

/**
 * Runs all the filesystem tests.
 * @return the number of tests failed
 */
int filesystem_tests();

}
}
#endif
