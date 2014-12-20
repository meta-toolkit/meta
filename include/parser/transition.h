/**
 * @file transition.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_TRANSITION_H_
#define META_PARSER_TRANSITION_H_

#include "meta.h"
#include "util/optional.h"
#include "util/comparable.h"

namespace meta
{
namespace parser
{

class transition : public util::comparable<transition>
{
  public:
    enum class type_t
    {
        SHIFT = 0,
        REDUCE_L,
        REDUCE_R,
        UNARY,
        FINALIZE,
        IDLE
    };

    transition(type_t t);

    transition(type_t t, class_label lbl);

    type_t type() const;

    const class_label& label() const;

    bool operator<(const transition& lhs) const;

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  private:
    type_t type_;
    util::optional<class_label> label_;
};

std::ostream& operator<<(std::ostream& os, const transition& trans);

}
}

#endif
