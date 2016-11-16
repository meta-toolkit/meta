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

#include "meta/index/inverted_index.h"
#include "meta/meta.h"

namespace meta
{

namespace corpus
{
class document;
}

namespace index
{
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
        // nothing
    }
    doc_id d_id;
    float score;
};

/**
 * Implementation details for indexing and ranking implementations.
 */
namespace detail
{
struct postings_context
{
    using postings_data_type = inverted_index::postings_data_type;
    using iterator = postings_stream<doc_id>::iterator;

    postings_stream<doc_id> stream;
    iterator begin;
    iterator end;
    term_id t_id;
    float query_term_weight;
    uint64_t doc_count;
    uint64_t corpus_term_count;

    postings_context(postings_stream<doc_id> strm, float qtf, term_id term)
        : stream{std::move(strm)},
          begin{stream.begin()},
          end{stream.end()},
          t_id{term},
          query_term_weight{qtf},
          doc_count{stream.size()},
          corpus_term_count{stream.total_counts()}
    {
        // nothing
    }
};

inline term_id get_term_id(disk_index& inv, const std::string& term)
{
    return inv.get_term_id(term);
}

inline term_id get_term_id(disk_index&, term_id tid)
{
    return tid;
}
}

/**
 * Stores a list of postings_stream and other relevant information for
 * performing document-at-a-time ranking. You should not generally have to
 * interact with this class unless implementing a new feedback method, in
 * which case you should only have to construct it and pass it off to
 * ranker::rank() directly afterward.
 *
 * ForwardIterator must dereference to a pair type (either std::pair or
 * hashing::kv_pair) which has a key type of either std::string or term_id
 * and a value type convertible to float.
 */
struct ranker_context
{
    template <class ForwardIterator, class FilterFunction>
    ranker_context(inverted_index& inv, ForwardIterator begin,
                   ForwardIterator end, FilterFunction&& filter)
        : idx(inv), cur_doc{idx.num_docs()}
    {
        postings.reserve(static_cast<std::size_t>(std::distance(begin, end)));

        query_length = 0.0;
        for (; begin != end; ++begin)
        {
            const auto& count = *begin;

            using kv_traits = hashing::kv_traits<
                typename std::decay<decltype(count)>::type>;

            query_length += kv_traits::value(count);
            auto term = detail::get_term_id(inv, kv_traits::key(count));
            auto pstream = idx.stream_for(term);
            if (!pstream)
                continue;

            postings.emplace_back(*pstream, kv_traits::value(count), term);

            while (postings.back().begin != postings.back().end
                   && !filter(postings.back().begin->first))
                ++postings.back().begin;

            if (postings.back().begin != postings.back().end)
            {
                if (postings.back().begin->first < cur_doc)
                    cur_doc = postings.back().begin->first;
            }
        }
    }

    inverted_index& idx;
    std::vector<detail::postings_context> postings;
    float query_length;
    doc_id cur_doc;
};

/**
 * Exception class for ranker interactions.
 */
class ranker_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * A ranker scores a query against all the documents in an inverted index,
 * returning a list of documents sorted by relevance.
 */
class ranker
{
  public:
    using filter_function_type = std::function<bool(doc_id did)>;

    static bool passthrough(doc_id)
    {
        return true;
    }

    /**
     * @param idx The index this ranker is operating on
     * @param begin A forward iterator to the beginning of the term
     * weights (pairs of std::string and a weight)
     * @param end A forward iterator to the end of the above range
     * @param num_results The number of results to return in the vector
     * @param filter A filtering function to apply to each doc_id; returns true
     * if the document should be included in results
     */
    template <class ForwardIterator, class Function = bool (*)(doc_id)>
    std::vector<search_result>
    score(inverted_index& idx, ForwardIterator begin, ForwardIterator end,
          uint64_t num_results = 10, Function&& filter = passthrough)
    {
        ranker_context ctx{idx, begin, end, filter};
        return rank(ctx, num_results, filter);
    }

    /**
     * @param idx The index this ranker is operating on
     * @param query The current query
     * @param num_results The number of results to return in the vector
     * @param filter A filtering function to apply to each doc_id; returns
     * true if the document should be included in results
     */
    std::vector<search_result>
    score(inverted_index& idx, const corpus::document& query,
          uint64_t num_results = 10,
          const filter_function_type& filter = [](doc_id) { return true; });

    /**
     * Default destructor.
     */
    virtual ~ranker() = default;

    /**
     * Saves the ranker to a stream. This should save the ranker's id,
     * followed by any parameters needed for reconstruction.
     */
    virtual void save(std::ostream& out) const = 0;

    /**
     * Scores a query using a document-at-a-time strategy. You should not
     * override this unless you desire a completely different ranking
     * strategy than document-at-a-time, which might be the case if you are
     * implementing a new pseudo-relevance feedback method.
     *
     * @param ctx The ranker_context holding the postings lists
     * @param num_results The number of search results to return
     * @param filter The filter function to be used
     */
    virtual std::vector<search_result> rank(ranker_context& ctx,
                                            uint64_t num_results,
                                            const filter_function_type& filter)
        = 0;
};

class ranking_function : public ranker
{
  public:
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

    virtual std::vector<search_result>
    rank(ranker_context& ctx, uint64_t num_results,
         const filter_function_type& filter) override final;
};
}
}
#endif
