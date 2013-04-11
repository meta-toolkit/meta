/**
 * @file select_corr.h
 */

#ifndef _SELECT_CORR_COEFF_H_
#define _SELECT_CORR_COEFF_H_

#include <vector>
#include <utility>
#include "classify/select.h"
#include "index/document.h"

namespace meta {
namespace classify {

class select_corr_coeff: public feature_select
{
    public:

        /**
         * Constructor.
         */
        select_corr_coeff(const std::vector<index::Document> & docs);

        /**
         * Calculates important features via Correlation Coefficient
         * @param docs The documents to extract features from
         * @return a vector of TermIDs sorted by importance
         */
        std::vector<std::pair<index::TermID, double>> select();

    private:
        /**
         * Calculates the correlation coefficient score for one term.
         * @param termID 
         * @param label
         * @param classes
         * @return the correlation coefficient score
         */
        double calc_corr_coeff(index::TermID termID, const std::string & label);
};

}
}

#endif
