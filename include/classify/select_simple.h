/**
 * @file select_simple.h
 */

#ifndef _SELECT_SIMPLE_H_
#define _SELECT_SIMPLE_H_

#include <vector>
#include <utility>
#include "classify/select.h"
#include "index/document.h"
#include "meta.h"

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
         * @param docs The documents containing features
         */
        select_simple(const std::vector<index::Document> & docs);

        /**
         * Calculates important features via Chi-square statistics (independence of
         * two random variables).
         * @param docs The documents to extract features from
         * @return a vector of term_ids sorted by importance
         */
        std::vector<std::pair<term_id, double>> select();

        /**
         * Performs feature selection on a collection of Documents, returning
         * each class's features sorted by usefulness.
         */
        std::unordered_map<class_label, std::vector<std::pair<term_id, double>>>
            select_by_class();

    protected:

        /**
         * Calculates the score for one term.
         * @param termID 
         * @param label
         * @return the score
         */
        virtual double calc_weight(term_id termID, const class_label & label) const = 0;
};

}
}

#endif
