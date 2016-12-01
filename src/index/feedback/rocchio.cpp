//
// Created by Collin Gress on 11/6/16.
//

#include "meta/index/feedback/rocchio.h"
#include "meta/index/postings_file.h"
#include "meta/index/ranker/ranker.h"
#include <iostream>
#include <set>

namespace meta
{
namespace index
{

const util::string_view rocchio::id = "rocchio";
const constexpr float rocchio::default_a;
const constexpr float rocchio::default_b;
const constexpr float rocchio::default_c;


rocchio::rocchio(float a, float b, float c) : a_{a}, b_{b}, c_{c} {}

rocchio::rocchio(std::istream& in)
        : a_{io::packed::read<float>(in)},
          b_{io::packed::read<float>(in)},
          c_{io::packed::read<float>(in)}
{

}

corpus::document rocchio::apply_feedback(corpus::document &q0,
                                         std::vector<search_result> &results,
                                         forward_index &fwd,
                                         inverted_index &idx)
{
    std::unordered_map<term_id, float> q0_vsm_map = q0.vsm_vector().map(); //map of original query vector
    std::unordered_map<term_id, float> qm;
    std::set<doc_id> relevant;
    size_t rel_size = results.size();
    uint64_t irrel_size = fwd.num_docs() - rel_size;

    if (q0_vsm_map.size() == 0)
    {
        // does this make sense? should we just return instead?
        throw feedback_exception{"q0 VSM empty"};
    }

    // a * q0
    if (a_ > 0)
    {
        for (auto it = q0_vsm_map.begin(); it != q0_vsm_map.end(); ++it)
        {
            term_id t_id = it->first;
            qm[t_id] = it->second * a_;
        }
    }

    /* iterate the documents that were originally returned as relevant.
     * for each document, iterate all the terms in the document. weight
     * the terms and add them to qm.
     */
    if (b_ > 0)
    {
        // TODO: IDF weighting. common words will have massive weights
        for (size_t i = 0; i < rel_size; i++)
        {
            doc_id d_id = results[i].d_id;
            relevant.insert(d_id);
            auto postings = fwd.search_primary(d_id);
            for (const auto& count : postings->counts())
            {
                term_id t_id = count.first;
                uint64_t term_count = idx.doc_freq(t_id);
                if (qm.find(t_id) == qm.end())
                {
                    qm[t_id] = 0;
                }
                qm[t_id] += term_count * b_ / rel_size;
            }
        }
    }

    // TODO: consider performance implications of this, especially on a large corpus
    if (c_ > 0)
    {
        std::vector<doc_id> all_docs = fwd.docs();
        for (auto it = all_docs.begin(); it != all_docs.end(); ++it)
        {
            doc_id d_id = *it;
            if (relevant.find(d_id) == relevant.end())
            {
                auto postings = fwd.search_primary(d_id);
                for (const auto& count : postings->counts())
                {
                    term_id t_id = count.first;
                    uint64_t term_count = idx.term_freq(t_id, d_id);
                    if (qm.find(t_id) == qm.end())
                    {
                        qm[t_id] = 0;
                    }
                    qm[t_id] -= term_count * b_ / irrel_size;
                }
            }
        }
    }
    corpus::document d;
    d.vsm_vector().map(qm);
    return d;
}

template <>
std::unique_ptr<feedback> make_feedback<rocchio>(const cpptoml::table& config)
{
    auto a = config.get_as<double>("a").value_or(rocchio::default_a);
    auto b = config.get_as<double>("b").value_or(rocchio::default_b);
    auto c = config.get_as<double>("c").value_or(rocchio::default_c);

    if (a < 0)
    {
        throw feedback_exception{"rocchio 'a' param must be non-negative"};
    }
    if (b < 0)
    {
        throw feedback_exception{"rocchio 'b' param must be non-negative"};
    }
    if (c < 0)
    {
        throw feedback_exception{"rocchio 'c' param must be non-negative"};
    }

    return make_unique<rocchio>(a, b, c);
}



}
}