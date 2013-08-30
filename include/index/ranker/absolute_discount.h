/**
 * @file absolute_discount.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _ABSOLUTE_DISCOUNT_H_
#define _ABSOLUTE_DISCOUNT_H_

#include "index/ranker/lm_ranker.h"

namespace meta {
namespace index {

/**
 * Implements the absolute discounting smoothing method.
 */
class absolute_discount: public language_model_ranker
{
    public:
        /**
         * @param delta 
         */
        absolute_discount(double delta = 0.7);

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
        /** the absolute discounting parameter */
        const double _delta;
};

}
}

#endif
