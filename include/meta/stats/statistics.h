/**
 * @file statistics.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_STATS_STATISTICS_H_
#define META_STATS_STATISTICS_H_

#include <cmath>
#include <type_traits>

#include "meta/config.h"

namespace meta
{
namespace stats
{

/**
 * Computation for \f$E_d[f(x)]\f$ where \f$d\f$ is specified by the
 * `dist` parameter and \f$f(x)\f$ is the `fun` parameter.
 *
 * `dist` must be a distribution over input type accepted by f.
 *
 * @param dist The distribution the expectation should be calculated
 * against
 * @param fun The function to compute the expectation of
 * @return the expected value of the function under the given distribution
 */
template <class Dist, class Fun>
double expected_value(Dist&& dist, Fun&& fun)
{
    using T = typename std::remove_reference<Dist>::type::event_type;
    auto total = 0.0;
    dist.each_seen_event(
        [&](const T& event) { total += dist.probability(event) * fun(event); });
    return total;
}

/**
 * Computes the entropy \f$H(X) = - \sum_{x \in X} p(x) \log_2 p(x)\f$.
 *
 * @param dist The distribution to compute the entropy over
 * @return the entropy of the supplied distribution
 */
template <class Dist>
double entropy(Dist&& dist)
{
    using T = typename std::remove_reference<Dist>::type::event_type;
    return expected_value(dist, [&](const T& event) {
        return -std::log2(dist.probability(event));
    });
}
}
}
#endif
