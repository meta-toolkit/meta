//
// Created by Greg Embree on 11/27/2016.
//

#ifndef META_IDE_H
#define META_IDE_H

#endif //META_IDE_H

#include "meta/index/feedback/feedback.h"
#include "meta/index/feedback/feedback_factory.h"

namespace meta
{
    namespace index
    {
        class ide : public feedback {
        private:
            const float a_; //constant for original query vector
            const float b_; //constant for  relevant doc. vector
            const float c_; //constant for top non-relevant doc.
            double dot_product(//non rel query vector, );

        public:
            //need default values for constants
            const static util::string_view id;
            const static constexpr float default_a = 1.0f;
            const static constexpr float default_b = 0.8f;
            const static constexpr float default_c = 0.0f;


            ide(float a = default_a, float default_b, float default_c);

            ide(std::istream& in);

            corpus::document apply_feedback(corpus::document &q0,
                                            std::vector <search_result> &results,
                                            forward_index &fwd,
                                            inverted_index &idx);

        };

        template <>
        std::unique_ptr<feedback> make_feedback<ide>(const cpptoml::table&);

    }
}

#endif //META_ROCCHIO_H
