/**
 * @file regressor.cpp
 * @author Chase Geigle
 */

#include "meta/logging/logger.h"
#include "meta/regression/models/regressor.h"
#include "meta/regression/regressor_factory.h"

namespace meta
{
namespace regression
{

metrics regressor::test(dataset_view_type docs) const
{
    metrics_accumulator m;
    for (const auto& instance : docs)
    {
        predicted_response predicted{predict(instance.weights)};
        response actual{docs.label(instance)};
        m.add(predicted, actual);
    }
    return m;
}

std::vector<metrics> cross_validate(const cpptoml::table& config,
                                    regressor::dataset_view_type docs,
                                    std::size_t k)
{
    using diff_type = decltype(docs.begin())::difference_type;
    // docs may have some ordering by response variable, so make sure
    // things are shuffled to ensure each fold has a higher chance of being
    // representative
    docs.shuffle();

    std::vector<metrics> res;
    res.reserve(k);
    auto step_size = static_cast<diff_type>(docs.size() / k);
    for (std::size_t i = 0; i < k; ++i)
    {
        LOG(info) << "Cross-validating fold " << (i + 1) << "/" << k << ENDLG;
        regressor::dataset_view_type train_view{docs, docs.begin() + step_size,
                                                docs.end()};
        auto reg = make_regressor(config, train_view);
        regressor::dataset_view_type test_view{docs, docs.begin(),
                                               docs.begin() + step_size};
        res.emplace_back(reg->test(test_view));
        docs.rotate(docs.size() / k);
    }

    return res;
}
}
}
