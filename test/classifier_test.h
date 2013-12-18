/**
 * @file classifier_test.h
 */

#ifndef _CLASSIFIER_TEST_H_
#define _CLASSIFIER_TEST_H_

#include <fstream>
#include <iostream>
#include "index_test.h"
#include "classify/classifier/all.h"
#include "caching/all.h"
#include "index/ranker/all.h"

namespace meta {
namespace testing {

    template <class Index, class Classifier>
    void check_cv(Index & idx, Classifier & c, double min_accuracy)
    {
        std::vector<doc_id> docs = idx.docs();
        classify::confusion_matrix mtx = c.cross_validate(docs, 5);
        ASSERT(mtx.accuracy() > min_accuracy);
        ASSERT(mtx.accuracy() < 100.0);
    }

    template <class Index, class Classifier>
    void check_split(Index & idx, Classifier & c, double min_accuracy)
    {
        // create splits
        std::vector<doc_id> docs = idx.docs();
        std::mt19937 gen(47);
        std::shuffle(docs.begin(), docs.end(), gen);
        size_t split_idx = docs.size() / 8;
        std::vector<doc_id> train_docs{docs.begin() + split_idx, docs.end()};
        std::vector<doc_id> test_docs{docs.begin(), docs.begin() + split_idx};

        // train and test
        c.train(train_docs);
        classify::confusion_matrix mtx = c.test(test_docs);
        ASSERT(mtx.accuracy() > min_accuracy);
        ASSERT(mtx.accuracy() < 100.0);
    }

    void classifier_tests()
    {
        system("/usr/bin/rm -rf ceeaus-inv ceeaus-fwd");
        create_config("file"); // TODO bug in knn when using line corpus

        auto i_idx = index::make_index<
            index::inverted_index, caching::no_evict_cache
        >("test-config.toml");
        auto f_idx = index::make_index<
            index::forward_index, caching::no_evict_cache
        >("test-config.toml");

        testing::run_test("naive-bayes-ceeaus-cv", 5, [&](){
            classify::naive_bayes nb{f_idx};
            check_cv(f_idx, nb, 0.86);
        });

        testing::run_test("naive-bayes-ceeaus-split", 5, [&](){
            classify::naive_bayes nb{f_idx};
            check_split(f_idx, nb, 0.84);
        });

        testing::run_test("knn-ceeaus-cv", 5, [&](){
            classify::knn<index::okapi_bm25> kn{i_idx, 10};
            check_cv(f_idx, kn, 0.90);
        });

        testing::run_test("knn-ceeaus-split", 5, [&](){
            classify::knn<index::okapi_bm25> kn{i_idx, 10};
            check_split(f_idx, kn, 0.88);
        });

        testing::run_test("sgd-ceeaus-cv", 5, [&](){
            classify::one_vs_all<classify::sgd<classify::loss::hinge>>
            hinge_sgd{f_idx};
            check_cv(f_idx, hinge_sgd, 0.94);
        });

        testing::run_test("sgd-ceeaus-split", 5, [&](){
            classify::one_vs_all<classify::sgd<classify::loss::hinge>>
            hinge_sgd{f_idx};
            check_split(f_idx, hinge_sgd, 0.90);
        });

        system("/usr/bin/rm -rf ceeaus-inv ceeaus-fwd");
    }

}
}

#endif
