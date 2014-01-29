/**
 * @file forward_index_test.h
 */

#ifndef _FORWARD_INDEX_TEST_H_
#define _FORWARD_INDEX_TEST_H_

#include <fstream>
#include <iostream>
#include "index/forward_index.h"
#include "inverted_index_test.h"  // for config file creation
#include "caching/all.h"
#include "cpptoml.h"

namespace meta {
namespace testing {

template <class Index>
void check_ceeaus_expected_fwd(Index& idx) {
    ASSERT(idx.num_docs() == 1008);
    ASSERT(idx.unique_terms() == 4003);

    std::ifstream in{"../data/ceeaus-metadata.txt"};
    uint64_t size;
    uint64_t unique;
    doc_id id{0};
    while (in >> size >> unique) {
        // don't care about unique terms per doc in forward_index (yet)
        ASSERT(idx.doc_size(id) == size);
        ++id;
    }

    // make sure there's exactly the correct amount
    ASSERT(id == idx.num_docs());
}

template <class Index>
void check_doc_id(Index & idx)
{
    doc_id d_id{47};
    term_id first;
    double second;
    std::ifstream in{"../data/ceeaus-doc-count.txt"};
    auto pdata = idx.search_primary(d_id);
    for(auto & count: pdata->counts())
    {
        in >> first;
        in >> second;
        ASSERT(first == count.first);
        ASSERT(second == count.second);
    }
}

void forward_index_tests() {
    create_config("file");

    testing::run_test("forward-index-build-file-corpus", 30, [&]() {
        system("/usr/bin/rm -rf ceeaus-*");
        auto idx =
            index::make_index<index::forward_index, caching::splay_cache>(
                "test-config.toml", uint32_t{10000});
        check_ceeaus_expected_fwd(idx);
        check_doc_id(idx);
    });

    testing::run_test("forward-index-read-file-corpus", 10, [&]() {
        auto idx =
            index::make_index<index::forward_index, caching::splay_cache>(
                "test-config.toml", uint32_t{10000});
        check_ceeaus_expected_fwd(idx);
        check_doc_id(idx);
        system("/usr/bin/rm -rf ceeaus-* test-config.toml");
    });

    create_config("line");

    testing::run_test("forward-index-build-line-corpus", 30, [&]() {
        system("/usr/bin/rm -rf ceeaus-*");
        auto idx =
            index::make_index<index::forward_index, caching::splay_cache>(
                "test-config.toml", uint32_t{10000});
        check_ceeaus_expected_fwd(idx);
        check_doc_id(idx);
    });

    testing::run_test("forward-index-read-line-corpus", 10, [&]() {
        auto idx =
            index::make_index<index::forward_index, caching::splay_cache>(
                "test-config.toml", uint32_t{10000});
        check_ceeaus_expected_fwd(idx);
        check_doc_id(idx);
        system("/usr/bin/rm -rf ceeaus-* test-config.toml");
    });
}
}
}

#endif
