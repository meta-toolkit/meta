/**
 * @file feature_selector.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "features/feature_selector.h"

namespace meta
{
namespace features
{
feature_selector::feature_selector(const std::string& prefix,
                                   std::shared_ptr<index::forward_index> idx)
    : prefix_{prefix},
      idx_{std::move(idx)},
      selected_{prefix + ".selected", idx_->unique_terms()}
{
}

void feature_selector::print_summary() const
{
    std::cout << "Feature summary" << std::endl;
}

double feature_selector::term_and_class(term_id term, label_id label) const
{
    return co_occur_[label][term];
}

double feature_selector::not_term_and_class(term_id term, label_id label) const
{
    return 1.0 - term_and_class(term, label) - not_term_and_class(term, label)
           - term_and_not_class(term, label);
}

double feature_selector::term_and_not_class(term_id term, label_id label) const
{
    return term_prob_[term] - term_and_class(term, label);
}

double feature_selector::not_term_and_not_class(term_id term,
                                                label_id label) const
{
    return class_prob_[label] - term_and_class(term, label);
}
}
}
