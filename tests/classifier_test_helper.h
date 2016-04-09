/**
 * @file classifier_test_helper.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TESTS_CLASSIFIER_TEST_HELPER_H_
#define META_TESTS_CLASSIFIER_TEST_HELPER_H_

#include "create_config.h"
#include "meta/caching/all.h"
#include "meta/classify/classifier/all.h"
#include "meta/classify/kernel/all.h"
#include "meta/index/forward_index.h"
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/all.h"
#include "meta/learn/loss/all.h"

namespace meta {
namespace tests {

template <class Index, class Creator,
          class = typename std::enable_if<!std::is_same<
              typename std::decay<Creator>::type, cpptoml::table>::value>::type>
inline void check_cv(Index& idx, Creator&& creator, double min_accuracy,
                     bool even_split = false) {
    using namespace classify;

    multiclass_dataset dataset{idx};
    multiclass_dataset_view mcdv{dataset, std::mt19937_64{47}};

    auto mtx
        = cross_validate(std::forward<Creator>(creator), mcdv, 5, even_split);
    AssertThat(mtx.accuracy(),
               Is().GreaterThan(min_accuracy).And().LessThan(100.0));
}

template <class Index>
inline void check_cv(Index& idx, const cpptoml::table& config,
                     double min_accuracy, bool even_split = false) {
    using namespace classify;
    check_cv(idx, [&](multiclass_dataset_view docs) {
        return make_classifier(config, std::move(docs));
    }, min_accuracy, even_split);
}

using creation_fn = std::function<std::unique_ptr<classify::classifier>(
    classify::multiclass_dataset_view)>;

template <class Index>
inline std::unique_ptr<classify::classifier>
check_split(Index& idx, creation_fn creator, double min_accuracy) {
    using namespace classify;

    multiclass_dataset dataset{idx};
    multiclass_dataset_view mcdv{dataset, std::mt19937_64{47}};

    using diff_type = decltype(mcdv.begin())::difference_type;

    // create splits
    mcdv.shuffle();
    auto split_idx = static_cast<diff_type>(mcdv.size() / 8);

    multiclass_dataset_view train_docs{mcdv, mcdv.begin() + split_idx,
                                       mcdv.end()};
    multiclass_dataset_view test_docs{mcdv, mcdv.begin(),
                                      mcdv.begin() + split_idx};

    auto c = creator(train_docs);
    auto mtx = c->test(test_docs);
    AssertThat(mtx.accuracy(),
               Is().GreaterThan(min_accuracy).And().LessThan(100.0));

    return c;
}

template <class Index>
inline std::unique_ptr<classify::classifier>
check_split(Index& idx, const cpptoml::table& config, double min_accuracy) {
    using namespace classify;
    return check_split(idx, [&](multiclass_dataset_view docs) {
        return make_classifier(config, std::move(docs));
    }, min_accuracy);
}

template <class Index>
inline void check_split(Index& idx, const classify::classifier& cls,
                        double min_accuracy) {
    using namespace classify;

    multiclass_dataset dataset{idx};
    multiclass_dataset_view mcdv{dataset, std::mt19937_64{47}};

    using diff_type = decltype(mcdv.begin())::difference_type;

    // create splits
    mcdv.shuffle();
    auto split_idx = static_cast<diff_type>(mcdv.size() / 8);

    multiclass_dataset_view test_docs{mcdv, mcdv.begin(),
                                      mcdv.begin() + split_idx};

    auto mtx = cls.test(test_docs);
    AssertThat(mtx.accuracy(),
               Is().GreaterThan(min_accuracy).And().LessThan(100.0));
}

template <class CreationMethod>
inline void run_save_load_single(std::shared_ptr<index::forward_index> idx,
                                 CreationMethod&& creation,
                                 double min_accuracy) {
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
}
}
#endif
