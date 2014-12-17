/**
 * @file parse_tree.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_PARSE_TREE_H_
#define META_PARSE_PARSE_TREE_H_

#include <memory>
#include "parser/trees/node.h"
#include "parser/trees/visitors/tree_transformer.h"

namespace meta
{
namespace parser
{

/**
 * Represents the parse tree for a sentence. This may either be a sentence
 * parsed from training data, or the output from a trained parser on test
 * data.
 *
 * @todo determine what parts of analyzers::parse_tree are worth
 * keeping---that class deals specifically with trees read from the output
 * of the Stanford parser. When we have our own, we may still want some of
 * that functionality to allow people to use parsers that are not our
 * own?
 */
class parse_tree
{
  public:
    /**
     * Creates a new parse tree by taking ownership of a subtree pointed
     * to by the argument.
     *
     * @param root The desired root of the parse tree
     */
    parse_tree(std::unique_ptr<node> root);

    void transform(tree_transformer&);

    template <class Visitor>
    typename std::remove_reference<Visitor>::type::result_type
        visit(Visitor&& vtor)
    {
        return root_->accept(vtor);
    }

    template <class Visitor>
    typename std::remove_reference<Visitor>::type::result_type
        visit(Visitor&& vtor) const
    {
        return root_->accept(vtor);
    }

    friend std::ostream& operator<<(std::ostream& os, const parse_tree& tree);

    friend bool operator==(const parse_tree& lhs, const parse_tree& rhs);

  private:
    /**
     * The root of the parse tree.
     */
    std::unique_ptr<node> root_;
};
}
}

#endif