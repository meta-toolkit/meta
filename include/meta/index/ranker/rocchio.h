/**
 * @file rocchio.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_ROCCHIO_H_
#define META_INDEX_ROCCHIO_H_

#include "meta/index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * Implements the Rocchio algorithm for pseudo-relevance feedback. This
 * implementation considers only positive documents for feedback. The top
 * `max_terms` from the centroid of the feedback set are selected according
 * to their weights provided by the wrapped ranker's `score_one` function.
 * These are then interpolated into the query in *count space*, and then
 * the results from running the wrapped ranker on the new query are
 * returned.
 *
 * Required config parameters:
 * ~~~toml
 * [ranker]
 * method = "rocchio"
 * ~~~
 *
 * Optional config parameters:
 * ~~~toml
 * alpha = 1.0    # original query weight parameter
 * beta = 1.0     # feedback document weight parameter
 * k = 10         # number of feedback documents to retrieve
 * max-terms = 50 # maximum number of feedback terms to use
 * [ranker.feedback]
 * method = # whatever ranker method you want to wrap
 * # other parameters for that ranker
 * ~~~
 *
 * @see https://en.wikipedia.org/wiki/Rocchio_algorithm
 */
class rocchio : public ranker
{
  public:
    /// Identifier for this ranker.
    const static util::string_view id;

    /// Default value of alpha, the original query weight parameter
    const static constexpr float default_alpha = 1.0f;

    /// Default value of beta, the positive document weight parameter
    const static constexpr float default_beta = 0.8f;

    /// Default value for k, the number of feedback documents to retrieve
    const static constexpr uint64_t default_k = 10;

    /**
     * Default value for max_terms, the number of new terms to add to the
     * new query.
     */
    const static constexpr uint64_t default_max_terms = 50;

    rocchio(std::shared_ptr<forward_index> fwd);

    rocchio(std::shared_ptr<forward_index> fwd,
            std::unique_ptr<ranker>&& initial_ranker,
            float alpha = default_alpha, float beta = default_beta,
            uint64_t k = default_k, uint64_t max_terms = default_max_terms);

    rocchio(std::istream& in);

    void save(std::ostream& out) const override;

    std::vector<search_result>
    rank(ranker_context& ctx, uint64_t num_results,
         const filter_function_type& filter) override;

  private:
    std::shared_ptr<forward_index> fwd_;
    std::unique_ptr<ranker> initial_ranker_;
    const float alpha_;
    const float beta_;
    const uint64_t k_;
    const uint64_t max_terms_;
};

/**
 * Specialization of the factory method used to create rocchio rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<rocchio>(const cpptoml::table& global,
                                             const cpptoml::table& local);
}
}
#endif
