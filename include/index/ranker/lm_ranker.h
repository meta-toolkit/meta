/**
 * @file lm_ranker.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _LM_RANKER_H_
#define _LM_RANKER_H_

#include <vector>
#include <utility>
#include "meta.h"
#include "index/inverted_index.h"
#include "index/ranker/ranker.h"

namespace meta {
namespace index {

/**
 * Scores documents according to one of three different smoothed language model
 * scoring methods described in "A Study of Smoothing Methods for Language
 * Models Applied to Ad Hoc Information Retrieval" by Zhai and Lafferty, 2001.
 */
class language_model_ranker: public ranker
{
    public:
        /**
         * @param idx
         * @param query
         * @param tpair
         * @param dpair
         * @param unique_terms
         */
        double score_one(inverted_index & idx,
                         const document & query,
                         const std::pair<term_id, uint64_t> & tpair,
                         const std::pair<doc_id, uint64_t> & dpair,
                         uint64_t unique_terms) const override;

        /**
         * Calculates the smoothed probability of a term.
         * @param idx
         * @param t_id
         * @param d_id
         */
        virtual double smoothed_prob(inverted_index & idx,
                                     term_id t_id,
                                     doc_id d_id) const = 0;

        /**
         * A document-dependent constant.
         * @param d_id The id of the document to calculate the constant for
         * @param idx
         */
        virtual double doc_constant(inverted_index & idx,
                                    doc_id d_id) const = 0;

        /**
         * Default destructor.
         */
        virtual ~language_model_ranker() = default;
};

}
}

#endif
