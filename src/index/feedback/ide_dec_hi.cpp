//
// Created by Greg Embree on 11/28/2016.

#include "meta/index/feedback/ide_dec_hi.h"
#include "meta/index/postings_file.h"
#include "meta/index/postings_data.h"
#include <set>

namespace meta 
{
namespace index 
{

const util::string_view ide_dec_hi::id = "ide_dec_hi";
const constexpr float ide_dec_hi::default_a;
const constexpr float ide_dec_hi::default_b;
const constexpr float ide_dec_hi::default_c;

ide_dec_hi::ide_dec_hi(float a, float b, float c) : a_{a}, b_{b}, c_{c} {}

ide_dec_hi::ide_dec_hi(std::istream &in)
        : a_{io::packed::read<float>(in)},
          b_{io::packed::read<float>(in)},
          c_{io::packed::read<float>(in)}
{
    
}

corpus::document ide_dec_hi::transform_vector(corpus::document &q0,
                                     std::vector<search_result> &results,
                                     forward_index &fwd,
                                     inverted_index &inv)
{
    std::unordered_map<term_id, float> q0_vsm_map = q0.vsm_vector().map();
    std::unordered_map<term_id, float> qm;
    std::set<doc_id> relevant;
    size_t rel_size = results.size();

    // first term: a * original query q0
    if (a_ > 0) {
        for (auto it = q0_vsm_map.begin(); it != q0_vsm_map.end(); ++it) 
        {
            term_id t_id = it->first;
            qm[t_id] = it->second * a_;
        }
    }

    /* second term: iterates the documents that are relevant.  for each doc, iterate
     * all the terms in the doc.  weight the terms and add them to new query vector qm
     */
    if (b_ > 0)
    {
        for(size_t i = 0; i < rel_size; i++)
        {
            doc_id d_id = results[i].d_id;
            relevant.insert(d_id);
            auto postings = fwd.search_primary(d_id);
            for (const auto& count : postings->counts())
            {
                term_id t_id = count.first;
                double term_count = count.second;
                if(qm.find(t_id) == qm.end())
                {
                    qm[t_id] = 0;
                }
                qm[t_id] += term_count * b_;
            }
        }
    }

    if(c_ > 0)
    {
        std::vector<doc_id> all_docs = fwd.docs();
        std::shared_ptr<forward_index::postings_data_type> max_sim_postings = nullptr;
        int max_sim = 0;
        for (auto it = all_docs.begin(); it != all_docs.end(); ++it)
        {
            doc_id d_id = *it;
            if (relevant.find(d_id) == relevant.end())
            {
                int sim = 0;
                auto postings = fwd.search_primary(d_id);
                for (const auto& count : postings->counts())
                {
                    term_id t_id = count.first;
                    double term_count = count.second;

                    auto q0_term_it = q0_vsm_map.find(t_id);
                    if (q0_term_it != q0_vsm_map.end())
                    {
                        sim += q0_term_it->second * term_count;
                    }
                }
                if (sim > max_sim)
                {
                    max_sim = sim;
                    max_sim_postings = postings;
                }
            }
        }
        if (max_sim_postings != nullptr)
        {
            for (const auto& count : max_sim_postings->counts())
            {
                term_id t_id = count.first;
                double term_count = count.second;
                if (qm.find(t_id) == qm.end())
                {
                    qm[t_id] = 0;
                }
                qm[t_id] -= term_count * c_;
            }
        }
    }

    corpus::document d;
    d.vsm_vector().map(qm);
    return d;
}

template <>
std::unique_ptr<feedback> make_feedback<ide_dec_hi>(const cpptoml::table& config)
{
    auto a = config.get_as<double>("a").value_or(ide_dec_hi::default_a);
    auto b = config.get_as<double>("b").value_or(ide_dec_hi::default_b);
    auto c = config.get_as<double>("c").value_or(ide_dec_hi::default_c);

    if (a < 0)
    {
        throw feedback_exception{"ide_dec_hi 'a' param must be non-negative"};
    }
    if (b < 0)
    {
        throw feedback_exception{"ide_dec_hi 'b' param must be non-negative"};
    }
    if (c < 0)
    {
        throw feedback_exception{"ide_dec_hi 'c' param must be non-negative"};
    }

    return make_unique<ide_dec_hi>(a, b, c);
}

}
}



