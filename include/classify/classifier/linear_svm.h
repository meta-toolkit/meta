/**
 * @file linear_svm.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_CLASSIFY_LINEAR_SVM_H_
#define _META_CLASSIFY_LINEAR_SVM_H_

#include "index/forward_index.h"
#include "classify/classifier/classifier.h"

namespace meta {
namespace classify {

/**
 * A classifier implemented using a coordinate descent algorithm for
 * solving the dual problem for L2- and L1-SVM as detailed in Hsieh et al
 * (ICML 2008). Utilizes the random permutation optimization as well as the
 * shrinking optimization.
 * 
 * @see http://www.csie.ntu.edu.tw/~cjlin/papers/cddual.pdf
 * @see Algorithm 3, in particular.
 */
class linear_svm : public classifier<index::forward_index> {
    public:
        /**
         * Loss function to be used by the SVM. The default is L2.
         */
        enum class loss_function { L2, L1 };

        /**
         * Creates an empty SVM model with the given parameters.
         * 
         * @param idx The index to run the classifier on
         * @param loss The loss function to be used (default: L2)
         * @param cost The cost parameter to be used (default: 1)
         * @param epsilon The termination criteria (default: 0.1)
         * @param max_iter The maximum allowed iterations in the solver.
         */
        linear_svm(index::forward_index & idx, 
                   loss_function loss = loss_function::L2, 
                   double cost = 1.0, 
                   double epsilon = 0.1, 
                   size_t max_iter = 1000 );

        virtual void train(const std::vector<doc_id> & docs) override;

        class_label classify(doc_id d_id) override;

        virtual void reset() override;

    private:
        /**
         * Used to safely index into a vector. Needed for testing because
         * the test vocabulary may exceed the training vocabulary.
         */
        double safe_at( const std::vector<double> & weight,
                        const term_id & id );

        /**
         * Trains a single, binary linear_svm using Algorithm 3 from Hseih
         * et al.
         * 
         * @param label The label for positive instances for this
         *  classifier.
         * @param weight The weight vector for this classifier.
         * @param docs The training examples.
         * @param diag The \f$D_{ii}\f$.
         * @param upper The upper bound \f$U\f$.
         * @param qbar_ii The entries of \f$\overbar{Q}_{ii}\f$.
         */
        void train_one( const class_label & label, 
                        std::vector<double> & weight,
                        const std::vector<doc_id> & docs, 
                        double diag,
                        double upper,
                        const std::vector<double> & qbar_ii );

        /**
         * Used for the shrinking optimization to remove an element j from
         * the current partition.
         */
        void shrink_partition(
                std::vector<size_t> & indices,
                size_t & j,
                size_t & partition_size );

        /**
         * The loss function for the problem.
         */
        loss_function loss_;

        /**
         * The cost parameter \f$C\f$.
         */
        double cost_;

        /**
         * The termination criteria \f$\eps\f$.
         */
        double epsilon_;

        /**
         * The maximum number of iterations for the solver.
         */
        size_t max_iter_;

        /**
         * The weight vectors for each internal binary classifier.
         */
        std::unordered_map<class_label, std::vector<double>> weights_;
};

}
}
#endif
