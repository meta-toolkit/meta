/**
 * @file topic_model.h
 * @author Matt Kelly
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/topics/bl_term_scorer.h"
#include "meta/math/fastapprox.h"
#include "meta/topics/topic_model.h"

namespace meta
{
namespace topics
{
bl_term_scorer::bl_term_scorer(const topic_model& model) : model_(model)
{
    // Compute the sums for each term's score
    sums_.reserve(model_.num_words());
    for (term_id t_id{0}; t_id < model_.num_words(); ++t_id)
    {
        double sum = 0.0;
        for (topic_id j{0}; j < model.num_topics(); ++j)
            sum += fastapprox::fastlog(
                    static_cast<float>(model_.term_probability(j, t_id)));
        sum *= (1.0 / model.num_topics());
        sums_.push_back(sum);
    }
}

double bl_term_scorer::operator()(topic_id k, term_id v) const
{
    auto prob = static_cast<float>(model_.term_probability(k, v));
    return prob * (fastapprox::fastlog(prob) - sums_[v]);
}
}
}
