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
#include "meta/parser/trees/node.h"
#include "meta/parser/trees/visitors/tree_transformer.h"

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

    /**
     * Copy constructor.
     */
    parse_tree(const parse_tree&);

    /**
     * Move constructor.
     */
    parse_tree(parse_tree&&) = default;

    /**
     * Assignment operator.
     */
    parse_tree& operator=(parse_tree);

    /**
     * Swaps this parse tree with the given parse tree.
     */
    void swap(parse_tree&);

    /**
     * Transforms the current parse tree by using the given tree_transformer.
     */
    void transform(tree_transformer&);

    /**
     * Runs a visitor over the parse tree. Non-const version.
     *
     * @return the result from the visitor.
     */
    template <class Visitor>
    typename std::remove_reference<Visitor>::type::result_type
        visit(Visitor&& vtor)
    {
        return root_->accept(vtor);
    }

    /**
     * Runs a visitor over the parse tree. Const version.
     *
     * @return the result from the visitor.
     */
    template <class Visitor>
    typename std::remove_reference<Visitor>::type::result_type
        visit(Visitor&& vtor) const
    {
        return root_->accept(vtor);
    }

    /**
     * Prints a parse tree to a stream. This is non-indented.
     * @param os The stream to print to
     * @param tree The tree to be printed
     * @return the stream
     */
    friend std::ostream& operator<<(std::ostream& os, const parse_tree& tree);

    /**
     * Prints a parse tree to a stream. This is indented.
     * @param os The stream to print to
     */
    void pretty_print(std::ostream& os) const;

    /**
     * @param lhs The left hand side of the expression
     * @param rhs The right hand side of the expression
     * @return whether the two parse trees are equivalent
     */
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
