/**
 * @file naive_bayes.h
 * @author Sean Massung
 */

#ifndef _NAIVE_BAYES_H_
#define _NAIVE_BAYES_H_

#include <unordered_map>
#include "classify/classifier.h"

namespace meta {
namespace classify {

/**
 * Implements the Naive Bayes classifier, a simplistic probabilistic classifier
 * that uses Bayes' theorem with strong feature independence assumptions.
 */
class naive_bayes: public classifier
{
    public:

        /**
         * Constructor: learns class models based on a collection of training
         * documents.
         * @param train_docs The tokenized training documents
         * @param alpha Optional smoothing parameter for term frequencies
         * @param beta Optional smoothing parameter for class frequencies
         */
        naive_bayes(const std::vector<index::Document> & train_docs,
                double alpha = 0.1, double beta = 0.1);

        /**
         * Classifies a document into a specific group, as determined by
         * training data.
         * @param doc The document to classify
         * @return the class it belongs to
         */
        std::string classify(const index::Document & doc) const;

    private:

        /**
         * Calculates P(term|class) and P(class) for all the training documents.
         */
        void calc_probs(const std::vector<index::Document> & docs);

        /**
         * Contains P(term|class) for each class.
         */
        std::unordered_map<std::string, std::unordered_map<index::TermID, double>> _term_probs;

        /**
         * Contains the number of documents in each class
         */
        std::unordered_map<std::string, size_t> _class_counts;

        /** The number of training documents */
        size_t _total_docs;

        /** smoothing parameter for term counts */
        const double _alpha;

        /** smoothing parameter for class counts */
        const double _beta;
};

}
}

#endif
