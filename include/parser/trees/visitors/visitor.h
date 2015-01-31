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

template <class T>
class const_visitor
{
  public:
    using result_type = T;
    virtual result_type operator()(const leaf_node&) = 0;
    virtual result_type operator()(const internal_node&) = 0;
};

template <class T>
class visitor
{
  public:
    using result_type = T;
    virtual result_type operator()(leaf_node&) = 0;
    virtual result_type operator()(internal_node&) = 0;
};
}
}

#endif
