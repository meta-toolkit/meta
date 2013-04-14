/**
 * @file select_chi_square.h
 */

#ifndef _SELECT_CHI_SQUARED_H_
#define _SELECT_CHI_SQUARED_H_

#include <vector>
#include <utility>
#include "classify/select_simple.h"
#include "index/document.h"

namespace meta {
namespace classify {

/**
 * Performs Chi square feature selection:
 * \f$ \chi^2(t, c_i) = \frac{(P(t,c_i)P(\overline{t},\overline{c_i})-P(t,\overline{c_i})P(\overline{t},c_i))^2}
 *  {P(t)P(\overline{t})P(c_i)P(\overline{c_i})} \f$
 */
class select_chi_square: public select_simple
{
    public:

        /**
         * Constructor.
         * @param docs The documents containing features
         */
        select_chi_square(const std::vector<index::Document> & docs);

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
