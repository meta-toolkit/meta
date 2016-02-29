/**
 * @file okapi_bm25.cpp
 * @author Sean Massung
 */

#include <cmath>
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/okapi_bm25.h"
#include "meta/index/score_data.h"
#include "meta/math/fastapprox.h"

namespace meta
{
namespace index
{

const util::string_view okapi_bm25::id = "bm25";
const constexpr float okapi_bm25::default_k1;
const constexpr float okapi_bm25::default_b;
const constexpr float okapi_bm25::default_k3;


okapi_bm25::okapi_bm25(float k1, float b, float k3) : k1_{k1}, b_{b}, k3_{k3}
{
    /* nothing */
}

okapi_bm25::okapi_bm25(std::istream& in)
    : k1_{io::packed::read<float>(in)},
      b_{io::packed::read<float>(in)},
      k3_{io::packed::read<float>(in)}
{
    // nothing
}

void okapi_bm25::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, k1_);
    io::packed::write(out, b_);
    io::packed::write(out, k3_);
}

float okapi_bm25::score_one(const score_data& sd)
{
    float doc_len = sd.doc_size;

    // add 1.0 to the IDF to ensure that the result is positive
    float IDF = fastapprox::fastlog(
        1.0f + (sd.num_docs - sd.doc_count + 0.5f) / (sd.doc_count + 0.5f));

    float TF = ((k1_ + 1.0f) * sd.doc_term_count)
               / ((k1_ * ((1.0f - b_) + b_ * doc_len / sd.avg_dl))
                  + sd.doc_term_count);

    float QTF = ((k3_ + 1.0f) * sd.query_term_weight)
                / (k3_ + sd.query_term_weight);

    return TF * IDF * QTF;
}

template <>
std::unique_ptr<ranker> make_ranker<okapi_bm25>(const cpptoml::table& config)
{
    auto k1 = config.get_as<double>("k1").value_or(okapi_bm25::default_k1);
    auto b = config.get_as<double>("b").value_or(okapi_bm25::default_b);
    auto k3 = config.get_as<double>("k3").value_or(okapi_bm25::default_k3);

    if (k1 < 0)
        throw ranker_exception{"bm25 k1 must be >= 0"};

    if (k3 < 0)
        throw ranker_exception{"bm25 k3 must be >= 0"};

    if (b < 0 || b > 1)
        throw ranker_exception{"bm25 b must be on [0,1]"};

    return make_unique<okapi_bm25>(k1, b, k3);
}
}
}
