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
#include "parser/trees/transformers/tree_transformer.h"
#include "util/shim.h"

namespace meta
{
namespace parser
{

template <class... Transformers>
class multi_transformer : public tree_transformer
{
  public:
    static_assert(sizeof...(Transformers) > 1, "multi_transformer needs to "
                                               "be specified with at least two "
                                               "transformers to be run");

    multi_transformer() : transforms_{make_unique<Transformers>()...}
    {
        // nothing
    }

    multi_transformer(std::unique_ptr<Transformers>... trans)
        : transforms_{trans...}
    {
        // nothing
    }

    std::unique_ptr<node> transform(const leaf_node& lnode) override
    {
        return run_transforms(lnode);
    }

    std::unique_ptr<node> transform(const internal_node& inode) override
    {
        return run_transforms(inode);
    }

  private:
    std::unique_ptr<node> run_transforms(const node& n)
    {
        auto res = n.accept(*transforms_[0]);
        for (size_t i = 1; i < transforms_.size(); ++i)
            res = res->accept(*transforms_[i]);
        return res;
    }

    std::array<std::unique_ptr<tree_transformer>, sizeof...(Transformers)>
        transforms_;
};

template <class... Transformers>
multi_transformer<Transformers...>
    make_transformer(std::unique_ptr<Transformers>... trans)
{
    return {trans...};
}
}
}
#endif
