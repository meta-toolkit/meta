/**
 * @file libsvm_parser_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LIBSVM_PARSER_TEST_H_
#define META_LIBSVM_PARSER_TEST_H_

#include "test/unit_test.h"
#include "io/libsvm_parser.h"

namespace meta
{
namespace testing
{
/**
 * Tests libsvm strings containing a class label.
 */
void label();

/**
 * Tests libsvm strings not containing a class label.
 */
void no_label();

/**
 * Tests libsvm strings containing a bad class label.
 */
void bad_label();

/**
 * Tests libsvm strings containing a bad count data.
 */
void bad_counts();

/**
 * Runs all the libsvm parser tests.
 * @return the number of tests failed
 */
int libsvm_parser_tests();
}
}

#endif
