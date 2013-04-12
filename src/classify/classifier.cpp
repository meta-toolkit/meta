/**
 * @file classifier.cpp
 */

#include "classify/classifier.h"

namespace meta {
namespace classify {

using std::vector;
using index::Document;

ConfusionMatrix classifier::classify_all(const vector<Document> & docs) const
{
    return ConfusionMatrix();
}

ConfusionMatrix classifier::cross_validate(const vector<Document> & docs, size_t k) const
{
    return ConfusionMatrix();
}

}
}
