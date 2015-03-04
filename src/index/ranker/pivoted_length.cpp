/**
 * @file pivoted_length.cpp
 * @author Sean Massung
 */

#include "index/inverted_index.h"
#include "index/ranker/pivoted_length.h"
#include "index/score_data.h"

namespace meta
{
namespace index
{

const std::string pivoted_length::id = "pivoted-length";

pivoted_length::pivoted_length(double s) : s_{s}
{
    /* nothing */
}

double pivoted_length::score_one(const score_data& sd)
{
    double doc_len = sd.idx.doc_size(sd.d_id);
    double TF = 1 + log(1 + log(sd.doc_term_count));
    double norm = (1 - s_) + s_ * (doc_len / sd.avg_dl);
    double IDF = log((sd.num_docs + 1) / (0.5 + sd.doc_count));

    return TF / norm * sd.query_term_count * IDF;
}

template <>
std::unique_ptr<ranker>
    make_ranker<pivoted_length>(const cpptoml::table& config)
{
    auto s = pivoted_length::default_s;
    if (auto c_s = config.get_as<double>("s"))
        s = *c_s;
    return make_unique<pivoted_length>(s);
}
}
}
