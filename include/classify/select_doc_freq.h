/**
 * @file select_doc_freq.h
 */

#ifndef _SELECT_DOC_FREQ_H_
#define _SELECT_DOC_FREQ_H_

#include <vector>
#include <utility>
#include "index/document.h"
#include "classify/select_simple.h"

namespace meta {
namespace classify {

class select_doc_freq: public select_simple
{
    public:

        /**
         * Constructor.
         */
        select_doc_freq(const std::vector<index::Document> & docs);

        /**
         * Calculates the doc freq score for one term.
         * @param termID 
         * @param label
         * @return the doc freq score
         */
        double calc_weight(index::TermID termID, const std::string & label) const;
};

}
}

#endif
