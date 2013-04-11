/**
 * @file select_simple.h
 */

#ifndef _SELECT_SIMPLE_H_
#define _SELECT_SIMPLE_H_

#include <vector>
#include <utility>
#include "classify/select.h"
#include "index/document.h"

namespace meta {
namespace classify {

/**
 * Provides a framework for a simple, parallelizing feature selection metric.
 * Inheriting classes need to implement a calc_weight function.
 */
class select_simple: public feature_select
{
    public:

        /**
         * Constructor.
         */
        select_simple(const std::vector<index::Document> & docs);

        /**
         * Calculates important features via Chi-square statistics (independence of
         * two random variables).
         * @param docs The documents to extract features from
         * @return a vector of TermIDs sorted by importance
         */
        std::vector<std::pair<index::TermID, double>> select();

    protected:

        /**
         * Calculates the score for one term.
         * @param termID 
         * @param label
         * @return the score
         */
        virtual double calc_weight(index::TermID termID, const std::string & label) const = 0;
};

}
}

#endif
