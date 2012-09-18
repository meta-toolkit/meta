#include <iostream>
#include <unordered_map>
#include "knn.h"

using std::unordered_map;
using std::multimap;
using std::string;
using std::vector;
using std::shared_ptr;

string KNN::classify(Document & query, shared_ptr<Index> index, size_t k)
{
    multimap<double, string> ranking = index->search(query);
    unordered_map<string, size_t> counts;
    size_t numResults = 0;
    for(auto result = ranking.rbegin(); result != ranking.rend() && numResults++ != k; ++result)
    {
        size_t space = result->second.find_first_of(" ") + 1;
        string category = result->second.substr(space, result->second.size() - space);
        counts[category]++;
    }

    string best = "[no results]";
    size_t high = 0;
    for(auto & count: counts)
    {
        if(count.second > high)
        {
            best = count.first;
            high = count.second;
        }
    }

    return best;
}

string KNN::classify(Document & query, vector<shared_ptr<Index>> indexes,
        vector<double> weights, size_t k)
{
    // make sure weights sum to 1.0
    double sum = 0.0;
    for(double weight: weights)
        sum += weight;

    if(sum != 1.0)
    {
        std::cerr << "[KNN::classify]: weights in ensemble do not add to 1.0" << std::endl;
        return "";
    }

    return "chuck testa";
}

multimap<double, string>
KNN::internal::normalize(const multimap<double, string> & scores)
{
    multimap<double, string> normalized;
    return normalized;
}
