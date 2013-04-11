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

class select_info_gain: public select_simple
{
    public:

        /**
         * Constructor.
         */
        select_info_gain(const std::vector<index::Document> & docs);

    private:

        /**
         * @param termID
         * @param label
         * @return the info gain score
         */
        double calc_weight(index::TermID termID, const std::string & label) const;
};

}
}

#endif
