/**
 * @file classifier_test.cpp
 * @author Sean Massung
 */

#include <random>

#include "test/classifier_test.h"
#include "classify/loss/all.h"

namespace meta
{
namespace testing
{

template <class Index, class Creator,
          class = typename std::
              enable_if<!std::is_same<typename std::decay<Creator>::type,
                                      cpptoml::table>::value>::type>
void check_cv(Index& idx, Creator&& creator, double min_accuracy)
{
    using namespace classify;

    multiclass_dataset dataset{idx};
    multiclass_dataset_view mcdv{dataset, std::mt19937_64{47}};

    auto mtx = cross_validate(std::forward<Creator>(creator), mcdv, 5);
    ASSERT_GREATER(mtx.accuracy(), min_accuracy);
    ASSERT_LESS(mtx.accuracy(), 100.0);
}

template <class Index>
void check_cv(Index& idx, const cpptoml::table& config, double min_accuracy)
{
    using namespace classify;
    check_cv(idx,
             [&](multiclass_dataset_view docs)
             {
                 return make_classifier(config, std::move(docs));
             },
             min_accuracy);
}

using creation_fn = std::function<std::unique_ptr<classify::classifier>(
    classify::multiclass_dataset_view)>;

template <class Index>
std::unique_ptr<classify::classifier>
    check_split(Index& idx, creation_fn creator, double min_accuracy)
{
    using namespace classify;

    multiclass_dataset dataset{idx};
    multiclass_dataset_view mcdv{dataset, std::mt19937_64{47}};

    // create splits
    mcdv.shuffle();
    size_t split_idx = mcdv.size() / 8;

    multiclass_dataset_view train_docs{mcdv, mcdv.begin() + split_idx,
                                       mcdv.end()};
    multiclass_dataset_view test_docs{mcdv, mcdv.begin(),
                                      mcdv.begin() + split_idx};

    auto c = creator(train_docs);
    auto mtx = c->test(test_docs);
    ASSERT_GREATER(mtx.accuracy(), min_accuracy);
    ASSERT_LESS(mtx.accuracy(), 100.0);

    return c;
}

template <class Index>
std::unique_ptr<classify::classifier>
    check_split(Index& idx, const cpptoml::table& config, double min_accuracy)
{
    using namespace classify;
    return check_split(idx,
                       [&](multiclass_dataset_view docs)
                       {
                           return make_classifier(config, std::move(docs));
                       },
                       min_accuracy);
}

template <class Index>
void check_split(Index& idx, const classify::classifier& cls,
                 double min_accuracy)
{
    using namespace classify;

    multiclass_dataset dataset{idx};
    multiclass_dataset_view mcdv{dataset, std::mt19937_64{47}};

    // create splits
    mcdv.shuffle();
    size_t split_idx = mcdv.size() / 8;

    multiclass_dataset_view test_docs{mcdv, mcdv.begin(),
                                      mcdv.begin() + split_idx};

    auto mtx = cls.test(test_docs);
    ASSERT_GREATER(mtx.accuracy(), min_accuracy);
    ASSERT_LESS(mtx.accuracy(), 100.0);
}

int run_tests(const std::string& type)
{
    using namespace classify;

    int num_failed = 0;

    // scope to ensure that the index objects are destroyed before trying
    // to delete their directory; this is needed for weirdness on NFS or
    // other filesystems that might lock opened files
    {
        auto config = create_config(type);
        auto i_idx = index::make_index<index::inverted_index>(*config);
        auto f_idx
            = index::make_index<index::forward_index, caching::no_evict_cache>(
                *config);

        num_failed += testing::run_test(
            "naive-bayes-cv-" + type, [&]()
            {
                auto cfg = cpptoml::make_table();
                cfg->insert("method", naive_bayes::id.to_string());
                check_cv(f_idx, *cfg, 0.95);
            });

        num_failed += testing::run_test(
            "naive-bayes-split-" + type, [&]()
            {
                auto cfg = cpptoml::make_table();
                cfg->insert("method", naive_bayes::id.to_string());
                check_split(f_idx, *cfg, 0.92);
            });

        num_failed += testing::run_test(
            "knn-cv-" + type, [&]()
            {
                check_cv(f_idx,
                         [&](multiclass_dataset_view docs)
                         {
                             return make_unique<knn>(
                                 std::move(docs), i_idx, 10,
                                 make_unique<index::okapi_bm25>());
                         },
                         0.93);
            });

        num_failed += testing::run_test(
            "knn-split-" + type, [&]()
            {
                check_split(f_idx,
                            [&](multiclass_dataset_view docs)
                            {
                                return make_unique<knn>(
                                    std::move(docs), i_idx, 10,
                                    make_unique<index::okapi_bm25>());
                            },
                            0.89);
            });

        num_failed += testing::run_test(
            "nearest-centroid-cv-" + type, [&]()
            {
                check_cv(f_idx,
                         [&](multiclass_dataset_view docs)
                         {
                             return make_unique<nearest_centroid>(
                                 std::move(docs), i_idx);
                         },
                         0.91);
            });

        num_failed += testing::run_test(
            "nearest-centroid-split-" + type, [&]()
            {
                check_split(f_idx,
                            [&](multiclass_dataset_view docs)
                            {
                                return make_unique<nearest_centroid>(
                                    std::move(docs), i_idx);
                            },
                            0.85);
            });

        // configure a one-vs-all ensemble of hinge-loss sgd
        auto hinge_sgd_cfg = cpptoml::make_table();
        hinge_sgd_cfg->insert("method", one_vs_all::id.to_string());

        auto hinge_base_cfg = cpptoml::make_table();
        hinge_base_cfg->insert("method", sgd::id.to_string());
        hinge_base_cfg->insert("loss", loss::hinge::id.to_string());

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
        perc_base_cfg->insert("loss", loss::perceptron::id.to_string());

        perc_sgd_cfg->insert("base", perc_base_cfg);

        // configure a one-vs-one ensemble of perceptron-loss sgd
        auto perc_sgd_ovo = cpptoml::make_table();
        perc_sgd_ovo->insert("method", one_vs_one::id.to_string());
        perc_sgd_ovo->insert("base", perc_base_cfg);

        num_failed
            += testing::run_test("ova-sgd-cv-" + type, [&]()
                                 {
                                     check_cv(f_idx, *hinge_sgd_cfg, 0.94);

                                     check_cv(f_idx, *perc_sgd_cfg, 0.93);
                                 });

        num_failed
            += testing::run_test("ova-sgd-split-" + type, [&]()
                                 {
                                     check_split(f_idx, *hinge_sgd_cfg, 0.91);
                                     check_split(f_idx, *perc_sgd_cfg, 0.90);
                                 });

        num_failed
            += testing::run_test("ovo-sgd-cv-" + type, [&]()
                                 {
                                     check_cv(f_idx, *hinge_sgd_ovo, 0.93);
                                     check_cv(f_idx, *perc_sgd_ovo, 0.91);
                                 });

        num_failed
            += testing::run_test("ovo-sgd-split-" + type, [&]()
                                 {
                                     check_split(f_idx, *hinge_sgd_ovo, 0.91);
                                     check_split(f_idx, *perc_sgd_ovo, 0.88);
                                 });

        num_failed += testing::run_test(
            "log-reg-cv-" + type, [&]()
            {
                auto cfg = cpptoml::make_table();
                cfg->insert("method", logistic_regression::id.to_string());
                check_cv(f_idx, *cfg, 0.91);
            });

        num_failed += testing::run_test(
            "log-reg-split-" + type, [&]()
            {
                auto cfg = cpptoml::make_table();
                cfg->insert("method", logistic_regression::id.to_string());
                check_split(f_idx, *cfg, 0.91);
            });

        num_failed += testing::run_test("winnow-cv-" + type, [&]()
                                        {
                                            auto cfg = cpptoml::make_table();
                                            cfg->insert("method",
                                                        winnow::id.to_string());
                                            check_cv(f_idx, *cfg, 0.85);
                                        });

        num_failed += testing::run_test("winnow-split-" + type, [&]()
                                        {
                                            auto cfg = cpptoml::make_table();
                                            cfg->insert("method",
                                                        winnow::id.to_string());
                                            check_split(f_idx, *cfg, 0.90);
                                        });

        num_failed += testing::run_test(
            "svm-wrapper-" + type, [&]()
            {
                auto svm_cfg = cpptoml::make_table();
                svm_cfg->insert("method", svm_wrapper::id.to_string());
                auto mod_path = config->get_as<std::string>("libsvm-modules");
                if (!mod_path)
                    throw std::runtime_error{"no path for libsvm-modules"};
                svm_cfg->insert("path", *mod_path);

                check_cv(f_idx, *svm_cfg, 0.94);
            });
    }

    filesystem::remove_all("ceeaus-inv");
    filesystem::remove_all("ceeaus-fwd");
    return num_failed;
}

template <class CreationMethod>
void run_save_load_single(std::shared_ptr<index::forward_index> idx,
                          CreationMethod&& creation, double min_accuracy)
{
    filesystem::remove_all("save-load-model");
    {
        auto c = check_split(idx, std::forward<CreationMethod>(creation),
                             min_accuracy);
        std::ofstream file{"save-load-model", std::ios::binary};
        c->save(file);
    }
    {
        std::ifstream file{"save-load-model", std::ios::binary};
        auto c = classify::load_classifier(file);
        check_split(idx, *c, min_accuracy);
    }
    filesystem::remove_all("save-load-model");
}

int run_load_save_tests()
{
    using namespace classify;
    int num_failed = 0;

    // scope to ensure that the index objects are destroyed before trying
    // to delete their directory; this is needed for weirdness on NFS or
    // other filesystems that might lock opened files
    {
        auto line_cfg = create_config("line");
        auto i_idx = index::make_index<index::inverted_index>(*line_cfg);
        auto f_idx = index::make_index<index::forward_index>(*line_cfg);

        num_failed += testing::run_test(
            "naive-bayes-save-load", [&]()
            {
                auto cfg = cpptoml::make_table();
                cfg->insert("method", naive_bayes::id.to_string());
                run_save_load_single(f_idx, *cfg, 0.92);
            });

        num_failed += testing::run_test(
            "knn-save-load", [&]()
            {
                run_save_load_single(f_idx,
                                     [&](multiclass_dataset_view docs)
                                     {
                                         return make_unique<knn>(
                                             std::move(docs), i_idx, 10,
                                             make_unique<index::okapi_bm25>());
                                     },
                                     0.89);
            });

        num_failed += testing::run_test(
            "nearest-centroid-save-load", [&]()
            {
                run_save_load_single(f_idx,
                                     [&](multiclass_dataset_view docs)
                                     {
                                         return make_unique<nearest_centroid>(
                                             std::move(docs), i_idx);
                                     },
                                     0.85);
            });

        // configure a one-vs-all ensemble of hinge-loss sgd
        auto hinge_sgd_cfg = cpptoml::make_table();
        hinge_sgd_cfg->insert("method", one_vs_all::id.to_string());

        auto hinge_base_cfg = cpptoml::make_table();
        hinge_base_cfg->insert("method", sgd::id.to_string());
        hinge_base_cfg->insert("loss", loss::hinge::id.to_string());

        hinge_sgd_cfg->insert("base", hinge_base_cfg);

        // configure a one-vs-one ensemble of hinge-loss sgd
        auto hinge_sgd_ovo = cpptoml::make_table();
        hinge_sgd_ovo->insert("method", one_vs_one::id.to_string());
        hinge_sgd_ovo->insert("base", hinge_base_cfg);

        num_failed += testing::run_test("ova-sgd-save-load", [&]()
                                        {
                                            run_save_load_single(
                                                f_idx, *hinge_sgd_cfg, 0.91);
                                        });

        num_failed += testing::run_test("ovo-sgd-save-load", [&]()
                                        {
                                            run_save_load_single(
                                                f_idx, *hinge_sgd_ovo, 0.91);
                                        });

        num_failed += testing::run_test(
            "log-reg-save-load", [&]()
            {
                auto cfg = cpptoml::make_table();
                cfg->insert("method", logistic_regression::id.to_string());
                run_save_load_single(f_idx, *cfg, 0.91);
            });

        num_failed += testing::run_test(
            "winnow-save-load", [&]()
            {
                auto cfg = cpptoml::make_table();
                cfg->insert("method", winnow::id.to_string());
                run_save_load_single(f_idx, *cfg, 0.90);
            });

        num_failed += testing::run_test(
            "svm-wrapper-save-load", [&]()
            {
                auto cfg = cpptoml::make_table();
                cfg->insert("method", svm_wrapper::id.to_string());
                auto mod_path = line_cfg->get_as<std::string>("libsvm-modules");
                if (!mod_path)
                    throw std::runtime_error{"no path for libsvm-modules"};
                cfg->insert("path", *mod_path);

                run_save_load_single(f_idx, *cfg, 0.88);
            });
    }

    filesystem::remove_all("ceeaus-inv");
    filesystem::remove_all("ceeaus-fwd");

    return num_failed;
}

int classifier_tests()
{
    int num_failed = 0;
    filesystem::remove_all("ceeaus-inv");
    filesystem::remove_all("ceeaus-fwd");
    num_failed += run_tests("file");
    num_failed += run_tests("line");
    num_failed += run_load_save_tests();
    return num_failed;
}
}
}
