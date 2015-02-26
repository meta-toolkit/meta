/**
 * @file dirichlet_prior.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_DIRICHLET_PRIOR_H_
#define META_DIRICHLET_PRIOR_H_

#include "index/ranker/lm_ranker.h"
#include "index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * Implements Bayesian smoothing with a Dirichlet prior.
 */
class dirichlet_prior : public language_model_ranker
{
  public:
    /// Identifier for this ranker.
    const static std::string id;

    /// Default value of mu
    const static constexpr double default_mu = 2000;

    /**
     * @param mu
     */
    dirichlet_prior(double mu = default_mu);

    /**
     * Calculates the smoothed probability of a term.
     * @param sd score_data for the current query
     */
    double smoothed_prob(const score_data& sd) const override;

    /**
     * A document-dependent constant.
     * @param sd score_data for the current query
     */
    double doc_constant(const score_data& sd) const override;

  private:
    /// the Dirichlet prior parameter
    const double mu_;
};

/**
 * Specialization of the factory method used to create dirichlet_prior
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<dirichlet_prior>(const cpptoml::table&);
}
}
#endif
