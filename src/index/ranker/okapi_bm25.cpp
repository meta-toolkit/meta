/**
 * @file okapi_bm25.cpp
 * @author Sean Massung
 */

#include "index/ranker/okapi_bm25.h"

namespace meta {
namespace index {

okapi_bm25::okapi_bm25(double k1 /* = 1.5 */,
                       double b  /* = 0.75 */,
                       double k3 /* = 500.0 */):
    _k1(k1), _b(b), _k3(k3)
{ /* nothing */ }

double okapi_bm25::score_one(inverted_index & idx,
                      const document & query,
                      const std::pair<term_id, uint64_t> & tpair,
                      const std::pair<doc_id, uint64_t> & dpair,
                      uint64_t unique_terms) const
{
    double avg_dl = idx.avg_doc_length();
    double num_docs = idx.num_docs();
    double doc_len = idx.doc_size(dpair.first);

    // add 1.0 to the IDF to ensure that the result is positive
    double IDF = log(1.0 + (num_docs - unique_terms + 0.5)
        / (unique_terms + 0.5));

    double TF = ((_k1 + 1.0) * dpair.second)
        / ((_k1 * ((1.0 - _b) + _b * doc_len / avg_dl)) + dpair.second);
    
    double QTF = ((_k3 + 1.0) * tpair.second) / (_k3 + tpair.second);

    return TF * IDF * QTF;
}

}
}
