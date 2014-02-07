/**
 * @file ir_eval.cpp
 * @author Sean Massung
 */

#include <iostream>

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "cpptoml.h"
#include "index/eval/ir_eval.h"
#include "util/mapping.h"
#include "util/printing.h"
#include "util/shim.h"

namespace meta
{
namespace index
{

ir_eval::ir_eval(const std::string& config_file)
{
    auto config = cpptoml::parse_file(config_file);
    auto path = config.get_as<std::string>("query-judgements");
    if (!path)
        throw ir_eval_exception{"query judgement file was not specified"};

    init_index(*path);
}

void ir_eval::init_index(const std::string& path)
{
    std::ifstream in{path};
    std::string line;

    // four (or three) fields per line
    query_id q_id;
    uint8_t unused; // TREC compatability, optional
    doc_id d_id;
    uint8_t relevance;

    while (in.good())
    {
        std::getline(in, line);
        bool trec = (std::count_if(line.begin(), line.end(), [](char ch)
            { return ch == ' '; }) == 3); // 3 spaces == 4 columns
        std::istringstream iss{line};
        iss >> q_id;
        if (trec)
            iss >> unused;
        iss >> d_id;
        iss >> relevance;

        if (relevance != '0')
            _qrels[q_id][d_id] = relevance;
    }
}

double ir_eval::precision(const std::vector<std::pair<doc_id, double>>& results,
                          query_id q_id) const
{
    if (results.size() == 0)
        return 0.0;

    auto ht = _qrels.find(q_id);
    if (ht == _qrels.end())
        return 0.0;

    return relevant_retrieved(results, q_id) / results.size();
}

double ir_eval::recall(const std::vector<std::pair<doc_id, double>>& results,
                       query_id q_id) const
{
    if (results.size() == 0)
        return 0.0;

    auto ht = _qrels.find(q_id);
    if (ht == _qrels.end())
        return 0.0;

    return relevant_retrieved(results, q_id) / ht->second.size();
}

double ir_eval::relevant_retrieved(const std::vector<
                                       std::pair<doc_id, double>>& results,
                                   query_id q_id) const
{
    double rel = 0.0;
    const auto& ht = _qrels.find(q_id);
    for (auto& res : results)
        if (map::safe_at(ht->second, res.first) != 0)
            ++rel;

    return rel;
}

double ir_eval::f1(const std::vector<std::pair<doc_id, double>>& results,
                   query_id q_id, double beta) const
{
    double p = precision(results, q_id);
    double r = recall(results, q_id);
    double denominator = (beta * beta * p) + r;

    if (denominator < 0.00000001)
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
