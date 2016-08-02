/**
 * @file absolute_discount.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include "cpptoml.h"
#include "meta/index/ranker/absolute_discount.h"
#include "meta/index/score_data.h"

namespace meta
{
namespace index
{

const util::string_view absolute_discount::id = "absolute-discount";
const constexpr float absolute_discount::default_delta;

absolute_discount::absolute_discount(float delta) : delta_{delta}
{
    /* nothing */
}

absolute_discount::absolute_discount(std::istream& in)
    : delta_{io::packed::read<float>(in)}
{
    // nothing
}

void absolute_discount::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, delta_);
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
    auto delta = config.get_as<double>("delta")
                     .value_or(absolute_discount::default_delta);
    if (delta < 0 || delta > 1)
        throw ranker_exception{"absolute-discount delta must be on [0,1]"};
    return make_unique<absolute_discount>(delta);
}
}
}
