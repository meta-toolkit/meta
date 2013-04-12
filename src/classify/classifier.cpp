/**
 * @file classifier.cpp
 */

#include "classify/classifier.h"

namespace meta {
namespace classify {

using std::vector;
using index::Document;

ConfusionMatrix classifier::test(const vector<Document> & docs) const
{
    ConfusionMatrix matrix;
    for(auto & d: docs)
        matrix.add(classify(d), d.getCategory());

    return matrix;
}

ConfusionMatrix classifier::cross_validate(const vector<Document> & docs, size_t k) const
{
    return ConfusionMatrix();
}

}
}
