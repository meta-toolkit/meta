/**
 * @file lda_cvb.cpp
 */

#include "topics/lda_cvb.h"

#include <iomanip>

namespace meta {
namespace topics {

lda_cvb::lda_cvb( std::vector<index::Document> & docs, size_t num_topics, 
                  double alpha, double beta ) 
        : lda_model{ docs, num_topics }, alpha_{ alpha }, beta_{ beta } { }


lda_cvb::~lda_cvb() { }

void lda_cvb::run( size_t num_iters, double convergence ) {
    std::cerr << "Running LDA inference...\n";
    initialize();
    common::end_progress("\t\t\t");
    for( size_t i = 0; i < num_iters; ++i ) {
        std::cerr << "Iteration " << i + 1 << "/" << num_iters << ":\r";
        double max_change = perform_iteration();
        std::cerr << "\t\t\t\t\t\tmaximum change in gamma: " << max_change
            << "    \r";
        common::end_progress("\t\t\t");
        if( max_change <= convergence ) {
            std::cerr << "\nFound convergence after " << i + 1 << " iterations!\n";
            break;
        }
    }
    std::cerr << "\nFinished maximum iterations, or found convergence!\n";
}

void lda_cvb::initialize() {
    std::random_device rdev;
    std::mt19937 rng( rdev() );
    std::cerr << "Initalizing::\r";
    for( size_t i = 0; i < docs_.size(); ++i ) {
        common::show_progress( i, docs_.size(), 10, "\t\t\t" );
        for( auto & freq : docs_[i].getFrequencies() ) {
            double sum = 0;
            for( size_t k = 0; k < num_topics_; ++k ) {
                double random = rng();
                gamma_[ i ][ freq.first ][ k ] = random;
                sum += random;
            }
            for( size_t k = 0; k < num_topics_; ++k ) {
                gamma_[ i ][ freq.first ][ k ] /= sum;
                double contrib = freq.second * gamma_[ i ][ freq.first ][ k ];
                doc_topic_mean_[i][k]           += contrib;
                topic_term_mean_[k][freq.first] += contrib;
                topic_mean_[k]                  += contrib;
            }
        }
    }
}

double lda_cvb::perform_iteration() {
    double max_change = 0;
    for( size_t i = 0; i < docs_.size(); ++i ) {
        common::show_progress( i, docs_.size(), 10, "\t\t\t" );
        for( auto & freq : docs_[i].getFrequencies() ) {
            // remove this word occurrence from means
            for( size_t k = 0; k < num_topics_; ++k ) {
                double contrib = freq.second * gamma_[i][freq.first][k];
                doc_topic_mean_[i][k]           -= contrib;
                topic_term_mean_[k][freq.first] -= contrib;
                topic_mean_[k]                  -= contrib;
            }
            double min = 0;
            double max = 0;
            std::unordered_map<topic_id, double> old_gammas = gamma_[i][freq.first];
            for( size_t k = 0; k < num_topics_; ++k ) {
                // recompute gamma using CVB0 formula
                gamma_[i][freq.first][k] = 
                    compute_term_topic_probability( freq.first, k )
                    * doc_topic_mean_.at( i ).at( k );
                min = std::min( min, gamma_[i][freq.first][k] );
                max = std::max( max, gamma_[i][freq.first][k] );
            }
            // normalize gamma and update means
            for( size_t k = 0; k < num_topics_; ++k ) {
                gamma_[i][freq.first][k] = ( gamma_[i][freq.first][k] - min ) / ( max - min );
                double contrib = freq.second * gamma_[i][freq.first][k];
                doc_topic_mean_[i][k]           += contrib;
                topic_term_mean_[k][freq.first] += contrib;
                topic_mean_[k]                  += contrib;
                max_change = std::max( max_change, std::abs( old_gammas[k] - gamma_[i][freq.first][k] ) );
            }
        }
    }
    return max_change;
}

double lda_cvb::compute_term_topic_probability( index::TermID term, size_t topic ) const {
    return ( topic_term_mean_.at(topic).at(term) + beta_ )
        / ( topic_mean_.at(topic) + num_words_ * beta_ );
}

double lda_cvb::compute_doc_topic_probability( size_t doc, size_t topic ) const {
    return ( doc_topic_mean_.at( doc ).at( topic ) + alpha_ )
        / ( docs_[ doc ].getLength() + num_topics_ * alpha_ );
}

}
}
