/**
 * @file evalb.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_EVALB_H_
#define META_PARSER_EVALB_H_

#include "meta/parser/trees/parse_tree.h"

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
    /**
     * @return the number of matched constituents
     */
    uint64_t matched() const
    {
        return proposed_correct_;
    }

    /**
     * @return the number of constituents in proposed parses
     */
    uint64_t proposed_total() const
    {
        return proposed_total_;
    }

    /**
     * @return the number of constituents in gold parses
     */
    uint64_t gold_total() const
    {
        return gold_total_;
    }

    /**
     * @return the precision of labeled constituents, as a percent.
     */
    double labeled_precision() const
    {
        return static_cast<double>(matched()) / proposed_total() * 100;
    }

    /**
     * @return the recall of labeled constituents, as a percent.
     */
    double labeled_recall() const
    {
        return static_cast<double>(matched()) / gold_total() * 100;
    }

    /**
     * @return the F1 measure of labeled constituents, as a percent.
     */
    double labeled_f1() const
    {
        return 2 * (labeled_precision() * labeled_recall())
               / (labeled_precision() + labeled_recall());
    }

    /**
     * @return the percent of trees that were a 100% match with the gold
     * tree
     */
    double perfect() const
    {
        return static_cast<double>(perfect_) / total_trees_ * 100;
    }

    /**
     * @return the average crossing for all of the trees
     */
    double average_crossing() const
    {
        return static_cast<double>(crossed_) / total_trees_;
    }

    /**
     * @return the percentage of trees that had no crossings
     */
    double zero_crossing() const
    {
        return static_cast<double>(zero_crossing_) / total_trees_ * 100;
    }

    /**
     * @param proposed The proposed parse
     * @param gold The gold standard parse
     */
    void add_tree(parse_tree proposed, parse_tree gold);

  private:
    /**
     * The number of correct constituents in proposed parse trees.
     */
    uint64_t proposed_correct_ = 0;

    /**
     * The number of total constituents in proposed parse trees.
     */
    uint64_t proposed_total_ = 0;

    /**
     * The number of total constituents in gold parse trees.
     */
    uint64_t gold_total_ = 0;

    /**
     * The number of parse trees that were perfect matches with gold trees.
     */
    uint64_t perfect_ = 0;

    /**
     * The total number of crossings in proposed trees.
     */
    uint64_t crossed_ = 0;

    /**
     * The total number of proposed trees that had no crossings.
     */
    uint64_t zero_crossing_ = 0;

    /**
     * The total number of parse trees.
     */
    uint64_t total_trees_ = 0;
};
}
}
#endif
