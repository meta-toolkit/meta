/**
 * @file inverted_index_test.cpp
 * @author Sean Massung
 */

#include <fstream>

#include "bandit/bandit.h"
#include "meta/caching/all.h"
#include "cpptoml.h"
#include "create_config.h"
#include "meta/index/inverted_index.h"
#include "meta/index/postings_data.h"
#include "meta/io/filesystem.h"

using namespace bandit;
using namespace meta;

namespace {

template <class Index>
void check_ceeaus_expected(Index& idx) {
    AssertThat(idx.num_docs(), Equals(1008ul));
    AssertThat(idx.avg_doc_length(), EqualsWithDelta(127.634, 0.001));
    AssertThat(idx.unique_terms(), Equals(4224ul));

    std::ifstream in{"../data/ceeaus-metadata.txt"};
    uint64_t size;
    uint64_t unique;
    doc_id id{0};
    while (in >> size >> unique) {
        AssertThat(idx.doc_size(id), Equals(size));
        AssertThat(idx.unique_terms(id), Equals(unique));
        ++id;
    }

    // make sure there's exactly the correct amount
    AssertThat(id, Equals(idx.num_docs()));
}

template <class Index>
void check_term_id(Index& idx) {
    term_id t_id = idx.get_term_id("japanes");
    AssertThat(idx.doc_freq(t_id), Equals(69ul));

    term_id first;
    double second;
    std::ifstream in{"../data/ceeaus-term-count.txt"};
    auto pdata = idx.search_primary(t_id);
    for (auto& count : pdata->counts()) {
        in >> first;
        in >> second;
        AssertThat(first, Equals(count.first));
        AssertThat(second, EqualsWithDelta(count.second, 0.001));
    }
}
}

go_bandit([]() {

    describe("[inverted-index] from file config", []() {

        auto file_cfg = tests::create_config("file");

        it("should create the index", [&]() {
            filesystem::remove_all("ceeaus-inv");
            auto idx = index::make_index<index::inverted_index>(*file_cfg);
            check_ceeaus_expected(*idx);
        });

        it("should load the index", [&]() {
            auto idx = index::make_index<index::inverted_index>(*file_cfg);
            check_ceeaus_expected(*idx);
            check_term_id(*idx);
        });
    });

    describe("[inverted-index] from line config", []() {

        filesystem::remove_all("ceeaus-inv");
        auto line_cfg = tests::create_config("line");

        it("should create the index", [&]() {
            auto idx = index::make_index<index::inverted_index>(*line_cfg);
            check_ceeaus_expected(*idx);
        });

        it("should load the index", [&]() {
            auto idx = index::make_index<index::inverted_index,
                                         caching::splay_cache>(*line_cfg,
                                                               uint32_t{10000});
            check_ceeaus_expected(*idx);
            check_term_id(*idx);
            check_term_id(*idx); // twice to check splay_caching
        });
    });

    describe("[inverted-index] with caches", []() {

        auto line_cfg = tests::create_config("line");

        it("should be able to use dblru_cache", [&]() {
            auto idx = index::make_index<index::inverted_index,
                                         caching::default_dblru_cache>(
                *line_cfg, uint64_t{1000});
            check_term_id(*idx);
            check_term_id(*idx);
        });

        it("should be able to use no_evict_cache", [&]() {
            auto idx = index::make_index<index::inverted_index,
                                         caching::no_evict_cache>(*line_cfg);
            check_term_id(*idx);
            check_term_id(*idx);
        });

        it("should be able to use shard_cache", [&]() {
            auto idx = index::make_index<index::inverted_index,
                                         caching::splay_shard_cache>(
                *line_cfg, uint8_t{8});
            check_term_id(*idx);
            check_term_id(*idx);
        });
    });

    describe("[inverted-index] with zlib", []() {

        filesystem::remove_all("ceeaus-inv");
        auto gz_cfg = tests::create_config("gz");

        it("should create the index", [&]() {
            auto idx = index::make_index<index::inverted_index>(*gz_cfg);
            check_ceeaus_expected(*idx);
        });

        it("should load the index", [&]() {
            auto idx = index::make_index<index::inverted_index>(*gz_cfg);
            check_ceeaus_expected(*idx);
            check_term_id(*idx);
        });

    });

    filesystem::remove_all("ceeaus-inv");
});
