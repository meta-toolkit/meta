/**
 * @file classifier.cpp
 */

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

confusion_matrix classifier::cross_validate(const vector<Document> & docs, size_t k) const
{
    return confusion_matrix();
}

}
}
