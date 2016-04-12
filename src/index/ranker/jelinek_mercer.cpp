/**
 * @file jelinek_mercer.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "meta/index/ranker/jelinek_mercer.h"
#include "meta/index/score_data.h"

namespace meta
{
namespace index
{

const util::string_view jelinek_mercer::id = "jelinek-mercer";
const constexpr float jelinek_mercer::default_lambda;

jelinek_mercer::jelinek_mercer(float lambda) : lambda_{lambda}
{
    /* nothing */
}

jelinek_mercer::jelinek_mercer(std::istream& in)
    : lambda_{io::packed::read<float>(in)}
{
    // nothing
}

void jelinek_mercer::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, lambda_);
}

float jelinek_mercer::smoothed_prob(const score_data& sd) const
{
    float max_likelihood = static_cast<float>(sd.doc_term_count)
                            / sd.doc_size;
    float pc = static_cast<float>(sd.corpus_term_count) / sd.total_terms;
    return (1.0f - lambda_) * max_likelihood + lambda_ * pc;
}

float jelinek_mercer::doc_constant(const score_data&) const
{
    return lambda_;
}

template <>
std::unique_ptr<ranker>
    make_ranker<jelinek_mercer>(const cpptoml::table& config)
{
    auto lambda = config.get_as<double>("lambda")
                     .value_or(jelinek_mercer::default_lambda);
    if (lambda < 0 || lambda > 1)
        throw ranker_exception{"jelinek-mercer lambda must be on [0,1]"};
    return make_unique<jelinek_mercer>(lambda);
}
}
}
