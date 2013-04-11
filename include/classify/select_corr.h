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

class select_corr_coeff: public select_simple
{
    public:

        /**
         * Constructor.
         */
        select_corr_coeff(const std::vector<index::Document> & docs);

    private:
        /**
         * Calculates the correlation coefficient score for one term.
         * @param termID 
         * @param label
         * @param classes
         * @return the correlation coefficient score
         */
        double calc_weight(index::TermID termID, const std::string & label) const;
};

}
}

#endif
