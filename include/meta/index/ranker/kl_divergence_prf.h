/**
 * @file kl_divergence_prf.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_KL_DIVERGENCE_PRF_H_
#define META_INDEX_KL_DIVERGENCE_PRF_H_

#include "meta/index/ranker/lm_ranker.h"
#include "meta/index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * Implements the two-component mixture model for pseudo-relevance
 * feedback in the KL-divergence retrieval model.
 *
 * @see http://dl.acm.org/citation.cfm?id=502654
 *
 * Required config parameters:
 * ~~~toml
 * [ranker]
 * method = "kl-divergence-prf"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * alpha = 0.5    # query interpolation parameter
 * lambda = 0.5   # mixture model interpolation parameter
 * k = 10         # number of feedback documents to retrieve
 * max-terms = 50 # maximum number of feedback terms to use
 *
 * [ranker.feedback]
 * method = "dirichlet-prior" # the initial model used to retrieve documents
 * # other parameters for that initial retrieval method
 * ~~~
 */
class kl_divergence_prf : public ranker
{
  public:
    /// Identifier for this ranker.
    const static util::string_view id;

    /// Default value of alpha, the query interpolation parameter
    const static constexpr float default_alpha = 0.5;

    /// Default value for lambda, the mixture model interpolation parameter
    const static constexpr float default_lambda = 0.5;

    /// Default value for k, the number of feedback documents to retrieve
    const static constexpr uint64_t default_k = 10;

    /**
     * Default value for max_terms, the number of feedback terms to
     * interpolate into the query model.
     */
    const static constexpr uint64_t default_max_terms = 50;

    kl_divergence_prf(std::shared_ptr<forward_index> fwd);

    kl_divergence_prf(std::shared_ptr<forward_index> fwd,
                      std::unique_ptr<language_model_ranker>&& initial_ranker,
                      float alpha = default_alpha,
                      float lambda = default_lambda, uint64_t k = default_k,
                      uint64_t max_terms = default_max_terms);

    kl_divergence_prf(std::istream& in);

    void save(std::ostream& out) const override;

    std::vector<search_result>
    rank(ranker_context& ctx, uint64_t num_results,
         const filter_function_type& filter) override;

  private:
    std::shared_ptr<forward_index> fwd_;
    std::unique_ptr<language_model_ranker> initial_ranker_;
    const float alpha_;
    const float lambda_;
    const uint64_t k_;
    const uint64_t max_terms_;
};

/**
 * Specialization of the factory method used to create kl_divergence_prf
 * rankers.
 */
template <>
std::unique_ptr<ranker>
make_ranker<kl_divergence_prf>(const cpptoml::table& global,
                               const cpptoml::table& local);
}
}
#endif
