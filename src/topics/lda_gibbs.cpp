/**
 * @file lda_gibbs.cpp
 */

#include "topics/lda_gibbs.h"
#include "util/common.h"

#include <cmath>
#include <cassert>

namespace topics {
    
lda_gibbs::lda_gibbs( std::vector<Document> & docs, size_t num_topics, 
                      double alpha, double beta ) :
        lda_model{ docs, num_topics }, alpha_{ alpha }, beta_{ beta } {
}

void lda_gibbs::run( size_t num_iters, double convergence /* = 1e-6 */ ) {
    std::cerr << "Running LDA inference...\n";
    initialize();
    double likelihood = corpus_likelihood();
    assert( !std::isnan( likelihood ) );
    for( size_t i = 0; i < num_iters; ++i ) {
        std::cerr << "Iteration " << i + 1 << "/" << num_iters << ":\r";
        perform_iteration();
        double likelihood_update = corpus_likelihood();
        double ratio = std::fabs( ( likelihood - likelihood_update ) 
                                  / likelihood );
        likelihood = likelihood_update;
        std::cerr << "\t\t\t\t\t\tlog likelihood: " << likelihood
            << "    \r";
        if( ratio <= convergence ) {
            std::cerr << "\nFound convergence after " << i + 1 << " iterations!\n";
            break;
        }
    }
    std::cerr << "\nFinished maximum iterations, or found convergence!\n";
}


size_t lda_gibbs::sample_topic( TermID term, size_t doc ) {
    std::vector<double> weights( num_topics_ );
    for( size_t j = 0; j < weights.size(); ++j )
        weights[ j ] = compute_probability( term, doc, j );
    std::discrete_distribution<> dist( weights.begin(), weights.end() );
    return dist( rng_ );
}

double lda_gibbs::compute_probability( TermID term, size_t doc, size_t topic ) const {
    return compute_term_topic_probability( term, topic )
           * compute_doc_topic_probability( doc, topic );
}

double lda_gibbs::compute_term_topic_probability( TermID term, size_t topic ) const {
    return ( count_term( term, topic ) + beta_ ) 
             / ( count_topic( topic ) + num_words_ * beta_ );
}

double lda_gibbs::compute_doc_topic_probability( size_t doc, size_t topic ) const {
    return ( count_doc( doc, topic ) + alpha_ )
             / ( count_doc( doc ) + num_topics_ * alpha_ );
}

double lda_gibbs::count_term( TermID term, size_t topic ) const {
    auto it = topic_term_count_.find( topic );
    if( it == topic_term_count_.end() )
        return 0;
    auto iit = it->second.find( term );
    if( iit == it->second.end() )
        return 0;
    return iit->second;
}

double lda_gibbs::count_topic( size_t topic ) const {
    auto it = topic_count_.find( topic );
    if( it == topic_count_.end() )
        return 0;
    return it->second;
}

double lda_gibbs::count_doc( size_t doc, size_t topic ) const {
    auto it = doc_topic_count_.find( doc );
    if( it == doc_topic_count_.end() )
        return 0;
    auto iit = it->second.find( topic );
    if( iit == it->second.end() )
        return 0;
    return iit->second;
}

double lda_gibbs::count_doc( size_t doc ) const {
    return docs_[ doc ].getLength();
}

void lda_gibbs::initialize() {
    std::cerr << "Initialization:\r";
    perform_iteration( true );
}

void lda_gibbs::perform_iteration( bool init /* = false */ ) {
    for( size_t i = 0; i < docs_.size(); ++i ) {
        Common::show_progress( i, docs_.size(), 10, "\t\t\t" );
        size_t n = 0; // term number within document---constructed
                      // so that each occurrence of the same term
                      // can still be assigned a different topic
        for( const auto & freq : docs_[i].getFrequencies() ) {
            for( size_t j = 0; j < freq.second; ++j ) {
                size_t old_topic = doc_word_topic_[ i ][ n ];
                // don't include current topic assignment in
                // probability calculation
                if( !init )
                    decrease_counts( old_topic, freq.first, i );
                
                // sample a new topic assignment
                size_t topic = sample_topic( freq.first, i );
                doc_word_topic_[ i ][ n ] = topic;
                
                // increase counts
                increase_counts( topic, freq.first, i );
                n += 1;
            }
        }
    }
}

void lda_gibbs::decrease_counts( size_t topic, TermID term, size_t doc ) {
    // decrease topic_term_count_ for the given assignment
    auto & tt_count = topic_term_count_.at( topic ).at( term );
    if( tt_count == 1 )
        topic_term_count_.at( topic ).erase( term );
    else
        tt_count -= 1;
    
    // decrease doc_topic_count_ for the given assignment
    auto & dt_count = doc_topic_count_.at( doc ).at( topic );
    if( dt_count == 1 )
        doc_topic_count_.at( doc ).erase( topic );
    else
        dt_count -= 1;
    
    // decrease topic count
    auto & tc = topic_count_.at( topic );
    if( tc == 1 )
        topic_count_.erase( topic );
    else
        tc -= 1;
}

void lda_gibbs::increase_counts( size_t topic, TermID term, size_t doc ) {
    topic_term_count_[ topic ][ term ] += 1;
    doc_topic_count_[ doc ][ topic ] += 1;
    topic_count_[ topic ] += 1;
}

double lda_gibbs::corpus_likelihood() const {
    double likelihood = num_topics_ * std::lgamma( num_words_ * beta_ );
    assert( !std::isnan(likelihood) );
    for( size_t j = 0; j < num_topics_; ++j ) {
        for( const auto & doc : docs_ ) {
            for( const auto & freq : doc.getFrequencies() ) {
                likelihood += 
                    freq.second 
                    * std::lgamma( count_term( freq.first, j )
                                   + beta_ );
                assert( !std::isnan(likelihood) );
            }
        }
        likelihood -= std::lgamma( count_topic( j ) + num_words_ * beta_ );
        assert( !std::isnan(likelihood) );
    }
    return likelihood;
}

}
