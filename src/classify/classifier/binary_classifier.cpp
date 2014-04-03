/**
 * @file binary_classifier.cpp
 * @author Chase Geigle
 */

#include "classify/classifier/binary_classifier.h"

namespace meta
{
namespace classify
{

binary_classifier::binary_classifier(std::shared_ptr<index::forward_index> idx,
                                     class_label positive, class_label negative)
    : classifier{std::move(idx)},
      positive_{std::move(positive)},
      negative_{std::move(negative)}
{
    // nothing
}

class_label binary_classifier::classify(doc_id d_id)
{
    double prediction = predict(d_id);
    return prediction >= 0 ? positive_ : negative_;
}

const class_label& binary_classifier::positive_label() const
{
    return positive_;
}

const class_label& binary_classifier::negative_label() const
{
    return negative_;
}

}
}
