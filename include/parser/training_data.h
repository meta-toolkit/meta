/**
 * @file training_data.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_TRAINING_DATA_H_
#define META_PARSER_TRAINING_DATA_H_

#include "parser/sr_parser.h"

namespace meta
{
namespace parser
{

class sr_parser::training_data
{
  public:
    training_data(training_options options, std::vector<parse_tree>& trees);

    transition_map preprocess();

    void shuffle();
    size_t size() const;
    const parse_tree& tree(size_t idx) const;
    const std::vector<trans_id>& transitions(size_t idx) const;

  private:
    training_options options;

    std::vector<parse_tree>& trees;
    std::vector<std::vector<trans_id>> all_transitions;

    std::vector<size_t> indices;

    std::default_random_engine rng;
};
}
}
#endif
