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

#include <utility>
#include <vector>

#include "meta.h"

namespace meta {

namespace corpus {
class document;
}

namespace index {
class inverted_index;
struct score_data;
}

}

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
                                                corpus::document & query) const;

        /**
         * @param sd
         */
        virtual double score_one(const score_data & sd) const = 0;

        /**
         * Default destructor.
         */
        virtual ~ranker() = default;
};

}
}

#endif
