//
// Created by Collin Gress on 11/6/16.
//

#ifndef META_ROCCHIO_H
#define META_ROCCHIO_H

#include "meta/index/feedback/feedback.h"

namespace meta
{
    namespace index
    {
        class rocchio : public feedback
        {
            private:
                const float a_;
                const float b_;
                const float c_;
            public:
                const static constexpr float default_a = 1.0f;
                const static constexpr float default_b = 0.8f;
                const static constexpr float default_c = 0.0f;
                rocchio(float a = default_a, float b = default_b,
                       float c = default_c);
                corpus::document apply_feedback(corpus::document q0,
                                                std::vector<corpus::document*> relevant,
                                                std::vector<corpus::document*> non_relevant,
                                                inverted_index &idx);
        };
    }
}




#endif //META_ROCCHIO_H
