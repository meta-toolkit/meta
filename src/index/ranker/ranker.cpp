/**
 * @file ranker.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <unordered_map>
#include "meta/corpus/document.h"
#include "meta/index/inverted_index.h"
#include "meta/index/postings_data.h"
#include "meta/index/ranker/ranker.h"
#include "meta/index/score_data.h"
#include "meta/util/fixed_heap.h"

namespace meta
{
namespace index
{

std::vector<search_result>
    ranker::score(inverted_index& idx, const corpus::document& query,
                  uint64_t num_results /* = 10 */,
                  const filter_function_type& filter /* return true */)
{
    auto counts = idx.tokenize(query);
    return score(idx, counts.begin(), counts.end(), num_results, filter);
}

std::vector<search_result> ranker::rank(detail::ranker_context& ctx,
                                        uint64_t num_results,
                                        const filter_function_type& filter)
{
    score_data sd{ctx.idx, ctx.idx.avg_doc_length(), ctx.idx.num_docs(),
                  ctx.idx.total_corpus_terms(), ctx.query_length};

    auto comp = [](const search_result& a, const search_result& b)
    {
        // comparison is reversed since we want a min-heap
        return a.score > b.score;
    };
    util::fixed_heap<search_result, decltype(comp)> results{num_results, comp};

    doc_id next_doc{ctx.idx.num_docs()};
    while (ctx.cur_doc < ctx.idx.num_docs())
    {
        sd.d_id = ctx.cur_doc;
        sd.doc_size = ctx.idx.doc_size(ctx.cur_doc);
        sd.doc_unique_terms = ctx.idx.unique_terms(ctx.cur_doc);

        auto score = initial_score(sd);
        for (auto& pc : ctx.postings)
        {
            if (pc.begin == pc.end)
                continue;

            if (pc.begin->first == ctx.cur_doc)
            {
                // set up this term
                sd.t_id = pc.t_id;
                sd.query_term_weight = pc.query_term_weight;
                sd.doc_count = pc.doc_count;
                sd.corpus_term_count = pc.corpus_term_count;
                sd.doc_term_count = pc.begin->second;

                score += score_one(sd);

                // advance over this position in the current postings context
                // until the next valid document
                do
                {
                    ++pc.begin;
                } while (pc.begin != pc.end && !filter(pc.begin->first));
            }

            if (pc.begin != pc.end)
            {
                // check if the document in the next position is the
                // smallest accepted doc_id
                if (pc.begin->first < next_doc)
                    next_doc = pc.begin->first;
            }
        }

        results.emplace(ctx.cur_doc, score);
        ctx.cur_doc = next_doc;
        next_doc = doc_id{ctx.idx.num_docs()};
    }

    return results.extract_top();
}

float ranker::initial_score(const score_data&) const
{
    return 0.0;
}
}
}
