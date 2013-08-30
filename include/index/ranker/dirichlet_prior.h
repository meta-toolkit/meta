/**
 * @file dirichlet_prior.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _DIRICHLET_PRIOR_H_
#define _DIRICHLET_PRIOR_H_

#include "index/ranker/lm_ranker.h"

namespace meta {
namespace index {

/**
 * Implements Bayesian smoothing with a Dirichlet prior.
 */
class dirichlet_prior: public language_model_ranker
{
    public:
        /**
         * @param mu
         */
        dirichlet_prior(double mu = 2000);

        /**
         * Calculates the smoothed probability of a term.
         * @param idx
         * @param t_id
         * @param d_id
         */
        double smoothed_prob(inverted_index & idx,
                             term_id t_id,
                             doc_id d_id) const;

        /**
         * A document-dependent constant.
         * @param d_id The id of the document to calculate the constant for
         * @param idx
         */
        double doc_constant(inverted_index & idx, doc_id d_id) const;

    private:
        /** the Dirichlet prior parameter*/
        const double _mu;
};

}
}

#endif
