/**
 * @file forward_index_test.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FORWARD_INDEX_TEST_H_
#define META_FORWARD_INDEX_TEST_H_

#include <fstream>
#include <iostream>
#include "test/unit_test.h"
#include "index/forward_index.h"
#include "test/inverted_index_test.h" // for config file creation
#include "caching/all.h"
#include "cpptoml.h"

namespace meta
{
namespace testing
{
/**
 * Creates a test-config.toml with the desired settings.
 */
void create_libsvm_config();

/**
 * Asserts that the bcancer corpus was created correctly.
 * @param idx The index to use
 */
template <class Index>
void check_bcancer_expected(Index& idx);

/**
 * Asserts that the bcancer corpus was created correctly.
 * @param idx The index to use
 */
template <class Index>
void check_ceeaus_expected_fwd(Index& idx);

/**
 * Asserts that the ceeaus corpus was created correctly.
 * @param idx The index to use
 */
template <class Index>
void check_bcancer_doc_id(Index& idx);

/**
 * Asserts that the ceeaus corpus was created correctly.
 * @param idx The index to use
 */
template <class Index>
void check_ceeaus_doc_id(Index& idx);

/**
 * Runs the ceeaus forward index tests.
 */
void ceeaus_forward_test();

/**
 * Runs the bcancer forward index tests.
 */
void bcancer_forward_test();

/**
 * Runs all the forward_index tests.
 * @return the number of tests failed
 */
int forward_index_tests();
}
}

#endif
