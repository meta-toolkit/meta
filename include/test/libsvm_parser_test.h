/**
 * @file libsvm_parser_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_LIBSVM_PARSER_TEST_H_
#define _META_LIBSVM_PARSER_TEST_H_

#include "test/unit_test.h"
#include "io/libsvm_parser.h"

namespace meta
{
namespace testing
{

void label();

void no_label();

void bad_label();

void bad_counts();

int libsvm_parser_tests();
}
}

#endif
