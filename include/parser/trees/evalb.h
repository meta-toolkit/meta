/**
 * @file evalb.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_EVALB_H_
#define META_PARSER_EVALB_H_

#include "parser/trees/parse_tree.h"

namespace meta
{
namespace parser
{

/**
 * A re-implementation of (some of) the evalb metrics. You should, of
 * course, *always* double check with the *real* evalb in any paper
 * results, but this can be used internally for e.g. convergence testing on
 * a dev set.
 */
class evalb
{
  public:
    uint64_t matched() const
    {
        return proposed_correct_;
    }

    uint64_t proposed_total() const
    {
        return proposed_total_;
    }

    uint64_t gold_total() const
    {
        return gold_total_;
    }

    double labeled_precision() const
    {
        return static_cast<double>(matched()) / proposed_total() * 100;
    }

    double labeled_recall() const
    {
        return static_cast<double>(matched()) / gold_total() * 100;
    }

    double labeled_f1() const
    {
        return 2 * (labeled_precision() * labeled_recall())
               / (labeled_precision() + labeled_recall());
    }

    double perfect() const
    {
        return static_cast<double>(perfect_) / total_trees_ * 100;
    }

    double average_crossing() const
    {
        return static_cast<double>(crossed_) / total_trees_;
    }

    double zero_crossing() const
    {
        return static_cast<double>(zero_crossing_) / total_trees_ * 100;
    }

    void add_tree(parse_tree proposed, parse_tree gold);

  private:
    uint64_t proposed_correct_ = 0;
    uint64_t proposed_total_ = 0;
    uint64_t gold_total_ = 0;
    uint64_t perfect_ = 0;
    uint64_t crossed_ = 0;
    uint64_t zero_crossing_ = 0;
    uint64_t total_trees_ = 0;
};
}
}
#endif
