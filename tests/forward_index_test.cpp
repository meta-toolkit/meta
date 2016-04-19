/**
 * @file forward_index_test.cpp
 * @author Sean Massung
 */

#include <fstream>
#include <iostream>
#include <unordered_set>

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
    config->insert("index", "bcancer");

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

    // make sure we have all the class label info
    std::unordered_set<label_id> label_ids;
    label_ids.insert(idx.id(class_label{"japanese"}));
    label_ids.insert(idx.id(class_label{"chinese"}));
    label_ids.insert(idx.id(class_label{"english"}));
    AssertThat(label_ids.find(label_id{1}),
               Is().Not().EqualTo(label_ids.end()));
    AssertThat(label_ids.find(label_id{2}),
               Is().Not().EqualTo(label_ids.end()));
    AssertThat(label_ids.find(label_id{3}),
               Is().Not().EqualTo(label_ids.end()));

    std::unordered_set<class_label> labels;
    labels.insert(idx.class_label_from_id(label_id{1}));
    labels.insert(idx.class_label_from_id(label_id{2}));
    labels.insert(idx.class_label_from_id(label_id{3}));
    AssertThat(labels.find(class_label{"japanese"}),
               Is().Not().EqualTo(labels.end()));
    AssertThat(labels.find(class_label{"chinese"}),
               Is().Not().EqualTo(labels.end()));
    AssertThat(labels.find(class_label{"english"}),
               Is().Not().EqualTo(labels.end()));

    AssertThrows(std::out_of_range, idx.id(class_label{"bogus"}));
    AssertThrows(std::out_of_range, idx.class_label_from_id(label_id{0}));
    AssertThrows(std::out_of_range, idx.class_label_from_id(label_id{4}));
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
            filesystem::remove_all("ceeaus");
            ceeaus_forward_test(*file_cfg);
        });

        it("should load the index", [&]() { ceeaus_forward_test(*file_cfg); });

        it("should uninvert if specified", [&]() {
            filesystem::remove_all("ceeaus");
            file_cfg->insert("uninvert", true);
            ceeaus_forward_test(*file_cfg);
        });
    });

    describe("[forward-index] from line config", []() {
        auto line_cfg = tests::create_config("line");

        it("should create the index", [&]() {
            filesystem::remove_all("ceeaus");
            ceeaus_forward_test(*line_cfg);
        });

        it("should load the index", [&]() { ceeaus_forward_test(*line_cfg); });

        it("should uninvert if specified", [&]() {
            filesystem::remove_all("ceeaus");
            line_cfg->insert("uninvert", true);
            ceeaus_forward_test(*line_cfg);
        });

        it("should analyze a new document with the current analyzer", [&]() {
            auto cfg = tests::create_config("line");
            auto idx = index::make_index<index::forward_index>(*cfg);
            std::string text{"I think smoking smoking bad."};
            corpus::document doc;
            doc.content(text);
            auto fvector = idx->tokenize(doc);

            auto begin_sent = idx->get_term_id("<s>");
            auto end_sent = idx->get_term_id("</s>");
            auto bad = idx->get_term_id("bad");
            auto smoke = idx->get_term_id("smoke");
            auto think = idx->get_term_id("think");

            AssertThat(fvector.at(begin_sent), Equals(1));
            AssertThat(fvector.at(end_sent), Equals(1));
            AssertThat(fvector.at(bad), Equals(1));
            AssertThat(fvector.at(smoke), Equals(2));
            AssertThat(fvector.at(think), Equals(1));

            auto oov = idx->get_term_id("somelongrandomword");
            AssertThat(fvector.at(oov), Equals(0));
        });
    });

    describe("[forward-index] from svm config", []() {
        auto svm_cfg = create_libsvm_config();

        it("should create the index", [&]() {
            filesystem::remove_all("bcancer");
            bcancer_forward_test(*svm_cfg);
        });

        it("should load the index", [&]() { bcancer_forward_test(*svm_cfg); });

        it("should not tokenize new docs", [&](){
            auto cfg = create_libsvm_config();
            auto idx = index::make_index<index::forward_index>(*cfg);
            std::string text{"This should fail"};
            corpus::document doc;
            doc.content(text);
            AssertThrows(index::forward_index_exception, idx->tokenize(doc));
        });
    });

    describe("[forward-index] with zlib", []() {

        filesystem::remove_all("ceeaus");
        auto gz_cfg = tests::create_config("gz");

        it("should create the index", [&]() {
            filesystem::remove_all("ceeaus");
            ceeaus_forward_test(*gz_cfg);
        });

        it("should load the index", [&]() { ceeaus_forward_test(*gz_cfg); });

    });

    filesystem::remove_all("ceeaus");
    filesystem::remove_all("bcancer");
});
