/**
 * @file pivoted_length.cpp
 * @author Sean Massung
 */

#include "meta/index/inverted_index.h"
#include "meta/index/ranker/pivoted_length.h"
#include "meta/index/score_data.h"
#include "meta/math/fastapprox.h"

namespace meta
{
namespace index
{

const util::string_view pivoted_length::id = "pivoted-length";
const constexpr float pivoted_length::default_s;

pivoted_length::pivoted_length(float s) : s_{s}
{
    /* nothing */
}

pivoted_length::pivoted_length(std::istream& in)
    : s_{io::packed::read<float>(in)}
{
    // nothing
}

void pivoted_length::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, s_);
}

float pivoted_length::score_one(const score_data& sd)
{
    float doc_len = sd.doc_size;
    float TF = 1.0f + fastapprox::fastlog(
                          1.0f + fastapprox::fastlog(sd.doc_term_count));
    float norm = (1.0f - s_) + s_ * (doc_len / sd.avg_dl);
    float IDF
        = fastapprox::fastlog((sd.num_docs + 1.0f) / (0.5f + sd.doc_count));
    return TF / norm * sd.query_term_weight * IDF;
}

template <>
std::unique_ptr<ranker>
    make_ranker<pivoted_length>(const cpptoml::table& config)
{
    auto s = config.get_as<double>("s").value_or(pivoted_length::default_s);
    if (s < 0 || s > 1)
        throw ranker_exception{"pivoted-length s must be on [0,1]"};
    return make_unique<pivoted_length>(s);
}
}
}
