/**
 * @file topics/lda_gibbs.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DST_LDA_GIBBS_H_
#define _DST_LDA_GIBBS_H_

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

#include "index/document.h"
#include "tokenizers/ngram/ngram_tokenizer.h"
#include "topics/lda_model.h"

namespace meta {
namespace topics {

/**
 * A LDA topic model implemented using a collapsed gibbs sampler.
 *
 * @see http://www.pnas.org/content/101/suppl.1/5228.full.pdf
 */
class lda_gibbs : public lda_model {
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
        lda_gibbs( std::vector<index::document> & docs, size_t num_topics, 
                   double alpha, double beta );

        /**
         * Destructor: virtual for potential subclassing.
         */
        virtual ~lda_gibbs() { }
        
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
        virtual void run( size_t num_iters, double convergence = 1e-6 );
        
    protected:
        
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
        size_t sample_topic( term_id term, size_t doc );
        
        /**
         * Computes \f$P(z_i = j | w, \boldsymbol{z})\f$.
         *
         * @param term The current word we are sampling for.
         * @param doc The document in which the term resides.
         * @param topic The topic \f$j\f$ we want to compute the
         *  probability for.
         */
        double compute_probability( term_id term, size_t doc, size_t topic ) const;

        /**
         * Computes the probability that the given term appears in the
         * given topic.
         *
         * @param term The term we are concerned with.
         * @param topic The topic we are concerned with.
         */
        virtual double compute_term_topic_probability( term_id term, size_t topic ) const;

        /**
         * Computes the probability that the given topic is picked for the
         * given document.
         *
         * @param doc The document we are concerned with.
         * @param topic The topic we are concerned with.
         */
        virtual double compute_doc_topic_probability( size_t doc, size_t topic ) const;
        
        /**
         * Computes how many times the given term has been assigned the
         * given topic in the vector of assignments \f$\boldsymbol{z}\f$.
         *
         * @param term The term we are concerned with.
         * @param topic The topic we are concerned with.
         */
        virtual double count_term( term_id term, size_t topic ) const;
        
        /**
         * Computes how many times the given topic has been assigned for
         * *any* word in the corpus.
         *
         * @param topic The topic we are concerned with.
         */
        virtual double count_topic( size_t topic ) const;
        
        /**
         * Computes how may times the given topic has been chosen for a
         * *any* word in the given document.
         *
         * @param doc The document we are concerned with.
         * @param topic The topic we are concerned with.
         */
        virtual double count_doc( size_t doc, size_t topic ) const;
        
        /**
         * Computes how may times a topic has been assigned to a word in
         * the given document.
         *
         * @param doc The document we are concerned with.
         */
        virtual double count_doc( size_t doc ) const;
        
        /**
         * Initializes the first set of topic assignments for inference.
         * Employs an online application of the sampler where counts are
         * only considered for the words observed so far through the loop.
         */
        virtual void initialize();
        
        /**
         * Performs a sampling iteration.
         *
         * @param init Whether or not to employ the online method.
         *  (defaults to `false`)
         */
        virtual void perform_iteration( bool init = false );

        /**
         * Decreases all counts associated with the given topic, term, and
         * document by one.
         *
         * @param topic The topic in question.
         * @param term The term in question.
         * @param doc The document in question.
         */
        virtual void decrease_counts( size_t topic, term_id term, size_t doc );
        
        /**
         * Increases all counts associated with the given topic, term, and
         * document by one.
         *
         * @param topic The topic in question.
         * @param term The term in question.
         * @param doc The document in question.
         */
        virtual void increase_counts( size_t topic, term_id term, size_t doc );
        
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
        
        using topic_id = size_t; // for clarity below
        
        /**
         * The topic assignment for every word in every document. Note that
         * the same word occurring multiple times in one document could
         * potentially have many different topics assigned to it, so we are
         * not using term_ids here, but our own contrived intra document term id.
         */
        std::unordered_map<doc_id, std::unordered_map<size_t, size_t>>
        doc_word_topic_;
        
        /**
         * Contains the counts for each word being assigned a given topic.
         */
        std::unordered_map<topic_id, std::unordered_map<term_id, size_t>> 
        topic_term_count_;
        
        /**
         * Contains the counts for each topic being assigned in a given
         * document.
         */
        std::unordered_map<doc_id, std::unordered_map<topic_id, size_t>>
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
         * The random number generator for the sampler.
         */
        std::mt19937 rng_;
        std::random_device dev_;

};

}
}

#endif
