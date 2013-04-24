#include <numeric>
#include <random>

#include "classify/classifier/perceptron.h"

namespace meta {
namespace classify {

perceptron::perceptron( double alpha, double gamma, double bias,
                        size_t max_iter ) 
        : alpha_{ alpha }, gamma_{ gamma }, bias_{ bias }, 
          max_iter_{ max_iter } { }


double perceptron::get_weight( const class_label & label, 
                               const term_id & term ) const {
    auto weight_it = weights_.find( label );
    if( weight_it == weights_.end() )
        return 0;
    auto term_it = weight_it->second.find( term );
    if( term_it == weight_it->second.end() )
        return 0;
    return term_it->second;
}

void perceptron::zero_weights( const std::vector<index::document> & docs ) {
    for( const auto & d : docs )
        weights_[ d.label() ] = {};
}

void perceptron::train( const std::vector<index::document> & docs ) {
    zero_weights( docs );
    std::vector<size_t> indices( docs.size() );
    std::iota( indices.begin(), indices.end(), 0 );
    std::random_device d;
    std::mt19937 g( d() );
    for( size_t iter = 0; iter < max_iter_; ++iter ) {
        std::shuffle( indices.begin(), indices.end(), g );
        double error_count = 0;
        for( size_t i = 0; i < indices.size(); ++i ) {
            class_label guess = classify( docs[i] );
            class_label actual = docs[i].label();
            if( guess != actual ) {
                error_count += 1;
                for( const auto & count : docs[i].frequencies() ) {
                    weights_[ guess ][ count.first ] -= alpha_ * count.second;
                    weights_[ actual ][ count.first ] += alpha_ * count.second;
                }
            }
        }
        if( error_count / docs.size() < gamma_ )
            break;
    }
}

class_label perceptron::classify( const index::document & doc ) {
    class_label best_label = weights_.begin()->first;
    double best_dot = 0;
    for( const auto & w : weights_ ) {
        double dot = bias_;
        for( const auto & count : doc.frequencies() ) {
            dot += count.second * get_weight( w.first, count.first );
        }
        if( dot > best_dot ) {
            best_dot = dot;
            best_label = w.first;
        }
    }
    return best_label;
}

void perceptron::reset() {
    weights_ = {};
}


}
}
