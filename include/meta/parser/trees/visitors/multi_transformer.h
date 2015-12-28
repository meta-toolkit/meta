/**
 * @file multi_transformer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PARSER_MULTI_TRANSFORMER_H_
#define META_PARSER_MULTI_TRANSFORMER_H_

#include <array>
#include "meta/parser/trees/visitors/tree_transformer.h"
#include "meta/parser/trees/node.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/util/shim.h"

namespace meta
{
namespace parser
{

/**
 * A template class for composing tree transformers. Each template
 * parameter is another transform to be run.
 */
template <class... Transformers>
class multi_transformer : public tree_transformer
{
  public:
    static_assert(sizeof...(Transformers) > 1, "multi_transformer needs to "
                                               "be specified with at least two "
                                               "transformers to be run");

    /**
     * Constructs each of the composed transformers using their default
     * constructor.
     */
    multi_transformer() : transforms_{{make_unique<Transformers>()...}}
    {
        // nothing
    }

    /**
     * Take a set of pointers to the transformers to be composed.
     * @param trans The transformers to be used
     */
    multi_transformer(std::unique_ptr<Transformers>... trans)
        : transforms_{trans...}
    {
        // nothing
    }

    std::unique_ptr<node> operator()(const leaf_node& lnode) override
    {
        return run_transforms(lnode);
    }

    std::unique_ptr<node> operator()(const internal_node& inode) override
    {
        return run_transforms(inode);
    }

  private:
    /**
     * Runs each of the transformers on a node in turn.
     * @param n The node to transform
     * @return the transformed node
     */
    std::unique_ptr<node> run_transforms(const node& n)
    {
        auto res = n.accept(*transforms_[0]);
        for (size_t i = 1; i < transforms_.size(); ++i)
            res = res->accept(*transforms_[i]);
        return res;
    }

    /**
     * The transforms to run.
     */
    std::array<std::unique_ptr<tree_transformer>, sizeof...(Transformers)>
        transforms_;
};

/**
 * Helper function for constructing a multi_transformer from a set of
 * pointers to transformers.
 *
 * @param trans The transformers to be used
 * @return a multi_transformer with the supplied transforms
 */
template <class... Transformers>
multi_transformer<Transformers...>
    make_transformer(std::unique_ptr<Transformers>... trans)
{
    return {trans...};
}
}
}
#endif
