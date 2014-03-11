/**
 * @file forward_index_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
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

void create_libsvm_config();

template <class Index>
void check_bcancer_expected(Index& idx);

template <class Index>
void check_ceeaus_expected_fwd(Index& idx);

template <class Index>
void check_bcancer_doc_id(Index& idx);

template <class Index>
void check_ceeaus_doc_id(Index& idx);

void ceeaus_forward_test();

void bcancer_forward_test();

int forward_index_tests();

}
}

#endif
