/**
 * @file metrics.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <cmath>
#include <numeric>

#include "meta/regression/metrics.h"

namespace meta
{
namespace regression
{

namespace
{
template <class Range, class BinaryOperator>
double accumulate(Range&& range, BinaryOperator&& binop)
{
    return std::accumulate(std::begin(range), std::end(range), 0.0,
                           std::forward<BinaryOperator>(binop));
}
}

void metrics_accumulator::add(predicted_response predicted, response actual)
{
    responses_.emplace_back(response_pair{static_cast<double>(predicted),
                                          static_cast<double>(actual)});
}

double metrics_accumulator::mean_absolute_error() const
{
    return accumulate(responses_,
                      [](double accum, const response_pair& rp)
                      {
                          return accum + std::abs(rp.predicted - rp.actual);
                      })
           / responses_.size();
}

double metrics_accumulator::mean_squared_error() const
{
    return accumulate(responses_,
                      [](double accum, const response_pair& rp)
                      {
                          auto diff = rp.predicted - rp.actual;
                          return accum + diff * diff;
                      })
           / responses_.size();
}

double metrics_accumulator::median_absolute_error() const
{
    std::vector<double> errors(responses_.size());

    std::transform(std::begin(responses_), std::end(responses_),
                   std::begin(errors), [](const response_pair& rp)
                   {
                       return std::abs(rp.predicted - rp.actual);
                   });

    using diff_type = std::vector<double>::iterator::difference_type;
    auto half = static_cast<diff_type>(errors.size() / 2);
    std::nth_element(std::begin(errors), std::begin(errors) + half,
                     std::end(errors));

    if (half % 2 == 1)
        return errors[errors.size() / 2];

    std::nth_element(std::begin(errors), std::begin(errors) + half - 1,
                     std::begin(errors) + half);

    return (errors[errors.size() / 2] + errors[errors.size() / 2 - 1]) / 2.0;
}

double metrics_accumulator::r2_score() const
{
    auto sq_err
        = accumulate(responses_, [](double accum, const response_pair& rp)
                     {
                         auto diff = rp.predicted - rp.actual;
                         return accum + diff * diff;
                     });

    auto mean = accumulate(responses_,
                           [](double accum, const response_pair& rp)
                           {
                               return accum + rp.actual;
                           })
                / responses_.size();

    auto sq_diff_from_mean
        = accumulate(responses_, [=](double accum, const response_pair& rp)
                     {
                         auto diff = rp.actual - mean;
                         return accum + diff * diff;
                     });

    return 1 - (sq_err / sq_diff_from_mean);
}

metrics_accumulator::operator metrics() const
{
    return {mean_absolute_error(), median_absolute_error(),
            mean_squared_error(), r2_score()};
}
}
}
