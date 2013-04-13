/**
 * @file select_slda.h
 */

#ifndef _SELECT_SLDA_H_
#define _SELECT_SLDA_H_

#include <vector>
#include <utility>
#include "classify/select.h"
#include "index/document.h"

namespace meta {
namespace classify {

class select_slda: public feature_select
{
    public:

        /**
         * Constructor.
         * @param docs The documents containing features
         */
        select_slda(const std::vector<index::Document> & docs);

        /**
         * Creates an sLDA input file from the given documents and runs the
         * supervised latent Dirichlet topic modeling algorithm on the data.
         * Features are sorted by weight, and then returned in order in a vector
         * like the other feature selection algorithms.
         * @return a vector of TermIDs sorted by importance
         */
        std::vector<std::pair<TermID, double>> select();

        /**
         * Performs feature selection on a collection of Documents, returning
         * each class's features sorted by usefulness.
         */
        std::unordered_map<ClassLabel, std::vector<std::pair<TermID, double>>>
            select_by_class();

    private:

        const std::vector<index::Document> & _docs;
};

}
}

#endif
