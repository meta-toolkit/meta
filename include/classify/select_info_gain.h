/**
 * @file select_info_gain.h
 */

#ifndef _SELECT_INFO_GAIN_H_
#define _SELECT_INFO_GAIN_H_

#include <vector>
#include "index/document.h"

namespace classify { namespace feature_select {

    /**
     * Calculates important features based on information gain; that is, the
     * reduction of entropy by knowing the existance or absense of a term in a
     * document.
     * @param docs The documents to extract features from
     * @return a vector of TermIDs sorted by importance
     */
    std::vector<TermID> info_gain(const std::vector<Document> & docs);

} }

#endif
