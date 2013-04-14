/**
 * @file select_info_gain.h
 */

#ifndef _SELECT_INFO_GAIN_H_
#define _SELECT_INFO_GAIN_H_

#include <vector>
#include <utility>
#include "classify/select_simple.h"
#include "index/document.h"

namespace meta {
namespace classify {

/**
 * Performs information gain feature selection:
 * \f$ IG(t, c_i) =
 * \sum_{c\in\{c_i,\overline{c_i}\}}\sum_{t'\in\{t,t'\}}P(t',c)\log\frac{P(t',c)}{P(t')P(c)} \f$
 */
class select_info_gain: public select_simple
{
    public:

        /**
         * Constructor.
         * @param docs The documents containing features
         */
        select_info_gain(const std::vector<index::Document> & docs);

    private:

        /**
         * Calculates weight for a specific termID, label pair using information
         * gain
         * @param termID
         * @param label
         * @return the info gain score
         */
        double calc_weight(term_id termID, const class_label & label) const;
};

}
}

#endif
