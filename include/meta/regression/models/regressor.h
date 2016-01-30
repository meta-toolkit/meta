/**
 * @file regressor.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_REGRESSION_REGRESSOR_H_
#define META_REGRESSION_REGRESSOR_H_

#include "meta/regression/metrics.h"
#include "meta/regression/regression_dataset_view.h"

namespace meta
{
namespace regression
{

/**
 * A regressor uses a document's feature space to identify a real-valued
 * response that corresponds to it. This is a base class that defines an
 * interface for regression models.
 */
class regressor
{
  public:
    using instance_type = regression_dataset::instance_type;
    using feature_vector = learn::feature_vector;
    using dataset_view_type = regression_dataset_view;

    /**
     * Default destructor is virtual for polymorphic delete.
     */
    virtual ~regressor() = default;

    /**
     * Predicts the response for a specific instance based on the current
     * model.
     *
     * @param instance The instance to predict a response for
     * @return the predicted response for this instance
     */
    virtual double predict(const feature_vector& instance) const = 0;

    /**
     * Predicts responses for a collection of documents; this function
     * will make repeated calls to predict().
     *
     * @param docs The documents to predict
     * @return the regression metrics for that subset of the data
     */
    virtual metrics test(dataset_view_type docs) const;

    /**
     * Saves the model to the output stream.
     * @param out The stream to write the model to
     */
    virtual void save(std::ostream& out) const = 0;
};

/**
 * Performs k-fold cross-validation on a set of instances.
 *
 * @param config The configuration to use to create the regressor
 * @param docs Testing documents
 * @param k The number of folds
 * @return a vector of metrics objects, one for each fold
 */
std::vector<metrics> cross_validate(const cpptoml::table& config,
                                    regressor::dataset_view_type docs,
                                    std::size_t k);
}
}
#endif
