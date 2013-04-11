/**
 * @file select_odds.h
 */

#ifndef _SELECT_ODDS_H_
#define _SELECT_ODDS_H_

#include <vector>
#include <utility>
#include "classify/select.h"
#include "index/document.h"

namespace meta {
namespace classify {

class select_odds_ratio: public feature_select
{
    public:

        /**
         * Constructor.
         */
        select_odds_ratio(const std::vector<index::Document> & docs);

        /**
         * Calculates important features via Odds Ratio 
         * @param docs The documents to extract features from
         * @return a vector of TermIDs sorted by importance
         */
        std::vector<std::pair<index::TermID, double>> select();

    private:
        /**
         * Calculates the chi-square score for one term.
         * @param termID 
         * @param label
         * @param classes
         * @return the odds ratio score
         */
        double calc_odds_ratio(index::TermID termID, const std::string & label);
};

}
}

#endif
