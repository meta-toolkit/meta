/**
 * @file select_doc_freq.h
 */

#ifndef _SELECT_DOC_FREQ_H_
#define _SELECT_DOC_FREQ_H_

#include <vector>
#include <utility>
#include "index/document.h"

namespace meta {
namespace classify {
namespace feature_select {

    /**
     * Calculates important features via their document frequency; that is,
     * features are assumed to be important if they appear many times per
     * document.
     * @param docs The documents to extract features from
     * @return a vector of TermIDs sorted by importance
     */
    std::vector<std::pair<index::TermID, double>> doc_freq(const std::vector<index::Document> & docs);

}
}
}

#endif
