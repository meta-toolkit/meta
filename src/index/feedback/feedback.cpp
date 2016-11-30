//
// Created by Collin Gress on 11/28/16.
//

#include "meta/index/feedback/feedback.h"

namespace meta
{
namespace index
{
    corpus::document feedback::apply_feedback(corpus::document &q0, std::vector<search_result> &results,
                                              index::forward_index &fwd, index::inverted_index &idx)
    {
        auto &vsm_vector = q0.vsm_vector();

        if (vsm_vector.map().size() == 0)
        {
            auto counts = idx.tokenize(q0);
            vsm_vector.from_feature_map(counts, idx);
        }

        return transform_vector(q0, results, fwd, idx);
    }

}

}