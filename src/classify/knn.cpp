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

class_label knn::classify(Document & query, shared_ptr<Index> index, size_t k)
{
    multimap<double, class_label> ranking = index->search(query);
    return findNN(ranking, k);
}

class_label knn::internal::findNN(const multimap<double, class_label> & ranking, size_t k)
{
    unordered_map<class_label, size_t> counts;
    size_t numResults = 0;
    vector<class_label> orderSeen;
    for(auto result = ranking.rbegin(); result != ranking.rend() && numResults++ != k; ++result)
    {
        size_t space = result->second.find_first_of(" ") + 1;
        class_label category = result->second.substr(space, result->second.size() - space);
        ++counts[category];
        if(std::find(orderSeen.begin(), orderSeen.end(), category) == orderSeen.end())
            orderSeen.push_back(category);
    }

    class_label best = "[no results]";
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

bool knn::internal::isHigherRank(const class_label & check, const class_label & best,
        const vector<class_label> & orderSeen)
{
    class_label catCheck = check.substr(check.find_first_of(" ") + 1);
    class_label catBest = best.substr(best.find_first_of(" ") + 1);
    for(auto & doc: orderSeen)
    {
        if(doc == catCheck)
            return true;
        if(doc == catBest)
            return false;
    }
    return false;
}

class_label knn::classify(Document & query, vector<shared_ptr<Index>> indexes, vector<double> weights, size_t k)
{
    // make sure weights sum to 1.0
    double sum = 0.0;
    for(double weight: weights)
        sum += weight;

    if(sum != 1.0)
        throw knn_exception("weights in ensemble do not add to 1.0");

    // create a vector of normalized results for each index
    vector<unordered_map<class_label, double>> results;
    for(auto & ptr: indexes)
    {
        Document tempQuery(query);
        multimap<double, class_label> result = ptr->search(tempQuery);
        unordered_map<class_label, double> normalized = normalize(result);
        results.push_back(normalized);
    }

    // iterate over the elements in the first hashtable (they should all be the same),
    //   and add the interpolated values into the final ranking
    multimap<double, class_label> ranking;
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

unordered_map<class_label, double>
knn::internal::normalize(const multimap<double, class_label> & scores)
{
    unordered_map<class_label, double> normalized;
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
