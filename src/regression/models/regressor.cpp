/**
 * @file regressor.cpp
 * @author Chase Geigle
 */

#include "regression/models/regressor.h"

namespace meta
{
namespace regression
{

metrics regressor::test(dataset_view_type docs) const
{
    metrics m;
    for (const auto& instance : docs)
    {
        predicted_response predicted{predict(instance.weights)};
        response actual{docs.label(instance)};
        m.add(predicted, actual);
    }
    return m;
}
}
}
