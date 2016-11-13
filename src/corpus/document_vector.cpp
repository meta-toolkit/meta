//
// Created by Collin Gress on 10/30/16.
//

#include <iostream>
#include "meta/analyzers/analyzer.h"
#include "meta/corpus/document_vector.h"
#include "meta/index/inverted_index.h"


namespace meta
{
namespace corpus
{
    void document_vector::from_feature_map(analyzers::feature_map<uint64_t>& feat_map,
                                           index::inverted_index& idx)
    {
        auto begin = feat_map.begin();
        auto end = feat_map.end();

        for (; begin != end; ++begin)
        {
            const auto &current = *begin;
            using kv_traits = hashing::kv_traits<typename std::decay<decltype(current)>::type>;

            auto term = kv_traits::key(current);
            auto term_count = kv_traits::value(current);

            term_id t_id = idx.get_term_id(term);

            vector_[t_id] = term_count;
        }
    }

    void document_vector::map(std::unordered_map<term_id, float>& map)
    {
        vector_ = map;
    }

    void document_vector::set_term(term_id t_id, float weight)
    {
        vector_[t_id] = weight;
    }

    std::unordered_map<term_id, float>& document_vector::map()
    {
        return vector_;
    }

    void document_vector::print()
    {
        for (auto it = vector_.begin(); it != vector_.end(); ++it)
        {
            std::cout << it->first << ": " << it->second << std::endl;
        }
    }
}
}