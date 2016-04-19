/**
 * @file regression_test.cpp
 * @author Chase Geigle
 */

#include "bandit/bandit.h"
#include "meta/index/make_index.h"
#include "meta/regression/regressor_factory.h"
#include "meta/stats/running_stats.h"

using namespace bandit;
using namespace meta;

namespace {
void check_cv(const cpptoml::table& cfg,
              const regression::regression_dataset& dataset,
              regression::metrics expected) {

    regression::regression_dataset_view rdv{dataset, std::mt19937{47}};
    auto results = regression::cross_validate(cfg, rdv, 5);

    stats::running_stats mae;
    stats::running_stats med_ae;
    stats::running_stats mse;
    stats::running_stats r2;

    for (const auto& m : results) {
        mae.add(m.mean_absolute_error);
        med_ae.add(m.median_absolute_error);
        mse.add(m.mean_squared_error);
        r2.add(m.r2_score);
    }

    AssertThat(mae.mean(), Is().GreaterThan(0.0).And().LessThan(
                               expected.mean_absolute_error));
    AssertThat(med_ae.mean(), Is().GreaterThan(0.0).And().LessThan(
                                  expected.median_absolute_error));
    AssertThat(mse.mean(), Is().GreaterThan(0.0).And().LessThan(
                               expected.mean_squared_error));
    AssertThat(r2.mean(), Is().GreaterThan(expected.r2_score));
}
}

go_bandit([]() {
    auto config = cpptoml::make_table();
    config->insert("prefix", "../data");

    auto anas = cpptoml::make_table_array();
    auto ana = cpptoml::make_table();
    ana->insert("method", "libsvm");
    anas->push_back(ana);
    config->insert("analyzers", anas);

    config->insert("dataset", "housing");
    config->insert("corpus", "libsvm.toml");
    config->insert("index", "housing");

    filesystem::remove_all("housing");
    auto f_idx = index::make_index<index::forward_index>(*config);

    regression::regression_dataset dataset{
        f_idx, [&](doc_id did) {
            return *f_idx->metadata(did).get<double>("response");
        }};

    describe("[regression] sgd", [&]() {
        it("should create an SGD regressor with least-squares loss", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", "sgd");
            cfg->insert("loss", "least-squares");
            check_cv(*cfg, dataset, {3.91, 2.65, 32.85, 0.61});
        });

        it("should create an SGD regressor with huber loss", [&]() {
            auto cfg = cpptoml::make_table();
            cfg->insert("method", "sgd");
            cfg->insert("loss", "huber");
            check_cv(*cfg, dataset, {4.08, 2.48, 39.58, 0.54});
        });

        it("should create an SGD regressor with strong L1-regularization",
           [&]() {
               auto cfg = cpptoml::make_table();
               cfg->insert("method", "sgd");
               cfg->insert("loss", "least-squares");
               cfg->insert("l2-regularization", 0.0);
               cfg->insert("l1-regularization", 1e-4);
               check_cv(*cfg, dataset, {5.00, 3.17, 53.05, 0.37});
           });

        it("should create an SGD regressor with both L1 and L2 regularization",
           [&]() {
               auto cfg = cpptoml::make_table();
               cfg->insert("method", "sgd");
               cfg->insert("loss", "least-squares");
               cfg->insert("l2-regularization", 1e-5);
               cfg->insert("l1-regularization", 1e-5);
               check_cv(*cfg, dataset, {3.90, 2.69, 33.08, 0.60});
           });
    });

    f_idx = nullptr;
    filesystem::remove_all("housing");
});
