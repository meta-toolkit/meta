/**
 * @file topic_model.h
 * @author Matt Kelly
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/topics/bl_term_scorer.h"

#include "meta/topics/default_term_scorer.h"
#include "meta/topics/topic_model.h"

namespace meta
{
namespace topics
{
bl_term_scorer::bl_term_scorer(topic_model model) : model_(model)
{
    // Compute the denominators for each term's normalized score
    denoms_.reserve(model_.num_words());
    for (term_id t_id{0}; t_id < model_.num_words(); ++t_id)
    {
        double denom = 1.0;
        for (topic_id j{0}; j < model_.num_topics(); ++j)
            denom *= model_.term_probability(j, t_id);
        denom = std::pow(denom, 1.0 / model_.num_topics());
        denoms_.push_back(denom);
    }
}

double bl_term_scorer::operator()(topic_id k, term_id v) const
{
    auto prob = model_.term_probability(k, v);
    return prob * std::log(prob / denoms_[v]);
}
}
}