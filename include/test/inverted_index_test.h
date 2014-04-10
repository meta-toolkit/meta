/**
 * @file inverted_index_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INVERTED_INDEX_TEST_H_
#define META_INVERTED_INDEX_TEST_H_

#include <fstream>
#include <iostream>
#include "test/unit_test.h"
#include "index/inverted_index.h"
#include "index/postings_data.h"
#include "caching/all.h"
#include "cpptoml.h"

namespace meta
{
namespace testing
{
/**
 * Creates test-config.toml with the desired settings.
 * @param corpus_type line or file corpus
 */
void create_config(const std::string& corpus_type);

/**
 * Checks that ceeaus index was built correctly.
 * @param idx The index to check
 */
template <class Index>
void check_ceeaus_expected(Index& idx);

/**
 * Checks that the term info is consistent with the correct one
 * @param idx The index to check
 */
template <class Index>
void check_term_id(Index& idx);

/**
 * Runs the inverted index tests.
 * @return the number of tests failed
 */
int inverted_index_tests();
}
}

#endif
