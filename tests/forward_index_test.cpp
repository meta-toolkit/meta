/**
 * @file forward_index_test.cpp
 * @author Sean Massung
 */

#include <fstream>
#include <iostream>

#include "bandit/bandit.h"
#include "create_config.h"
#include "meta/caching/all.h"
#include "cpptoml.h"
#include "meta/index/forward_index.h"
#include "meta/index/postings_data.h"
#include "meta/io/filesystem.h"

using namespace bandit;
using namespace meta;

namespace {

std::shared_ptr<cpptoml::table> create_libsvm_config() {
    auto orig_config = cpptoml::parse_file("../config.toml");

    auto config = cpptoml::make_table();
    config->insert("prefix", *orig_config->get_as<std::string>("prefix"));
    config->insert("corpus", "libsvm.toml");
    config->insert("dataset", "breast-cancer");
    config->insert("forward-index", "bcancer-fwd");
    config->insert("inverted-index", "bcancer-inv");

    auto anas = cpptoml::make_table_array();
    auto ana = cpptoml::make_table();
    ana->insert("method", "libsvm");
    anas->push_back(ana);
    config->insert("analyzers", anas);

    return config;
}

template <class Index>
void check_bcancer_expected(Index& idx) {
    AssertThat(idx.num_docs(), Equals(683ul));
    AssertThat(idx.unique_terms(), Equals(10ul));

    std::ifstream in{"../data/bcancer-metadata.txt"};
    doc_id id{0};
    uint64_t size;
    while (in >> size) {
        AssertThat(idx.doc_size(doc_id{id}), Equals(size));
        ++id;
    }
    AssertThat(id, Equals(idx.num_docs()));
}

template <class Index>
void check_ceeaus_expected_fwd(Index& idx) {
    AssertThat(idx.num_docs(), Equals(1008ul));
    AssertThat(idx.unique_terms(), Equals(4224ul));

    std::ifstream in{"../data/ceeaus-metadata.txt"};
    uint64_t size;
    uint64_t unique;
    doc_id id{0};
    while (in >> size >> unique) {
        // don't care about unique terms per doc in forward_index (yet)
        AssertThat(idx.doc_size(id), Equals(size));
        ++id;
    }

    // make sure there's exactly the correct amount
    AssertThat(id, Equals(idx.num_docs()));
}

template <class Index>
void check_bcancer_doc_id(Index& idx) {
    doc_id d_id{47};
    term_id first;
    double second;
    std::ifstream in{"../data/bcancer-doc-count.txt"};
    auto pdata = idx.search_primary(d_id);
    for (auto& count : pdata->counts()) {
        in >> first;
        in >> second;
        AssertThat(first - 1, Equals(count.first)); // - 1 because libsvm format
        AssertThat(second, EqualsWithDelta(count.second, 0.001));
    }
}

template <class Index>
void check_ceeaus_doc_id(Index& idx) {
    doc_id d_id{47};
    term_id first;
    double second;
    std::ifstream in{"../data/ceeaus-doc-count.txt"};
    auto pdata = idx.search_primary(d_id);
    for (auto& count : pdata->counts()) {
        in >> first;
        in >> second;
        AssertThat(first, Equals(count.first));
        AssertThat(second, EqualsWithDelta(count.second, 0.001));
    }
}

void ceeaus_forward_test(const cpptoml::table& conf) {
    auto idx = index::make_index<index::forward_index, caching::splay_cache>(
        conf, uint32_t{10000});
    check_ceeaus_expected_fwd(*idx);
    check_ceeaus_doc_id(*idx);
}

void bcancer_forward_test(const cpptoml::table& conf) {
    auto idx = index::make_index<index::forward_index, caching::splay_cache>(
        conf, uint32_t{10000});
    check_bcancer_expected(*idx);
    check_bcancer_doc_id(*idx);
}
}

go_bandit([]() {

    describe("[forward-index] from file config", []() {
        auto file_cfg = tests::create_config("file");

        it("should create the index", [&]() {
            filesystem::remove_all("ceeaus-inv");
            filesystem::remove_all("ceeaus-fwd");
            ceeaus_forward_test(*file_cfg);
        });

        it("should load the index", [&]() { ceeaus_forward_test(*file_cfg); });

        it("should uninvert if specified", [&]() {
            filesystem::remove_all("ceeaus-inv");
            filesystem::remove_all("ceeaus-fwd");
            file_cfg->insert("uninvert", true);
            ceeaus_forward_test(*file_cfg);
        });
    });

    describe("[forward-index] from line config", []() {
        auto line_cfg = tests::create_config("line");

        it("should create the index", [&]() {
            filesystem::remove_all("ceeaus-inv");
            filesystem::remove_all("ceeaus-fwd");
            ceeaus_forward_test(*line_cfg);
        });

        it("should load the index", [&]() { ceeaus_forward_test(*line_cfg); });

        it("should uninvert if specified", [&]() {
            filesystem::remove_all("ceeaus-inv");
            filesystem::remove_all("ceeaus-fwd");
            line_cfg->insert("uninvert", true);
            ceeaus_forward_test(*line_cfg);
        });
    });

    describe("[forward-index] from svm config", []() {
        auto svm_cfg = create_libsvm_config();

        it("should create the index", [&]() {
            filesystem::remove_all("bcancer-fwd");
            bcancer_forward_test(*svm_cfg);
        });

        it("should load the index", [&]() { bcancer_forward_test(*svm_cfg); });
    });

#if META_HAS_ZLIB
    describe("[forward-index] with zlib", []() {

        filesystem::remove_all("ceeaus-fwd");
        auto gz_cfg = tests::create_config("gz");

        it("should create the index", [&]() {
            filesystem::remove_all("ceeaus-inv");
            filesystem::remove_all("ceeaus-fwd");
            ceeaus_forward_test(*gz_cfg);
        });

        it("should load the index", [&]() { ceeaus_forward_test(*gz_cfg); });

    });
#endif

    filesystem::remove_all("ceeaus-inv");
    filesystem::remove_all("ceeaus-fwd");
    filesystem::remove_all("bcancer-fwd");
});
