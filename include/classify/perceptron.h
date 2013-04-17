/**
 * @file perceptron.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_PERCEPTRON_H_
#define _META_CLASSIFY_PERCEPTRON_H_

#include <vector>
#include <unordered_map>

#include "classify/classifier.h"
#include "meta.h"

namespace meta {
namespace classify {

/**
 * Implements the Perceptron classifier, a simplistic linear classifier for
 * linearly-separable data.
 */
class perceptron : public classifier {
    public:
        /**
         * Constructs a perceptron classifier with the given learning rate,
         * error threshold, and maximum iterations.
         * 
         * @param alpha \f$alpha\f$, the learning rate
         * @param gamma \f$gamma\f$, the error threshold
         * @param bias \f$b\f$, the bias
         * @param max_iter The maximum number of iterations for training.
         */ 
        perceptron( double alpha = 0.1, double gamma = 0.05, double bias = 0, 
                    size_t max_iter = 100 );

        /**
         * Trains the perceptron on the given training documents.
         * Maintains a set of weight vectors \f$w_1,\ldots,w_K\f$ where
         * \f$K\f$ is the number of classes and updates them for each
         * training document seen in each iteration. This continues until
         * the error threshold is met or the maximum number of iterations
         * is completed.
         * @param docs The training set.
         */
        void train( const std::vector<index::Document> & docs ) override;

        /**
         * Classifies the given document. 
         * The class label returned is 
         * \f$\argmax_k(w_k^\intercal x_n + b)\f$---in other words, the
         * class whose associated weight vector gives the highest result.
         * 
         * @param doc The document to be classified.
         * @return The class label determined for the document.
         */
        class_label classify( const index::Document & doc ) override;

        /**
         * Resets all learned information for this perceptron so it may be
         * re-learned.
         */
        void reset() override;

    private:

        /**
         * Returns the given term's weight in the weight vector for the
         * given class.
         * 
         * @param label The class label for the weight vector we want.
         * @param term The term whose weight should be returned.
         */
        double get_weight( const class_label & label, const term_id & term ) const;

        /**
         * Initializes the weight vectors to zero for every class label.
         * 
         * @param docs The set of documents to collect class labels from.
         */
        void zero_weights( const std::vector<index::Document> & docs );

        /**
         * The weight vectors for each class label.
         */
        std::unordered_map<class_label, std::unordered_map<term_id, double>> weights_;

        const double alpha_;    /// \f$\alpha\f$, the learning rate.
        const double gamma_;    /// \f$\gamma\f$, the error threshold.
        const double bias_;     /// \f$b\f$, the bias.
        const size_t max_iter_; /// The maximum number of iterations for training.
};

}
}

#endif
