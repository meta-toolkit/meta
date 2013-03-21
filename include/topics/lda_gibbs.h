/**
 * @file topics/lda_gibbs.h
 */

#ifndef _DST_LDA_GIBBS_H_
#define _DST_LDA_GIBBS_H_

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

#include "index/document.h"
#include "tokenizers/ngram_tokenizer.h"

namespace topics {

/**
 * A LDA topic model implemented using a collapsed gibbs sampler.
 *
 * @see http://www.pnas.org/content/101/suppl.1/5228.full.pdf
 */
class lda_gibbs {
    public:
        /**
         * Constructs the lda model over the given documents, with the
         * given number of topics, and hyperparameters \f$\alpha\f$ and
         * \f$\beta\f$ for the priors on \f$\phi\f$ (topic distributions)
         * and \f$\theta\f$ (topic proportions), respectively.
         *
         * Assumes that the given vector of documents will live for as long
         * as or longer than the lda_gibbs instance.
         *
         * @param docs The documents for the LDA model
         * @param num_topics The number of topics to infer
         * @param alpha The hyperparameter for the Dirichlet prior over
         *  \f$\phi\f$.
         * @param beta The hyperparameter for the Dirichlet prior over
         *  \f$\theta\f$.
         */
        lda_gibbs( std::vector<Document> & docs, size_t num_topics, 
                   double alpha, double beta );
        
        /**
         * Runs the sampler for a maximum number of iterations, or until
         * the given convergence criterion is met. The convergence
         * criterion is determined as the relative difference in log
         * corpus likelihood between two iterations.
         *
         * @param num_iters The maximum number of iterations to run the
         *  sampler for.
         * @param convergence The lowest relative difference in log corpus
         *  likelihood to be allowed before considering the sampler to have
         *  converged.
         */
        void run( size_t num_iters, double convergence = 1e-6 );
        
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
        
    private:
        
        /**
         * Samples a topic from the full conditional distribution
         * \f$P(z_i = j | w, \boldsymbol{z})\f$. Used in both initialization
         * and each normal iteration of the sampler, after removing the
         * current value of \f$z_i\f$ from the vector of assignments
         * \f$\boldsymbol{z}\f$.
         *
         * @param term The term we are sampling a topic assignment for.
         * @param doc The document the term resides in.
         */
        size_t sample_topic( TermID term, size_t doc );
        
        /**
         * Computes \f$P(z_i = j | w, \boldsymbol{z})\f$.
         *
         * @param term The current word we are sampling for.
         * @param doc The document in which the term resides.
         * @param topic The topic \f$j\f$ we want to compute the
         *  probability for.
         */
        double compute_probability( TermID term, size_t doc, size_t topic ) const;

        /**
         * Computes the probability that the given term appears in the
         * given topic.
         *
         * @param term The term we are concerned with.
         * @param topic The topic we are concerned with.
         */
        double compute_term_topic_probability( TermID term, size_t topic ) const;

        /**
         * Computes the probability that the given topic is picked for the
         * given document.
         *
         * @param doc The document we are concerned with.
         * @param topic The topic we are concerned with.
         */
        double compute_doc_topic_probability( size_t doc, size_t topic ) const;
        
        /**
         * Computes how many times the given term has been assigned the
         * given topic in the vector of assignments \f$\boldsymbol{z}\f$.
         *
         * @param term The term we are concerned with.
         * @param topic The topic we are concerned with.
         */
        double count_term( TermID term, size_t topic ) const;
        
        /**
         * Computes how many times the given topic has been assigned for
         * *any* word in the corpus.
         *
         * @param topic The topic we are concerned with.
         */
        double count_topic( size_t topic ) const;
        
        /**
         * Computes how may times the given topic has been chosen for a
         * *any* word in the given document.
         *
         * @param doc The document we are concerned with.
         * @param topic The topic we are concerned with.
         */
        double count_doc( size_t doc, size_t topic ) const;
        
        /**
         * Computes how may times a topic has been assigned to a word in
         * the given document.
         *
         * @param doc The document we are concerned with.
         */
        double count_doc( size_t doc ) const;
        
        /**
         * Initializes the first set of topic assignments for inference.
         * Employs an online application of the sampler where counts are
         * only considered for the words observed so far through the loop.
         */
        void initialize();
        
        /**
         * Performs a sampling iteration.
         *
         * @param init Whether or not to employ the online method.
         *  (defaults to `false`)
         */
        void perform_iteration( bool init = false );

        /**
         * Decreases all counts associated with the given topic, term, and
         * document by one.
         *
         * @param topic The topic in question.
         * @param term The term in question.
         * @param doc The document in question.
         */
        void decrease_counts( size_t topic, TermID term, size_t doc );
        
        /**
         * Increases all counts associated with the given topic, term, and
         * document by one.
         *
         * @param topic The topic in question.
         * @param term The term in question.
         * @param doc The document in question.
         */
        void increase_counts( size_t topic, TermID term, size_t doc );
        
        /**
         * Computes the current courpus log likelihood, given the vector of
         * assignments \f$\boldsymbol{z}\f$.
         */
        double corpus_likelihood() const;
        
        /**
         * lda_gibbs cannot be copy assigned.
         */
        lda_gibbs & operator=( const lda_gibbs & ) = delete;
        
        /**
         * lda_gibbs cannot be copy constructed.
         */
        lda_gibbs( const lda_gibbs & other ) = delete;
        
        /**
         * The tokenizer used for tokenizing the corpus.
         */
        NgramTokenizer tokenizer_;
        
        /**
         * A reference to the corpus.
         */
        std::vector<Document> & docs_;
        
        using topic_id = size_t; // for clarity below
        
        /**
         * The topic assignment for every word in every document. Note that
         * the same word occurring multiple times in one document could
         * potentially have many different topics assigned to it, so we are
         * not using TermIDs here, but our own contrived intra document term id.
         */
        std::unordered_map<DocID, std::unordered_map<size_t, size_t>>
        doc_word_topic_;
        
        /**
         * Contains the counts for each word being assigned a given topic.
         */
        std::unordered_map<topic_id, std::unordered_map<TermID, size_t>> 
        topic_term_count_;
        
        /**
         * Contains the counts for each topic being assigned in a given
         * document.
         */
        std::unordered_map<DocID, std::unordered_map<topic_id, size_t>>
        doc_topic_count_;
        
        /**
         * Contains the number of times the given topic has been assigned
         * to a word. Can be inferred from the above maps, but is included
         * here for performance reasons.
         */
        std::unordered_map<topic_id, size_t> topic_count_;
        
        /**
         * Hyperparameter for the Dirichlet prior over \f$\theta\f$.
         */
        double alpha_;
        
        /**
         * Hyperparameter for the Dirichlet prior over \f$\phi\f$.
         */
        double beta_;
        
        /**
         * The number of topics in the model.
         */
        size_t num_topics_;
        /**
         * The number of words in the corpus vocabulary.
         */
        size_t num_words_;
        
        /**
         * The random number generator for the sampler.
         */
        std::mt19937 rng_;

};

}

#endif
