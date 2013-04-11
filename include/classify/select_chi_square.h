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

class select_chi_square: public select_simple
{
    public:

        /**
         * Constructor.
         */
        select_chi_square(const std::vector<index::Document> & docs);

    private:

        /**
         * Calculates the chi-square score for one term.
         * @param termID 
         * @param label
         * @return the chi-square score
         */
        double calc_weight(index::TermID termID, const std::string & label) const;
};

}
}

#endif
