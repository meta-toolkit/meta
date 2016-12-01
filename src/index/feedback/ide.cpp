//
// Created by Greg Embree on 11/28/2016.

#include "meta/index/feedback/ide.h"
#include "meta/index/postings_file.h"
#include "meta/index/ranker/ranker.h"
#include <iostream>
#include <set>
#include <cmath>

namespace meta {

    namespace index {

        const util::string_view ide::id = "ide";
        const constexpr float ide::default_a;
        const constexpr float ide::default_b;
        const constexpr float ide::default_c;

        ide::ide(float a, float b, float c) : a_{a}, b_{b}, c_{c} {} //constructor

        ide::ide(std::istream &in)
                : a_{io:packed::read<float>(in)},
                  b_{io:packed::read<float>(in)},
                  c_{io:packed::read<float>(in)} {}

        corpus::document ide::apply_feedback(corpus::document &q0,
                                             std::vector<search_result> &results,
                                             forward_index &fwd,
                                             inverted_index &idx) {
            std::unordered_map<term_id, float> q0_vsm_map = q0.vsm_vector().map();
            std::unordered_map<term_id, float> qm;
            std::set<doc_id> relevant;
            //size_t rel_size = results.size();
            //uint64_t irrel_size = fwd.num_docs() - rel_size;

            //first term: a * original query q0
            if (a_ > 0) {
                for (auto it = q0_vsm_map.begin(); it != q0_vsm_map.end(); ++it) {
                    term_id t_id = it->first;
                    qm[t_id] = it->second * a_;
                }
            }
            /*second term: iterates the documents that are relevant.  for each doc, iterate
            * all the terms in the doc.  weight the terms and add them to new query vector qm
            */

            if (b_ > 0)
            {
                for(size_t i = 0; i<rel_size; i++)
                {
                    doc_id d_id = results[i].d_id;
                    relevant.insert(d.id);
                    auto postings = fwd.search_primary(d_id);
                    for (const auto& count : postings->counts())
                    {
                        term_id t_id = count.first;
                        uint64_t term_count = idx.doc_freq(t_id);
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
                //find the highest ranked  irrelevant document
                //calculate similarity measure using dot product maybe with std::inner_product
            }
        }
    }
}



