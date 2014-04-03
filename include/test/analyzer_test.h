/**
 * @file analyzer_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
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
/**
 * @param ana The anlyzer to use
 * @param doc The document to analyze
 * @param num_unique Number of unique terms
 * @param length Number of terms
 */
template <class Analyzer>
void check_analyzer_expected(Analyzer& ana, corpus::document doc,
                              uint64_t num_unique, uint64_t length);

/**
 * Test tokenization on documents with content.
 * @return the number of tests failed
 */
int content_tokenize();

/**
 * Test tokenization on documents that read content from files.
 * @return the number of tests failed
 */
int file_tokenize();

/**
 * Runs the analyzer tests.
 * @return the number of tests failed
 */
int analyzer_tests();
}
}

#endif
