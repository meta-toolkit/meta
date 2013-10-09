/**
 * @file chi_square.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _SELECT_CHI_SQUARED_H_
#define _SELECT_CHI_SQUARED_H_

#include <vector>
#include <utility>
#include "classify/feature_select/select_simple.h"
#include "corpus/document.h"

namespace meta {
namespace classify {

/**
 * Performs Chi square feature selection:
 * \f$ \chi^2(t, c_i) = \frac{(P(t,c_i)P(\overline{t},\overline{c_i})-P(t,\overline{c_i})P(\overline{t},c_i))^2}
 *  {P(t)P(\overline{t})P(c_i)P(\overline{c_i})} \f$
 */
class chi_square: public select_simple
{
    public:

        /**
         * Constructor.
         * @param docs The documents containing features
         */
        chi_square(const std::vector<corpus::document> & docs);

    private:

        /**
         * Calculates the chi-square score for one term.
         * @param termID 
         * @param label
         * @return the chi-square score
         */
        double calc_weight(term_id termID, const class_label & label) const;
};

}
}

#endif
