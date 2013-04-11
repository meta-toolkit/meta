/**
 * @file select_info_gain.h
 */

#ifndef _SELECT_INFO_GAIN_H_
#define _SELECT_INFO_GAIN_H_

#include <vector>
#include <utility>
#include "classify/select.h"
#include "index/document.h"

namespace meta {
namespace classify {

class select_info_gain: public feature_select
{
    public:

        /**
         * Constructor.
         */
        select_info_gain(const std::vector<index::Document> & docs);

        /**
         * Calculates important features based on information gain; that is, the
         * reduction of entropy by knowing the existance or absense of a term in a
         * document.
         * @param docs The documents to extract features from
         * @return a vector of TermIDs sorted by importance
         */
        std::vector<std::pair<index::TermID, double>> select();

    private:
        /**
         * @param termID
         * @param label
         * @param total_classes
         * @param total_terms
         * @param classes
         */
        double calc_info_gain(index::TermID termID, const std::string & label);
};

}
}

#endif
