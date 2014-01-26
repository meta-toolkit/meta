/**
 * @file pivoted_length.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _PIVOTED_LENGTH_H_
#define _PIVOTED_LENGTH_H_

#include "index/ranker/ranker.h"

namespace meta {
namespace index {

/**
 * The pivoted document length normalization ranking function
 * @see Amit Singal, Chris Buckley, and Mandar Mitra. Pivoted document length
 * normalization. SIGIR '96, pages 21-29.
 */
class pivoted_length: public ranker
{
    public:
        /**
         * @param s
         */
        pivoted_length(double s = 0.20);

        /**
         * @param sd
         */
        double score_one(const score_data & sd) const override;

    private:

        const double _s;
};

}
}

#endif
