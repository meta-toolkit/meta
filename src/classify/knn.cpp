/**
 * @file knn.cpp
 */

#include <algorithm>
#include <cmath>
#include <utility>
#include "index/document.h"
#include "classify/knn.h"

namespace meta {
namespace classify {

using std::pair;
using std::unordered_map;
using std::multimap;
using std::vector;
using std::shared_ptr;

using namespace knn::internal;
using index::Index;
using index::Document;

ClassLabel knn::classify(Document & query, shared_ptr<Index> index, size_t k)
{
    multimap<double, ClassLabel> ranking = index->search(query);
    return findNN(ranking, k);
}

ClassLabel knn::internal::findNN(const multimap<double, ClassLabel> & ranking, size_t k)
{
    unordered_map<ClassLabel, size_t> counts;
    size_t numResults = 0;
    vector<ClassLabel> orderSeen;
    for(auto result = ranking.rbegin(); result != ranking.rend() && numResults++ != k; ++result)
    {
        size_t space = result->second.find_first_of(" ") + 1;
        ClassLabel category = result->second.substr(space, result->second.size() - space);
        ++counts[category];
        if(std::find(orderSeen.begin(), orderSeen.end(), category) == orderSeen.end())
            orderSeen.push_back(category);
    }

    ClassLabel best = "[no results]";
    size_t high = 0;
    for(auto & count: counts)
    {
        if(count.second > high)
        {
            best = count.first;
            high = count.second;
        }
        // tie break based on initial ranking
        else if(count.second == high && isHigherRank(count.first, best, orderSeen))
        {
            best = count.first;
            high = count.second;
        }
    }

    return best;
}

bool knn::internal::isHigherRank(const ClassLabel & check, const ClassLabel & best,
        const vector<ClassLabel> & orderSeen)
{
    ClassLabel catCheck = check.substr(check.find_first_of(" ") + 1);
    ClassLabel catBest = best.substr(best.find_first_of(" ") + 1);
    for(auto & doc: orderSeen)
    {
        if(doc == catCheck)
            return true;
        if(doc == catBest)
            return false;
    }
    return false;
}

ClassLabel knn::classify(Document & query, vector<shared_ptr<Index>> indexes, vector<double> weights, size_t k)
{
    // make sure weights sum to 1.0
    double sum = 0.0;
    for(double weight: weights)
        sum += weight;

    if(sum != 1.0)
        throw knn_exception("weights in ensemble do not add to 1.0");

    // create a vector of normalized results for each index
    vector<unordered_map<ClassLabel, double>> results;
    for(auto & ptr: indexes)
    {
        Document tempQuery(query);
        multimap<double, ClassLabel> result = ptr->search(tempQuery);
        unordered_map<ClassLabel, double> normalized = normalize(result);
        results.push_back(normalized);
    }

    // iterate over the elements in the first hashtable (they should all be the same),
    //   and add the interpolated values into the final ranking
    multimap<double, ClassLabel> ranking;
    for(auto & rank: results[0])
    {
        double score = 0.0;
        for(size_t i = 0; i < results.size(); ++i)
            score += log(results[i][rank.first]);
            //score += (results[i][rank.first] * weights[i]);
        ranking.insert(make_pair(score, rank.first));
    }

    return findNN(ranking, k);
}

unordered_map<ClassLabel, double>
knn::internal::normalize(const multimap<double, ClassLabel> & scores)
{
    unordered_map<ClassLabel, double> normalized;
    if(scores.empty())
        return normalized;

    double low = scores.begin()->first;
    double high = scores.rbegin()->first;
    double range = high - low;

    for(auto & score: scores)
    {
        double newScore = (score.first - low) / range;
        normalized.insert(make_pair(score.second, newScore));
    }

    return normalized;
}

}
}
