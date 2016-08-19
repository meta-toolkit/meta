/**
 * @file odds_ratio.cpp
 * @author Sean Massung
 */

#include "meta/features/odds_ratio.h"

namespace meta
{
namespace features
{
const std::string odds_ratio::id = "odds-ratio";

double odds_ratio::score(const class_label& lbl, term_id tid) const
{
    double p_tc = term_and_class(tid, lbl);
    double p_tnc = term_and_not_class(tid, lbl);
    double numerator = p_tc * (1.0 - p_tnc);
    double denominator = (1.0 - p_tc) * p_tnc;

    // avoid divide by zero
    if (denominator == 0.0)
        return 0.0;

    return std::log(numerator / denominator);
}
}
}
