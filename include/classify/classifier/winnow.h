/**
 * @file winnow.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _WINNOW_H_
#define _WINNOW_H_

#include <vector>
#include <unordered_map>
#include "index/forward_index.h"
#include "classify/classifier/classifier.h"
#include "meta.h"

namespace meta {
namespace classify {

/**
 * Implements the Winnow classifier, a simplistic linear classifier for
 * linearly-separable data. As opposed to winnow (which uses an additive
 * update rule), winnow uses a multiplicative update rule.
 */
class winnow: public classifier
{
    public:
        /**
         * Constructs a winnow classifier with the given multiplier,
         * error threshold, and maximum iterations.
         *
         * @param idx The index to run the classifier on
         * @param m \f$m\f$, the multiplicative learning rate
         * @param gamma \f$gamma\f$, the error threshold
         * @param max_iter The maximum number of iterations for training.
         */
        winnow(std::shared_ptr<index::forward_index> idx, double m = 1.5,
               double gamma = 0.05, size_t max_iter = 100);

        /**
         * Trains the winnow on the given training documents.
         * Maintains a set of weight vectors \f$w_1,\ldots,w_K\f$ where
         * \f$K\f$ is the number of classes and updates them for each
         * training document seen in each iteration. This continues until
         * the error threshold is met or the maximum number of iterations
         * is completed.
         * @param docs The training set.
         */
        void train(const std::vector<doc_id> & docs) override;

        /**
         * Classifies the given document.
         * The class label returned is
         * \f$\argmax_k(w_k^\intercal x_n + b)\f$---in other words, the
         * class whose associated weight vector gives the highest result.
         *
         * @param doc The document to be classified.
         * @return The class label determined for the document.
         */
        class_label classify(doc_id d_id) override;

        /**
         * Resets all learned information for this winnow so it may be
         * re-learned.
         */
        void reset() override;

        const static constexpr char* id = "winnow";

    private:

        /**
         * Returns the given term's weight in the weight vector for the
         * given class.
         *
         * @param label The class label for the weight vector we want.
         * @param term The term whose weight should be returned.
         */
        double get_weight(const class_label & label,
                          const term_id & term) const;

        /**
         * Initializes the weight vectors to zero for every class label.
         *
         * @param docs The set of documents to collect class labels from.
         */
        void zero_weights(const std::vector<doc_id> & docs);

        /**
         * The weight vectors for each class label.
         */
        std::unordered_map<
            class_label,
            std::unordered_map<term_id, double>
        > weights_;

        /// \f$\m\f$, the multiplicative learning rate.
        const double m_;

        /// \f$\gamma\f$, the error threshold.
        const double gamma_;

        /// The maximum number of iterations for training.
        const size_t max_iter_;
};

}
}

#endif
