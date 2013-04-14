/**
 * @file classifier.cpp
 */

#include <iostream>
#include <random>
#include "classify/classifier.h"

namespace meta {
namespace classify {

using std::vector;
using index::Document;

confusion_matrix classifier::test(const vector<Document> & docs) const
{
    confusion_matrix matrix;
    for(auto & d: docs)
        matrix.add(classify(d), d.getCategory());

    return matrix;
}

confusion_matrix classifier::cross_validate(const vector<Document> & input_docs, size_t k, int seed)
{
    // docs might be ordered by class, so we have to make sure things are shuffled
    vector<Document> docs(input_docs);
    std::mt19937 gen(seed);
    std::shuffle(docs.begin(), docs.end(), gen);

    confusion_matrix matrix;
    size_t step_size = docs.size() / k;
    for(size_t i = 0; i < k; ++i)
    {
        std::cerr << "Cross-validating fold " << (i + 1) << "/" << k << std::endl;
        reset(); // clear any learning data already calculated
        train(vector<Document>(docs.begin() + step_size, docs.end()));
        matrix += test(vector<Document>(docs.begin(), docs.begin() + step_size));
        std::rotate(docs.begin(), docs.begin() + step_size, docs.end());
    }

    return matrix;
}

}
}
