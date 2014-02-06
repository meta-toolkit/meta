/**
 * @file ir_eval.cpp
 */

#include <iomanip>
#include "cpptoml.h"
#include "index/eval/ir_eval.h"
#include "util/printing.h"

namespace meta
{
namespace index
{

ir_eval::ir_eval(const std::string& config_file)
{
}

double ir_eval::precision(const std::vector<std::pair<doc_id, double>>& results,
                          query_id q_id) const
{
    return 0.0;
}

double ir_eval::recall(const std::vector<std::pair<doc_id, double>>& results,
                       query_id q_id) const
{
    return 0.0;
}

double ir_eval::f1(const std::vector<std::pair<doc_id, double>>& results,
                   query_id q_id, double beta) const
{
    double p = precision(results, q_id);
    double r = recall(results, q_id);
    double denominator = (beta * beta * p) + r;

    if(denominator < 0.00000001)
        return 0.0;

    double numerator = (1.0 + beta * beta) * p * r;
    return numerator / denominator;
}

void ir_eval::print_stats(const std::vector<std::pair<doc_id, double>>& results,
                          query_id q_id, std::ostream& out) const
{
    auto w = std::setw(10);
    out << w << printing::make_bold("  F1 Score:") << w << f1(results, q_id)
        << w << printing::make_bold("  Precision:") << w
        << precision(results, q_id) << w << printing::make_bold("  Recall:")
        << w << recall(results, q_id) << std::endl;
}
}
}
