/**
 * @file okapi_bm25.cpp
 * @author Sean Massung
 */

#include <cmath>
#include "index/inverted_index.h"
#include "index/ranker/okapi_bm25.h"
#include "index/score_data.h"
#include "util/fastapprox.h"

namespace meta
{
namespace index
{

const util::string_view okapi_bm25::id = "bm25";

okapi_bm25::okapi_bm25(float k1, float b, float k3) : k1_{k1}, b_{b}, k3_{k3}
{
    /* nothing */
}

float okapi_bm25::score_one(const score_data& sd)
{
    float doc_len = sd.idx.doc_size(sd.d_id);

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

    return make_unique<okapi_bm25>(k1, b, k3);
}
}
}
