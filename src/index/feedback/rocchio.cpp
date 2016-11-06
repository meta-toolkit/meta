//
// Created by Collin Gress on 11/6/16.
//

#include "meta/index/feedback/rocchio.h"

namespace meta
{
    namespace index
    {
        rocchio::rocchio(float a, float b, float c) : a_{a}, b_{b}, c_{c} {}

        corpus::document rocchio::apply_feedback(corpus::document q0,
                                                 std::vector<corpus::document*> relevant,
                                                 std::vector<corpus::document*> non_relevant,
                                                 inverted_index &idx)
        {
            auto q0_vect = q0.vector();

        }
    }
}