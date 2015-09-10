/**
 * @file chi_square.cpp
 * @author Sean Massung
 */

#include "features/chi_square.h"

namespace meta
{
namespace features
{
const std::string chi_square::id = "chi-square";

double chi_square::score(label_id lid, term_id tid) const
{
    double p_tc = term_and_class(tid, lid);
    double p_ntnc = not_term_and_not_class(tid, lid);
    double p_ntc = not_term_and_class(tid, lid);
    double p_tnc = term_and_not_class(tid, lid);
    double p_c = prob_class(lid);
    double p_t = prob_term(tid);

    double numerator = p_tc * p_ntnc - p_ntc * p_tnc;
    double denominator = p_c * (1.0 - p_c) * p_t * (1.0 - p_t);

    return (numerator * numerator) / denominator;
}
}
}
