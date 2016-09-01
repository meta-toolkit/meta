/**
 * @file training_data.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_TRAINING_DATA_H_
#define META_PARSER_TRAINING_DATA_H_

#include "meta/config.h"
#include "meta/parser/sr_parser.h"

namespace meta
{
namespace parser
{

/**
 * Training data for the parser.
 */
class sr_parser::training_data
{
  public:
    /**
     * @param trees The raw training data
     * @param seed The seed to used for seeding the rng for shuffling
     * examples
     */
    training_data(std::vector<parse_tree>& trees,
                  std::default_random_engine::result_type seed);

    /**
     * Preprocesses all of the training trees. This currently runs the
     * following transformations across all of the training data:
     *
     * - annotation_remover
     * - empty_remover
     * - unary_chain_remover
     * - head_finder
     * - binarizer
     *
     * @return a transition_map to associate all transition names with ids
     * in the binarized training data
     */
    transition_map preprocess();

    /**
     * Shuffles the training data.
     */
    void shuffle();

    /**
     * @return the number of training examples
     */
    size_t size() const;

    /**
     * @param idx The index to seek into the training data
     * @return the parse tree at that position in the training data
     */
    const parse_tree& tree(size_t idx) const;

    /**
     * @param idx The index to seek into the training data
     * @return the transitions taken to assemble the gold tree
     */
    const std::vector<trans_id>& transitions(size_t idx) const;

  private:
    /**
     * A reference to the collection of training trees.
     */
    std::vector<parse_tree>& trees_;

    /**
     * The gold standard transitions for each tree.
     */
    std::vector<std::vector<trans_id>> all_transitions_;

    /**
     * The vector of indices used for fast shuffling.
     */
    std::vector<size_t> indices_;

    /**
     * The random number generator used for shuffling.
     */
    std::default_random_engine rng_;
};
}
}
#endif
