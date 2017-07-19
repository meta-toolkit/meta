/**
 * @file topic_model.h
 * @author Matt Kelly
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOPICS_DEFAULT_TERM_SCORER_H_
#define META_TOPICS_DEFAULT_TERM_SCORER_H_

#include "meta/index/forward_index.h"
#include "meta/meta.h"
#include "meta/topics/topic_model.h"

namespace meta
{
namespace topics
{

/* Scores terms in some way Chase can probably articulate and or source */
class default_term_scorer
{
  public:
    /**
     * @param model The topic model to score
     */
    default_term_scorer(topics::topic_model model);

    /**
     * @param k The topic id
     * @param v The term id
     */
    double operator()(topic_id k, term_id v) const;

  private:
    /**
     * The topic model
     */
    topics::topic_model model_;

    /**
     * The sums
     */
    std::vector<double> sums_;
};
}
}

#endif