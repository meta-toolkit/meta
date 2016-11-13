//
// Created by Collin Gress on 11/5/16.
//

#ifndef META_FEEDBACK_H_H
#define META_FEEDBACK_H_H

#include <vector>
#include "meta/index/ranker/ranker.h"
#include "meta/index/inverted_index.h"
#include "meta/index/forward_index.h"

namespace meta
{
    namespace index
    {
        struct search_result;

        class feedback_exception : public std::runtime_error
        {
            public:
                using std::runtime_error::runtime_error;
        };

        class feedback
        {
            public:
                virtual corpus::document apply_feedback(corpus::document &q0,
                                                        std::vector<search_result> &results,
                                                        index::forward_index &fwd,
                                                        index::inverted_index &idx) = 0; // pure virtual
        };
    }
}

#endif //META_FEEDBACK_H_H
