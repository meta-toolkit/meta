/**
 * @file chi_square.cpp
 * @author Sean Massung
 */

#include "meta/features/chi_square.h"

namespace meta
{
namespace features
{
const std::string chi_square::id = "chi-square";

double chi_square::score(const class_label& lbl, term_id tid) const
{
    double p_tc = term_and_class(tid, lbl);
    double p_ntnc = not_term_and_not_class(tid, lbl);
    double p_ntc = not_term_and_class(tid, lbl);
    double p_tnc = term_and_not_class(tid, lbl);
    double p_c = prob_class(lbl);
    double p_t = prob_term(tid);

    double numerator = p_tc * p_ntnc - p_ntc * p_tnc;
    double denominator = p_c * (1.0 - p_c) * p_t * (1.0 - p_t);

    return (numerator * numerator) / denominator;
}
}
}
