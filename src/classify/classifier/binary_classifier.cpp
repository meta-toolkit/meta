/**
 * @file binary_classifier.cpp
 * @author Chase Geigle
 */

#include "meta/classify/classifier/binary_classifier.h"

namespace meta
{
namespace classify
{

bool binary_classifier::classify(const feature_vector& instance) const
{
    auto prediction = predict(instance);
    return prediction >= 0;
}
}
}
