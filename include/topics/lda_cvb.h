/**
 * @file lda_cvb.h
 */

#ifndef _DST_TOPICS_LDA_CVB_H_
#define _DST_TOPICS_LDA_CVB_H_

#include <cassert>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "index/document.h"
#include "tokenizers/ngram_tokenizer.h"
#include "topics/lda_model.h"
#include "util/common.h"
namespace topics {

/**
 * lda_cvb: An implementation of LDA that uses collapsed variational bayes
 * for inference. Specifically, it uses the CVB0 algorithm detailed in
 * Asuncion et. al.
 * 
 * @see http://www.ics.uci.edu/~asuncion/pubs/UAI_09.pdf
 */
class lda_cvb : public lda_model {
    public:
        lda_cvb( std::vector<Document> & docs, size_t num_topics, 
                 double alpha, double beta ) 
                : lda_model{ docs, num_topics }, alpha_{ alpha }, 
                  beta_{ beta } {
        }

        void run( size_t num_iters, double convergence = 1e-3 ) {
            std::cerr << "Running LDA inference...\n";
            initialize();
            for( size_t i = 0; i < num_iters; ++i ) {
                std::cerr << "Iteration " << i + 1 << "/" << num_iters << ":\r";
                double max_change = perform_iteration();
                std::cerr << "\t\t\t\t\t\tmaximum change in gamma: " << max_change
                    << "    \r";
                if( max_change <= convergence ) {
                    std::cerr << "\nFound convergence after " << i + 1 << " iterations!\n";
                    break;
                }
            }
            std::cerr << "\nFinished maximum iterations, or found convergence!\n";
        }

    protected:
        void initialize() {
            std::random_device rdev;
            std::mt19937 rng( rdev() );
            std::cerr << "Initalizing::\r";
            for( size_t i = 0; i < docs_.size(); ++i ) {
                Common::show_progress( i, docs_.size(), 10, "\t\t\t" );
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

        double perform_iteration() {
            double max_change = 0;
            for( size_t i = 0; i < docs_.size(); ++i ) {
                Common::show_progress( i, docs_.size(), 10, "\t\t\t" );
                for( auto & freq : docs_[i].getFrequencies() ) {
                    // remove this word occurrence from means
                    for( size_t k = 0; k < num_topics_; ++k ) {
                        double contrib = freq.second * gamma_[i][freq.first][k];
                        doc_topic_mean_[i][k]           -= contrib;
                        topic_term_mean_[k][freq.first] -= contrib;
                        topic_mean_[k]                  -= contrib;
                    }
                    double sum = 0;
                    std::unordered_map<topic_id, double> old_gammas = gamma_[i][freq.first];
                    for( size_t k = 0; k < num_topics_; ++k ) {
                        // recompute gamma using CVB0 formula
                        gamma_[i][freq.first][k] = 
                            compute_term_topic_probability( freq.first, k )
                            * doc_topic_mean_.at( i ).at( k );
                        sum += gamma_[i][freq.first][k];
                    }
                    // normalize gamma (is that necessary with CVB0?) and update means
                    for( size_t k = 0; k < num_topics_; ++k ) {
                        gamma_[i][freq.first][k] /= sum;
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

        virtual double compute_term_topic_probability( TermID term, size_t topic ) const {
            return ( topic_term_mean_.at(topic).at(term) + beta_ )
                     / ( topic_mean_.at(topic) + num_words_ * beta_ );
        }

        virtual double compute_doc_topic_probability( size_t doc, size_t topic ) const {
            return ( doc_topic_mean_.at( doc ).at( topic ) + alpha_ )
                     / ( docs_[ doc ].getLength() + num_topics_ * alpha_ );
        }

        using topic_id = size_t;

        std::unordered_map<DocID, std::unordered_map<topic_id, double>>
        doc_topic_mean_;

        std::unordered_map<topic_id, std::unordered_map<TermID, double>>
        topic_term_mean_;

        std::unordered_map<topic_id, double> topic_mean_;

        std::unordered_map<
            DocID, 
            std::unordered_map<TermID, std::unordered_map<topic_id, double>>
        > gamma_;

        double alpha_;
        double beta_;
};

}

#endif
