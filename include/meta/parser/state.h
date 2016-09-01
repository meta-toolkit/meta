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

#include "meta/config.h"
#include "meta/parser/transition.h"
#include "meta/parser/trees/node.h"
#include "meta/parser/trees/parse_tree.h"
#include "meta/sequence/sequence.h"
#include "meta/util/persistent_stack.h"

namespace meta
{
namespace parser
{

/**
 * Represents the current parser state of a shift-reduce parser. Parser
 * states consist of a stack of partial parse trees, a queue of
 * pre-terminals, and whether or not the state is "finished".
 *
 * The stack is represented using a persistent stack structure to ensure
 * that updates to the parser state occur in O(1) time. The queue is
 * static, so it can be represented as a vector + an index and updated in
 * O(1) time.
 */
class state
{
  public:
    /**
     * A persistent stack of partial parse trees.
     */
    using stack_type = util::persistent_stack<std::unique_ptr<node>>;

    /**
     * The underlying queue type.
     */
    using queue_type = std::vector<std::unique_ptr<leaf_node>>;

    /**
     * Constructs a state from a parse tree. This is used to generate the
     * starting state for parsing during training.
     */
    state(const parse_tree& tree);

    /**
     * Constructs a state from a POS-tagged sequence. This generates teh
     * starting state for parsing during test time.
     */
    state(const sequence::sequence& sentence);

    /**
     * Advances the current state by taking the given transition.
     * @param trans The transition to take to move the state forward
     */
    state advance(const transition& trans) const;

    /**
     * Checks if a transition is legal from a current state.
     *
     * @see http://www.aclweb.org/anthology/W09-3825 Appendix
     * @param trans The transition to check
     * @return whether that transition is legal
     */
    bool legal(const transition& trans) const;

    /**
     * Returns a transition used in situations where there is not a
     * transition in the model satisfying the constraints given in the
     * original paper so that the parser can at least make progress.
     */
    transition emergency_transition() const;

    /**
     * @param depth The depth to seek to in the stack
     * @return the node on the stack at the given depth
     */
    const node* stack_item(size_t depth) const;

    /**
     * @param depth The depth to seek to in the queue
     * @return the node on teh queue at the given depth
     */
    const leaf_node* queue_item(int64_t depth) const;

    /**
     * @return the number of partial parse trees on the stack.
     */
    size_t stack_size() const;

    /**
     * @return the number of preterminals on the queue.
     */
    size_t queue_size() const;

    /**
     * @return whether or not this state has finished parsing.
     */
    bool finalized() const;

  private:
    state(stack_type stack, std::shared_ptr<queue_type> queue, size_t q_idx,
          bool done);

    /**
     * The stack of partial parse trees.
     */
    stack_type stack_;

    /**
     * The queue of preterminals.
     */
    std::shared_ptr<queue_type> queue_;

    /**
     * The index of the front of the queue.
     */
    size_t q_idx_;

    /**
     * Whether or not this state has finished parsing.
     */
    bool done_;
};
}
}
#endif
