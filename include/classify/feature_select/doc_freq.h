/**
 * @file doc_freq.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _SELECT_DOC_FREQ_H_
#define _SELECT_DOC_FREQ_H_

#include <vector>
#include <utility>
#include "corpus/document.h"
#include "classify/feature_select/select_simple.h"

namespace meta {
namespace classify {

/**
 * Performs feature selection based on term document frequency:
 * \f$ P(t,c_i) \f$
 */
class doc_freq: public select_simple
{
    public:

        /**
         * Constructor.
         * @param docs The documents containing features
         */
        doc_freq(const std::vector<corpus::document> & docs);

        /**
         * Calculates the doc freq score for one term.
         * @param termID 
         * @param label
         * @return the doc freq score
         */
        double calc_weight(term_id termID, const class_label & label) const;
};

}
}

#endif
