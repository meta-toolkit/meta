/**
 * @file select_chi_square.h
 */

#ifndef _SELECT_CHI_SQUARED_H_
#define _SELECT_CHI_SQUARED_H_

#include <vector>
#include <utility>
#include "classify/select.h"
#include "index/document.h"

namespace meta {
namespace classify {

class select_chi_square: public feature_select
{
    public:

        /**
         * Constructor.
         */
        select_chi_square(const std::vector<index::Document> & docs);

        /**
         * Calculates important features via Chi-square statistics (independence of
         * two random variables).
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
         * @return the chi-square score
         */
        double calc_chi_square(index::TermID termID, const std::string & label);
};

}
}

#endif
