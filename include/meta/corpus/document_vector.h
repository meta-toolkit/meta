//
// Created by Collin Gress on 10/30/16.
//

#ifndef META_DOCUMENT_VECTOR_H
#define META_DOCUMENT_VECTOR_H

#include <unordered_map>
#include "meta/index/score_data.h"
#include "meta/analyzers/analyzer.h"

namespace meta
{
namespace corpus
{
class document_vector
{
    public:
        /**
         * initializes the unordered map ("query vector") using the feature map
         * from tokenizing the document
         * @param feat_map The feature map we got from tokenizing the document using the inverted index
         * @param idx A reference to our inverted index
         */
        void from_feature_map(analyzers::feature_map<uint64_t>& feat_map, index::inverted_index& idx);

        /**
         * Set a term weight based on term id
         * @param t_id a term id for which we want to set a weight
         * @param weight the weight to assign to this term id in the unordered map
         */
        void set_term(term_id t_id, float weight);

        /*
         * @return the unordered map, i.e. the vector representation
         */
        std::unordered_map<term_id, float>& map();

        /**
         * Set the "vector" directly
         * @param m reference to an unordered map to be stored as vector_
         */
        void map(std::unordered_map<term_id, float>& m);

        /*
         * iterates through the map and prints to std out
         */
        void print();

        bool isEmpty() const;
    private:
        std::unordered_map<term_id, float> vector_;
    };
}
}

#endif //META_DOCUMENT_VECTOR_H
