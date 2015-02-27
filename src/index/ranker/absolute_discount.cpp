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

absolute_discount::absolute_discount(double delta) : delta_{delta}
{
    /* nothing */
}

double absolute_discount::smoothed_prob(const score_data& sd) const
{
    double pc = static_cast<double>(sd.corpus_term_count) / sd.total_terms;
    double numerator = std::max<double>(sd.doc_term_count - delta_, 0);
    double denominator = sd.doc_size;
    return numerator / denominator + doc_constant(sd) * pc;
}

double absolute_discount::doc_constant(const score_data& sd) const
{
    double unique = sd.doc_unique_terms;
    return delta_ * unique / sd.doc_size;
}

template <>
std::unique_ptr<ranker>
    make_ranker<absolute_discount>(const cpptoml::table& config)
{
    if (auto gamma = config.get_as<double>("gamma"))
        return make_unique<absolute_discount>(*gamma);
    return make_unique<absolute_discount>();
}
}
}
