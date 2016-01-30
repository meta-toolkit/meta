/**
 * @file lm_ranker.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LM_RANKER_H_
#define META_LM_RANKER_H_

#include "meta/index/ranker/ranker.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace index
{

/**
 * Scores documents according to one of three different smoothed language model
 * scoring methods described in "A Study of Smoothing Methods for Language
 * Models Applied to Ad Hoc Information Retrieval" by Zhai and Lafferty, 2001.
 */
class language_model_ranker : public ranker
{
  public:
    /// The identifier for this ranker.
    const static util::string_view id;

    /**
     * @param sd
     */
    float score_one(const score_data& sd) override;

    float initial_score(const score_data& sd) const override;

    /**
     * Calculates the smoothed probability of a term.
     * @param sd
     */
    virtual float smoothed_prob(const score_data& sd) const = 0;

    /**
     * A document-dependent constant.
     * @param sd
     */
    virtual float doc_constant(const score_data& sd) const = 0;

    /**
     * Default destructor.
     */
    virtual ~language_model_ranker() = default;
};
}
}

#endif
