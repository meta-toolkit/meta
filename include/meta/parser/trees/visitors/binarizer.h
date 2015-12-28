/**
 * @file binarizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_BINARIZER_H_
#define META_PARSER_BINARIZER_H_

#include <stdexcept>
#include "meta/parser/trees/visitors/tree_transformer.h"

namespace meta
{
namespace parser
{

/**
 * A tree transformer that converts any n-ary productions to binary
 * productions using provided head annotations. This class assumes that its
 * input has its heads annotated by e.g. running a head_finder first.
 */
class binarizer : public tree_transformer
{
  public:
    std::unique_ptr<node> operator()(const leaf_node&) override;
    std::unique_ptr<node> operator()(const internal_node&) override;
};

/**
 * Simple exception class for tree binarizer operations.
 */
class tree_binarizer_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#endif
