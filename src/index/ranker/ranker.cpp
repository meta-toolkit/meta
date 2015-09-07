/**
 * @file ranker.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <unordered_map>
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/postings_data.h"
#include "index/ranker/ranker.h"
#include "index/score_data.h"
#include "util/fixed_heap.h"

namespace meta
{
namespace index
{

namespace
{
struct postings_context
{
    using postings_data_type = inverted_index::postings_data_type;
    using iterator = postings_stream<doc_id>::iterator;

    postings_stream<doc_id> stream;
    iterator begin;
    iterator end;
    term_id t_id;
    double query_term_weight;
    uint64_t doc_count;
    uint64_t corpus_term_count;

    postings_context(postings_stream<doc_id> strm, double qtf, term_id term)
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
}

std::vector<search_result> ranker::score(
    inverted_index& idx, corpus::document& query,
    uint64_t num_results /* = 10 */,
    const std::function<bool(doc_id d_id)>& filter /* return true */)
{
    if (query.counts().empty())
        idx.tokenize(query);

    score_data sd{idx, idx.avg_doc_length(), idx.num_docs(),
                  idx.total_corpus_terms(), query};

    auto comp = [](const search_result& a, const search_result& b)
    {
        // comparison is reversed since we want a min-heap
        return a.score > b.score;
    };
    util::fixed_heap<search_result, decltype(comp)> results{num_results, comp};

    std::vector<postings_context> postings;
    postings.reserve(query.counts().size());

    doc_id cur_doc{idx.num_docs()};
    for (const auto& count : query.counts())
    {
        auto term = idx.get_term_id(count.first);
        auto pstream = idx.stream_for(term);
        if (!pstream)
            continue;

        postings.emplace_back(*pstream, count.second, term);

        while (postings.back().begin != postings.back().end
               && !filter(postings.back().begin->first))
            ++postings.back().begin;

        if (postings.back().begin != postings.back().end)
        {
            if (postings.back().begin->first < cur_doc)
                cur_doc = postings.back().begin->first;
        }
    }

    doc_id next_doc{idx.num_docs()};
    while (cur_doc < idx.num_docs())
    {
        sd.d_id = cur_doc;
        sd.doc_size = idx.doc_size(cur_doc);
        sd.doc_unique_terms = idx.unique_terms(cur_doc);

        auto score = initial_score(sd);
        for (auto& pc : postings)
        {
            if (pc.begin == pc.end)
                continue;

            if (pc.begin->first == cur_doc)
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

        results.emplace(cur_doc, score);
        cur_doc = next_doc;
        next_doc = doc_id{idx.num_docs()};
    }

    return results.reverse_and_clear();
}

float ranker::initial_score(const score_data&) const
{
    return 0.0;
}
}
}
