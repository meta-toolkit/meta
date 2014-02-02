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

okapi_bm25::okapi_bm25(double k1 /* = 1.5 */, double b /* = 0.75 */,
                       double k3 /* = 500.0 */)
    : _k1{k1}, _b{b}, _k3{k3}
{/* nothing */
}

double okapi_bm25::score_one(const score_data& sd)
{
    double doc_len = sd.idx.doc_size(sd.d_id);

    // add 1.0 to the IDF to ensure that the result is positive
    double IDF = std::log(1.0 + (sd.num_docs - sd.doc_count + 0.5)
                                / (sd.doc_count + 0.5));

    double TF = ((_k1 + 1.0) * sd.doc_term_count)
                / ((_k1 * ((1.0 - _b) + _b * doc_len / sd.avg_dl))
                   + sd.doc_term_count);

    double QTF = ((_k3 + 1.0) * sd.query_term_count)
                 / (_k3 + sd.query_term_count);

    return TF * IDF * QTF;
}
}
}
