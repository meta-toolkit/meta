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

class select_odds_ratio: public select_simple
{
    public:

        /**
         * Constructor.
         */
        select_odds_ratio(const std::vector<index::Document> & docs);

    private:
        /**
         * Calculates the chi-square score for one term.
         * @param termID 
         * @param label
         * @return the odds ratio score
         */
        double calc_weight(index::TermID termID, const std::string & label) const;
};

}
}

#endif
