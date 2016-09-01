/**
 * @file transition_finder.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_TRANSITION_FINDER_H_
#define META_PARSER_TRANSITION_FINDER_H_

#include <vector>

#include "meta/config.h"
#include "meta/parser/transition.h"
#include "meta/parser/trees/visitors/visitor.h"

namespace meta
{
namespace parser
{

/**
 * This is a visitor that converts a parse tree into a list of transitions
 * that a shift-reduce parser would have to take in order to generate it.
 *
 * The transitions are built up via side-effect on the visitor, so its
 * visiting functions return void, but the visitor itself has a function to
 * obtain the list of transitions after visiting has been completed.
 */
class transition_finder : public const_visitor<void>
{
  public:
    void operator()(const leaf_node&) override;
    void operator()(const internal_node&) override;

    /**
     * Extracts the transitions out of the visitor. This moves the
     * transition vector from the visitor.
     *
     * @return the transtions found when running the visitor on the tree
     */
    std::vector<transition> transitions();

  private:
    /**
     * Storage for the transitions.
     */
    std::vector<transition> transitions_;
};

/**
 * Basic exception for transition finder operations.
 */
class transition_finder_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#endif
