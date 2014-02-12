/**
 * @file classifier_test.h
 */

#ifndef _CLASSIFIER_TEST_H_
#define _CLASSIFIER_TEST_H_

#include <fstream>
#include <iostream>
#include "inverted_index_test.h"
#include "classify/classifier/all.h"
#include "classify/kernel/all.h"
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

    void run_tests(const std::string & type)
    {
        using namespace classify;

        auto i_idx = index::make_index<
            index::inverted_index, caching::no_evict_cache
        >("test-config.toml");
        auto f_idx = index::make_index<
            index::forward_index, caching::no_evict_cache
        >("test-config.toml");

        testing::run_test("naive-bayes-cv-" + type, 5, [&](){
            naive_bayes nb{f_idx};
            check_cv(f_idx, nb, 0.86);
        });

        testing::run_test("naive-bayes-split-" + type, 5, [&](){
            naive_bayes nb{f_idx};
            check_split(f_idx, nb, 0.84);
        });

        testing::run_test("knn-cv-" + type, 5, [&](){
            knn<index::okapi_bm25> kn{i_idx, f_idx, 10};
            check_cv(f_idx, kn, 0.90);
        });

        testing::run_test("knn-split-" + type, 5, [&](){
            knn<index::okapi_bm25> kn{i_idx, f_idx, 10};
            check_split(f_idx, kn, 0.88);
        });

        testing::run_test("sgd-cv-" + type, 5, [&](){
            one_vs_all<sgd<loss::hinge>> hinge_sgd{f_idx};
            check_cv(f_idx, hinge_sgd, 0.94);
            one_vs_all<sgd<loss::perceptron>> perceptron{f_idx};
            check_cv(f_idx, perceptron, 0.90);
        });

        testing::run_test("sgd-split-" + type, 5, [&](){
            one_vs_all<sgd<loss::hinge>> hinge_sgd{f_idx};
            check_split(f_idx, hinge_sgd, 0.90);
            one_vs_all<sgd<loss::perceptron>> perceptron{f_idx};
            check_split(f_idx, perceptron, 0.85);
        });

        testing::run_test("winnow-cv-" + type, 5, [&](){
            winnow win{f_idx};
            check_cv(f_idx, win, 0.80);
        });

        testing::run_test("winnow-split-" + type, 5, [&](){
            winnow win{f_idx};
            check_split(f_idx, win, 0.79); // this is really low
        });

        testing::run_test("svm-wrapper-" + type, 5, [&](){
            auto config = cpptoml::parse_file("test-config.toml");
            auto mod_path = config.get_as<std::string>("libsvm-modules");
            if(!mod_path)
                throw std::runtime_error{"no path for libsvm-modules"};
            svm_wrapper svm{f_idx, *mod_path};
            check_cv(f_idx, svm, .80);
        });

        testing::run_test("dual-perceptron-cv-" + type, 10, [&](){
            auto dp_p = make_perceptron(f_idx, kernel::polynomial{});
            check_cv(f_idx, dp_p, 0.84);
            auto dp_rb = make_perceptron(f_idx, kernel::radial_basis{0.5});
            check_cv(f_idx, dp_rb, 0.84);
            auto dp_s = make_perceptron(f_idx, kernel::sigmoid{1, 1});
            check_cv(f_idx, dp_s, 0.84);
        });

        system("/usr/bin/rm -rf ceeaus-*");
    }

    void classifier_tests()
    {
        system("/usr/bin/rm -rf ceeaus-*");
        create_config("file");
        run_tests("file");
        create_config("line");
        run_tests("line");
    }

}
}

#endif
