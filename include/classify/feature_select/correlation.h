/**
 * @file correlation.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _SELECT_CORR_COEFF_H_
#define _SELECT_CORR_COEFF_H_

#include <vector>
#include <utility>
#include "classify/feature_select/select_simple.h"
#include "index/document.h"

namespace meta {
namespace classify {

/**
 * Selects features based on the correlation coefficient:
 * \f$ CC(t, c_i) =
 * \frac{P(t,c_i)P(\overline{t},\overline{c_i})-P(t,\overline{c_i})P(\overline{t},c_i)}
 *  {\sqrt{P(t)P(\overline{t})P(c_i)P(\overline{c_i})}} \f$
 */
class correlation: public select_simple
{
    public:

        /**
         * Constructor.
         * @param docs The documents containing features
         */
        correlation(const std::vector<index::document> & docs);

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
