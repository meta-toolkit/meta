/**
 * @file select_odds.h
 */

#ifndef _SELECT_ODDS_H_
#define _SELECT_ODDS_H_

#include <vector>
#include <utility>
#include "classify/select_simple.h"
#include "index/document.h"

namespace meta {
namespace classify {

/**
 * Performs feature selection using odds ratios:
 * \f$ OR(t,c_i) = \log\frac{P(t|c_i)\cdot (1 - P(t|\overline{c_i}))}
 *      {(1 - P(t|c_i))\cdot P(t|\overline{c_i})}  \f$
 */
class select_odds_ratio: public select_simple
{
    public:

        /**
         * Constructor.
         * @param docs The documents containing features
         */
        select_odds_ratio(const std::vector<index::Document> & docs);

    private:
        /**
         * Calculates the odds ratio score for one term.
         * @param termID 
         * @param label
         * @return the odds ratio score
         */
        double calc_weight(term_id termID, const class_label & label) const;
};

}
}

#endif
