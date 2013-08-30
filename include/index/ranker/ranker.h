/**
 * @file ranker.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _RANKER_H_
#define _RANKER_H_

#include "index/inverted_index.h"

namespace meta {
namespace index {

/**
 * A ranker scores a query against all the documents in an inverted index,
 * returning a list of documents sorted by relevance.
 */
class ranker
{
    public:
        /**
         * @param idx
         * @param query
         */
        std::vector<std::pair<doc_id, double>> score(inverted_index & idx,
                                                     document & query) const;

        /**
         * @param idx
         * @param query
         * @param tpair
         * @param dpair
         * @param unique_terms
         */
        virtual double score_one(
                inverted_index & idx,
                const document & query,
                const std::pair<term_id, uint64_t> & tpair,
                const std::pair<doc_id, uint64_t> & dpair,
                uint64_t unique_terms) const = 0;

        /**
         * Default destructor.
         */
        virtual ~ranker() = default;
};

}
}

#endif
