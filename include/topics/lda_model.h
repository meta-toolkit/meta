/**
 * @file topics/lda_model.h
 */

#ifndef _DST_TOPICS_LDA_MODEL_H_
#define _DST_TOPICS_LDA_MODEL_H_

#include <cstddef>
#include <vector>
#include <string>

#include "index/document.h"
#include "tokenizers/ngram_word_tokenizer.h"

namespace meta {
namespace topics {

/**
 * An LDA topic model base class.
 */
class lda_model {
    public:
        lda_model( std::vector<index::Document> & docs, size_t num_topics );

        virtual ~lda_model() { }

        virtual void run( size_t num_iters, double convergence ) = 0;

        /**
         * Saves the topic proportions \f$\theta_d\f$ for each document to
         * the given file. Saves the distributions in a simple "human
         * readable" plain-text format.
         *
         * @param filename The file to save \f$theta\f$ to.
         */
        void save_doc_topic_distributions( const std::string & filename ) const;
        
        /**
         * Saves the term distributions \f$\phi_j\f$ for each topic to the
         * given file. Saves the distributions in a simple "human readable"
         * plain-text format.
         *
         * @param filename The file to save \f$\phi\f$ to.
         */
        void save_topic_term_distributions( const std::string & filename ) const;
        
        /**
         * Saves the TermID --> String mapping to the given file.
         *
         * @param filename The file to save the term mapping to.
         */
        void save_term_mapping( const std::string & filename ) const;
        
        /**
         * Saves the current model to a set of files beginning with prefix:
         * prefix.phi, prefix.theta, and prefix.terms.
         *
         * @param prefix The prefix for all generated files over this
         *  model.
         */
        void save( const std::string & prefix ) const;

    protected:
        
        lda_model & operator=( const lda_model & ) = delete;
        lda_model( const lda_model & ) = delete;

        /**
         * Computes the probability that the given term appears in the
         * given topic.
         *
         * @param term The term we are concerned with.
         * @param topic The topic we are concerned with.
         */
        virtual double compute_term_topic_probability( index::TermID term, size_t topic ) const = 0;

        /**
         * Computes the probability that the given topic is picked for the
         * given document.
         *
         * @param doc The document we are concerned with.
         * @param topic The topic we are concerned with.
         */
        virtual double compute_doc_topic_probability( size_t doc, size_t topic ) const = 0;

        std::vector<index::Document> & docs_;
        tokenizers::ngram_word_tokenizer tokenizer_;
        size_t num_topics_;
        size_t num_words_;
};

}
}

#endif
