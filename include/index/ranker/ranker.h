/**
 * @file ranker.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_RANKER_H_
#define META_RANKER_H_

#include <utility>
#include <vector>

#include "meta.h"

namespace meta
{

namespace corpus
{
class document;
}

namespace index
{
class inverted_index;
struct score_data;
}
}

namespace meta
{
namespace index
{

/**
 * A simple struct to hold scored document data.
 */
struct search_result
{
    search_result(doc_id id, float s) : d_id{id}, score{s}
    {
    }
    doc_id d_id;
    float score;
};

/**
 * A ranker scores a query against all the documents in an inverted index,
 * returning a list of documents sorted by relevance.
 */
class ranker
{
  public:
    /**
     * @param idx The index this ranker is operating on
     * @param query The current query
     * @param num_results The number of results to return in the vector
     * @param filter A filtering function to apply to each doc_id; returns true
     * if the document should be included in results
     */
    std::vector<search_result>
        score(inverted_index& idx, corpus::document& query,
              uint64_t num_results = 10,
              const std::function<bool(doc_id d_id)>& filter = [](doc_id)
              {
                  return true;
              });

    /**
     * Computes the contribution to the score of a document for a matched
     * query term.
     * @param sd The score_data for this query
     */
    virtual float score_one(const score_data& sd) = 0;

    /**
     * Computes the constant contribution to the score of a particular
     * document.
     * @param sd The score_data for the query
     */
    virtual float initial_score(const score_data& sd) const;

    /**
     * Default destructor.
     */
    virtual ~ranker() = default;
};
}
}

#endif
