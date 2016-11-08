/**
 * @file unigram_mixture.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef UNIGRAM_MIXTURE_H_
#define UNIGRAM_MIXTURE_H_

#include <cassert>
#include <cstdint>
#include <limits>

#include "meta/config.h"
#include "meta/learn/dataset_view.h"
#include "meta/stats/multinomial.h"

namespace meta
{
namespace index
{
namespace feedback
{

/**
 * @param dset A collection of documents to fit a language model to
 * @return the maximum likelihood estimate for the language model
 */
stats::multinomial<term_id> maximum_likelihood(const learn::dataset_view& dset)
{
    stats::multinomial<term_id> model;
    for (const auto& inst : dset)
    {
        for (const auto& weight : inst.weights)
        {
            model.increment(weight.first, weight.second);
        }
    }
    return model;
}

struct training_options
{
    /// The fixed probability of the background model
    double lambda = 0.5;
    /// The maximum number of iterations for running EM
    uint64_t max_iter = 50;
    /// The convergence threshold as the relative change in log likelihood
    double delta = 1e-5;
};

/**
 * Learns the feedback model component of a two-component unigram mixture
 * model. The BackgroundModel is a unary function that returns the
 * probability of a term. This is used as the first component of the
 * mixture model, which has fixed probability options.lambda of being
 * selected. This function used the EM algorithm to fit the second
 * component language model and returns it.
 *
 * @param background The background language model
 * @param dset The feedback documents to fit the feedback model to
 * @param options The training options for the EM algorithm
 * @return the feedback model
 */
template <class BackgroundModel>
stats::multinomial<term_id>
unigram_mixture(BackgroundModel&& background, const learn::dataset_view& dset,
                const training_options& options = {})
{
    auto feedback = maximum_likelihood(dset);
    auto old_ll = std::numeric_limits<double>::lowest();
    auto relative_change = std::numeric_limits<double>::max();

    for (uint64_t i = 1;
         i <= options.max_iter && relative_change >= options.delta; ++i)
    {
        stats::multinomial<term_id> model;
        double ll = 0;

        for (const auto& inst : dset)
        {
            for (const auto& weight : inst.weights)
            {
                auto p_wc = background(weight.first);
                auto p_wf = feedback.probability(weight.first);

                auto numerator = options.lambda * p_wc;
                auto denominator = numerator + (1.0 - options.lambda) * p_wf;

                auto p_zw = numerator / denominator;

                model.increment(weight.first, (1.0 - p_zw) * weight.second);
                ll += weight.second * std::log(denominator);
            }
        }

        feedback = model;
        assert(ll > old_ll);
        relative_change = (old_ll - ll) / old_ll;
        old_ll = ll;
    }

    return feedback;
}
}
}
}
#endif
