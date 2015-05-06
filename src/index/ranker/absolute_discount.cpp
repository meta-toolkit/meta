/**
 * @file absolute_discount.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include "cpptoml.h"
#include "index/ranker/absolute_discount.h"
#include "index/score_data.h"

namespace meta
{
namespace index
{

const std::string absolute_discount::id = "absolute-discount";

absolute_discount::absolute_discount(float delta) : delta_{delta}
{
    /* nothing */
}

float absolute_discount::smoothed_prob(const score_data& sd) const
{
    float pc = static_cast<float>(sd.corpus_term_count) / sd.total_terms;
    float numerator = std::max<float>(sd.doc_term_count - delta_, 0.0f);
    float denominator = sd.doc_size;
    return numerator / denominator + doc_constant(sd) * pc;
}

float absolute_discount::doc_constant(const score_data& sd) const
{
    float unique = sd.doc_unique_terms;
    return delta_ * unique / sd.doc_size;
}

template <>
std::unique_ptr<ranker>
    make_ranker<absolute_discount>(const cpptoml::table& config)
{
    if (auto delta = config.get_as<double>("delta"))
        return make_unique<absolute_discount>(*delta);
    return make_unique<absolute_discount>();
}
}
}
