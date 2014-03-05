/**
 * @file sgd.cpp
 */

#include <numeric>
#include <random>

#include "classify/classifier/sgd.h"
#include "index/postings_data.h"

namespace meta {
namespace classify {

template <class LossFunction>
sgd<LossFunction>::sgd(std::shared_ptr<index::forward_index> idx,
                       double alpha,
                       double gamma,
                       double bias,
                       double lambda,
                       size_t max_iter):
    classifier{std::move(idx)},
    positive_label_{""}, // this will be inferred later
    alpha_{alpha},
    gamma_{gamma},
    bias_{0},
    bias_weight_{bias},
    lambda_{lambda},
    max_iter_{max_iter}
{ /* nothing */ }

template <class LossFunction>
sgd<LossFunction>::sgd(std::shared_ptr<index::forward_index> idx,
                       class_label positive_label,
                       double alpha,
                       double gamma,
                       double bias,
                       double lambda,
                       size_t max_iter):
    classifier{std::move(idx)},
    positive_label_{positive_label},
    alpha_{alpha},
    gamma_{gamma},
    bias_{0},
    bias_weight_{bias},
    lambda_{lambda},
    max_iter_{max_iter}
{ /* nothing */ }


template <class LossFunction>
double sgd<LossFunction>::predict(doc_id d_id) const {
    auto pdata = _idx->search_primary(d_id);
    return predict(pdata->counts());
}

template <class LossFunction>
double sgd<LossFunction>::predict(const counts_t & doc) const {
    double dot = coeff_ * bias_ * bias_weight_;
    for (const auto & count : doc)
        dot += coeff_ * count.second * weights_[count.first];
    return dot;
}

template <class LossFunction>
void sgd<LossFunction>::train( const std::vector<doc_id> & docs ) {
    weights_.resize(_idx->unique_terms());
    if( positive_label_ == class_label{""} )
        positive_label_ = class_label{ _idx->label(docs[0]) };

    auto it = std::find_if(docs.begin(), docs.end(), [&](doc_id doc) {
        return _idx->label(doc) != positive_label_;
    });
    negative_label_ = class_label{_idx->label(*it)};

    std::vector<size_t> indices( docs.size() );
    std::vector<int> labels( docs.size() );
    for( size_t i = 0; i < docs.size(); ++i ) {
        indices[i] = i;
        labels[i] = _idx->label(docs[i]) == positive_label_ ? 1 : -1;
    }
    std::random_device d;
    std::mt19937 g{ d() };
    size_t t = 0;
    double sum_loss = 0;
    double prev_sum_loss = std::numeric_limits<double>::max();
    for( size_t iter = 0; iter < max_iter_; ++iter ) {
        std::shuffle( indices.begin(), indices.end(), g );
        for( size_t i = 0; i < indices.size(); ++i ) {
            t += 1;

            // check for convergence every 10th of the dataset
            if (t % (docs.size() / 10) == 0) {
                sum_loss /= docs.size() / 10;
                if (std::abs(prev_sum_loss - sum_loss) < gamma_)
                    return;
                prev_sum_loss = sum_loss;
                sum_loss = 0;
            }

            auto pdata = _idx->search_primary(docs[indices[i]]);
            const counts_t & doc = pdata->counts();

            // get output prediction
            // this is the binary case where p is either +1 or -1
            double prediction = predict(doc);
            int actual = labels[indices[i]];

            sum_loss += loss_.loss(prediction, actual);

            double error_derivative = loss_.derivative(prediction, actual);
            coeff_ *= (1 - alpha_ * lambda_);

            // renormalize vector of coefficient is too small
            if( coeff_ < 1e-9 ) {
                bias_ *= coeff_;
                for( auto & w : weights_ )
                    w *= coeff_;
                coeff_ = 1;
            }

            double update = -alpha_ * error_derivative / coeff_;
            if( update != 0 ) {
                for (const auto & count : doc) {
                    weights_[ count.first ] += update * count.second;
                }
                bias_ += update * bias_weight_;
            }
        }
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
    weights_.clear();
    coeff_ = 1;
    bias_ = 0;
}


}
}
