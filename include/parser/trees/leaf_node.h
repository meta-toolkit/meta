/**
 * @file leaf_node.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSE_LEAF_NODE_H_
#define META_PARSE_LEAF_NODE_H_

#include "parser/trees/node.h"
#include "util/optional.h"

namespace meta
{
namespace parser
{

class leaf_node : public node
{
  public:
    /// Use the category constructor from the node class
    using node::node;

    /**
     * Constructs a new leaf node with the given category and word.
     * @param cat The category for the new leaf node
     * @param word The word for the new leaf node
     */
    leaf_node(class_label cat, std::string word);

    /**
     * @return the (optional) word for this leaf node
     */
    const util::optional<std::string>& word() const;

    bool is_leaf() const override;

    bool equal(const node& other) const override;

  private:
    /**
     * The optional word for this leaf node
     */
    const util::optional<std::string> word_;
};
}
}

#endif
