/**
 * @file jelinek_mercer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_JELINEK_MERCER_H_
#define META_JELINEK_MERCER_H_

#include "index/ranker/lm_ranker.h"
#include "index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * Implements the Jelinek-Mercer smoothed ranking model. This smoothing method
 * can be viewed as a linear interpolation between the query term probablity
 * and the collection term probability. The model parameter lambda is the
 * weighting of this interpolation.
 */
class jelinek_mercer : public language_model_ranker
{
  public:
    /// The identifier for this ranker.
    const static std::string id;

    /// Default value of lambda
    const static constexpr double default_lambda = 0.7;

    /**
     * @param lambda
     */
    jelinek_mercer(double lambda = default_lambda);

    /**
     * Calculates the smoothed probability of a term.
     * @param sd
     */
    double smoothed_prob(const score_data& sd) const override;

    /**
     * A document-dependent constant.
     * @param sd
     */
    double doc_constant(const score_data& sd) const override;

  private:
    /// the JM parameter
    const double lambda_;
};

/**
 * Specialization of the factory method used to create jelinek_mercer
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<jelinek_mercer>(const cpptoml::table&);
}
}

#endif
