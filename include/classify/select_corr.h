/**
 * @file select_corr.h
 */

#ifndef _SELECT_CORR_COEFF_H_
#define _SELECT_CORR_COEFF_H_

#include <vector>
#include <utility>
#include "classify/select_simple.h"
#include "index/document.h"

namespace meta {
namespace classify {

/**
 * Selects features based on the correlation coefficient:
 * \f$ CC(t, c_i) =
 * \frac{P(t,c_i)P(\overline{t},\overline{c_i})-P(t,\overline{c_i})P(\overline{t},c_i)}
 *  {\sqrt{P(t)P(\overline{t})P(c_i)P(\overline{c_i})}} \f$
 */
class select_corr_coeff: public select_simple
{
    public:

        /**
         * Constructor.
         * @param docs The documents containing features
         */
        select_corr_coeff(const std::vector<index::Document> & docs);

    private:
        /**
         * Calculates the correlation coefficient score for one term.
         * @param termID 
         * @param label
         * @return the correlation coefficient score
         */
        double calc_weight(term_id termID, const class_label & label) const;
};

}
}

#endif
