/**
 * @file inverted_index_test.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
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

void create_config(const std::string& corpus_type);

template <class Index>
void check_ceeaus_expected(Index& idx);

template <class Index>
void check_term_id(Index& idx);

int inverted_index_tests();

}
}

#endif
