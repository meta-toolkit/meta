#include <algorithm>
#include <limits>
#include <random>
#include <utility>

#include "classify/classifier/linear_svm.h"
#include "parallel/parallel_for.h"
#include "util/common.h"

namespace meta {
namespace classify {

linear_svm::linear_svm(std::unique_ptr<index::forward_index> & idx,
                       loss_function loss,
                       double cost, 
                       double epsilon, 
                       size_t max_iter):
    classifier{idx},
    loss_{ loss }, 
    cost_{ cost }, 
    epsilon_{ epsilon }, 
    max_iter_{ max_iter }
{ /* nothing */ }

void linear_svm::train( const std::vector<doc_id> & docs ) {
    double diag;
    double upper;
    if( loss_ == loss_function::L2 ) {
        diag = 1.0 / ( 2.0 * cost_ );
        upper = std::numeric_limits<double>::infinity();
    } else {
        diag = 0;
        upper = cost_;
    }
    std::vector<double> qbar_ii( docs.size() );
    std::unordered_set<class_label> classes;
    term_id max_id = 0;
    for( size_t i = 0; i < docs.size(); ++i ) {
        classes.insert( _idx->label(docs[i]) );
        qbar_ii[i] = diag;
        for( const auto & count : _idx->counts(docs[i]) ) {
            qbar_ii[i] += count.second * count.second;
            max_id = std::max( max_id, count.first );
        }
    }

    for( auto & label : classes ) {
        weights_[ label ].resize( max_id + 1 );
    }

    parallel::parallel_for( weights_.begin(), 
                            weights_.end(), 
                            [&]( const std::pair<class_label, std::vector<double> &> & p ) {
        train_one( p.first, p.second, docs, diag, upper, qbar_ii );
    });
}

class_label linear_svm::classify( doc_id d_id ) {
    class_label best_label = weights_.begin()->first;
    double best_dot = std::numeric_limits<double>::min();
    for( const auto & w : weights_ ) {
        double dot = 0;
        for( const auto & count : _idx->counts(d_id) ) {
            dot += count.second * safe_at( w.second, count.first );
        }
        if( dot > best_dot ) {
            best_dot = dot;
            best_label = w.first;
        }
    }
    return best_label;
}

void linear_svm::reset() {
    weights_.clear();
}

double linear_svm::safe_at( const std::vector<double> & weight, const term_id & id ) {
    if( id >= weight.size() )
        return 0.0;
    return weight[ id ];
}

void linear_svm::train_one( const class_label & label, 
                            std::vector<double> & weight,
                            const std::vector<doc_id> & docs, 
                            double diag,
                            double upper,
                            const std::vector<double> & qbar_ii ) {
    auto labeler = [&label](std::unique_ptr<index::forward_index> & idx, doc_id d_id ) { 
        if( idx->label(d_id) == label )
            return 1;
        return -1;
    };

    std::vector<double> alpha( docs.size() );
    std::vector<size_t> indices( docs.size() );
    for( size_t i = 0; i < docs.size(); ++i ) {
        alpha[i] = 0.0;
        indices[i] = i;
    }

    size_t partition_size = indices.size();

    std::random_device d;
    std::mt19937 g( d() );

    double pg_max_old = std::numeric_limits<double>::max();
    double pg_min_old = std::numeric_limits<double>::min();

    for( size_t iter = 0; iter < max_iter_; ++iter ) {
        double pg_max = 0;
        double pg_min = std::numeric_limits<double>::max();
        std::shuffle( indices.begin(), indices.begin() + partition_size, g );
        for( size_t j = 0; j < partition_size; ++j ) {
            size_t i = indices[j];
            double grad = 0.0;
            for( auto & count : _idx->counts(docs[i]) ) {
                grad += count.second * safe_at( weight, count.first );
            }
            grad = grad * labeler( _idx, docs[i] ) - 1 + diag * alpha[i];

            double projected_grad = 0;
            if( alpha[i] == 0 ) {
                if( grad > pg_max_old ) {
                    shrink_partition( indices, j, partition_size );
                    continue;
                }
                if( grad < 0 )
                    projected_grad = grad;
            } else if( alpha[i] == upper ) {
                if( grad < pg_min_old ) {
                    shrink_partition( indices, j, partition_size );
                    continue;
                }
                if( grad > 0 )
                    projected_grad = grad;
            } else {
                projected_grad = grad;
            }

            pg_max = std::max( projected_grad, pg_max );
            pg_min = std::min( projected_grad, pg_min );

            if( std::fabs( projected_grad ) > 1e-12 ) {
                double abar = alpha[i];
                alpha[i] = std::min( std::max( alpha[i] - grad / qbar_ii[i], 0.0 ), 
                                     upper );
                double w = ( alpha[i] - abar ) * labeler( _idx, docs[i] );
                for( auto & count : _idx->counts(docs[i]) ) {
                    weight[ count.first ] += w * count.second;
                }
            }
        }

        if( pg_max - pg_min < epsilon_ ) {
            if( partition_size == docs.size() ) {
                break;
            } else {
                partition_size = docs.size();
                pg_max_old = std::numeric_limits<double>::max();
                pg_min_old = std::numeric_limits<double>::min();
                continue;
            }
        }

        pg_max_old = ( pg_max <= 0 ) ? std::numeric_limits<double>::max() : pg_max;
        pg_min_old = ( pg_min >= 0 ) ? std::numeric_limits<double>::min() : pg_min;
    }
}

void linear_svm::shrink_partition( std::vector<size_t> & indices, size_t & j, size_t & partition_size ) {
    partition_size -= 1;
    std::swap( indices[j], indices[partition_size] );
    j -= 1;
}

}
}
