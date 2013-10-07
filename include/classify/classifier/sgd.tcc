/**
 * @file sgd.cpp
 */

#include <numeric>
#include <random>

#include "classify/classifier/sgd.h"

namespace meta {
namespace classify {

template <class LossFunction>
sgd<LossFunction>::sgd(index::forward_index & idx,
                       double alpha,
                       double gamma,
                       double bias,
                       double lambda,
                       size_t max_iter):
    classifier{idx},
    positive_label_{""}, // this will be inferred later
    alpha_{alpha},
    gamma_{gamma},
    bias_{bias},
    lambda_{lambda},
    max_iter_{max_iter}
{ /* nothing */ }

template <class LossFunction>
sgd<LossFunction>::sgd(index::forward_index & idx,
                       class_label positive_label,
                       double alpha,
                       double gamma,
                       double bias,
                       double lambda,
                       size_t max_iter):
    classifier{idx},
    positive_label_{positive_label},
    alpha_{alpha},
    gamma_{gamma},
    bias_{bias},
    lambda_{lambda},
    max_iter_{max_iter}
{ /* nothing */ }

template <class LossFunction>
double sgd<LossFunction>::predict(doc_id d_id) const {
    double dot =  coeff_ * bias_;
    auto pdata = _idx.search_primary(d_id);
    for( const auto & count : pdata->counts() ) {
        dot += coeff_ * count.second * common::safe_at( weights_, count.first );
    }
    return dot;
}


template <class LossFunction>
void sgd<LossFunction>::train( const std::vector<doc_id> & docs ) {
    weights_ = {};

    if( positive_label_ == class_label{""} )
        positive_label_ = class_label{ _idx.label(docs[0]) };

    auto it = std::find_if(docs.begin(), docs.end(), [&](doc_id doc) {
        return _idx.label(doc) != positive_label_;
    });
    negative_label_ = class_label{_idx.label(*it)};

    std::vector<size_t> indices( docs.size() );
    std::iota( indices.begin(), indices.end(), 0 );
    std::random_device d;
    std::mt19937 g{ d() };
    for( size_t iter = 0; iter < max_iter_; ++iter ) {
        double lr = alpha_ / (1 + lambda_ * iter);
        std::shuffle( indices.begin(), indices.end(), g );
        uint64_t error_count = 0;
        for( size_t i = 0; i < indices.size(); ++i ) {
            doc_id doc{docs[indices[i]]};

            // get output prediction
            // this is the binary case where p is either +1 or -1
            double prediction = predict(doc);

            int actual = -1;
            if( _idx.label(doc) == positive_label_ )
                actual = 1;

            if( actual * prediction < 0 )
                ++error_count;

            double error_derivative = loss_.derivative(prediction, actual);
            auto pdata = _idx.search_primary(doc);
            coeff_ *= (1 - lr * lambda_);

            // renormalize vector of coefficient is too small
            if( coeff_ < 1e-4 ) {
                bias_ *= coeff_;
                for( auto & w : weights_ )
                    w.second *= coeff_;
                coeff_ = 1;
            }

            for( const auto & count : pdata->counts() ) {
                weights_[ count.first ] -=
                    lr * error_derivative * count.second / coeff_;
            }
            bias_ -= lr * error_derivative / coeff_;
        }
        if( static_cast<double>(error_count) / docs.size() < gamma_ )
            break;
    }
}

template <class LossFunction>
class_label sgd<LossFunction>::classify(doc_id d_id)
{
    double prediction = predict(d_id);
    if( prediction >= 0 )
        return positive_label_;
    return negative_label_;
}

template <class LossFunction>
void sgd<LossFunction>::reset() {
    weights_ = {};
}


}
}
