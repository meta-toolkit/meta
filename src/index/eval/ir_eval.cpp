/**
 * @file ir_eval.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <numeric>
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

    if (!in)
        throw ir_eval_exception{"query judgements file unable to be opened!"};

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
            qrels_[q_id][d_id] = relevance;
    }
}

double ir_eval::precision(const std::vector<std::pair<doc_id, double>>& results,
                          query_id q_id, uint64_t num_docs) const
{
    if (results.empty())
        return 0.0;

    const auto& ht = qrels_.find(q_id);
    if (ht == qrels_.end())
        return 0.0;

    uint64_t denominator = std::min<uint64_t>(results.size(), num_docs);
    return relevant_retrieved(results, q_id, num_docs) / denominator;
}

double ir_eval::recall(const std::vector<std::pair<doc_id, double>>& results,
                       query_id q_id, uint64_t num_docs) const
{
    if (results.empty())
        return 0.0;

    const auto& ht = qrels_.find(q_id);
    if (ht == qrels_.end())
        return 0.0;

    return relevant_retrieved(results, q_id, num_docs) / ht->second.size();
}

double ir_eval::relevant_retrieved(const std::vector
                                   <std::pair<doc_id, double>>& results,
                                   query_id q_id, uint64_t num_docs) const
{
    double rel = 0.0;
    const auto& ht = qrels_.find(q_id);
    uint64_t i = 1;
    for (auto& res : results)
    {
        if (map::safe_at(ht->second, res.first) != 0)
            ++rel;
        if (i++ == num_docs)
            break;
    }

    return rel;
}

double ir_eval::f1(const std::vector<std::pair<doc_id, double>>& results,
                   query_id q_id, uint64_t num_docs, double beta) const
{
    double p = precision(results, q_id, num_docs);
    double r = recall(results, q_id, num_docs);
    double denominator = (beta * beta * p) + r;

    if (denominator < 0.00000001)
        return 0.0;

    double numerator = (1.0 + beta * beta) * p * r;
    return numerator / denominator;
}

double ir_eval::ndcg(const std::vector<std::pair<doc_id, double>>& results,
                     query_id q_id, uint64_t num_docs) const
{
    // find this query's judgements
    const auto& ht = qrels_.find(q_id);
    if (ht == qrels_.end() || results.empty())
        return 0.0;

    // calculate discounted cumulative gain
    double dcg = 0.0;
    uint64_t i = 1;
    for (auto& res : results)
    {
        double rel = map::safe_at(ht->second, res.first); // 0 if non-relevant
        dcg += (std::pow(2.0, rel) - 1.0) / std::log(i + 1.0);
        if (i++ == num_docs)
            break;
    }

    // calculate ideal DCG
    std::vector<uint8_t> rels;
    for (const auto& s : ht->second)
        rels.push_back(s.second);
    std::sort(rels.begin(), rels.end());

    double idcg = 0.0;
    i = 1;
    for (auto& rel : rels)
    {
        idcg += (std::pow(2.0, rel) + 1.0) / std::log(i + 1.0);
        if (i++ == num_docs)
            break;
    }

    return dcg / idcg;
}

double ir_eval::avg_p(const std::vector<std::pair<doc_id, double>>& results,
                      query_id q_id, uint64_t num_docs)
{
    const auto& ht = qrels_.find(q_id);
    if (ht == qrels_.end() || results.empty())
    {
        scores_.push_back(0.0);
        return 0.0;
    }

    // the total number of *possible* relevant documents given the num_docs
    // cutoff point
    auto total_relevant = std::min<uint64_t>(num_docs, ht->second.size());
    uint64_t i = 1;
    double avgp = 0.0;
    double num_rel = 1;
    for (auto& res : results)
    {
        if (map::safe_at(ht->second, res.first) != 0)
        {
            avgp += num_rel / i;
            ++num_rel;
        }
        if (i++ == total_relevant)
            break;
    }

    scores_.push_back(avgp / total_relevant);
    return avgp / total_relevant;
}

double ir_eval::map() const
{
    return std::accumulate(scores_.begin(), scores_.end(), 0.0)
           / scores_.size();
}

double ir_eval::gmap() const
{
    double sum = 0.0;
    for (auto& s : scores_)
        sum += std::log(s + 1.0);
    return sum / scores_.size();
}

void ir_eval::print_stats(const std::vector<std::pair<doc_id, double>>& results,
                          query_id q_id, std::ostream& out)
{
    auto w1 = std::setw(8);
    auto w2 = std::setw(6);
    size_t p = 3;
    uint64_t max = 5;
    out << w1 << printing::make_bold("  NDCG:") << w2 << std::setprecision(p)
        << ndcg(results, q_id, max);
    out << w1 << printing::make_bold("  Avg. P:") << w2 << std::setprecision(p)
        << avg_p(results, q_id, max);
    out << w1 << printing::make_bold("  F1 Score:") << w2
        << std::setprecision(p) << f1(results, q_id);
    out << w1 << printing::make_bold("  Precision:") << w2
        << std::setprecision(p) << precision(results, q_id, max);
    out << w1 << printing::make_bold("  Recall:") << w2 << std::setprecision(p)
        << recall(results, q_id, max);
    out << std::endl;
}

void ir_eval::reset_stats()
{
    scores_.clear();
}
}
}
