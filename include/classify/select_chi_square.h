/**
 * @file select_chi_square.h
 */

#ifndef _SELECT_CHI_SQUARED_H_
#define _SELECT_CHI_SQUARED_H_

#include <vector>
#include <utility>
#include "index/document.h"

namespace classify { namespace feature_select {

    /**
     * Calculates important features via Chi-square statistics (independence of
     * two random variables).
     * @param docs The documents to extract features from
     * @return a vector of TermIDs sorted by importance
     */
    std::vector<std::pair<TermID, double>> chi_square(const std::vector<Document> & docs);

    /**
     * Calculates the chi-square score for one term.
     * @param termID 
     * @param label
     * @param classes
     * @return the chi-square score
     */
    double calc_chi_square(TermID termID, const std::string & label,
            size_t total_docs, size_t total_terms,
            const std::unordered_map<std::string, std::vector<Document>> & classes);
} }

#endif
