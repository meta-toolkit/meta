/**
 * @file okapi_bm25.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _OKAPI_BM25_H_
#define _OKAPI_BM25_H_

#include <vector>
#include <utility>
#include "index/inverted_index.h"
#include "index/ranker/ranker.h"

namespace meta {
namespace index {

/**
 * The Okapi BM25 scoring function.
 */
class okapi_bm25: public ranker
{
    public:
        /**
         * @param k1
         * @param b
         * @param k3
         */
        okapi_bm25(double k1 = 1.5, double b = 0.75, double k3 = 500.0);

        /**
         * @param idx
         * @param query
         * @param tpair
         * @param dpair
         * @param unique_terms
         */
        double score_one(inverted_index & idx,
                         const document & query,
                         const std::pair<term_id, uint64_t> & tpair,
                         const std::pair<doc_id, uint64_t> & dpair,
                         uint64_t unique_terms) const;

    private:

        const double _k1;
        const double _b;
        const double _k3;
};

}
}

#endif
