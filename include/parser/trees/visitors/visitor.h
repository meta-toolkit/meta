/**
 * @file tree_visitor.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_TREE_VISITOR_H_
#define META_PARSE_TREE_VISITOR_H_

namespace meta
{
namespace parser
{

class leaf_node;
class internal_node;

/**
 * Abstract base class for visitors over parse trees that do not modify
 * the underlying tree.
 */
template <class T>
class const_visitor
{
  public:
    /**
     * The result of running the visitor over the tree.
     */
    using result_type = T;

    /**
     * @return the result of running the visitor on the supplied leaf node
     */
    virtual result_type operator()(const leaf_node&) = 0;

    /**
     * @return the result of running the visitor on the supplied internal
     * node
     */
    virtual result_type operator()(const internal_node&) = 0;
};

/**
 * Abstract base class for visitors over parse trees that are allowed to
 * modify the underlying tree.
 */
template <class T>
class visitor
{
  public:
    /**
     * The result of running the visitor over the tree.
     */
    using result_type = T;

    /**
     * @return the result of running the visitor on the supplied leaf node
     */
    virtual result_type operator()(leaf_node&) = 0;

    /**
     * @return the result of running the visitor on the supplied internal
     * node
     */
    virtual result_type operator()(internal_node&) = 0;
};
}
}

#endif
