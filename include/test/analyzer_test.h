/**
 * @file analyzer_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_ANALYZER_TEST_H_
#define META_ANALYZER_TEST_H_

#include <string>
#include "test/unit_test.h"
#include "analyzers/all.h"

namespace meta
{
namespace testing
{
template <class Tokenizer>
void check_analyzer_expected(Tokenizer& tok, corpus::document doc,
                              uint64_t num_unique, uint64_t length);

int content_tokenize();

int file_tokenize();

int analyzer_tests();
}
}

#endif
