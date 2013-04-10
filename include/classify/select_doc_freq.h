/**
 * @file select_doc_freq.h
 */

#ifndef _SELECT_DOC_FREQ_H_
#define _SELECT_DOC_FREQ_H_

#include <vector>
#include <utility>
#include "index/document.h"
#include "classify/select.h"

namespace meta {
namespace classify {

class select_doc_freq: public feature_select
{
    public:
        /**
         * Calculates important features via their document frequency; that is,
         * features are assumed to be important if they appear many times per
         * document.
         * @param docs The documents to extract features from
         * @return a vector of TermIDs sorted by importance
         */
        std::vector<std::pair<index::TermID, double>> select(const std::vector<index::Document> & docs);
};

}
}

#endif
