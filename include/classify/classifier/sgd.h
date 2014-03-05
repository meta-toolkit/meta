/**
 * @file sgd.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_SGD_H_
#define _META_CLASSIFY_SGD_H_

#include <vector>
#include <unordered_map>
#include "index/forward_index.h"
#include "classify/classifier/classifier.h"
#include "classify/loss/all.h"
#include "meta.h"

namespace meta {
namespace classify {

/**
 * Implements stochastic gradient descent for learning binary linear
 * classifiers. These may be extended to multiclass classification using
 * the one_vs_all or all_vs_all adapters.
 */
template <class LossFunction>
class sgd : public classifier {
    public:
        /**
         * @param idx The index to run the classifier on
         * @param alpha \f$alpha\f$, the learning rate
         * @param gamma \f$gamma\f$, the error threshold
         * @param bias \f$b\f$, the bias
         * @param lambda \f$\lambda\f$, the regularization constant
         * @param max_iter The maximum number of iterations for training.
         */
        sgd(index::forward_index & idx,
            double alpha = 0.001,
            double gamma = 1e-6,
            double bias = 1,
            double lambda = 0.0001,
            size_t max_iter = 50);

        /**
         * @param idx The index to run the classifier on
         * @param positive_label The label for the positive class (all
         *  others are assumed to be negative)
         * @param alpha \f$alpha\f$, the learning rate
         * @param gamma \f$gamma\f$, the error threshold
         * @param bias \f$b\f$, the bias
         * @param lambda \f$\lambda\f$, the regularization constant
         * @param max_iter The maximum number of iterations for training.
         */
        sgd(index::forward_index & idx,
            class_label positive_label,
            double alpha = 0.001,
            double gamma = 1e-6,
            double bias = 1,
            double lambda = 0.0001,
            size_t max_iter = 50);

        /**
         * Trains the sgd on the given training documents.
         * Maintains a set of weight vectors \f$w_1,\ldots,w_K\f$ where
         * \f$K\f$ is the number of classes and updates them for each
         * training document seen in each iteration. This continues until
         * the error threshold is met or the maximum number of iterations
         * is completed.
         * @param docs The training set.
         */
        void train( const std::vector<doc_id> & docs ) override;

        /**
         * Classifies the given document.
         * The class label returned is
         * \f$\argmax_k(w_k^\intercal x_n + b)\f$---in other words, the
         * class whose associated weight vector gives the highest result.
         *
         * @param doc The document to be classified.
         * @return The class label determined for the document.
         */
        class_label classify( doc_id d_id ) override;

        /**
         * Returns the dot product with the current weight vector. Used
         * mainly for generalization of a binary decision problem to a
         * multiclass decision problem.
         *
         * @param doc The document to compute the dot product with.
         */
        double predict(doc_id d_id) const;

        /**
         * Resets all learned information for this sgd so it may be
         * re-learned.
         */
        void reset() override;

    private:

        /// The label for positive examples.
        class_label positive_label_;

        /// The label for negative examples.
        class_label negative_label_;

        /// The weights vector.
        std::vector<double> weights_;

        /// The scalar coefficient for the weights vector.
        double coeff_{1.0};

        /// \f$\alpha\f$, the learning rate.
        const double alpha_;

        /// \f$\gamma\f$, the error threshold.
        const double gamma_;

        /// \f$b\f$, the bias.
        double bias_;

        /// The weight of the bias term for each document (defaults to 1)
        double bias_weight_;

        /// \f$\lambda\f$, the regularization constant
        const double lambda_;

        /// The maximum number of iterations for training.
        const size_t max_iter_;

        /// The loss function to be used for the update.
        LossFunction loss_;

        /**
         * Typedef for the sparse vector training/test instances.
         */
        using counts_t = std::vector<std::pair<term_id, double>>;

        /**
         * Helper function that takes a sparse vector. Used as a
         * performance optimization during training.
         *
         * @param doc the document to form a prediction for
         */
        double predict(const counts_t & doc) const;
};

}
}

#include "classify/classifier/sgd.tcc"
#endif
