/**
 * @file okapi_bm25.h
 */

#ifndef _OKAPI_BM25_H_
#define _OKAPI_BM25_H_

#include <vector>
#include <utility>
#include "index/inverted_index.h"

namespace meta {
namespace index {

/**
 *
 */
class okapi_bm25
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
         */
        std::vector<std::pair<doc_id, double>> score(inverted_index & idx,
                                                     document & query);

    private:

        double _k1;
        double _b;
        double _k3;
};

}
}

#endif
