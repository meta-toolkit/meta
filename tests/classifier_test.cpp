/**
 * @file classifier_test.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <fstream>
#include <iostream>
#include <random>

#include "bandit/bandit.h"
#include "classifier_test_helper.h"
#include "cpptoml.h"

using namespace bandit;
using namespace meta;

namespace {
void run_tests(const std::string& index_type) {

    using namespace classify;
    using namespace tests;

    auto config = tests::create_config(index_type);
    auto i_idx = index::make_index<index::inverted_index>(*config);
    auto f_idx
        = index::make_index<index::forward_index, caching::no_evict_cache>(
            *config);

    std::string describe_msg{"[classifier] (multiclass) from " + index_type
                             + "index"};
    describe(describe_msg.c_str(), [&]() {

        it("should create naive bayes classifier with CV", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", naive_bayes::id.to_string());
            check_cv(f_idx, *cfg, 0.95);
        });

        it("should create naive-bayes classifier with train/test split", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", naive_bayes::id.to_string());
            check_split(f_idx, *cfg, 0.92);
        });

        it("should create KNN classifier with CV", [&]() {
            check_cv(f_idx, [&](multiclass_dataset_view docs) {
                return make_unique<knn>(std::move(docs), i_idx, 10,
                                        make_unique<index::okapi_bm25>());
            }, 0.93);
        });

        it("should create KNN classifier with train/test split", [&]() {
            check_split(f_idx, [&](multiclass_dataset_view docs) {
                return make_unique<knn>(std::move(docs), i_idx, 10,
                                        make_unique<index::okapi_bm25>());
            }, 0.89);
        });

        it("should create nearest centroid classifier with CV", [&]() {
            check_cv(f_idx, [&](multiclass_dataset_view docs) {
                return make_unique<nearest_centroid>(std::move(docs), i_idx);
            }, 0.91);
        });

        it("should create nearest centroid classifier with train/test "
           "split",
           [&]() {
               check_split(f_idx, [&](multiclass_dataset_view docs) {
                   return make_unique<nearest_centroid>(std::move(docs), i_idx);
               }, 0.85);
           });
    });

    describe_msg
        = "[classifier] ensemble methods from " + index_type + " index";
    describe(describe_msg.c_str(), [&]() {
        using namespace classify;
        using namespace tests;

        // configure a one-vs-all ensemble of hinge-loss sgd
        auto hinge_sgd_cfg = cpptoml::make_table();
        hinge_sgd_cfg->insert("method", one_vs_all::id.to_string());

        auto hinge_base_cfg = cpptoml::make_table();
        hinge_base_cfg->insert("method", sgd::id.to_string());
        hinge_base_cfg->insert("loss", learn::loss::hinge::id.to_string());
        hinge_sgd_cfg->insert("base", hinge_base_cfg);

        // configure a one-vs-one ensemble of hinge-loss sgd
        auto hinge_sgd_ovo = cpptoml::make_table();
        hinge_sgd_ovo->insert("method", one_vs_one::id.to_string());
        hinge_sgd_ovo->insert("base", hinge_base_cfg);

        // configure a one-vs-all ensemble of perceptron-loss sgd
        auto perc_sgd_cfg = cpptoml::make_table();
        perc_sgd_cfg->insert("method", one_vs_all::id.to_string());

        auto perc_base_cfg = cpptoml::make_table();
        perc_base_cfg->insert("method", sgd::id.to_string());
        perc_base_cfg->insert("loss", learn::loss::perceptron::id.to_string());

        perc_sgd_cfg->insert("base", perc_base_cfg);

        // configure a one-vs-one ensemble of perceptron-loss sgd
        auto perc_sgd_ovo = cpptoml::make_table();
        perc_sgd_ovo->insert("method", one_vs_one::id.to_string());
        perc_sgd_ovo->insert("base", perc_base_cfg);

        it("should run one-vs-all using SGD with CV", [&]() {
            check_cv(f_idx, *hinge_sgd_cfg, 0.94);
            check_cv(f_idx, *perc_sgd_cfg, 0.93);
        });

        it("should run one-vs-all using SGD with train/test split", [&]() {
            check_split(f_idx, *hinge_sgd_cfg, 0.91);
            check_split(f_idx, *perc_sgd_cfg, 0.90);
        });

        // disable l2 regularization and add a harsh l1 regularizer
        hinge_base_cfg->insert("l2-regularization", 0.0);
        hinge_base_cfg->insert("l1-regularization", 1e-4);

        it("should run one-vs-all using L1 SGD with CV",
           [&]() { check_cv(f_idx, *hinge_sgd_cfg, 0.88); });

        // enable both l1 and l2 regularization with rather harsh settings
        hinge_base_cfg->erase("l2-regularization");
        hinge_base_cfg->erase("l1-regularization");
        hinge_base_cfg->insert("l2-regularization", 1e-3);
        hinge_base_cfg->insert("l1-regularization", 1e-4);

        it("should run one-vs-all using L1 and L2 SGD with CV",
           [&]() { check_cv(f_idx, *hinge_sgd_cfg, 0.84); });

        hinge_base_cfg->erase("l2-regularization");
        hinge_base_cfg->erase("l1-regularization");

        it("should run one-vs-one using SGD with CV", [&]() {
            check_cv(f_idx, *hinge_sgd_ovo, 0.93);
            check_cv(f_idx, *perc_sgd_ovo, 0.91);
        });

        it("should run one-vs-one using SGD with train/test split", [&]() {
            check_split(f_idx, *hinge_sgd_ovo, 0.904);
            check_split(f_idx, *perc_sgd_ovo, 0.88);
        });

        it("should run logistic regression with CV", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", logistic_regression::id.to_string());
            check_cv(f_idx, *cfg, 0.89);
        });

        it("should run logistic regression with train/test split", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", logistic_regression::id.to_string());
            check_split(f_idx, *cfg, 0.87);
        });

        it("should run winnow with CV", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", winnow::id.to_string());
            check_cv(f_idx, *cfg, 0.85);
        });

        it("should run winnow with train/test split", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", winnow::id.to_string());
            check_split(f_idx, *cfg, 0.86);
        });

        it("should run CV using an even class split", [&]() {
            check_cv(f_idx, *hinge_sgd_cfg, 0.70, true);
            check_cv(f_idx, *perc_sgd_cfg, 0.70, true);
        });
    });

    describe("[classifier] SVM wrapper", [&]() {
        auto svm_cfg = cpptoml::make_table();
        svm_cfg->insert("method", svm_wrapper::id.to_string());
        auto mod_path = config->get_as<std::string>("libsvm-modules");
        if (!mod_path)
            throw std::runtime_error{"no path for libsvm-modules"};
        svm_cfg->insert("path", *mod_path);

        it("should run with CV", [&]() { check_cv(f_idx, *svm_cfg, 0.94); });

        it("should run with train/test split",
           [&]() { check_split(f_idx, *svm_cfg, 0.88); });
    });
}
}

go_bandit([]() {

    filesystem::remove_all("ceeaus");
    run_tests("line");

    filesystem::remove_all("ceeaus");
    run_tests("file");

    describe("[classifier] saving and loading model files", [&]() {
        using namespace classify;

        auto line_cfg = tests::create_config("line");
        auto i_idx = index::make_index<index::inverted_index>(*line_cfg);
        auto f_idx = index::make_index<index::forward_index>(*line_cfg);

        it("should save and load naive bayes models", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", naive_bayes::id.to_string());
            tests::run_save_load_single(f_idx, *cfg, 0.92);
        });

        it("should save and load KNN models", [&]() {
            tests::run_save_load_single(
                f_idx, [&](multiclass_dataset_view docs) {
                    return make_unique<knn>(std::move(docs), i_idx, 10,
                                            make_unique<index::okapi_bm25>());
                }, 0.89);
        });

        it("should save and load nearest centroid models", [&]() {
            tests::run_save_load_single(
                f_idx, [&](multiclass_dataset_view docs) {
                    return make_unique<nearest_centroid>(std::move(docs),
                                                         i_idx);
                }, 0.85);
        });

        // configure a one-vs-all ensemble of hinge-loss sgd
        auto hinge_sgd_cfg = cpptoml::make_table();
        hinge_sgd_cfg->insert("method", one_vs_all::id.to_string());

        auto hinge_base_cfg = cpptoml::make_table();
        hinge_base_cfg->insert("method", sgd::id.to_string());
        hinge_base_cfg->insert("loss", learn::loss::hinge::id.to_string());

        hinge_sgd_cfg->insert("base", hinge_base_cfg);

        // configure a one-vs-one ensemble of hinge-loss sgd
        auto hinge_sgd_ovo = cpptoml::make_table();
        hinge_sgd_ovo->insert("method", one_vs_one::id.to_string());
        hinge_sgd_ovo->insert("base", hinge_base_cfg);

        it("should save and load one-vs-all SGD models",
           [&]() { tests::run_save_load_single(f_idx, *hinge_sgd_cfg, 0.91); });

        it("should save and load one-vs-one SGD models", [&]() {
            tests::run_save_load_single(f_idx, *hinge_sgd_ovo, 0.904);
        });

        it("should save and load logistic regression models", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", logistic_regression::id.to_string());
            tests::run_save_load_single(f_idx, *cfg, 0.87);
        });

        it("should save and load winnow models", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", winnow::id.to_string());
            tests::run_save_load_single(f_idx, *cfg, 0.86);
        });

        it("should save and SVM wrapper models", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", svm_wrapper::id.to_string());
            auto mod_path = line_cfg->get_as<std::string>("libsvm-modules");
            if (!mod_path)
                throw std::runtime_error{"no path for libsvm-modules"};
            cfg->insert("path", *mod_path);
            tests::run_save_load_single(f_idx, *cfg, 0.88);
        });
    });

    filesystem::remove_all("ceeaus");

    describe("[classifier] confusion matrix", [&]() {

        // We have 3 classes {A, B, C} and get the following predictions:
        std::vector<std::pair<predicted_label, class_label>> preds;
        preds.emplace_back("A"_pl, "A"_cl);
        preds.emplace_back("B"_pl, "A"_cl);
        preds.emplace_back("C"_pl, "A"_cl);
        preds.emplace_back("B"_pl, "B"_cl);
        preds.emplace_back("B"_pl, "B"_cl);
        preds.emplace_back("B"_pl, "B"_cl);
        preds.emplace_back("A"_pl, "C"_cl);
        preds.emplace_back("A"_pl, "C"_cl);
        preds.emplace_back("A"_pl, "C"_cl);

        classify::confusion_matrix mtx;
        for (auto& pair : preds)
            mtx.add(pair.first, pair.second);

        const double delta = 0.000001;
        AssertThat(mtx.accuracy(), EqualsWithDelta(4.0 / 9, delta));

        it("should calculate precision", [&]() {
            AssertThat(mtx.precision("A"_cl), EqualsWithDelta(1.0 / 4, delta));
            AssertThat(mtx.precision("B"_cl), EqualsWithDelta(3.0 / 4, delta));
            AssertThat(mtx.precision("C"_cl), EqualsWithDelta(0.0, delta));
            AssertThat(mtx.precision(), EqualsWithDelta(1.0 / 3, delta));
        });

        it("should calculate recall", [&]() {
            AssertThat(mtx.recall("A"_cl), EqualsWithDelta(1.0 / 3, delta));
            AssertThat(mtx.recall("B"_cl), EqualsWithDelta(1.0, delta));
            AssertThat(mtx.recall("C"_cl), EqualsWithDelta(0.0, delta));
            AssertThat(mtx.recall(), EqualsWithDelta(4.0 / 9, delta));
        });

        it("should calculate F1", [&]() {
            AssertThat(mtx.f1_score("A"_cl), EqualsWithDelta(2.0 / 7, delta));
            AssertThat(mtx.f1_score("B"_cl), EqualsWithDelta(6.0 / 7, delta));
            AssertThat(mtx.f1_score("C"_cl), EqualsWithDelta(0.0, delta));
            AssertThat(mtx.f1_score(),
                       EqualsWithDelta((2.0 / 7 + 6.0 / 7) / 3, delta));
        });
    });
});
