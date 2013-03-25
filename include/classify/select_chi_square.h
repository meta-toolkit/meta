/**
 * @file select_chi_square.h
 */

#ifndef _SELECT_CHI_SQUARED_H_
#define _SELECT_CHI_SQUARED_H_

#include <vector>
#include "index/document.h"

namespace classify { namespace feature_select {

    /**
     * Calculates important features via Chi-square statistics (independence of
     * two random variables).
     * @param docs The documents to extract features from
     * @return a vector of TermIDs sorted by importance
     */
    std::vector<TermID> chi_square(const std::vector<Document> & docs);

} }

#endif
