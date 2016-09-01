/**
 * @file running_stats.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_STATS_RUNNING_STATISTICS_H_
#define META_STATS_RUNNING_STATISTICS_H_

#include <cstddef>

#include "meta/config.h"

namespace meta
{
namespace stats
{

/**
 * Online computation for mean and standard deviation using Welford's
 * method as presented by Knuth.
 */
class running_stats
{
  public:
    /**
     * Constructs a blank accumulator for the running mean/variance
     * calculation.
     */
    running_stats();

    /**
     * Adds a value to the calculation.
     * @param value The value to be added
     */
    void add(double value);

    /**
     * @return the mean of all values add()ed
     */
    double mean() const;

    /**
     * @return the standard deviation of all values add()ed
     */
    double stddev() const;

    /**
     * @return the variance of all values add()ed
     */
    double variance() const;

  private:
    /// the current running mean
    double m_k_;
    /// the current running numerator for variance
    double s_k_;
    /// the total number of items seen thus far
    std::size_t num_items_;
};
}
}
#endif
