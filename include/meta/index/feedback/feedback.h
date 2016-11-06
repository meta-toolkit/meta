//
// Created by Collin Gress on 11/5/16.
//

#ifndef META_FEEDBACK_H_H
#define META_FEEDBACK_H_H

#include <vector>
#include "meta/index/inverted_index.h"

namespace meta
{
    namespace index
    {
        class feedback
        {
            public:
                virtual corpus::document apply_feedback(corpus::document q0,
                                                        std::vector<corpus::document*> relevant,
                                                        std::vector<corpus::document*> non_relevant,
                                                        inverted_index &idx
                );
        };
    }
}

#endif //META_FEEDBACK_H_H
