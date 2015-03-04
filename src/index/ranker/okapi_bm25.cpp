/**
 * @file okapi_bm25.cpp
 * @author Sean Massung
 */

#include <cmath>
#include "index/inverted_index.h"
#include "index/ranker/okapi_bm25.h"
#include "index/score_data.h"

namespace meta
{
namespace index
{

const std::string okapi_bm25::id = "bm25";

okapi_bm25::okapi_bm25(double k1, double b, double k3) : k1_{k1}, b_{b}, k3_{k3}
{
    /* nothing */
}

double okapi_bm25::score_one(const score_data& sd)
{
    double doc_len = sd.idx.doc_size(sd.d_id);

    // add 1.0 to the IDF to ensure that the result is positive
    double IDF = std::log(
        1.0 + (sd.num_docs - sd.doc_count + 0.5) / (sd.doc_count + 0.5));

    double TF = ((k1_ + 1.0) * sd.doc_term_count)
                / ((k1_ * ((1.0 - b_) + b_ * doc_len / sd.avg_dl))
                   + sd.doc_term_count);

    double QTF = ((k3_ + 1.0) * sd.query_term_count)
                 / (k3_ + sd.query_term_count);

    return TF * IDF * QTF;
}

template <>
std::unique_ptr<ranker> make_ranker<okapi_bm25>(const cpptoml::table& config)
{
    auto k1 = okapi_bm25::default_k1;
    if (auto c_k1 = config.get_as<double>("k1"))
        k1 = *c_k1;

    auto b = okapi_bm25::default_b;
    if (auto c_b = config.get_as<double>("b"))
        b = *c_b;

    auto k3 = okapi_bm25::default_k3;
    if (auto c_k3 = config.get_as<double>("k3"))
        k3 = *c_k3;

    return make_unique<okapi_bm25>(k1, b, k3);
}
}
}
