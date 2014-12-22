/**
 * @file state.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_STATE_H_
#define META_PARSER_STATE_H_

#include <memory>
#include <vector>

#include "parser/transition.h"
#include "parser/trees/node.h"
#include "parser/trees/parse_tree.h"
#include "util/persistent_stack.h"

namespace meta
{
namespace parser
{

class state
{
  public:
    using stack_type = util::persistent_stack<std::unique_ptr<node>>;

    state(const parse_tree& tree);

    void advance(transition trans);

    const node* stack_item(size_t depth) const;
    const leaf_node* queue_item(size_t depth) const;

    size_t stack_size() const;
    size_t queue_size() const;

  private:
    stack_type stack_;
    std::vector<std::unique_ptr<leaf_node>> queue_;
    size_t q_idx_;
    bool done_;
};
}
}
#endif
