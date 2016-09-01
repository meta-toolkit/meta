/**
 * @file transition.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_TRANSITION_H_
#define META_PARSER_TRANSITION_H_

#include "meta/config.h"
#include "meta/meta.h"
#include "meta/util/comparable.h"
#include "meta/util/optional.h"

namespace meta
{
namespace parser
{

MAKE_NUMERIC_IDENTIFIER(trans_id, uint16_t)

/**
 * Represents a transition taken by the parser. Consists of a type and, for
 * UNARY or REDUCE_L and REDUCE_R actions, a label.
 */
class transition : public util::comparable<transition>
{
  public:
    /**
     * The transition types.
     */
    enum class type_t : uint8_t
    {
        SHIFT = 0,
        REDUCE_L,
        REDUCE_R,
        UNARY,
        FINALIZE,
        IDLE
    };

    /**
     * Constructs a transition given a type. This constructor should be
     * used for SHIFT, FINALIZE, and IDLE transitions.
     *
     * @param t The transition type
     */
    transition(type_t t);

    /**
     * Constructs a transition given a type and label. This constructor
     * should be used for REDUCE_L, REDUCE_R, and UNARY transitions.
     *
     * @param t The transition type
     * @param lbl The label for the transition.
     */
    transition(type_t t, class_label lbl);

    /**
     * @return the type of this transition
     */
    type_t type() const;

    /**
     * @return the label for this transition
     */
    const class_label& label() const;

    /**
     * @param rhs The transition to compare against
     * @return whether the current transition is lexicographically less
     * than the other transition
     */
    bool operator<(const transition& rhs) const;

    /**
     * Exception thrown during interactions with transitions.
     */
    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  private:
    /**
     * The type of this transition.
     */
    type_t type_;

    /**
     * The label of this transition, if it exists.
     */
    util::optional<class_label> label_;
};

/**
 * Prints a transition to the stream.
 * @param os The stream to print to
 * @param trans The transition to be printed
 * @return the stream
 */
std::ostream& operator<<(std::ostream& os, const transition& trans);

/**
 * Prints a transition type to the stream.
 * @param os The stream to print to
 * @param type The transition type to print
 * @return the stream
 */
std::ostream& operator<<(std::ostream& os, const transition::type_t type);
}
}

#endif
