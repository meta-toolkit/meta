/**
 * @file knn.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_KNN_H_
#define _META_KNN_H_

#include <unordered_set>
#include "index/inverted_index.h"
#include "index/forward_index.h"
#include "index/ranker/ranker.h"
#include "classify/classifier/classifier.h"

namespace meta {
namespace classify {

/**
 * Implements the k-Nearest Neighbor lazy learning classification algorithm.
 */
template <class Ranker>
class knn: public classifier
{
    public:
        /**
         * @param idx The index to run the classifier on
         * @param ranker
         * @param k The value of k in k-NN
         * @param args Arguments to the chosen ranker constructor
         */
        template <class... Args>
        knn(index::inverted_index & idx, index::forward_index & f_idx,
                uint16_t k, Args &&... args);

        /**
         * Creates a classification model based on training documents.
         * @param docs The training documents
         */
        void train(const std::vector<doc_id> & docs) override;

        /**
         * Classifies a document into a specific group, as determined by
         * training data.
         * @param d_id The document to classify
         * @return the class it belongs to
         */
        class_label classify(doc_id d_id) override;

        /**
         * Resets any learning information associated with this classifier.
         */
        void reset() override;

    private:
        /**
         * @param scored
         * @param sorted
         */
        class_label select_best_label(
            const std::vector<std::pair<doc_id, double>> & scored,
            const std::vector<std::pair<class_label, uint16_t>> & sorted) const;

        /** the inverted index used for ranking */
        index::inverted_index & _inv_idx;

        /** the value of k in k-NN */
        uint16_t _k;

        /**
         * The ranker that is used to score the queries in the index.
         */
        Ranker _ranker;

        /** documents that are "legal" to be used in the results */
        std::unordered_set<doc_id> _legal_docs;

    public:
        /**
         * Basic exception for knn interactions.
         */
      class knn_exception : public std::runtime_error
      {
        public:
          using std::runtime_error::runtime_error;
      };
};

}
}

#include "classify/classifier/knn.tcc"
#endif
