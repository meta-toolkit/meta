/**
 * @file metrics.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_REGRESSION_METRICS_H_
#define META_REGRESSION_METRICS_H_

#include <vector>

#include "meta/config.h"
#include "meta/util/identifiers.h"

namespace meta
{
namespace regression
{

MAKE_NUMERIC_IDENTIFIER_UDL(predicted_response, double, _prsp)
MAKE_NUMERIC_IDENTIFIER_UDL(response, double, _rsp)

/**
 * Metrics computed from a metrics_accumulator.
 *
 * - mean absolute error (MAE) as mean_absolute_error
 * - mean squared error (MSE) as mean_squared_error
 * - median absolute error (MedAE) as median_absolute_error
 * - \f$R^2\f$-score (coefficient of determination) as r2_score
 */
struct metrics
{
    const double mean_absolute_error;
    const double median_absolute_error;
    const double mean_squared_error;
    const double r2_score;
};

/**
 * Contains information needed to compute several regression evaluation
 * metrics. This currently supports the following metrics:
 *
 * - mean absolute error (MAE)
 * - mean squared error (MSE)
 * - median absolute error (MedAE)
 * - \f$R^2\f$-score (coefficient of determination)
 *
 * Several metrics require knowledge about all of the previous response
 * pairs. Thus, this class uses linear space and queries are performed in
 * linear time in the number of pairs add()ed to the structure.
 */
class metrics_accumulator
{
  public:
    /**
     * Adjusts the metrics for a new (predicted, actual) response pair.
     *
     * @param predicted The predicted response
     * @param actual The actual response
     */
    void add(predicted_response predicted, response actual);

    /**
     * Computes the mean absolute error (MAE) as follows:
     * \f[
         MAE = \frac{1}{n} \sum_{i=1}^n |y_i - \hat{y_i}|
       \f]
     * where \f$y_i\f$ is the actual response value for the instance and
     * \f$\hat{y_i}\f$ is the predicted response value for the instance.
     * @return the mean absolute error (MAE)
     */
    double mean_absolute_error() const;

    /**
     * Computes the mean squared error (MSE) as follows:
     * \f[
         MSE = \frac{1}{n} \sum_{i=1}^n (y_i - \hat{y_i})^2
       \f]
     * where \f$y_i\f$ is the actual response value for the instance and
     * \f$\hat{y_i}\f$ is the predicted response value for the instance.
     *
     * @return the mean squared error (MSE)
     */
    double mean_squared_error() const;

    /**
     * Computes the median absolute error (MedAE) as follows:
     * \f[
         MedAE = median\{|y_1 - \hat{y_1}|, |y_2 - \hat{y_2}|, \ldots\}
       \f]
     * where \f$y_i\f$ is the actual response value for the instance and
     * \f$\hat{y_i}\f$ is the predicted response value for the instance.
     * This metric is insensitive to outliers.
     *
     * @return the median absolute error (MedAE).
     */
    double median_absolute_error() const;

    /**
     * Computes the coefficient of determination (or \f$R^2\f$ score) as
     * follows:
     * \f[
         R^2 = 1 - \frac{\sum_{i=1}^n (y_i - \hat{y_i})^2}
                        {\sum_{i=1}^n (y_i - \bar{y})^2}
     * \f]
     * where \f$y_i\f$ is the actual response value for the instance,
     * \f$\hat{y_i}\f$ is the predicted response value for the instance,
     * and \f$\bar{y} = \frac{1}{n} \sum_{i=1}^n y_i\f$.
     *
     * @return the coefficient of determination (\f$R^2\f$ score)
     */
    double r2_score() const;

    /**
     * Obtains a metrics object from this metrics_accumulator.
     */
    operator metrics() const;

  private:
    struct response_pair
    {
        double predicted;
        double actual;
    };
    std::vector<response_pair> responses_;
};
}
}
#endif
