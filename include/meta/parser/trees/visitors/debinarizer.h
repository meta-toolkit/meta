/**
 * @file debinarizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_DEBINARIZER_H_
#define META_PARSER_DEBINARIZER_H_

#include "meta/meta.h"
#include "meta/parser/trees/visitors/tree_transformer.h"

namespace meta
{
namespace parser
{

/**
 * A tree transformer that converts binarized trees back into n-ary trees.
 * This class assumes that its input is a tree binarized in the same format
 * as would be output from the binarizer.
 */
class debinarizer : public tree_transformer
{
  public:
    std::unique_ptr<node> operator()(const leaf_node&) override;
    std::unique_ptr<node> operator()(const internal_node&) override;
};
}
}

#endif
