/**
 * @file slda.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _SLDA_H_
#define _SLDA_H_

#include <string>
#include <vector>
#include "index/document.h"
#include "util/invertible_map.h"

namespace meta {
namespace topics {

/**
 * Provides a class wrapper for Chong Wang's supervised latent Dirichlet
 * allocation implementation: http://www.cs.cmu.edu/~chongw/slda/
 */
class slda
{
    public:

        /**
         * @param alpha The value of alpha, the parameter for a uniform
         * Dirichlet prior
         */
        slda(const std::string & slda_path, double alpha);

        /**
         * Estimates topic models based on each document's class label.
         * @param docs The "training" corpus
         */
        void estimate(const std::vector<index::document> & docs);

        /**
         * @return distribution of terms for each label, sorted by weight
         */
        std::unordered_map<class_label, std::vector<std::pair<term_id, double>>>
            class_distributions() const;

        /**
         * @return a sorted list of terms across all classes sorted by weight
         */
        std::vector<std::pair<term_id, double>> select_features() const;

        /**
         * Infers labels based on a collection of documents.
         * @param docs The "testing" corpus
         */
        void infer(const std::vector<index::document> & docs);

    private:

        /**
         * @return a vector of betas for each class
         */
        std::vector<std::vector<double>> get_probs() const;

        /**
         * @param docs
         * @return the number of classes seen in the corpus
         */
        size_t create_input_files(const std::vector<index::document> & docs);

        /** value of alpha for sLDA */
        const double _alpha;

        /** path to the sLDA library */
        const std::string _slda_path;

        /** maps classes to integers for slda i/o */
        util::invertible_map<class_label, int> _mapping;
};

}
}

#endif
